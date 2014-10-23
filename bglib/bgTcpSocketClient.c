/*
File: bgTcpSocketClientServer.c
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
#include <ethsock.h>
#include <astcp.h>
#include <brsystem.h>
#include <sys_lib.h>

/*
BG components
===============================================================================
*/
#include "bgdebug.h"
#include "bgstring.h"


#include "bgTcpSocketClientServer.h"
#include "bgHttpProtocol.h"


/*
Tcp Socket Client
*/
static void bgTcpSocketClientDefaultSessionHandler(bgTcpSocketSession_typ* pSession);
static void bgTcpSocketClientSessionStateMachine(bgTcpSocketSession_typ* pSession);
static void	bgTcpSocketClientExecuteSessionHandler(bgTcpSocketSession_typ* pSession, int init);

static void bgTcpSocketClientMemAlloc(bgTcpSocketClient_typ* pClient);
static void bgTcpSocketClientSessionStart(bgTcpSocketClient_typ* pClient);
static void	bgTcpSocketClientExecuteSessionHandler(bgTcpSocketSession_typ* pSession, int init);
static void bgTcpSocketClientSessionCyclic(bgTcpSocketClient_typ* pClient);
static void bgTcpSocketClientSessionLauncher(bgTcpSocketClient_typ* pClient);
static void setSocketAddressData(ETH_sockaddr_typ *adrData, char *hostIpAddressString, unsigned short hostPort);
static void swapBytesInShort(unsigned short *value);


/*
TCP Socket Client
######################################################################################################
######################################################################################################
######################################################################################################
######################################################################################################
*/
void bgTcpSocketClientInit(bgTcpSocketClient_typ* pClient,
    int sessionSendBufSize,
    int sessionRecvBufSize,
    int applicationBufSize,
    char* pRecvBuf,							
    void (*pSessionHandler)(bgTcpSocketSession_typ*),
    void (*pApplicationHandler)(bgTcpSocketSession_typ*))
{   
    pClient->serverPort = 0;

	pClient->recvBuffer = pRecvBuf;

	/*Recv buffer can't be allocated! recv DOES NOT WORK if the buffer is not in PV space!?!?!?!?*/
	if(!pRecvBuf){
		DBGCRT("bgTcpSocketClientInit: recvBuf not defined!\r\n");
		pClient->memAllocStatus2 = 1;
		return;
	}
	
    pClient->serverIpAddr[0] = 0;

    pClient->numSupportedSessions = 1;

    pClient->sessionSendBufSize = sessionSendBufSize;

    pClient->sessionRecvBufSize = sessionRecvBufSize;

	pClient->applicationBufSize = applicationBufSize;

	/*Recv buffer can't be allocated! recv DOES NOT WORK if the buffer is not in PV space!?!?!?!?*/
    pClient->sessionTotalBufSize = pClient->sessionSendBufSize + pClient->applicationBufSize;

    pClient->totalBufSize = pClient->sessionTotalBufSize * pClient->numSupportedSessions;
    
    pClient->pSessionHandler = pSessionHandler;

    pClient->pApplicationHandler = pApplicationHandler;    

	pClient->sockStateData = BGTCPSOCK_STATE_NONE;

	pClient->dbgDumpSendBuf = 0;
	pClient->dbgDumpRecvBuf = 0;

	/*allocate memory for send/recv buffers*/
	bgTcpSocketClientMemAlloc(pClient);

	if(pClient->memAllocStatus2 ){
			DBGCRT("bgTcpSocketClientInit: memory error! memAllocStatus2=%d\r\n", pClient->memAllocStatus2);
	}
}

void bgTcpSocketClientCyclic(bgTcpSocketClient_typ* pClient)
{   
int ret;
	/*non-blocking I/O control*/
static ETH_fd_set_typ fdR;
static ETH_fd_set_typ fdW;
static ETH_timeval_typ timeOut;


	/*if memalloc fails, can't do much!*/
	if(pClient->memAllocStatus2)
		return;

	/*monitor upper layer requests to start a client session*/
	bgTcpSocketClientSessionLauncher(pClient);
	
	/*execute session state machine*/
	bgTcpSocketClientSessionCyclic(pClient);

	/*client state machine*/
	switch(pClient->sockStateData){
		case BGTCPSOCK_STATE_NONE:
    		return;
		break;	
        
		case BGTCPSOCK_STATE_INITOPEN:

 			/*create socket*/
			pClient->tcpOpen.ident = socket(ETH_AF_INET, ETH_SOCK_STREAM, 0);

			if( pClient->tcpOpen.ident == ERR_SOCKET ){
				DBGMSG("socket create error (init open); EthGetError=%d\r\n", EthGetError());
				pClient->sockStateData = BGTCPSOCK_STATE_NONE;
				break;
			}

			pClient->tcpClient.ident = pClient->tcpOpen.ident;
			pClient->tcpClose.ident = pClient->tcpOpen.ident;

			/*set non-blocking socket operation*/
			{
				UDINT arg = 1;
				ioctl(pClient->tcpOpen.ident, ETH_FIONBIO, (UDINT)&arg);
			}

#if 0
			/*get sock options*/
			{
				ret = getsockopt(pClient->tcpOpen.ident, ETH_SOL_SOCKET, ETH_SO_SNDTIMEO, (UDINT)&opt1, (UDINT)&opt1size);
				DBGMSG("getsockopt ret=%d\r\n", ret);
			}
#endif

			/*reset select flags*/			
			fd_zero((UDINT)&fdR);
			fd_zero((UDINT)&fdW);

			/*set time-out to 10ms*/
			timeOut.tv_sec = 0;
			timeOut.tv_usec = 1000 * 10;
			
			pClient->sockStateData = BGTCPSOCK_STATE_OPEN;

		case BGTCPSOCK_STATE_OPEN:
					
			/*connect to remote host*/
			{
			ETH_sockaddr_typ serverAdr;		
			setSocketAddressData(&serverAdr, pClient->serverIpAddr, pClient->serverPort);
			ret = connect(pClient->tcpOpen.ident, (UDINT)&serverAdr, sizeof(ETH_sockaddr_typ) );
			}
			
			if(ret == ERR_SOCKET){
				UINT errCode = EthGetError();
				if(errCode == ERR_ETH_INPROGRESS){
					/*next: wait connect to complete*/
					DBGMSG("socket connect - wait connect to complete!\r\n");
					pClient->sockStateData = BGTCPSOCK_STATE_CLIENT;
				    break;
				}else
				if(errCode == ERR_ETH_ISCONN){
					/*it's connected -> done*/
					DBGMSG("socket connect ok(1)!\r\n");
	                bgTcpSocketClientSessionStart(pClient);
					pClient->sockStateData = BGTCPSOCK_STATE_CONNECTED;
					break;
				}
				else{
					DBGMSG("socket connect error(open); EthGetError=%d\r\n", EthGetError());
					pClient->sockStateData = BGTCPSOCK_STATE_NONE;
					break;
				}
			}
			/*connected!*/
			/*it's connected -> done*/
			DBGMSG("socket connect ok(2)!\r\n");
            bgTcpSocketClientSessionStart(pClient);
			pClient->sockStateData = BGTCPSOCK_STATE_CONNECTED;
		break;
		case BGTCPSOCK_STATE_CLIENT:
			/*select waits for connection to complete*/
			fd_set(pClient->tcpOpen.ident, (UDINT)&fdR);
			fd_set(pClient->tcpOpen.ident, (UDINT)&fdW);
			ret = sock_select(32, (UDINT)&fdR, (UDINT)&fdW, 0L, (UDINT)&timeOut);
			
			if(ret == ERR_SOCKET){
				DBGMSG("socket connect error; (cli) EthGetError=%d\r\n", EthGetError());
				pClient->sockStateData = BGTCPSOCK_STATE_CLOSE;
				break;
			}

			if(ret == 0){


			}else{
				if( fd_isset(pClient->tcpOpen.ident, (UDINT)&fdW) ){					
					DBGMSG("socket connect(long) ok!\r\n");
	                bgTcpSocketClientSessionStart(pClient);
					pClient->sockStateData = BGTCPSOCK_STATE_CONNECTED;					
				}
			}
		break;
        case BGTCPSOCK_STATE_CONNECTED:
			/*Monitor session operation; generate OnResponseReady & OnResponseError triggers*/
            /*TODO: implement timeout here -> kill sessions if stuck*/
			if(pClient->session[0].socketState == BGTCPSOCK_SESSION_STATE_NONE){
				pClient->sockStateData = BGTCPSOCK_STATE_NONE;				
			}			
        break;            
        case BGTCPSOCK_STATE_CLOSE:
			close(pClient->tcpClose.ident);
	    	pClient->sockStateData = BGTCPSOCK_STATE_NONE;
			DBGMSG("bgTcpSocketClientSessionStateMachine(): closing socket\r\n");
        break;            
        case BGTCPSOCK_STATE_ERROR:
        break;            
	 }

    return;
}

static void bgTcpSocketClientMemAlloc(bgTcpSocketClient_typ* pClient)
{
unsigned int memSize;
int i;

	/*Get System Memory Information*/
	pClient->MEMInfo.enable = 1;
	MEMInfo(&pClient->MEMInfo);
	
	memSize = pClient->totalBufSize;
    
	/*allocate space for send/recv buffers*/
	if(pClient->sendRecvBuffer == 0){
		/* allocate new memory from RAM*/
    	pClient->memAllocStatus2 = TMP_alloc(memSize, (void**)&pClient->sendRecvBuffer); 	
	}else{
		/*memory is allready allocated -> free it first!*/
    	pClient->memAllocStatus2 = TMP_free(memSize, (void**)pClient->sendRecvBuffer); 	

    	/*allocate new memory from RAM*/
    	pClient->memAllocStatus2 = TMP_alloc(memSize, (void**)pClient->sendRecvBuffer); 	
	}

	/*assign send/recv buffer ptrs*/
    pClient->session[0].pSendBuf = (char*)(pClient->sendRecvBuffer + i * pClient->sessionTotalBufSize);
	pClient->session[0].sendBufLength = pClient->sessionSendBufSize;
	

    pClient->session[0].pAppBuf = (char*)(pClient->session[0].pSendBuf + pClient->sessionSendBufSize);
	pClient->session[0].appBufLength = pClient->applicationBufSize;


    pClient->session[0].pRecvBuf = pClient->recvBuffer;
	pClient->session[0].recvBufLength = pClient->sessionRecvBufSize;
}


static void bgTcpSocketClientSessionStart(bgTcpSocketClient_typ* pClient)
{
int i;
bgTcpSocketSession_typ *pSession;

	/* client: pClient->numSupportedSessions = 1 ALWAYS!*/
    for(i=0; i < pClient->numSupportedSessions; i++){
        if( pClient->session[i].socketState == BGTCPSOCK_SESSION_STATE_NONE){
            pSession = &pClient->session[i];
            pSession->socketState = BGTCPSOCK_SESSION_STATE_SEND;
    
			pSession->tcpSend.enable = 1;
			pSession->tcpSend.ident = pClient->tcpOpen.ident;   		/* copy the  Ident number */
			pSession->tcpSend.flags = 0;
			pSession->tcpSend.pData = (UDINT)pSession->pSendBuf;

			pSession->tcpRecv.enable = 1;
			pSession->tcpRecv.ident = pClient->tcpOpen.ident;     		/* copy the  Ident number */

			pSession->tcpRecv.pData = (UDINT)pSession->pRecvBuf;        /* address of the buffer */;

			pSession->tcpRecv.datamax = pClient->sessionRecvBufSize;    /* length of the buffer */;

			pSession->tcpClose.enable = 1;
   			pSession->tcpClose.ident = pClient->tcpOpen.ident;    		/* copy the  Ident number */

			pSession->pTcpLayer = (char*)pClient;	                    /*session has ptr to tcp layer*/

            pSession->rxByteCount = 0;
            strcpy(pSession->clientIpAddr, pClient->clientIpAddr);


			pSession->dbgDumpRecvBuf = pClient->dbgDumpRecvBuf;
			pSession->dbgDumpSendBuf = pClient->dbgDumpSendBuf;

			/*init session specific handler - call the handler with init pSession->flag=1*/
			bgTcpSocketClientExecuteSessionHandler(pSession, 1);

			DBGMSG("bgTcpSocketClientSessionStart(): connection to %s, starting session Id= %d\r\n", pClient->serverIpAddr, i);
            return;
        }
    } 

	DBGCRT("bgTcpSocketClientSessionStart(): client busy! activeSessions= %d\r\n", pClient->numSupportedSessions);
    pClient->sockStateData = BGTCPSOCK_STATE_TOOBUSY;
}


static void	bgTcpSocketClientExecuteSessionHandler(bgTcpSocketSession_typ* pSession, int init)
{
bgTcpSocketClient_typ* pClient = (bgTcpSocketClient_typ*)pSession->pTcpLayer;

	pSession->init = 0;			
	if( init )
		pSession->init = 1;			


	DBGMSG("bgTcpSocketClientExecuteSessionHandler\r\n p1=%d p2=%d\r\n", pClient->pSessionHandler, bgHttpClientSessionHandler);

/*
	bgHttpClientSessionHandler(pSession);
*/
    if(pClient->pSessionHandler)
    	(*pClient->pSessionHandler)(pSession);
    else
    	bgTcpSocketClientDefaultSessionHandler(pSession);                        			

	pSession->init = 0;			
}

static void bgTcpSocketClientSessionCyclic(bgTcpSocketClient_typ* pClient)
{
int i;
bgTcpSocketSession_typ *pSession;
int activeSessions = 0;

    for(i=0; i < pClient->numSupportedSessions; i++){
        pSession = &pClient->session[i];
        
        if( pSession->socketState != BGTCPSOCK_SESSION_STATE_NONE){
            activeSessions++;
            bgTcpSocketClientSessionStateMachine(pSession);            
        }
    } 
    pClient->numActiveSessions = activeSessions;
}


static void bgTcpSocketClientSessionLauncher(bgTcpSocketClient_typ* pClient)
{
int i;
bgTcpSocketSession_typ *pSession;

    for(i=0; i < pClient->numSupportedSessions; i++){
        pSession = &pClient->session[i];

		if( (pSession->bSendData) ){
			pSession->bSendData = 0;
			if(pClient->sockStateData == BGTCPSOCK_STATE_NONE){
				pClient->sockStateData = BGTCPSOCK_STATE_INITOPEN;
			}else{
				DBGMSG("bgTcpSocketClientSessionLauncher: client session start rejected; client is busy! cliState=%d\r\n", 
					pClient->sockStateData);
			}
		}        
    } 
}
static void bgTcpSocketClientSessionStateMachine(bgTcpSocketSession_typ* pSession)
{
int ret;

    switch(pSession->socketState){
		case BGTCPSOCK_SESSION_STATE_SEND:

        	ret = send(pSession->tcpSend.ident, (UDINT)pSession->pSendBuf, pSession->tcpSend.datalen, 0L);			
        		
        	if(ret == ERR_SOCKET){
				DBGMSG("send error EthGetError=%d\r\n", EthGetError());
				pSession->socketState = BGTCPSOCK_SESSION_STATE_CLOSE;    /*close this socket*/       					
				return;
        	}   	
			
			/*remember the send length*/
			DBG("send() ok; bytes=%d\r\n", (int)ret);
			pSession->tcpSend.sentlen = ret;				
	   		pSession->socketState = BGTCPSOCK_SESSION_STATE_RECEIVE;    /*close this socket session*/       					

		/*break;*/

		case BGTCPSOCK_SESSION_STATE_RECEIVE:
	       	/*call receive (NON BLOCKING MODE!)*/
        	ret = recv(pSession->tcpRecv.ident, (UDINT)pSession->tcpRecv.pData, pSession->tcpRecv.datamax, 0L);			
        	
        	if(ret == ERR_SOCKET){
        		UINT errCode = EthGetError();
        		/*none blocking receive return this following value if data is not available yet*/
        		if( errCode == ERR_ETH_WOULDBLOCK ){
        			/*wait response*/
					return;
        		}
				DBGMSG("RxError EthGetError=%d\r\n", errCode);
	   			pSession->socketState = BGTCPSOCK_SESSION_STATE_CLOSE;    /*close this socket session*/       					
				break;				
        	}

			pSession->tcpRecv.recvlen = ret;


            if(pSession->tcpRecv.recvlen){
				DBGMSG("rx OK! rxLen=%d\r\n", ret);
				
				bgTcpSocketClientExecuteSessionHandler(pSession, 0);

				/*uper layer completed processing? */
				if( pSession->bCloseSession){
					pSession->bCloseSession = 0;
	   					pSession->socketState = BGTCPSOCK_SESSION_STATE_CLOSE;    /*close this socket session*/       					
				}
            }else{
				DBGMSG("bgTcpSocketClient..: RMT SOCK CLOSED!\r\n");
	   			pSession->socketState = BGTCPSOCK_SESSION_STATE_CLOSE;    /*close this socket session*/       					
			}
			return;
		break;

		case BGTCPSOCK_SESSION_STATE_CLOSE:
			close(pSession->tcpClose.ident);
	    	pSession->socketState = BGTCPSOCK_SESSION_STATE_NONE;
			DBGMSG("bgTcpSocketClientSessionStateMachine(): stopping session Id= %d\r\n", pSession->ident);
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

#if 0
static void bgTcpSocketClientSessionSendData(bgTcpSocketSession_typ* pSession, int len)
{		
	pSession->tcpSend.datalen = len;
	pSession->socketState = BGTCPSOCK_STATE_INITOPEN;								

#if 0
{
int i;
char text[90];
/*session/application/protocol specific data structure*/
httpRequest_typ* r = (httpRequest_typ*)pSession->pProtocol;

	DBGMSG("bgHttpSend(): lenght(bytes): total=%d(%d) header=%d content=%d\r\n", 
		len, r->sendTotalLength, r->sendHeaderLength, r->sendContentLength);

	DBGMSG("bgHttpSend dump:\r\n");
	DBGMSG("============================================\r\n");
	for(i=0;i<len; i++){		
 		if( ((i+1)*80) < len ){
			memcpy(&text[0], &pSession->pSendBuf[i*80], 80);
			text[80]=0;			
			DBGMSG("%s", &text[0]);
		}else{
			memcpy(&text[0], &pSession->pSendBuf[i*80], len - (i*80));
			text[len - (i*80)]=0;			
			DBGMSG("%s", &text[0]);
			break;			
		}
	}
	DBGMSG("============================================\r\n\r\n");
}	
#endif

}
#endif
/*
Default session handler
######################################################################################################
######################################################################################################
######################################################################################################
######################################################################################################
*/
static void bgTcpSocketClientDefaultSessionHandler(bgTcpSocketSession_typ* pSession)
{
    /*default session handler -> send out http error response "internal server error"*/
/*    httpSendErrorResponse(pSession, 500);
*/
}

static void setSocketAddressData(ETH_sockaddr_typ *adrData, char *hostIpAddressString, unsigned short hostPort)
{

	unsigned short sin_family;
	unsigned short family;
	unsigned short length;
	unsigned short port;
	
	family = ETH_AF_INET;
	length = sizeof(ETH_sockaddr_typ); 
	sin_family = (family << 8) + length;	

	adrData->sin_family = sin_family;
	port = hostPort;
	swapBytesInShort(&port);
	adrData->sin_port = port;
	if( strlen(hostIpAddressString) < 8)
		adrData->sin_addr = ETH_INADDR_ANY;
	else	
		adrData->sin_addr = inet_addr( (unsigned long) hostIpAddressString);
	adrData->sin_zero[0]=0;
	adrData->sin_zero[1]=0;
	adrData->sin_zero[2]=0;
	adrData->sin_zero[3]=0;
	adrData->sin_zero[4]=0;
	adrData->sin_zero[5]=0;
	adrData->sin_zero[6]=0;
	adrData->sin_zero[7]=0;
}

static void swapBytesInShort(unsigned short *value)
{

union{
	unsigned short 	shortValue;
	unsigned char	bytes[2];	
}swapValue;

unsigned char tempByte;

	swapValue.shortValue = *value;
	tempByte = swapValue.bytes[0];
	swapValue.bytes[0] = swapValue.bytes[1];
	swapValue.bytes[1] = tempByte;
	*value = swapValue.shortValue;
}		

