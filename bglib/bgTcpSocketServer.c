/*
File: bgTcpSocketServer.c
=====

B&R Libraries:
==============
Make sure to add following .a files to the task using this source file:
libsys_lib.a
libbrsystem.a
libastcp.a
*/


/*
B&R components:
===============================================================================
*/
#include <bur/plc.h>
#include <bur/plctypes.h>
#include <astcp.h>
#include <brsystem.h>
#include <sys_lib.h>

/*
BG components
===============================================================================
*/
#include "bgdebug.h"
#include "bgstring.h"
#include "bgsscanf.h"
#include "bgTcpSocketClientServer.h"

static void bgTcpSocketServerMemAlloc(bgTcpSocketServer_typ* p);
static void bgTcpSocketServerSessionStart(bgTcpSocketServer_typ* p);
static void bgTcpSocketServerSessionCyclic(bgTcpSocketServer_typ* p);
static void bgTcpSocketServerSessionStateMachine(bgTcpSocketSession_typ* pSession);
static void	bgTcpSocketServerExecuteSessionHandler(bgTcpSocketSession_typ* pSession, int init);
static void bgTcpSocketServerSessionSendData(bgTcpSocketSession_typ* pSession);
static void bgTcpSocketServerDefaultSessionHandler(bgTcpSocketSession_typ* pSession);
static void bgTcpSocketSendErrorResponse(bgTcpSocketSession_typ* pSession, int responseCode, char* msg);

/*
TCP Socket Server
######################################################################################################
######################################################################################################
######################################################################################################
######################################################################################################
*/
void bgTcpSocketServerInit(bgTcpSocketServer_typ* pServer,
    int portNumber, 
    int numberOfSessions,
    int sessionSendBufSize,
    int sessionRecvBufSize,
    int applicationBufSize,
    void (*pSessionHandler)(bgTcpSocketSession_typ*),
    void (*pApplicationHandler)(bgTcpSocketSession_typ*))
{   
    pServer->serverPort = portNumber;

    if(numberOfSessions <= BGTCPSOCKSRV_MAX_SESSIONS )
        pServer->numSupportedSessions = numberOfSessions;
    else{
        DBGCRT("bgTcpSocketServerInit(): max sessions= %d; %d sessions is to many!\r\n", 
            BGTCPSOCKSRV_MAX_SESSIONS, numberOfSessions);
        pServer->numSupportedSessions = BGTCPSOCKSRV_MAX_SESSIONS;
    }        

    pServer->sessionSendBufSize = sessionSendBufSize;

    pServer->sessionRecvBufSize = sessionRecvBufSize;

	pServer->applicationBufSize = applicationBufSize;
	
    pServer->sessionTotalBufSize = pServer->sessionSendBufSize + pServer->sessionRecvBufSize + pServer->applicationBufSize;

    pServer->totalBufSize = pServer->sessionTotalBufSize * pServer->numSupportedSessions;
    
    pServer->pSessionHandler = pSessionHandler;

    pServer->pApplicationHandler = pApplicationHandler;    

	pServer->sockStateData = BGTCPSOCK_STATE_INITOPEN;

	pServer->dbgDumpSendBuf = 0;
	pServer->dbgDumpRecvBuf = 0;

	/*allocate memory for send/recv buffers*/
	bgTcpSocketServerMemAlloc(pServer);

	if(pServer->memAllocStatus2){
			DBGCRT("bgTcpSocketServerCyclic: memory alloc error! memAllocStatus2=%d\r\n", pServer->memAllocStatus2);
	}
}

void bgTcpSocketServerCyclic(bgTcpSocketServer_typ* pServer)
{   
int i;

	/*if memalloc fails, can't do much!*/
	if(pServer->memAllocStatus2)
		return;

	/*execute session state machine*/
	bgTcpSocketServerSessionCyclic(pServer);

	/*server state machine*/
	switch(pServer->sockStateData){
		case BGTCPSOCK_STATE_NONE:
    		return;
		break;	
        
		case BGTCPSOCK_STATE_INITOPEN:
			pServer->tcpOpen.enable = 1;			
			pServer->tcpOpen.port = pServer->serverPort;        /* server listening port number*/
			pServer->tcpOpen.pIfAddr = 0;
			pServer->tcpOpen.options = tcpOPT_REUSEADDR;				
    		pServer->sockStateData = BGTCPSOCK_STATE_OPEN;
		break;

        case BGTCPSOCK_STATE_OPEN:
			TcpOpen(&pServer->tcpOpen);

			if( pServer->tcpOpen.status == 0xFFFF )
				return;
		
			if( !pServer->tcpOpen.status ){				
				pServer->tcpServer.enable = 1;
				pServer->tcpServer.ident = pServer->tcpOpen.ident;
				pServer->tcpServer.backlog = 5; 
	    		pServer->tcpServer.pIpAddr = (unsigned long)pServer->clientIpAddr;           					
				pServer->sockStateData = BGTCPSOCK_STATE_LISTEN;

                /*clear all sessions*/
                for(i=0; i < BGTCPSOCKSRV_MAX_SESSIONS; i++){
                    pServer->session[i].ident = i;
                    pServer->session[i].socketState = BGTCPSOCK_SESSION_STATE_NONE;
                }
                
				return;
			}	

            /* TODO: TcpOpen error handling! */
			pServer->sockStateData = BGTCPSOCK_STATE_NONE;
		break;

        case BGTCPSOCK_STATE_LISTEN:
    		TcpServer(&pServer->tcpServer);					/* wait connections from the clients */

			if(pServer->tcpServer.status == 0xFFFF)
				return;
			
			if(!pServer->tcpServer.status){
                bgTcpSocketServerSessionStart(pServer);
				return;
			}
			
            /*TODO: what to do on error?*/
			goto errorExit;
		break;

        case BGTCPSOCK_STATE_TOOBUSY:
            if( pServer->numActiveSessions < pServer->numSupportedSessions ){
                DBGMSG("bgTcpSocketServerCyclic(): session available, restaring server listening\r\n");
    			pServer->sockStateData = BGTCPSOCK_STATE_LISTEN;
            }

            /*TODO: implement timeout here -> kill sessions if stuck*/
        break;            
	 }

    return;

errorExit:
	
/*TODO: what to do on error?*/
	DBGCRT("bgTcpSocketServer: ERROR! tcpServer.status=%d\r\n", pServer->tcpServer.status);
	return;		
}

static void bgTcpSocketServerMemAlloc(bgTcpSocketServer_typ* pServer)
{
unsigned int memSize;
int i;

	/*Get System Memory Information*/
	pServer->MEMInfo.enable = 1;
	MEMInfo(&pServer->MEMInfo);
	
	memSize = pServer->totalBufSize;
    
	/*allocate space for send/recv buffers*/
	if(pServer->sendRecvBuffer == 0){
		/* allocate new memory from RAM*/
    	pServer->memAllocStatus2 = TMP_alloc(memSize, (void**)&pServer->sendRecvBuffer); 	
	}else{
		/*memory is allready allocated -> free it first!*/
    	pServer->memAllocStatus2 = TMP_free(memSize, (void**)pServer->sendRecvBuffer); 	

    	/*allocate new memory from RAM*/
    	pServer->memAllocStatus2 = TMP_alloc(memSize, (void**)pServer->sendRecvBuffer); 	
	}

	/*assign send/recv buffer ptrs*/
    for(i=0; i < pServer->numSupportedSessions; i++){
        pServer->session[i].pSendBuf = (char*)(pServer->sendRecvBuffer + i * pServer->sessionTotalBufSize);
		pServer->session[i].sendBufLength = pServer->sessionSendBufSize;
		
        pServer->session[i].pRecvBuf = (char*)(pServer->session[i].pSendBuf + pServer->sessionSendBufSize);
		pServer->session[i].recvBufLength = pServer->sessionRecvBufSize;

        pServer->session[i].pAppBuf = (char*)(pServer->session[i].pRecvBuf + pServer->sessionRecvBufSize);
		pServer->session[i].appBufLength = pServer->applicationBufSize;
    }
}

static void bgTcpSocketServerSessionStart(bgTcpSocketServer_typ* pServer)
{
int i;
bgTcpSocketSession_typ *pSession;

    for(i=0; i < pServer->numSupportedSessions; i++){
        if( pServer->session[i].socketState == BGTCPSOCK_SESSION_STATE_NONE){
            pSession = &pServer->session[i];
            pSession->socketState = BGTCPSOCK_SESSION_STATE_RECEIVE;
    
			pSession->tcpSend.enable = 1;
			pSession->tcpSend.ident = pServer->tcpServer.identclnt;   /* copy the  Ident number */
			pSession->tcpSend.flags = 0;
			pSession->tcpSend.pData = (UDINT)pSession->pSendBuf;

			pSession->tcpRecv.enable = 1;
			pSession->tcpRecv.ident = pServer->tcpServer.identclnt;     /* copy the  Ident number */
			pSession->tcpRecv.pData = (UDINT)pSession->pRecvBuf;        /* address of the buffer */;
			pSession->tcpRecv.datamax = pSession->recvBufLength;    	/* length of the buffer */;

			pSession->tcpClose.enable = 1;
   			pSession->tcpClose.ident = pServer->tcpServer.identclnt;    /* copy the  Ident number */

			pSession->pTcpLayer = (char*)pServer;	                    /*each session has ptr to tcp layer (client or server) data*/

            pSession->rxByteCountTotal = 0;
            pSession->txByteCountTotal = 0;
            pSession->rxByteCount = 0;
            pSession->txByteCount = 0;
            strcpy(pSession->clientIpAddr, pServer->clientIpAddr);

			pSession->dbgDumpRecvBuf = pServer->dbgDumpRecvBuf;
			pSession->dbgDumpSendBuf = pServer->dbgDumpSendBuf;

			/*init session specific handler - call the handler with init pSession->flag=1*/
			bgTcpSocketServerExecuteSessionHandler(pSession, 1);


			DBGMSG("bgTcpSocketServerSessionStart(): connection from %s, starting session Id= %d\r\n", pServer->clientIpAddr, i);
            return;
        }
    } 

	DBGCRT("bgTcpSocketServerSessionStart(): server busy! activeSessions= %d\r\n", pServer->numSupportedSessions);
    pServer->sockStateData = BGTCPSOCK_STATE_TOOBUSY;
}

static void bgTcpSocketServerSessionCyclic(bgTcpSocketServer_typ* pServer)
{
int i;
bgTcpSocketSession_typ *pSession;
int activeSessions = 0;

    for(i=0; i < pServer->numSupportedSessions; i++){
        pSession = &pServer->session[i];
        
        if( pSession->socketState != BGTCPSOCK_SESSION_STATE_NONE){
            activeSessions++;
            bgTcpSocketServerSessionStateMachine(pSession);            
        }
    } 
    pServer->numActiveSessions = activeSessions;
}

static void bgTcpSocketServerSessionStateMachine(bgTcpSocketSession_typ* pSession)
{
    switch(pSession->socketState){
		case BGTCPSOCK_SESSION_STATE_SEND:

            TcpSend(&pSession->tcpSend);            				/* send the data */

            if(!pSession->tcpSend.status){
            	pSession->txByteCount += pSession->tcpSend.sentlen;

				DBGMSG("bgTcpSocketServerSessionStateMachine(): SEND complete! status=%d; curLen=%d; cumLen=%d\r\n", 
					pSession->tcpSend.status, pSession->tcpSend.sentlen, 
					pSession->txByteCount);			
/*
following simplifies testing of multiple sessions:
for normal operation use 
#if 1
for delayed operation (testing) use 
#if 0
*/
#if 1
    			pSession->socketState = BGTCPSOCK_SESSION_STATE_CLOSE;            
#else
    			p->socketState = BGTCPSOCK_SESSION_STATE_DELAY;            
				p->delayCounter = 50;
#endif
				break;
			}
			
			/*in case of the following error -> close connection*/
            if( (pSession->tcpSend.status == 27210) 	||
            	(pSession->tcpSend.status == 27135)	||
            	(pSession->tcpSend.status == tcpERR_NOT_CONNECTED)	||
            	(pSession->tcpSend.status == tcpERR_SYSTEM)	||
            	(pSession->tcpSend.status == 27249)
            ){
				DBGMSG("bgTcpSocketServerSessionStateMachine(): SEND error=%d\r\n", pSession->tcpSend.status);
    			pSession->socketState = BGTCPSOCK_SESSION_STATE_CLOSE;            
				break;
			}else
			if(pSession->tcpSend.status == tcpERR_SENTLEN){
				/*long send! call send buf handler to fill in more data*/
			    if(pSession->pSendBufHandler){
    				(*pSession->pSendBufHandler)(pSession);

					DBGMSG("bgHttpSendBufCallback(): SEND status=%d; curLen=%d; cumLen=%d; remLen=%d\r\n", 
						pSession->tcpSend.status, 
						pSession->tcpSend.sentlen, 
						pSession->txByteCount,
						pSession->tcpSend.datalen);			
			    }
				else{
					DBGMSG("bgTcpSocketServerSessionStateMachine(): SEND incomplete! sendBuf handler NOT defined!? status=%d\r\n", pSession->tcpSend.status);			
		   			pSession->socketState = BGTCPSOCK_SESSION_STATE_CLOSE;            
				}
			}else{
				DBGMSG("bgTcpSocketServerSessionStateMachine(): SEND status=%d\r\n", pSession->tcpSend.status);			
 			}

		break;

		case BGTCPSOCK_SESSION_STATE_RECEIVE:
            TcpRecv(&pSession->tcpRecv);          				/* recv data*/

            if(!pSession->tcpRecv.status){
                if(pSession->tcpRecv.recvlen){
					bgTcpSocketServerExecuteSessionHandler(pSession, 0);
                }else{
					/*remote socket has closed*/
    				pSession->socketState = BGTCPSOCK_SESSION_STATE_CLOSE;    /*close this server socket*/       					
				}
            }				
		break;

		case BGTCPSOCK_SESSION_STATE_CLOSE:
			pSession->tcpClose.how = tcpSHUT_RD & tcpSHUT_WR;			

            TcpClose(&pSession->tcpClose);          				/* set the socket free */

			if(pSession->tcpClose.status == 0xFFFF)
				return;

	    	pSession->socketState = BGTCPSOCK_SESSION_STATE_NONE;
			DBGMSG("bgTcpSocketServerSessionStateMachine(): stopping session Id= %d\r\n", pSession->ident);
		break;

		case BGTCPSOCK_SESSION_STATE_DELAY:
			pSession->delayCounter--;
			if(pSession->delayCounter <= 0)
    			pSession->socketState = BGTCPSOCK_SESSION_STATE_CLOSE;    /*close this server socket*/       									
		break;			

		default:

		break;
	}				
}


static void	bgTcpSocketServerExecuteSessionHandler(bgTcpSocketSession_typ* pSession, int init)
{
bgTcpSocketServer_typ* pServer = (bgTcpSocketServer_typ*)pSession->pTcpLayer;

	                    
	pSession->init = 0;			
	if( init )
		pSession->init = 1;			
	
    if(pServer->pSessionHandler)
    	(*pServer->pSessionHandler)(pSession);
    else
    	bgTcpSocketServerDefaultSessionHandler(pSession);                        			

	pSession->init = 0;			

	/*has upper layer requested action!?*/
	if( pSession->bSendData ){
		pSession->bSendData = 0;
		bgTcpSocketServerSessionSendData(pSession);
	}
}

static void bgTcpSocketServerSessionSendData(bgTcpSocketSession_typ* pSession)
{		
	pSession->tcpSend.datalen = pSession->txByteCountTotal;
	pSession->socketState = BGTCPSOCK_SESSION_STATE_SEND;								
}

/*
Default session handler
######################################################################################################
######################################################################################################
######################################################################################################
######################################################################################################
*/
static void bgTcpSocketServerDefaultSessionHandler(bgTcpSocketSession_typ* pSession)
{
    /*default session handler -> send out http error response "internal server error"*/
    bgTcpSocketSendErrorResponse(pSession, 500, "Internal server error. (Session handler not installed.)");
}

static void bgTcpSocketSendErrorResponse(bgTcpSocketSession_typ* pSession, int responseCode, char *msg)
{
char textBuffer[256]; 
char* pDst = pSession->pSendBuf;
int len;
	
	pDst[0]=0;
	bgsprintf(textBuffer, "%s %d %s\r\n", BGHTTP_CONST_STRING_VERSION, responseCode, msg );
	strcpy(pDst, textBuffer);

	len = strlen(pSession->pSendBuf);
	pSession->txByteCountTotal = len;
	bgTcpSocketServerSessionSendData(pSession);
}
