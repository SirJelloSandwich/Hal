/*
File: bgTelnetSrvr-astcp.c
=====


Modified on Jyly 2007:
THIS IS AN UPDATED VERSION OF THE TELNET SERVER WITH A SUPPORT FOR NEW B&R TCP LIBRARY!
OLD B&R TCP LIBRARY: ETHERNET.LIB
NEW B&R TCP LIBRARY: ASTCP.LIB

B&R Libraries:
==============
Make sure to add following .a files to the task using this source file:
libsys_lib.a
libbrsystem.a
libastcp.a

About:
======
- this cyclic task is an implemenation of a telnet server for a B&R PCC
- when this task is active, it should be possible to connect remote telnet client to this PCC
- once the remote telnet client is connected, this task will start outputing debug messages 
posted into the globaly defined RingBuffer structure
- following diagram shows the implemenation:


							+----------------->[ring buffer memory]
							|
					+-------*-------+
write to RingBuff	|gRingBuffer	|	read from RingBuff
			+------>|				|---------------+
			|		+---------------+				|
			|		global RingBuffer struct		|
			|										|
+=======+	|										|		+===============+
|task1	|-->+										+------>|bgtelnet		|---> if(telnet_client_is_connected)
+=======+	|												|(this task)	|		send_data_to_telnet_client
			|												+===============+
+=======+	|
|task2	|-->+
+=======+	|
			|
+=======+	|
|task3	|-->+
+=======+

- ring buffer memory is allocated at startup, and the size can be adjusted here (the default buffer size is 256kB)

WARNING: is the ring buffer memory is too small for the amount of data written into the buffer, 
messages may get overwritten & lost!!!

- any task that needs to send debug messages via telnet needs following:

1) Include following at the top of the C module:
#include "bglib/bgdebug.h"

2) Within the code use following sprintf style debug function to define dbg messages
Examples:
a)
DBG("My debug message\n\r");

b)
{
int Value;
char Text[64];
strcpy(&Text[0], "Hello World\r\n");
DBG("Value=%d; Text=%s\n\r", Value, Text);
}

*/

/*
B&R components:
===============================================================================
*/
#include <bur/plc.h>
#include <bur/plctypes.h>
#include <astcp.h>
#include <asstring.h>
#include <brsystem.h>
#include <sys_lib.h>

/*
BG components
===============================================================================
*/
#include "bgsprintf.h"
#include "bgringbuffer.h"
#include "bgstring.h"
/*Include required .c library files here. That way it's not needed in AS.*/
#include "bgringbuffer.c"
#include "bgsprintf.c"
#include "bgstring.c"


#include "bgdebug.h"



/*Ring buffer size in bytes->adjust as needed*/
#define RB_SIZE 256000					

/*User Private Library Error Numbers (value assigned as suggested in the B&R help file)*/
#define USER_ERROR_MIN  0xC350
#define USER_ERROR_MAX  0xEA5F
#define USER_ERROR_FTP  USER_ERROR_MIN + 10

/*telnet implemenation specifics*/
#define CMD_DONE	0
#define CMD_BUSY	1
#define CMD_ERROR	2

#define TELNET_SEND_BUFFER_SIZE 2048
#define TELNET_RECV_BUFFER_SIZE 256

typedef struct bgSocketServer_typ{
	unsigned int sockStateData; 	/*state of the server data socket*/
	unsigned int telnetRecvState; 	/*state of the telnet server receiver*/
	unsigned int telnetSendState; 	/*state of the telnet server receiver*/

	unsigned int sockSendOk;
	
	int	cmdError;
	int	cmdErrorCode;

	char isConnected;
	char isLoggedIn;
	char isDataPortReady;

	char isEcho;

	char sockData_Timer;
	int sockData_TimerTime;
	int sockData_TimerCounter;

#if 0
/*following is used during development*/
	char devDataSendBuf[TELNET_SEND_BUFFER_SIZE];
	char devDataRecvBuf[TELNET_RECV_BUFFER_SIZE];
	char devLineRecvBuf[TELNET_RECV_BUFFER_SIZE];
#endif
	char *dataSendBuf;
	char *dataRecvBuf;
	char *lineRecvBuf;
	
	UDINT *ringDataBuffer;	/*ptr to allocated space used as a RingBuffer*/
	UDINT *sendRecvBuffer;	/*ptr to allocated space used for send/recv buffers*/

	char iacRecvBuf[10];
	
	DINT lineIndex;
	DINT lineIndexPrev;

	TcpOpen_typ TCPopen_1;
	TcpServer_typ TCPserv_1;
	TcpSend_typ TCPsend_1;
	TcpRecv_typ TCPrecv_1;
	TcpClose_typ TCPclose_1;
	TcpIoctl_typ TCPioctl_1;

	MEMInfo_typ MEMInfo;
	unsigned int memAllocStatus1;
	unsigned int memAllocStatus2;
	
	struct counter {
		int recv_frames;
		int iac;
		int lines;
		int send_frames;
	}counter;

	int test;
	int testCounter;
	STRING clientIpAddr[32];
	int delayCounter;
}bgSocketServer_typ;



/* Telnet protocol characters (don't change)*/
#define IAC     (255) /*"Interpret As Command"*/
#define DONT    (254)
#define DO      (253)
#define WONT    (252)
#define WILL    (251)
#define theNULL (0)

#define BS		(8)		/*0x8 - back space char*/
#define LF		(10)	/*0xa - line feed*/
#define CR		(13)	/*0xd - carrige return*/
#define EC      (247)  /*# Erase Character*/


/*telnet server related local PVs*/
_LOCAL bgSocketServer_typ dbgTerm;

int bgSocketServerIsSendReady(bgSocketServer_typ *p);
int bgSocketServer(int init, bgSocketServer_typ *p);
void bgSocketServerRecv(bgSocketServer_typ *p);
void bgTelnetServerProcessCmd(bgSocketServer_typ *p, int length);
void bgTelnetServerProcessLine(bgSocketServer_typ *p);
void bgTelnetServerWelcomeMsg(bgSocketServer_typ *p);
void bgSocketServerSendData(bgSocketServer_typ *p, int len);
char* bgSocketGetSendBuffer(bgSocketServer_typ *p);
int bgSocketGetSendBufferSize(bgSocketServer_typ *p);
void bgTelnetSrvrMemAlloc(bgSocketServer_typ *p);
void bgTelnetServerCLI(bgSocketServer_typ *p);



void bgCLIHelp();

void bgCLIDebug(bgStringList_typ *p);
void bgCLIDebugHelp(bgStringList_typ *p);
void bgCLIDebugLevel(bgStringList_typ *p);
void bgCLIDebugFormat(bgStringList_typ *p);
void bgCLIDebugFilter(bgStringList_typ *p);


void bgCLIErrLog(bgStringList_typ *p);

void bgCLIPV(bgStringList_typ *p);
void bgCLIPVHelp(bgStringList_typ *p);
void bgCLIPVList(bgStringList_typ *p);
void bgCLIPVGet(bgStringList_typ *p);

const char *bgCLIGetPVTypeString(int pvTypeIndex);
void bgCLIPVPrintValue(unsigned int data_typ, unsigned int data_len, unsigned int dimension, unsigned int adr);


#if 0
_LOCAL char buffer[1024];
#endif


#define PV_TYPE_MAX_INDEX	15
static const char *bgPVTypeStrings[PV_TYPE_MAX_INDEX + 1]={
"STRUCT",
"BOOL  ",
"INT8  ",
"INT16 ",
"INT32 ",
"BYTE  ",
"WORD  ",
"LONG  ",
"FLOAT ",
"VISTR ",
"OCTET ",
"DATE  ",
"TIME  ",
"DIFF  ",
"BIT   ",
"ARRAY ",
};

static const char *bgDbgLevels[5]={
"CRITICAL",
"ERROR",
"WARNING",
"INFO",
"DEBUG",
};


/*
========================================================================================
*/
_INIT void bgTelnetSrvrInit()
{

	/*allocate memory for ringBuffer and send/recv buffers*/
	bgTelnetSrvrMemAlloc(&dbgTerm);
	
	/*init global RingBuffer*/	
	bgRingBuffInit(&gRingBuffer, (char*)dbgTerm.ringDataBuffer, RB_SIZE);

	/*init telnet server*/
	bgSocketServer(1, &dbgTerm);
}
/*
========================================================================================
*/
_CYCLIC void bgTelnetSrvrCyclic()
{
int len;

	/*if memalloc failed -> this task is useless*/
	if(dbgTerm.memAllocStatus1 || dbgTerm.memAllocStatus2)
		return;

	/*testing only; to enable it, use PV monitor to set variable > 10*/
	if(dbgTerm.testCounter > 10){
		dbgTerm.testCounter++;
		if(!(dbgTerm.testCounter % 10)){
			/*example of debug messages using different debug levels*/
			DBG("DBG: Server Test Output %d\r\n", dbgTerm.testCounter);

			DBGCRT("CRITICAL: Server Test Output %d\r\n", dbgTerm.testCounter);
			DBGERR("ERROR: Server Test Output %d\r\n", dbgTerm.testCounter);
			DBGWAR("WARNING: Server Test Output %d\r\n", dbgTerm.testCounter);
			DBGINF("INFO: Server Test Output %d\r\n", dbgTerm.testCounter);
			DBGDBG("DEBUG: Server Test Output %d\r\n", dbgTerm.testCounter);
		}	
	}

	/*monitor if server is ready to send*/
	if( bgSocketServerIsSendReady(&dbgTerm) ){
		/*get data from the ring buffer*/
		len = bgRingBuffGet(&gRingBuffer, bgSocketGetSendBuffer(&dbgTerm), bgSocketGetSendBufferSize(&dbgTerm) );
		
		if(len){
			/*send data*/
			bgSocketServerSendData(&dbgTerm, len);
		}
	}

	/*execute server state machine cyclicly*/
    bgSocketServer(0, &dbgTerm);
}

/*
========================================================================================
*/
void bgTelnetSrvrMemAlloc(bgSocketServer_typ *p)
{
unsigned int memSize;

	/*Get System Memory Information*/
	p->MEMInfo.enable = 1;
	MEMInfo(&p->MEMInfo);
	
/*TODO: use meminfo to see how much to allocated for ring buffer*/

	/*allocate space for RingBuffer data*/
	if(p->ringDataBuffer == 0){
		/* allocate new memory from RAM*/
    	p->memAllocStatus1 = TMP_alloc(RB_SIZE, (void**)&p->ringDataBuffer); 	
	}else{
		/*memory is allready allocated -> free it first!*/
    	p->memAllocStatus1 = TMP_free(RB_SIZE, (void**)p->ringDataBuffer); 	

    	/*allocate new memory from RAM*/
    	p->memAllocStatus1 = TMP_alloc(RB_SIZE, (void**)p->ringDataBuffer); 	
	}


	memSize = TELNET_SEND_BUFFER_SIZE + 2 * TELNET_RECV_BUFFER_SIZE;
	/*allocate space for send/recv buffers*/
	if(p->sendRecvBuffer == 0){
		/* allocate new memory from RAM*/
    	p->memAllocStatus2 = TMP_alloc(RB_SIZE, (void**)&p->sendRecvBuffer); 	
	}else{
		/*memory is allready allocated -> free it first!*/
    	p->memAllocStatus2 = TMP_free(RB_SIZE, (void**)p->sendRecvBuffer); 	

    	/*allocate new memory from RAM*/
    	p->memAllocStatus2 = TMP_alloc(RB_SIZE, (void**)p->sendRecvBuffer); 	
	}

	/*assign send/recv buffer ptrs*/
	p->dataSendBuf = (char*)p->sendRecvBuffer;
	p->dataRecvBuf = (char*)(p->sendRecvBuffer + TELNET_SEND_BUFFER_SIZE);
	p->lineRecvBuf = (char*)(p->sendRecvBuffer + TELNET_SEND_BUFFER_SIZE + TELNET_RECV_BUFFER_SIZE);

/*
Development only!
*/
/*
	p->dataSendBuf = (char*)p->devDataSendBuf;
	p->dataRecvBuf = (char*)p->devDataRecvBuf;
	p->lineRecvBuf = (char*)p->devLineRecvBuf;
*/
}


/*
========================================================================================
*/
#define TELNETD_STATE_NONE 0
#define TELNETD_STATE_INITOPEN 1
#define TELNETD_STATE_OPEN 2
#define TELNETD_STATE_SERVER 3
#define TELNETD_STATE_SEND 4
#define TELNETD_STATE_RECEIVE 5
#define TELNETD_STATE_CLOSE 6
#define TELNETD_STATE_CLOSE_DELAY 7

void bgSocketServerSendData(bgSocketServer_typ *p, int len)
{		
	p->TCPsend_1.datalen = len;
	p->sockStateData = TELNETD_STATE_SEND;								
}

char* bgSocketGetSendBuffer(bgSocketServer_typ *p)
{		
	return (p->dataSendBuf);
}

int bgSocketGetSendBufferSize(bgSocketServer_typ *p)
{		
	return (TELNET_SEND_BUFFER_SIZE);
}

int bgSocketServerIsSendReady(bgSocketServer_typ *p)
{
	return(p->sockSendOk);
}



int bgSocketServer(int init, bgSocketServer_typ *p)
{   
    /*open data socket (listen)*/
	if( (init == 1) && !p->sockStateData)
		p->sockStateData = 1;

    /*close data socket (listen stop)*/
	if( init == 2){
		/*command socket is telling us to receive data from server
		after this event, keep receiving until we don't get any data
		for a certain period of time
		*/
		p->sockData_Timer = 1;
		p->sockData_TimerTime = 10;
		p->sockData_TimerCounter = 0;
	}

	
	switch(p->sockStateData){
		case TELNETD_STATE_NONE:
    		return CMD_DONE;
		break;	
		case TELNETD_STATE_INITOPEN:
			p->sockData_Timer = 0;
			p->isEcho = 1;
			p->lineIndex = 0;
			p->lineIndexPrev = 0;
			
			p->TCPopen_1.enable = 1;			/* port number of the server socket*/
			p->TCPopen_1.port = 23;				/* port number of the server socket*/
			p->TCPopen_1.pIfAddr = 0;
			p->TCPopen_1.options = tcpOPT_REUSEADDR;				
    		p->sockStateData = TELNETD_STATE_OPEN;
		break;
		case TELNETD_STATE_OPEN:
			TcpOpen(&p->TCPopen_1);

			if( p->TCPopen_1.status == 0xFFFF )
				return CMD_BUSY;
		
			if( !p->TCPopen_1.status ){				
				p->TCPserv_1.enable = 1;
				p->TCPserv_1.ident = p->TCPopen_1.ident;
				p->TCPserv_1.backlog = 0; 
	    		p->TCPserv_1.pIpAddr = p->clientIpAddr;           					
				p->sockStateData = TELNETD_STATE_SERVER;
				return CMD_BUSY;
			}	

/* TODO: TcpOpen error handling! */
			p->sockStateData = TELNETD_STATE_NONE;
			p->isDataPortReady = 0; 
		break;
		case TELNETD_STATE_SERVER:
    		TcpServer(&p->TCPserv_1);					/* makes a connection with the clients */

			if(p->TCPserv_1.status == 0xFFFF)
				return CMD_BUSY;
			
			if(!p->TCPserv_1.status){
				p->TCPsend_1.enable = 1;
   				p->TCPsend_1.ident = p->TCPserv_1.identclnt;   /* copy the  Ident number */
				p->TCPsend_1.flags = 0;
				p->TCPsend_1.pData = (UDINT)p->dataSendBuf;


   				p->TCPrecv_1.enable = 1;
   				p->TCPrecv_1.ident = p->TCPserv_1.identclnt;   /* copy the  Ident number */
   				p->TCPrecv_1.pData = (UDINT)p->dataRecvBuf; /* address of the buffer */;
   				p->TCPrecv_1.datamax = TELNET_RECV_BUFFER_SIZE; /* length of the buffer */;

	   			p->TCPclose_1.ident = p->TCPserv_1.identclnt;    /* copy the  Ident number */

    			p->sockStateData = TELNETD_STATE_RECEIVE;
				bgTelnetServerWelcomeMsg(p);
				return CMD_BUSY;
			}
			
			
/*TODO: what to do on error?*/
			goto errorExit;
		break;
		case TELNETD_STATE_SEND:
			p->sockSendOk = 0;
            TcpSend(&p->TCPsend_1);            				/* send the data */
            if(!p->TCPsend_1.status){
            	p->counter.send_frames++;
    			p->sockStateData = TELNETD_STATE_RECEIVE;            
			}
			
			/*in case of the following error -> close connection*/
            if( (p->TCPsend_1.status == 27210) 	||
            	(p->TCPsend_1.status == 27135)	||
            	(p->TCPsend_1.status == tcpERR_NOT_CONNECTED)	||
            	(p->TCPsend_1.status == tcpERR_SYSTEM)	||
            	(p->TCPsend_1.status == 27249)
            ){
    			p->sockStateData = TELNETD_STATE_CLOSE;            
			}
		break;
		case TELNETD_STATE_RECEIVE:
            TcpRecv(&p->TCPrecv_1);          				/* recv data*/

            if(!p->TCPrecv_1.status){
            	p->counter.recv_frames++;
                if(p->TCPrecv_1.recvlen){
                    bgSocketServerRecv(p);                    
                }else{
					/*remote socket has closed*/
    				p->sockStateData = TELNETD_STATE_CLOSE;    /*close this server socket*/       					
				}
            }else{
    			p->sockSendOk = 1;							/*enable others to send*/                        
            }
				
		break;
		case TELNETD_STATE_CLOSE:
			p->TCPclose_1.enable = 1;
			p->TCPclose_1.how = tcpSHUT_RD & tcpSHUT_WR;
			
            TcpClose(&p->TCPclose_1);          				/* set the socket free */

			if(p->TCPclose_1.status == 0xFFFF){
				return CMD_BUSY;
			}

			p->delayCounter = 10;			
	    	p->sockStateData = TELNETD_STATE_CLOSE_DELAY;
		break;
		case TELNETD_STATE_CLOSE_DELAY:
			p->delayCounter--;
			if(!p->delayCounter){
	    		p->sockStateData = TELNETD_STATE_SERVER;
				p->isDataPortReady = 0; 
				p->sockData_Timer = 0;
			} 

		break;			
		default:
		break;
		}				

    return CMD_BUSY;

errorExit:
	
    TcpClose(&p->TCPclose_1);          /* set the socket free */
	p->sockStateData = 1;
	p->isDataPortReady = 0; 
	return CMD_ERROR;		
}

void bgSocketServerRecv(bgSocketServer_typ *p){

int len = p->TCPrecv_1.recvlen;
int i, k;
unsigned char dataChr;

	for(i=0, k=0;i<len;i++){
		dataChr = p->dataRecvBuf[i];
		
		switch(p->telnetRecvState){
			case 0:/*normal receive; check for telnet command chars*/

				/*0 - NOP*/
				if( dataChr == theNULL )
					break;

				/*"Interpret As Command"?*/
				if( dataChr == IAC ){
					p->iacRecvBuf[0] = dataChr;
					p->telnetRecvState++;	
				}else
				if( (dataChr == BS) && p->lineIndex ){
					p->lineIndex--;													
					p->lineIndexPrev = p->lineIndex;													
					DBGMSG("%c", BS);
				}else
				if( (dataChr == BS) && !p->lineIndex ){
					p->lineIndexPrev = p->lineIndex;													
				}else{
				
					/*regular char -> store into the line rcv buffer*/
					p->lineRecvBuf[p->lineIndex++] = dataChr;

					/*LineFeed or Carrige Return? -> EOL (End of Line)*/
					if( dataChr == LF )
						bgTelnetServerProcessLine(p);

					/*limit input -> overflow is danger!!!*/
					if( p->lineIndex >= (TELNET_RECV_BUFFER_SIZE - 5) )
						bgTelnetServerProcessLine(p);					
				}
			break;	
			case 1:/*get 2.nd telnet command char; <type of operation>*/

				/*check if there is a third char in command or not!?*/
				if( (dataChr >= WILL) && (dataChr <= DONT) ){
					p->iacRecvBuf[1] = dataChr;	
					p->telnetRecvState++;
				}else{
					p->iacRecvBuf[1] = dataChr;	
					p->telnetRecvState = 0;
					bgTelnetServerProcessCmd(p, 2);
				}
				
			break;	
			case 2:/*get 3.rd telnet command char; <option>*/
			
				p->iacRecvBuf[2] = dataChr;	
				p->telnetRecvState = 0;
				bgTelnetServerProcessCmd(p, 3);
				
			break;	

		}
	}

	if(p->isEcho && (p->lineIndex > p->lineIndexPrev) ){
		int chrCount = p->lineIndex - p->lineIndexPrev;
		memcpy(p->dataSendBuf, &p->lineRecvBuf[p->lineIndexPrev], chrCount);
		bgSocketServerSendData(p, chrCount);
		p->lineIndexPrev = p->lineIndex;
	}	
	
}

void bgTelnetServerProcessCmd(bgSocketServer_typ *p, int length)
{
	p->counter.iac++;
}

void bgTelnetServerProcessLine(bgSocketServer_typ *p)
{
	p->counter.lines++;

	bgTelnetServerCLI(p);

	p->lineIndex = 0;
	p->lineIndexPrev = 0;	

	DBGMSG("\r\n>");
}

void bgTelnetServerWelcomeMsg(bgSocketServer_typ *p)
{
	DBGMSG("###########################################################\r\n");
	DBGMSG("#                 Welcome!                                #\r\n");
	DBGMSG("#Debug and Command Interface to B&R controller via Telnet.#\r\n");
	DBGMSG("#Type '?' or 'help' for more information.                 #\r\n");
	DBGMSG("###########################################################\r\n");
	DBGMSG("#Connection from: %s", p->clientIpAddr);
	DBGMSG("###########################################################\r\n");
	DBGMSG("\r\n>");
}

void bgTelnetServerCLI(bgSocketServer_typ *p)
{
int lineLen = p->lineIndex - 1;
char *pLine;
char *pWord;
bgStringList_typ listOfWordsInLine;
int wordCount;

	/*terminate received line with 0 -> line is now a "C" string */
	p->lineRecvBuf[lineLen] = 0;	

#if 0
	/*for AS watch window*/
	strcpy(buffer, p->lineRecvBuf);
#endif

	/*check if CR present in line*/
	if(lineLen && (p->lineRecvBuf[lineLen-1] == CR) ){
		lineLen--;
		p->lineRecvBuf[lineLen] = 0;	
	}	

	if( !lineLen )
		return;
	
#if 0		
	DBG("\n\rCLI: Len=[%d],  Line=[%s]\n\r", lineLen, p->lineRecvBuf);	
#endif

	/*line is received!*/
	pLine = p->lineRecvBuf;

	/*split line into list of words*/
	listOfWordsInLine.src = pLine;
	wordCount = bgStringSplitLine(&listOfWordsInLine, ' ');

	if( wordCount == 0)
		return;

/*
#############################################################################################
#############################################################################################
*/

	/*first word -> command*/
	pWord = bgStringLower(listOfWordsInLine.items[0]);

	/*if first character is '#'->comment, nothing to do*/
	if( *pWord == '#' ){
		return;
	}

	/*search for commands*/
	if( bgStringIsMatch(pWord, "?", 1) ){
		bgCLIHelp();
		return;
	}

	if( bgStringIsMatch(pWord, "help", 1) ){
		bgCLIHelp();
		return;
	}


	if( bgStringIsMatch(pWord, "debug", 1) ){
		bgCLIDebug(&listOfWordsInLine);
		return;
	}

	if( bgStringIsMatch(pWord, "errlog", 1) ){
		bgCLIErrLog(&listOfWordsInLine);
		return;
	}

	if( bgStringIsMatch(pWord, "pv", 1) ){
		bgCLIPV(&listOfWordsInLine);
		return;
	}


	DBGMSG("\n\rUnknown command!\n\rPlease type '?' for list of valid commands!\r\n");

}


/*
#############################################################################################
#############################################################################################
help
#############################################################################################
#############################################################################################
*/
void bgCLIHelp()
{

	DBGMSG("\r\n");
	DBGMSG("###########################################################\r\n");
	DBGMSG("#Debug and Command Interface to B&R controller via Telnet.#\r\n");
	DBGMSG("#Version: 1.0                                             #\r\n");
	DBGMSG("#Date   : 2005-10-01                                      #\r\n");
	DBGMSG("#Author : Belgor Inc.                                     #\r\n\r\n");
	DBGMSG("#Available commands:                                      #\r\n");
	DBGMSG("# ?, help  - display this help text                       #\r\n");
	DBGMSG("# debug    - adjust & control debug output                #\r\n");
	DBGMSG("# errlog   - display controller's error log entries       #\r\n");
	DBGMSG("# pv       - access application's PVs (Process Variables) #\r\n");
	DBGMSG("# '#'      - comment (line starting with '#' is ignored)  #\r\n\r\n");
	DBGMSG("#For more information about individual commands type:     #\r\n");
	DBGMSG("#<CommandName> ?                                          #\r\n");
	DBGMSG("#or                                                       #\r\n");
	DBGMSG("#<CommandName> help                                       #\r\n");
	DBGMSG("###########################################################\r\n");
	DBGMSG("\r\n");
}


/*
#############################################################################################
#############################################################################################
debug
#############################################################################################
#############################################################################################
*/
void bgCLIDebug(bgStringList_typ *p)
{

int cliParamIndex;
char *pWord;

	cliParamIndex = 1;

	if( p->itemCount <= cliParamIndex ){
		bgCLIDebugHelp(p);
		return;
	}

	pWord = bgStringLower( p->items[cliParamIndex] );
	
	if( bgStringIsMatch(pWord, "?", 1) ){
		bgCLIDebugHelp(p);
		return;
	}

	if( bgStringIsMatch(pWord, "help", 1) ){
		bgCLIDebugHelp(p);
		return;
	}


	if( bgStringIsMatch(pWord, "level", 1) ){
		bgCLIDebugLevel(p);
		return;
	}

	if( bgStringIsMatch(pWord, "format", 1) ){
		bgCLIDebugFormat(p);
		return;
	}

	if( bgStringIsMatch(pWord, "filter", 1) ){
		bgCLIDebugFilter(p);
		return;
	}

	bgCLIDebugHelp(p);

}


/*
>debug ?
>debug help
*/
void bgCLIDebugHelp(bgStringList_typ *p)
{
	DBGMSG("\r\n");
	DBGMSG("Command: debug\n\r");
	DBGMSG("===========================================================\n\r");
	DBGMSG("- adjust and control debug output\r\n");
	DBGMSG("Following is the list of valid 'debug' commands:\r\n\r\n");
	DBGMSG(" debug\r\n");
	DBGMSG(" debug ?\r\n");
	DBGMSG(" debug help\r\n");
	DBGMSG(" --- displays this message\r\n\r\n");
	DBGMSG(" debug level\r\n");
	DBGMSG(" --- display current debug level, change (set) debug level\r\n\r\n");
	DBGMSG(" debug format\r\n");
	DBGMSG(" --- display current debug format, change (set) debug format\r\n");
	DBGMSG("===========================================================\n\r");
	DBGMSG("\r\n");
}


/*
>debug level
*/
void bgCLIDebugLevel(bgStringList_typ *p)
{
unsigned int value;

	if( p->itemCount <= 2){
		if( gRingBuffer.dbgLevel > BG_DEBUG_LEVEL_DEBUG )
			gRingBuffer.dbgLevel = 0;
			
		DBGMSG("\n\rCurrent Debug Level : %d (%s)\r\n", gRingBuffer.dbgLevel, bgDbgLevels[gRingBuffer.dbgLevel]);
		goto Exit;
	}


	if( bgAsciiToUint( p->items[2], &value) ){

		if( value <= BG_DEBUG_LEVEL_DEBUG){		
			DBGMSG("\n\rCurrent Debug Level : %d (%s)\r\n", gRingBuffer.dbgLevel, bgDbgLevels[gRingBuffer.dbgLevel]);
			gRingBuffer.dbgLevel = value;
			    DBGMSG("New Debug Level     : %d (%s)\r\n", gRingBuffer.dbgLevel, bgDbgLevels[gRingBuffer.dbgLevel]);
			return;
		}else{
			DBGMSG("\n\rCurrent Debug Level : %d (%s)\r\n", gRingBuffer.dbgLevel, bgDbgLevels[gRingBuffer.dbgLevel]);
			DBGMSG("\n\rInput Parameter Error! Debug Level value '%d' out of range!\r\n", value);
			goto Exit;
		}

	}else{

		DBGMSG("\n\rInput Parameter Error! Entered value '%s' not a numeric string!\r\n", p->items[2]);
		goto Exit;

	}

Exit:
	
		DBGMSG("\n\rPossible Debug Levels are:\r\n");
		DBGMSG("0 - CRITICAL\n\r");
		DBGMSG("1 - ERROR\n\r");
 		DBGMSG("2 - WARNING\n\r");
		DBGMSG("3 - INFO\n\r");
		DBGMSG("4 - DEBUG\n\r");
		DBGMSG("\n\r");
		DBGMSG("To set debug level use:\n\r");
		DBGMSG("debug level [0|1|2|3|4]\n\r");
		DBGMSG("\n\r");
		DBGMSG("Example (Setting debug level to 4 = DEBUG):\n\r");
		DBGMSG("debug level 4\n\r");		

}

/*
>debug format
*/
void bgCLIDebugFormat(bgStringList_typ *p)
{
unsigned int value;

	if( p->itemCount <= 2){
		DBGMSG("\n\rCurrent Debug Format : %x\r\n", gRingBuffer.dbgFormat);
		goto Exit;
	}


	if( bgAsciiToUint( p->items[2], &value) ){
		DBGMSG("\n\rCurrent Debug Format : %x\r\n", gRingBuffer.dbgFormat);
		gRingBuffer.dbgFormat = value;
		    DBGMSG("New Debug Format     : %x\r\n", gRingBuffer.dbgFormat);
		return;	
	}else{

		DBGMSG("\n\rInput Parameter Error! Entered value '%s' not a numeric string!\r\n", p->items[2]);
		goto Exit;

	}

Exit:
	
		DBGMSG("\n\rDebug Format Value is a bit field with following fields:\r\n");
		DBGMSG("Bit[0] - FileName     (0-off, 1-on)\n\r");
		DBGMSG("Bit[1] - FunctionName (0-off, 1-on)\n\r");
		DBGMSG("Bit[2] - TimeStamp    (0-off, 1-on)\n\r");
		DBGMSG("Bit[3] - Level        (0-off, 1-on)\n\r");
		DBGMSG("\n\r");
		DBGMSG("To set debug format value use:\n\r");
		DBGMSG("debug format [value]");
		DBGMSG("\n\r");
		DBGMSG("Example (Set debug format to 3 (bit[0]=1, bit[1]=1) -> Enable FileName and FunctionName display:\n\r");
		DBGMSG("debug format 3\n\r");		
}




/*
>debug filter
*/
void bgCLIDebugFilter(bgStringList_typ *p)
{
	DBGMSG("\n\rCmd='debug filter' TODO!\r\n");
}
/*
#############################################################################################
#############################################################################################
errlog
#############################################################################################
#############################################################################################
*/
void bgCLIErrLog(bgStringList_typ *p)
{
ERR_xtyp errStruct;
int i;
unsigned int status;
	DBGMSG("\n\rList Error Log Entries\n\r\n\r");
	DBGMSG("=============================================================+==========\n\r\n\r");
	DBGMSG("#  | Time stamp             | Errno | ErrInfo    | T | Task  | \n\r");
	DBGMSG("---+------------------------+-------+------------+---+-------+----------\n\r");
/*	DBGMSG("01 | 2005/09/21 12:50:30.44 | 12345 | 0x12345678 | 1 | syslog| \n\r");*/
/*  DBGMSG("[%02d] | %s | %05d | 0x%08x | %d | '%s' | '%s'\n\r"*/ 


	for(i=0;i<10;i++){
		status = ERRxread(i, &errStruct);    
		if(!status){
			char timeStamp[64];
			bgsprintf(timeStamp, "%04d/%02d/%02d %02d:%02d:%02d.%02d", 
				errStruct.err_year, 
				errStruct.err_month, 
				errStruct.err_day, 
				errStruct.err_hour, 
				errStruct.err_minute, 
				errStruct.err_second,
				errStruct.err_millisec);
			
			DBGMSG("%02d | %s | %05d | 0x%08x | %d | '%s' | '%s'\n\r", 
			i, timeStamp, errStruct.err_nr, errStruct.err_info, errStruct.err_type, errStruct.t_name, errStruct.err_string);
		}
	}
}

/*
#############################################################################################
#############################################################################################
pv
#############################################################################################
#############################################################################################
*/
void bgCLIPV(bgStringList_typ *p)
{
int cliParamIndex;

	cliParamIndex = 1;

	if( p->itemCount <= cliParamIndex ){
		bgCLIPVHelp(p);
		return;
	}

	
	if( bgStringIsMatch(p->items[cliParamIndex], "?", 1) ){
		bgCLIPVHelp(p);
		return;
	}

	if( bgStringIsMatch(p->items[cliParamIndex], "help", 1) ){
		bgCLIPVHelp(p);
		return;
	}


	if( bgStringIsMatch(p->items[cliParamIndex], "list", 1) ){
		bgCLIPVList(p);
		return;
	}

	if( bgStringIsMatch(p->items[cliParamIndex], "get", 1) ){
		bgCLIPVGet(p);
		return;
	}

	bgCLIPVHelp(p);
}

/*
>pv ?
>pv help
*/
void bgCLIPVHelp(bgStringList_typ *p)
{
	DBGMSG("\r\n");
	DBGMSG("Command: pv\n\r");
	DBGMSG("===========================================================\n\r");
	DBGMSG("- list, get and set application PVs (Process Variables)\r\n");
	DBGMSG("Following is the list of valid 'pv' commands:\r\n\r\n");
	DBGMSG(" pv ?\r\n");
	DBGMSG(" pv help\r\n");
	DBGMSG(" --- display this message\r\n");
	DBGMSG(" pv list\r\n");
	DBGMSG(" --- display list of available PVs in the system\r\n");
	DBGMSG(" pv get\r\n");
	DBGMSG(" --- get current value of a PV\r\n");
	DBGMSG(" pv set\r\n");
	DBGMSG(" --- set value of a PV\r\n");
	DBGMSG("===========================================================\n\r");
	DBGMSG("\r\n");
}


/*
>pv list
*/
void bgCLIPVList(bgStringList_typ *p)
{
PV_xList_typ pvxList;
int i;
unsigned int status;
unsigned int prev_index;
unsigned int next_index;


	DBGMSG("\n\rPV List:\n\r\n\r");
	DBGMSG("==========================================================\n\r");
	DBGMSG("#    | Type   | Len  | Dim  | Address    | Name\n\r");
	DBGMSG("-----+--------+------+------+------------+----------------\n\r");
	prev_index = 0xFFFF;
	for(i=0; i<50; i++){
		status = PV_xlist(prev_index, &next_index, &pvxList);
		prev_index = next_index;

		if(!status){
			DBGMSG("%4d | %s | %4d | %4d | 0x%08x | %s\n\r",
				i,  bgCLIGetPVTypeString(pvxList.data_typ), pvxList.data_len, pvxList.dimension, pvxList.adress,  pvxList.name);	
		}else
			return;
	}
}


/*
>pv get
*/
void bgCLIPVGet(bgStringList_typ *p)
{
int cliParamIndex;
unsigned int status;
unsigned int data_typ;
unsigned int data_len;
unsigned int dimension;
unsigned int adr;

	cliParamIndex = 2;

	if( p->itemCount <= cliParamIndex )
		return;

	status = PV_ninfo(p->items[cliParamIndex], (UDINT*)&data_typ, (UDINT*)&data_len, (UINT*)&dimension);


	if( status ){
		DBGMSG("\n\rError! Can't GET value for PV='%s'; Error#=%d\n\r", p->items[cliParamIndex], status);
		return;
	}

	
	status = PV_xgetadr(p->items[cliParamIndex], (UDINT*)&adr, (UDINT*)&data_len);
	
	if( status ){
		DBG("Can't GET address for PV='%s'; Error=%d\n\r", p->items[cliParamIndex], status);
		return;
	}


	DBGMSG("\n\r=========================================================\n\r");
	DBGMSG("\n\rGet PV Value\n\r");
	DBGMSG("=========================================================\n\r");
	DBGMSG("PV='%s' Type=%s Len=%d Dim=%d Address=0x%08x\n\r",
			p->items[cliParamIndex], bgCLIGetPVTypeString(data_typ), data_len, dimension, adr);	

	
	bgCLIPVPrintValue(data_typ, data_len, dimension, adr);


}


void bgCLIPVPrintValue(unsigned int data_typ, unsigned int data_len, unsigned int dimension, unsigned int adr)
{
/*
	if( dimension != 1)
		DBGMSG("TODO: display multidimensional values...");
*/
	switch(data_typ){
		case PB_DT_STRUCT:
			DBGMSG("TODO: display struct values...");
		break;	
		case PB_DT_BOOL:
			DBGMSG("Value=%d", *( (char*)adr ));
		break;	
		case PB_DT_INT8:
			DBGMSG("Value=%d, (0x%x)", *( (char*)adr), *( (char*)adr ) );
		break;	
		case PB_DT_INT16:
			DBGMSG("Value=%d, (0x%x)", *( (short*)adr ), *( (short*)adr ) );
		break;	
		case PB_DT_INT32:
			DBGMSG("Value=%d, (0x%x)", *( (int*)adr ), *( (int*)adr ));
		break;	
		case PB_DT_BYTE:
			DBGMSG("Value=%u, (0x%x)", *( (unsigned char*)adr), *( (unsigned char*)adr ));
		break;	
		case PB_DT_WORD:
			DBGMSG("Value=%u, (0x%x)", *( (unsigned short*)adr), *( (unsigned short*)adr ));
		break;	
		case PB_DT_LONG:
			DBGMSG("Value=%u, (0x%x)", *( (unsigned int*)adr), *( (unsigned int*)adr ));
		break;	
		case PB_DT_FLOAT:
			DBGMSG("TODO: display float values...");
		break;	
		case PB_DT_VIS:
			DBGMSG("Value='%s'", (char*)adr );
		break;	
		case PB_DT_OCTET:
			DBGMSG("TODO: display octet values...");
		break;	
		case PB_DT_DATE:
			DBGMSG("TODO: display date values...");
		break;	
		case PB_DT_TIME:
			DBGMSG("TODO: display time values...");
		break;	
		case PB_DT_DIFF:
			DBGMSG("TODO: display time-diff values...");
		break;	
		case PB_DT_BIT:
			DBGMSG("TODO: display bit values...");
		break;	
		case PB_DT_ARRAY:
			DBGMSG("TODO: display array values...");
		break;	
	}
}

#if 0
  status_adr = PV_xgetadr("PV_var1", &pv_adr, &data_len);   /* get the address 
                                                             and the lenght of  PV */
  strcpy(PV_var2.name, "This is a test");       /* copy the string into PV */
  memcpy((UDINT*)pv_adr, &PV_var2, sizeof(PV_test));  /* copy the second to the first PV */
  status_item = PV_item("PV_var1", 0, itemname);      /* info of the first item of PV1 */
  status_ninfo = PV_ninfo("PV_var2", &data_typ, &data_len2, &dimension);/* info of PV2 */


#endif

const char *bgCLIGetPVTypeString(int pvTypeIndex){
	if(pvTypeIndex >= PV_TYPE_MAX_INDEX){
		return "TYPE=?";
	}

	return bgPVTypeStrings[pvTypeIndex];
}




/*
TELNET CONSTANTS
#######################################################################################################################
*/
#if 0
#define SE      (240)  /*# Subnegotiation End*/
#define NOP     (241)  /*# No Operation*/
#define DM      (242)  /*# Data Mark*/
#define BRK     (243)  /*# Break*/
#define IP      (244)  /*# Interrupt process*/
#define AO      (245)  /*# Abort output*/
#define AYT     (246)  /*# Are You There*/
#define EC      (247)  /*# Erase Character*/
#define EL      (248)  /*# Erase Line*/
#define GA      (249)  /*# Go Ahead*/
#define SB      (250)  /*# Subnegotiation Begin*/

/*
# Telnet protocol options code (don't change)
# These ones all come from arpa/telnet.h
*/
#define BINARY  (0) /*# 8-bit data path*/
#define ECHO    (1) /*# echo*/
#define RCP     (2) /*# prepare to reconnect*/
#define SGA     (3) /*# suppress go ahead*/
#define NAMS    (4) /*# approximate message size*/
#define STATUS  (5) /*# give status*/
#define TM      (6) /*# timing mark*/
#define RCTE    (7) /*# remote controlled transmission and echo*/
#define NAOL    (8) /*# negotiate about output line width*/
#define NAOP    (9) /*# negotiate about output page size*/
#define NAOCRD  (10) /*# negotiate about CR disposition*/
#define NAOHTS  (11) /*# negotiate about horizontal tabstops*/
#define NAOHTD  (12) /*# negotiate about horizontal tab disposition*/
#define NAOFFD  (13) /*# negotiate about formfeed disposition*/
#define NAOVTS  (14) /*# negotiate about vertical tab stops*/
#define NAOVTD  (15) /*# negotiate about vertical tab disposition*/
#define NAOLFD  (16) /*# negotiate about output LF disposition*/
#define XASCII  (17) /*# extended ascii character set*/
#define LOGOUT  (18) /*# force logout*/
#define BM      (19) /*# byte macro*/
#define DET     (20) /*# data entry terminal*/
#define SUPDUP  (21) /*# supdup protocol*/
#define SUPDUPOUTPUT  (22) /*# supdup output*/
#define SNDLOC  (23) /*# send location*/
#define TTYPE   (24) /*# terminal type*/
#define EOR     (25) /*# end or record*/
#define TUID    (26) /*# TACACS user identification*/
#define OUTMRK  (27) /*# output marking*/
#define TTYLOC  (28) /*# terminal location number*/
#define VT3270REGIME  (29) /*# 3270 regime*/
#define X3PAD   (30) /*# X.3 PAD*/
#define NAWS    (31) /*# window size*/
#define TSPEED  (32) /*# terminal speed*/
#define LFLOW   (33) /*# remote flow control*/
#define LINEMODE    (34) /*# Linemode option*/
#define XDISPLOC    (35) /*# X Display Location*/
#define OLD_ENVIRON (36) /*# Old - Environment variables*/
#define AUTHENTICATION  (37) /*# Authenticate*/
#define ENCRYPT (38) /*# Encryption option*/
#define NEW_ENVIRON (39) /*# New - Environment variables*/
/*
# the following ones come from
# http://www.iana.org/assignments/telnet-options
# Unfortunately, that document does not assign identifiers
# to all of them, so we are making them up
*/
#define TN3270E     (40) /*# TN3270E*/
#define XAUTH       (41) /*# XAUTH*/
#define CHARSET     (42) /*# CHARSET*/
#define RSP         (43) /*# Telnet Remote Serial Port*/
#define COM_PORT_OPTION     (44) /*# Com Port Control Option*/
#define SUPPRESS_LOCAL_ECHO (45) /*# Telnet Suppress Local Echo*/
#define TLS         (46) /*# Telnet Start TLS*/
#define KERMIT      (47) /*# KERMIT*/
#define SEND_URL    (48) /*# SEND-URL*/
#define FORWARD_X   (49) /*# FORWARD_X*/
#define PRAGMA_LOGON    (138) /*# TELOPT PRAGMA LOGON*/
#define SSPI_LOGON      (139) /*# TELOPT SSPI LOGON*/
#define PRAGMA_HEARTBEAT  (140) /*# TELOPT PRAGMA HEARTBEAT*/
#define EXOPL       (255) /*# Extended-Options-List*/
#define NOOPT       (0)     /**/
#endif
/*
TELNET INFO
#######################################################################################################################


The Network Virtual Terminal:
============================
Communication is established using TCP/IP and is based on a Network Virtual Terminal (NVT). On the 
client, the Telnet program is responsible for translating incoming NVT codes to codes understood 
by the client's display device as well as for translating client-generated keyboard codes into outgoing
NVT codes.

The NVT uses 7-bit codes for characters. The display device, referred to as a printer in the RFC, is 
only required to display the standard printing ASCII characters represented by 7-bit codes and to 
recognize and process certain control codes. The 7-bit characters are transmitted as 8-bit bytes with 
the most significant bit set to zero. An end-of-line is transmitted as a carriage return (CR) followed 
by a line feed (LF). If you want to transmit an actual carriage return, this is transmitted as a 
carriage return followed by a NUL (all bits zero) character. 

NVT ASCII is used by many other Internet protocols like SMTP and FTP.

The following control codes are required to be understood by the NVT. 

Name            Code    Decimal Value   Function 
=================================================
NULL            NUL     0               No operation 
Line Feed       LF      10              Moves the printer to the next print line, keeping the same horizontal position.  
Carriage Return CR      13              Moves the printer to the left margin of the current line.  


The following further control codes are optional but should have the indicated defined effect on the 
display. 

Name            Code    Decimal Value Function 
==============================================
BELL            BEL     7       Produces an audible or visible signal (which does NOT move the print head.  
Back Space      BS      8       Moves the print head one character position towards the left margin. (On a printing device, this mechanism was commonly used to form composite characters by printing two basic characters on top of each other.) 
Horizontal Tab  HT      9       Moves the printer to the next horizontal tab stop. It remains unspecified how either party determines or establishes where such tab stops are located. 
Vertical Tab    VT      11      Moves the printer to the next vertical tab stop. It remains unspecified how either party determines or establishes where such tab stops are located.  
Form Feed       FF      12      Moves the printer to the top of the next page, keeping the same horizontal position. (On visual displays, this commonly clears the screen and moves the cursor to the top left corner.) 

The NVT keyboard is specified as being capable of generating all 128 ASCII codes by using keys, key 
combinations, or key sequences

Commands:
=========
The Telnet protocol uses various commands to control the client-server connection. These commands are 
transmitted within the data stream. The commands are distinguished from the data by setting the most 
significant bit to 1. (Remember that data is transmitted as 7-bits with the eighth bit set to 0) 
Commands are always introduced by the Interpret as command (IAC) character. 


Here is the complete set of commands:

Name        Decimal Code    Meaning Comment 
===========================================
SE          240             End of subnegotiation parameters  
NOP         241             No operation  
DM          242             Data mark Indicates the position of a Synch event within the data stream. This should always be accompanied by a TCP urgent notification. 
BRK         243             Break Indicates that the "break" or "attention" key was hi.  
IP          244             Suspend Interrupt or abort the process to which the NVT is connected. 
AO          245             Abort output Allows the current process to run to completion but does not send its output to the user.  
AYT         246             Are you there Send back to the NVT some visible evidence that the AYT was received. 
EC          247             Erase character The receiver should delete the last preceding undeleted character from the data stream.  
EL          248             Erase line Delete characters from the data stream back to but not including the previous CRLF.  
GA          249             Go ahead Under certain circumstances used to tell the other end that it can transmit. 
SB          250             Subnegotiation Subnegotiation of the indicated option follows. 
WILL        251             will Indicates the desire to begin performing, or confirmation that you are now performing, the indicated option. 
WONT        252             wont Indicates the refusal to perform, or continue performing, the indicated option. 
DO          253             do Indicates the request that the other party perform, or confirmation that you are expecting the other party to perform, the indicated option. 
DONT        254             dont Indicates the demand that the other party stop performing, or confirmation that you are no longer expecting the other party to perform, the indicated option.  
IAC         255             Interpret as command Interpret as a command 


Telnet Options:
===============
Options give the client and server a common view of the connection. They can be negotiated at any time during the 
connection by the use of commands. They are described in separate RFCs. 

The following are examples of common options:

Either end of a telnet dialogue can enable or disable an option either locally or remotely. 
The initiator sends a 3 byte command of the form 

    IAC,<type of operation>,<option>

The response is of the same form. 


Operation is one of 

Description     Decimal Code    Action
======================================
WILL            251             Sender wants to do something.  
DO              252             Sender wants the other end to do something.  
WONT            253             Sender doesn't want to do something.  
DONT            254             Sender wants the other not to do something.  


Associated with each of the these there are various possible responses 

Sender Sent     Receiver Responds   Implication  
===============================================
WILL            DO                  The sender would like to use a certain facility if the receiver can handle it. Option is now in effect  
WILL            DONT                Receiver says it cannot support the option. Option is not in effect.  
DO              WILL                The sender says it can handle traffic from the sender if the sender wishes to use a certain option. Option is now in effect.  
DO              WONT                Receiver says it cannot support the option. Option is not in effect.  
WONT            DONT                Option disabled. DONT is only valid response.  
DONT            WONT                Option disabled. WONT is only valid response.  


For example if the sender wants the other end to suppress go-ahead it would send the byte sequence: 

    255(IAC),251(WILL),3

The final byte of the three byte sequence identifies the required action. 
For some of the negotiable options values need to be communicated once support of the option has 
been agreed. This is done using sub-option negotiation. Values are communicated via an exchange of 
value query commands and responses in the following form. 

    IAC,SB,<option code number>,1,IAC,SE

and 


    IAC,SB,<option code>,0,<value>,IAC,SE

For example if the client wishes to identify the terminal type to the server the following exchange 
might take place 

Client   255(IAC),251(WILL),24
Server   255(IAC),253(DO),24
Server   255(IAC),250(SB),24,1,255(IAC),240(SE)
Client   255(IAC),250(SB),24,0,'V','T','2','2','0',255(IAC),240(SE)

The first exchange establishes that terminal type (option number 24) will be handled, the server 
then enquires of the client what value it wishes to associate with the terminal type. The sequence 
SB,24,1 implies sub-option negotiation for option type 24, value required (1). The IAC,SE sequence 
indicates the end of this request. The repsonse IAC,SB,24,0,'V'... implies sub-option negotiation 
for option type 24, value supplied (0), the IAC,SE sequence indicates the end of the response 
(and the supplied value). The encoding of the value is specific to the option but a sequence of 
characters, as shown above, is common. 

Telnet Negotiable Options
=========================
=========================
Many of those listed are self-evident, but some call for more comments. 


Suppress Go Ahead 
=========================
The original telnet implementation defaulted to "half duplex" operation. This means that data traffic 
could only go in one direction at a time and specific action is required to indicate the end of traffic 
in one direction and that traffic may now start in the other direction. [This similar to the use of 
"roger" and "over" by amateur and CB radio operators.] The specific action is the inclusion of a GA 
character in the data stream. 

Modern links normally allow bi-directional operation and the "suppress go ahead" option is enabled. 

echo 
=========================
The echo option is enabled, usually by the server, to indicate that the server will echo every 
character it receives. A combination of "suppress go ahead" and "echo" is called character at a time 
mode meaning that each character is separately transmitted and echoed. 

There is an understanding known as kludge line mode which means that if either "suppress go ahead" 
or "echo" is enabled but not both then telnet operates in line at a time mode meaning that complete 
lines are assembled at each end and transmitted in one "go". 

linemode 
=========================
This option replaces and supersedes the line mode kludge. 

remote flow control 
=========================
This option controls where the special flow control effects of Ctrl-S/Ctrl-Q are implemented. 


Telnet control functions
=========================
=========================
The telnet protocol includes a number of control functions. These are initiated in response to 
conditions detected by the client (usually certain special keys or key combinations) or server. 
The detected condition causes a special character to be incorporated in the data stream. 


Interrupt Process 
=========================
This is used by the client to cause the suspension or termination of the server process. Typically the 
user types Ctrl-C on the keyboard. An IP (244) character is included in the data stream. 

Abort Output 
=========================
This is used to suppress the transmission of remote process output. An AO (238) character is included 
in the data stream. 

Are You There 
=========================
This is used to trigger a visible response from the other end to confirm the operation of the link and 
the remote process. An AYT (246) character is incorporated in the data stream. 

Erase character 
=========================
Sent to the display to tell it to delete the immediately preceding character from the display. An EC 
(247) character is incorporated in the data stream. 

Erase line 
=========================
Causes the deletion of the current line of input. An EL (248) character is incorporated in the data 
stream. 

Data Mark 
=========================
Some control functions such as AO and IP require immediate action and this may cause difficulties 
if data is held in buffers awaiting input requests from a (possibly misbehaving) remote process. 
To overcome this problem a DM (242) character is sent in a TCP Urgent segment, this tells the receiver 
to examine the data stream for "interesting" characters such as IP, AO and AYT. This is known as the 
telnet synch mechanism. 

A DM not in a TCP Urgent segment has no effect.
*/
/*EOF*/
