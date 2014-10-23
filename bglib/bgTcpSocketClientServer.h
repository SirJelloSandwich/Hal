#ifndef _bgTcpSocketClientServer_h_
#define _bgTcpSocketClientServer_h_

#ifdef WIN32
#define STRING char
#define plcbit int
#define UDINT char
typedef struct TcpClose
{
	/* VAR_INPUT (analog) */
	unsigned long ident;
	unsigned long how;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR (analog) */
	unsigned short i_state;
	unsigned short i_result;
	unsigned long i_tmp;
	/* VAR_INPUT (digital) */
	plcbit enable;
} TcpClose_typ;

typedef struct TcpSend
{
	/* VAR_INPUT (analog) */
	unsigned long ident;
	unsigned long pData;
	unsigned long datalen;
	unsigned long flags;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	unsigned long sentlen;
	/* VAR (analog) */
	unsigned short i_state;
	unsigned short i_result;
	unsigned long i_tmp;
	/* VAR_INPUT (digital) */
	plcbit enable;
} TcpSend_typ;

typedef struct TcpRecv
{
	/* VAR_INPUT (analog) */
	unsigned long ident;
	unsigned long pData;
	unsigned long datamax;
	unsigned long flags;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	unsigned long recvlen;
	/* VAR (analog) */
	unsigned short i_state;
	unsigned short i_result;
	unsigned long i_tmp;
	/* VAR_INPUT (digital) */
	plcbit enable;
} TcpRecv_typ;

#else

#include <bur/plctypes.h>
#include <astcp.h>
#include <brsystem.h>
#include <sys_lib.h>

#endif /*ifdef WIN32*/

#include "bgstring.h"
#include "bgBufMemAlloc.h"

#define BGTCPSOCKSRV_MAX_SESSIONS 4
#define BGTCPSOCKCLI_MAX_SESSIONS 1

#define BGTCPSOCK_STATE_NONE 		0
#define BGTCPSOCK_STATE_INITOPEN 	1
#define BGTCPSOCK_STATE_OPEN 		2
#define BGTCPSOCK_STATE_LISTEN 		3	
#define BGTCPSOCK_STATE_TOOBUSY 	4
#define BGTCPSOCK_STATE_CLIENT 		5
#define BGTCPSOCK_STATE_IOCONTROL 	6
#define BGTCPSOCK_STATE_CONNECTED 	7
#define BGTCPSOCK_STATE_CLOSE 		8
#define BGTCPSOCK_STATE_ERROR 		9

#define BGTCPSOCK_SESSION_STATE_NONE 	0
#define BGTCPSOCK_SESSION_STATE_SEND 	1
#define BGTCPSOCK_SESSION_STATE_RECEIVE 2
#define BGTCPSOCK_SESSION_STATE_CLOSE 	3
#define BGTCPSOCK_SESSION_STATE_DELAY 	4


#define BGHTTP_REQUEST_MAX_HEADER_ITEMS 5

#define BGHTTP_REQUEST_COMMAND_GET 1
#define BGHTTP_REQUEST_COMMAND_POST 2

#define BGHTTP_CONST_STRING_VERSION "HTTP/1.0"


#define BGHTTP_CONST_STRING_CRLF "\r\n"

#define BGHTTP_HEADER_KEY_CONLEN "Content-Length"	/*header keyword: Content-Length*/
#define BGHTTP_HEADER_VAL_CONLEN "1234567890"		/*place holder for the keyword value*/


#define BGTCPSOCKSRV_MAX_SESSIONMEMBLOCKS 8

typedef struct httpMessage_typ{
    /*Note: following lists require external ("allocated") memory for storing list of ptr-s!*/
    bgStringListEx_typ listHeaderLines;             /*list of lines (char*) in the HTTP request header (if any)*/    
    bgStringListEx_typ list1;                 		/*list for different small tasks (i.e. parsing lines, etc)*/

	char* command;                                  /*HTTP command name (i.e. GET, PUT, POST*/
	int commandId;                                  /*HTTP command id (i.e. BGHTTP_REQUEST_COMMAND_XXX)*/

	char* url;                                      /*HTTP command url (i.e. /index.html, /dir1/file2.txt, etc.)*/
	
	char* versionString;                            /*HTTP version string (i.e. HTTP/1.0)*/
    int version;                                    /*Version number (i.e. 10)*/

    int header1stLine;                              /*HTTP header: line index of the line (in listOfLines) where header starts*/
    int headerLineCount;                            /*HTTP header: line count (in listOfLines) that are header lines*/

    int contentLength;                           	/*HTTP content: length of the content extracted from the header keyword "Content-Length"*/	
	int contentPosition;							/*HTTP content: position in recvBuf where the content starts*/

#if 0
	int send1stLineLength;							/*HTTP send message; 1st line (cmd response) length*/
#endif
	int sendHeaderLength;							/*HTTP send message; 1st line + header length*/
	int sendContentLength;							/*HTTP send message; content length*/
	int sendTotalLength;							/*HTTP send message; total length*/
	
}httpMessage_typ;


typedef struct bgTcpSocketSession_typ {
	int init;                                       /*init request flag, set to 1 in order to init session layer*/
	int initOk;                                     /*init complete flag, set to 1 if init is all ok*/

    int ident;                                      /*session id (i.e. 0,1,...BGTCPSOCKSRV_MAX_SESSIONS-1)*/

    int socketState;                            
        
	TcpSend_typ tcpSend;
	TcpRecv_typ tcpRecv;
	TcpClose_typ tcpClose;        


	char* pSendBuf;                                 /**<ptr session send buf*/
	int sendBufLength;								/**<total size (bytes) session send buffer*/	
	int curIdxSendBuf;								/**<index into send buf current spot; for incremental write*/	

	char* pRecvBuf;                                 /**<ptr session recv buf*/    
	int recvBufLength;								/**<total size (bytes) session recv buf*/
    
	char* pAppBuf;									/*app/session specific buffer space/ memory*/ 
	int appBufLength;
	bgBufMemAlloc_typ sessionMem;					/*app mem may be divided into app specific mem blocks*/


    int rxByteCountTotal;                           /*recv total length*/
    int txByteCountTotal;                           /*send total length*/
	int rxByteCount;								/*total recv data lenght (bytes)*/
	int txByteCount;								/*total send data lenght (bytes)*/
    
	STRING clientIpAddr[32];                        


	int delayCounter;

	char* pTcpLayer;                                /*ptr to lower layer (server)*/
	char* pProtocol;                                /*ptr to upper layer (protocol)*/


	int	bCloseSession;								/*flag (1 shot); set by protocol (upper) layer to request session (socket) close*/
	int	bSendData;									/*flag (1 shot); set by protocol (upper) layer to request send operation*/

    void (*pSendBufHandler)(struct bgTcpSocketSession_typ*);	/*callback routine for sending long data; dataLen > TCPSendBufSize (8192 bytes default)*/

	int dbgDumpSendBuf;
	int dbgDumpRecvBuf;
}bgTcpSocketSession_typ;


typedef struct bgTcpSocketServer_typ{
    int serverPort;                                 /*port address of the server socket; listening port number; i.e. http standard is 80*/

    int numSupportedSessions;                       /*number of sessions supported by this server <= BGTCPSOCKSRV_MAX_SESSIONS*/

	STRING serverIpAddr[32];                        /*server own ip address*/
	STRING clientIpAddr[32];                        /*place to store address of the currently connected client*/
    
	unsigned int sockStateData;                     /*state of the server data socket*/

#ifndef WIN32
	TcpOpen_typ tcpOpen;
    TcpServer_typ tcpServer;
	TcpIoctl_typ tcpIoctl;
#endif
    int numActiveSessions;                          /*number of currently active sessions -for client, only 1*/
    bgTcpSocketSession_typ session[BGTCPSOCKSRV_MAX_SESSIONS];

                                                    /*server specific memory allocation*/

	UDINT *sendRecvBuffer;	                        /*ptr to allocated space used for send/recv/app buffers*/
#ifndef WIN32
	MEMInfo_typ MEMInfo;
#endif
	unsigned int memAllocStatus2;
    int sessionSendBufSize;
    int sessionRecvBufSize;
    int sessionTotalBufSize;
    int applicationBufSize;
    int totalBufSize;
	                                                /*session and application speficic handlers*/
    void (*pSessionHandler)(bgTcpSocketSession_typ*);
    void (*pApplicationHandler)(bgTcpSocketSession_typ*);
                        
	int test;
	int testCounter;
	int delayCounter;
	int dbgDumpSendBuf;
	int dbgDumpRecvBuf;
}bgTcpSocketServer_typ;


/*
Basic Overview of the TcpSocketServer:

    TcpSocket (listen & spawn sessions) ----+-<>---- Session -----<>----- Application
                                            |       (i.e. HttpProtocol)   (i.e. WebServer; XmlRpcServer)
                                            |                                   
                                            +-<>---- Session
                                            |
                                            :                                            :
*/

typedef struct bgTcpSocketClient_typ{
    int serverPort;                                 /*port address of the server socket*/

    int numSupportedSessions;                       /*number of sessions supported by this server <= BGTCPSOCKCLI_MAX_SESSIONS*/

	STRING serverIpAddr[32];                        /*server ip address*/
	STRING clientIpAddr[32];                        /*client's own ip address*/
    
	unsigned int sockStateData;                     /*state of the client data socket*/

#ifndef WIN32
	TcpOpen_typ tcpOpen;
    TcpClient_typ tcpClient;
    TcpClose_typ tcpClose;
	TcpIoctl_typ tcpIoctl;	
#endif
    int numActiveSessions;                          /*number of currently active sessions -for client, only 1*/
    bgTcpSocketSession_typ session[BGTCPSOCKCLI_MAX_SESSIONS];
                                                    /*client specific memory allocation*/

	UDINT *sendRecvBuffer;	                        /*ptr to allocated space used for send/recv/app buffers*/
	char* recvBuffer;	                        	/*ptr recv buffer only! IMPORTANT: client recv buffer must be a PV variable!!!!*/

#ifndef WIN32
	MEMInfo_typ MEMInfo;
#endif
	unsigned int memAllocStatus2;
    int sessionSendBufSize;
    int sessionRecvBufSize;
    int sessionTotalBufSize;
    int applicationBufSize;
    int totalBufSize;
	                                                /*session and application speficic handlers*/
    void (*pSessionHandler)(bgTcpSocketSession_typ*);
    void (*pApplicationHandler)(bgTcpSocketSession_typ*);
                        
	int test;
	int testCounter;
	int delayCounter;
	int dbgDumpSendBuf;
	int dbgDumpRecvBuf;
}bgTcpSocketClient_typ;






/*
Tcp Socket Server
*/
void bgTcpSocketServerInit(bgTcpSocketServer_typ* pServer, 
    int portNumber,
    int numberOfSessions,
    int sessionSendBufSize,
    int sessionRecvBufSize,
    int applicationBufSize,
    void (*pSessionHandler)(bgTcpSocketSession_typ*),
    void (*pApplicationHandler)(bgTcpSocketSession_typ*));

void bgTcpSocketServerCyclic(bgTcpSocketServer_typ* pServer);


/*
Tcp Socket Client
*/
void bgTcpSocketClientInit(bgTcpSocketClient_typ* pClient,
    int sessionSendBufSize,
    int sessionRecvBufSize,
    int applicationBufSize,
    char* pRecvBuf,
    void (*pSessionHandler)(bgTcpSocketSession_typ*),
    void (*pApplicationHandler)(bgTcpSocketSession_typ*));


void bgTcpSocketClientCyclic(bgTcpSocketClient_typ* pClient);
#endif
