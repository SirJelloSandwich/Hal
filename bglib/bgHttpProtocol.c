/*
File: bgHttpProtocol.c
=====

*/

/*
B&R components:
===============================================================================
*/
#ifndef WIN32
#include <bur/plc.h>
#include <bur/plctypes.h>
#include <astcp.h>
#include <string.h>
#include <brsystem.h>
#include <sys_lib.h>
#endif

/*
BG components
===============================================================================
*/
#include "bgdebug.h"
#include "bgstring.h"
#include "bgsscanf.h"
#include "bgTcpSocketClientServer.h"
#include "bgHttpProtocol.h"


static void addLine(bgTcpSocketSession_typ* pSession, char* s)
{
char* pDst = (pSession->pSendBuf + pSession->curIdxSendBuf);
int len = strlen(s);

/*
	if( bgHttpSendBufMonitorOverflow(pSession) )
		return;
*/

	strcpy(pDst, s);
	pSession->curIdxSendBuf += len;
	strcat(pDst, BGHTTP_CONST_STRING_CRLF);
	pSession->curIdxSendBuf += strlen(BGHTTP_CONST_STRING_CRLF);
}

static int bgHttpClientProcessResponse(bgTcpSocketSession_typ* pSession);
static void bgHttpClientSessionAppMemAlloc(bgTcpSocketSession_typ* pSession);
static void bgHttpServerSessionAppMemAlloc(bgTcpSocketSession_typ* pSession);
static void bgHttpSendBufReset(bgTcpSocketSession_typ* pSession);
static void bgHttpSendBufApndHeaderLine(bgTcpSocketSession_typ* pSession, char* headerLine);
static void bgHttpSendBufApndHeaderEnd(bgTcpSocketSession_typ* pSession);
static void bgHttpSendBufApndContentLine(bgTcpSocketSession_typ* pSession, char* contentLine);
static void bgHttpSendBufApndContentEnd(bgTcpSocketSession_typ* pSession);
static int bgHttpSendBufMonitorOverflow(bgTcpSocketSession_typ* pSession, int newLineSize);
static void bgHttpSendBufFillContentSize(bgTcpSocketSession_typ* pSession);
static char* bgHttpGetResponseMsg(int responseCode);
static int bgHttpServerProcessRequest(bgTcpSocketSession_typ* p);
static int bgHttpReceiverGetContentLengthAndPosition(bgTcpSocketSession_typ* pSession);
static int bgHttpReceiver(bgTcpSocketSession_typ* pSession);
static void bgHttpSendBufApndHeaderLineAndFormat(bgTcpSocketSession_typ* pSession, char* headerKeyword, char* value);
static void bgHttpServerSendResponseSetHeader(bgTcpSocketSession_typ* pSession, int responseCode, int contentSize, char* contentType);
static void bgHttpSendBufCallback(bgTcpSocketSession_typ* pSession);
static void bgHttpSendBufDebug(bgTcpSocketSession_typ* pSession);
static void bgHttpRecvBufDebug(bgTcpSocketSession_typ* pSession);


/*
HTTP Protocol
######################################################################################################
######################################################################################################
######################################################################################################
######################################################################################################
*/
void bgHttpServerSessionHandler(bgTcpSocketSession_typ* pSession)
{
	if(pSession->init){
		DBGMSG("bgHttpServerSessionHandler(): init\r\n");
		bgHttpServerSessionAppMemAlloc(pSession);
		/*init receive byte count*/
		pSession->rxByteCount = 0;
		return;
	}


	if( !pSession->initOk ){
		DBGMSG("bgHttpServerSessionHandler(): can't execute session; init NOT OK\r\n");
		return;
	}
	
	DBGMSG("bgHttpServerSessionHandler(): execute\r\n");

	if( bgHttpReceiver(pSession) )
		return;		/*http receive is NOT complete; continue with receiving*/

	if( pSession->dbgDumpRecvBuf )
		bgHttpRecvBufDebug(pSession);
	
	if( bgHttpServerProcessRequest(pSession) )
        return;		/*http request error; terminate session*/

    /*http receive success; execute application/protocol layer*/
    {
        bgTcpSocketServer_typ* pServer = (bgTcpSocketServer_typ*)pSession->pTcpLayer;
        if( pServer->pApplicationHandler )
            (*pServer->pApplicationHandler)(pSession);
        else
            bgHttpSendErrorResponse(pSession, 501);
    }
}


void bgHttpServerSendResponse(bgTcpSocketSession_typ* pSession, int responseCode, char* pContent , int contentSize, char* contentType)
{

    	/*init/reset send buffer*/
	bgHttpSendBufReset(pSession);

    /*define HTTP header entries*/
    bgHttpServerSendResponseSetHeader(pSession, responseCode, contentSize, contentType);
    		
	/*append content, if any*/
	if(pContent && contentSize)
		strcat(pSession->pSendBuf, pContent);

	pSession->tcpSend.datalen = strlen(pSession->pSendBuf);
	bgHttpSendData(pSession);
	DBGMSG("bgHttpServerSendResponse: rspCode=[%d], totalLen=%d, contentSize=%d\r\n", 
     responseCode, pSession->tcpSend.datalen, contentSize);
}

void bgHttpServerSendResponseBeg(bgTcpSocketSession_typ* pSession, int responseCode, char* contentType)
{
    /*init/reset send buffer*/
	bgHttpSendBufReset(pSession);

    /*define HTTP header entries*/
    bgHttpServerSendResponseSetHeader(pSession, responseCode, 1234567, contentType);    		
	DBGMSG("bgHttpServerSendResponseBeg: rspCode=[%d]\r\n", responseCode);
}

void bgHttpServerSendResponseEnd(bgTcpSocketSession_typ* pSession)
{
    		
	bgHttpSendBufApndContentEnd(pSession);
	bgHttpSendBufFillContentSize(pSession);
	pSession->tcpSend.datalen = strlen(pSession->pSendBuf);
	bgHttpSendData(pSession);
	DBGMSG("bgHttpServerSendResponseEnd: totalLen=%d\r\n", pSession->tcpSend.datalen);
}



static void bgHttpServerSessionAppMemAlloc(bgTcpSocketSession_typ* pSession)
{
/*
 	HTTP session:
 	On init, "alocate" memory within the session application space.

	Session/Application memory allocation:

	-each session/application is given  memory space
	-size of the app space (APPLICATION BUFFER SIZE) is defined in the server init call:
		i.e. bgTcpSocketServerInit(&bgXmlRpcServerData, ... 2048, ...);		

	- depending on the application/session needs, this space can be further devided into
	"allocated" memory blocks. See bgBufMemAllocXXX object

	pAppBuffer allocation scheme for bgHttpServerSessionHandler:
		struct httpMessage_typ		--- sizeof(httpMessage_typ) bytes
		listHeaderLines Ptr-s		--- 32 * sizeof(char*) = 128 bytes (holds max 32 items)
		list1 						--- 32 * sizeof(char*) = 128 bytes (holds max 32 items)
		remainig space: listContentLines ptr-s (i.e. 1kBytes of space is 256 items (lines) in the list 

*/
char* pMem;
int sizeMem;
httpMessage_typ* pHttpMsg;
	
	/*set initOk to NOT OK*/
	pSession->initOk = 0;
	
	bgBufMemAllocInit(&pSession->sessionMem, pSession->pAppBuf, pSession->appBufLength);


	/*1) protocol/application specific data structure, upper layer*/
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, sizeof(httpMessage_typ) + sizeof(char*), &pMem, &sizeMem) )
	{
    	DBGMSG("bgHttpServerSessionHandler(): init; FATAL ERROR (1); No memory\r\n");
		return;
	}
	
	pHttpMsg = (httpMessage_typ*)pMem;
	pSession->pProtocol = (char*)pHttpMsg; 

	/*2) set storage for list of http header lines (anticipated short list of items)*/		
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, 33 * sizeof(char *), &pMem, &sizeMem) )
	{
    	DBGMSG("bgHttpServerSessionHandler(): init; FATAL ERROR (2); No memory\r\n");
		return;
	}
	bgStringListExMemSet(&pHttpMsg->listHeaderLines, pMem, 32 * sizeof(char *));

	/*3) set storage for short helper list (short list of items, i.e. 32)*/
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, 33 * sizeof(char *), &pMem, &sizeMem) )
	{
    	DBGMSG("bgHttpServerSessionHandler(): init; FATAL ERROR (3); No memory\r\n");
		return;
	}
    bgStringListExMemSet(&pHttpMsg->list1, pMem, 32 * sizeof(char *));

#if 0
	/*4) set storage for line list (long list of items, i.e. several hundred items, depending on the memory)*/
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, 0, &pMem, &sizeMem) )
	{
    	DBGMSG("bgHttpServerSessionHandler(): init; FATAL ERROR (4); No memory\r\n");
		return;
	}
    bgStringListExMemSet(&pHttpMsg->listContentLines, pMem, sizeMem);
#endif

	/*init OK*/
	pSession->initOk = 1;

#if 0
	/*dump appliction buffer memory allocation*/
	{
	int i;
	DBGMSG("bgHttpServerSessionHandler: bufMemAlloc total=%d, used=%d, blocks=%d\r\n", 
		pSession->sessionMem.lengthBufMemBytes,
		pSession->sessionMem.lengthUsedMemBytes,
		pSession->sessionMem.countUsedBlocks);
	for(i=0; i<pSession->sessionMem.countUsedBlocks; i++){
		DBGMSG("bgHttpServerSessionHandler: bufMemAlloc i=%d, p=%x, l=%d\r\n", 
		i,
		pSession->sessionMem.memBlock[i].pMemBlock, 
		pSession->sessionMem.memBlock[i].lengthMemBlockBytes);
	}
	}
#endif
}

/*
####################################################################################################
####################################################################################################
####################################################################################################
*/
void bgHttpClientSetConnection(bgTcpSocketClient_typ* pClient, char* serverIpAddr, int serverPortNumber)
{
	if(serverIpAddr)
		strcpy(pClient->serverIpAddr, serverIpAddr);

	if(serverPortNumber)
		pClient->serverPort = serverPortNumber;

	DBGMSG("bgHttpClientSetConnection: ip=[%s], port=%d\r\n", serverIpAddr?serverIpAddr:"none", serverPortNumber);

}

void bgHttpClientSendRequest(bgTcpSocketClient_typ* pClient, char* pCmd, char* pContent , int contentSize, char* contentType)
{
bgTcpSocketSession_typ *pSession = &pClient->session[0];

	if( pSession->socketState == BGTCPSOCK_SESSION_STATE_NONE){

        /*init client app specific memory*/
		bgHttpClientSessionAppMemAlloc(pSession);

        /*init/reset send buffer*/
		bgHttpSendBufReset(pSession);

        /*define HTTP header entries*/
        bgHttpClientSendRequestSetHeader(pSession, pCmd, contentSize, contentType);
		
		/*append content, if any*/
		if(pContent && contentSize)
			strcat(pSession->pSendBuf, pContent);

		pSession->tcpSend.datalen = strlen(pSession->pSendBuf);
		bgHttpSendData(pSession);
		DBGMSG("bgHttpClientSendRequest: cmd=[%s], totalLen=%d, contentSize=%d\r\n", pCmd?pCmd:"none", pSession->tcpSend.datalen,contentSize);
	}else{
		DBGMSG("bgHttpClientSendRequest(): rejected! Client is busy!\r\n");
	}
}

void bgHttpClientSessionHandler(bgTcpSocketSession_typ* pSession)
{
	if(pSession->init){
		DBGMSG("bgHttpClientSessionHandler(): init\r\n");
#if 0
NOTE: client memalloc takes place during send request
		bgHttpClientSessionAppMemAlloc(pSession);
#endif
		/*init receive byte count*/
		pSession->rxByteCount = 0;
		return;
	}

	if( !pSession->initOk ){
		DBGMSG("bgHttpClientSessionHandler(): can't execute session; init NOT OK\r\n");
		return;
	}

	
	DBGMSG("bgHttpClientSessionHandler(): execute\r\n");

	if( bgHttpReceiver(pSession) )
		return;		/*http receive is NOT complete; continue with receiving*/

	DBGMSG("bgHttpClientSessionHandler: recv complete!\r\n");


	if( pSession->dbgDumpRecvBuf )
		bgHttpRecvBufDebug(pSession);

		
	if( bgHttpClientProcessResponse(pSession) ){
		/*terminate session*/
		DBGMSG("bgHttpClientSessionHandler: http response error\r\n");
		pSession->bCloseSession = 1;
        return;		/*error in http response; terminate session*/
	}


    /*http receive success; execute application/protocol layer*/
    {
        bgTcpSocketClient_typ* pClient = (bgTcpSocketClient_typ*)pSession->pTcpLayer;
        if( pClient->pApplicationHandler )
            (*pClient->pApplicationHandler)(pSession);
        else
			DBGMSG("bgHttpClientSessionHandler: app handler not defined!\r\n");
    }

	/*terminate session*/
	pSession->bCloseSession = 1;
}


static void bgHttpClientSessionAppMemAlloc(bgTcpSocketSession_typ* pSession)
{
/*
 	HTTP session:
 	On init, "alocate" memory within the session application space.

	Session/Application memory allocation:

	-each session/application is given  memory space
	-size of the app space (APPLICATION BUFFER SIZE) is defined in the server init call:
		i.e. bgTcpSocketServerInit(&bgXmlRpcServerData, ... 2048, ...);		

	- depending on the application/session needs, this space can be further devided into
	"allocated" memory blocks. See bgBufMemAllocXXX object

	pAppBuffer allocation scheme for bgHttpClientSessionHandler:
		struct httpMessage_typ		--- sizeof(httpMessage_typ) bytes
		listHeaderLines Ptr-s		--- 32 * sizeof(char*) = 128 bytes (holds max 32 items)
		list1 						--- 32 * sizeof(char*) = 128 bytes (holds max 32 items)
		remainig space: listContentLines ptr-s (i.e. 1kBytes of space is 256 items (lines) in the list 

*/		
httpMessage_typ* pHttpMsg;
char* pMem;
int sizeMem;

    /*simple, session specific memory allocation*/
	pSession->initOk = 0;
	
	bgBufMemAllocInit(&pSession->sessionMem, pSession->pAppBuf, pSession->appBufLength);


	/*1) protocol/application specific data structure, upper layer*/
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, sizeof(httpMessage_typ) + sizeof(char*), &pMem, &sizeMem) )
	{
    	DBGMSG("bgHttpClientSessionAppMemAlloc(): FATAL ERROR (1); No memory\r\n");
		return;
	}
	pHttpMsg = (httpMessage_typ*)pMem;
	pSession->pProtocol = (char*)pHttpMsg; 

	/*2) set storage for list of http header lines (anticipated short list of items)*/		
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, 33 * sizeof(char *), &pMem, &sizeMem) )
	{
    	DBGMSG("bgHttpClientSessionAppMemAlloc(): FATAL ERROR (2); No memory\r\n");
		return;
	}
	bgStringListExMemSet(&pHttpMsg->listHeaderLines, pMem, 32 * sizeof(char *));

	/*3) set storage for short helper list (short list of items, i.e. 32)*/
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, 33 * sizeof(char *), &pMem, &sizeMem) )
	{
    	DBGMSG("bgHttpClientSessionAppMemAlloc(): FATAL ERROR (3); No memory\r\n");
		return;
	}
    bgStringListExMemSet(&pHttpMsg->list1, pMem, 32 * sizeof(char *));

	pSession->initOk = 1;

#if 0
	/*dump appliction buffer memory allocation*/
	{
	int i;
	DBGMSG("bgHttpClientSessionHandler: bufMemAlloc total=%d, used=%d, blocks=%d\r\n", 
		pSession->sessionMem.lengthBufMemBytes,
		pSession->sessionMem.lengthUsedMemBytes,
		pSession->sessionMem.countUsedBlocks);
	for(i=0; i<pSession->sessionMem.countUsedBlocks; i++){
		DBGMSG("bgHttpClientSessionHandler: bufMemAlloc i=%d, p=%x, l=%d\r\n", 
		i,
		pSession->sessionMem.memBlock[i].pMemBlock, 
		pSession->sessionMem.memBlock[i].lengthMemBlockBytes);
	}
	}
#endif
}

/*
####################################################################################################
####################################################################################################
####################################################################################################
*/

 int bgHttpReceiver(bgTcpSocketSession_typ* pSession)
{
/*session/application/protocol specific data structure*/
httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;
int contentSizeReceived = 0;
int rxInitFlag = pSession->rxByteCount ? 0 : 1;

	DBGMSG("bgHttpReceiver: rxLen=%d, rxInit=%d\r\n", pSession->tcpRecv.recvlen, rxInitFlag);

	/*keep track of bytes received*/
	pSession->rxByteCount += pSession->tcpRecv.recvlen;

	/*http is a character based protocol! always terminate rxBuffer for later string processing*/
	pSession->pRecvBuf[pSession->rxByteCount] = 0;
		
	/*is this is a first chunk of data recv for this session?*/
	if(rxInitFlag){
		/*check if http content definiton is present in http header; 
		if yes, set following values: 
		r->contentLength 	is the value extracted from the http header attribute "Content-Length"
		r->contentPosition 	is the offset (position) of the content start within the recvBuf 
		*/
		if( !bgHttpReceiverGetContentLengthAndPosition(pSession) ){
			/*header line with "Content-Length" not received; recv complete; next: process request*/
			return 0;
		}

		DBGMSG("bgHttpReceiver: content=YES; contentLength=%d, contentOffset=%d\r\n", 
			r->contentLength, r->contentPosition);

		/*check if the entire content is received*/
		if( r->contentPosition > pSession->rxByteCount){
			/*upps! adjust here*/
			DBGMSG("bgHttpReceiver: WARNING conStart=%d > rxLen=%d. Adjusting...\r\n", 
			r->contentPosition, pSession->rxByteCount);
			r->contentPosition = pSession->rxByteCount;
		}	
	}

	/*monitor if the entire http message is received, including content*/	
	contentSizeReceived = pSession->rxByteCount - r->contentPosition;

	/*if received content is less then 2/3 of needed content > continue receiving*/
	if(contentSizeReceived < ((r->contentLength * 2) / 3 )){
		DBGMSG("bgHttpReceiver: incomplete; totalBytesRcvd=%d contentRcvd=%d, contentNeeded=%d\r\n", 
			pSession->rxByteCount,
			contentSizeReceived, 
			r->contentLength);

		/*continue receiving!*/

		/*adjust parameters of the tcprecv*/
		pSession->tcpRecv.pData = (UDINT)(&pSession->pRecvBuf[pSession->rxByteCount]);	/* address of the buffer */;
		pSession->tcpRecv.datamax = pSession->recvBufLength - pSession->rxByteCount; 	/* length of the buffer */;
		return 1;
	}
	
	DBGMSG("bgHttpReceiver: Complete; totalBytesRcvd=%d contentRcvd=%d, contentNeeded=%d\r\n", 
		pSession->rxByteCount,
		contentSizeReceived, 
		r->contentLength);

	return 0;
}

 int bgHttpReceiverGetContentLengthAndPosition(bgTcpSocketSession_typ* pSession)
{
/*session/application/protocol specific data structure*/
httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;

int position=0;
int positionHeaderContentLenBegin=0;
int positionHeaderContentLenEnd=0;
char textBuffer[255];


	r->contentLength = 0;
	r->contentPosition = 0;
		
	/*find if "Content-Length" keyword is present in the request*/
	positionHeaderContentLenBegin = bgStrFindEx(pSession->pRecvBuf, BGHTTP_HEADER_KEY_CONLEN, -1, -1, 1);

	if( positionHeaderContentLenBegin < 0){
		/*content length not present; receive is complete; next: process http request*/
		DBGMSG("bgHttpReceiver: no content\r\n");		
		return 0;
	}
	
	/*content is present! extract the content size & find where it starts!*/
	positionHeaderContentLenEnd = bgStrFindEOL(pSession->pRecvBuf, positionHeaderContentLenBegin);

	/*sanity check*/
	if( (positionHeaderContentLenEnd < 0) || 
		(positionHeaderContentLenEnd >= pSession->rxByteCount) ||
		((positionHeaderContentLenEnd - positionHeaderContentLenBegin) >= 255) ){
		/*can't extract content length; receive is complete; next: process http request*/
		DBGMSG("bgHttpReceiver: can't find end marker\r\n");		
		return 0;
	}

	/*extract header line "Content-Length"*/
	bgStrCopyFromTo(pSession->pRecvBuf, positionHeaderContentLenBegin, positionHeaderContentLenEnd, textBuffer);
	/*find value*/
	position = bgStrFind(textBuffer, ":", -1, -1);
	if( position < 0 ){
		/*can't extract content length; receive is complete; next: process http request*/
		DBGMSG("bgHttpReceiver: can't find separator marker [%s]\r\n", textBuffer);		
		return 0;
	}
	/*convert received value*/
	bgsscanf(&textBuffer[position+1], "%d", &r->contentLength);
	DBGMSG("bgHttpReceiver: headerLine=[%s], length=%d\r\n", textBuffer, r->contentLength);		

	if( !r->contentLength ){
		/*content length not present; receive is complete; next: process http request*/
		DBGMSG("bgHttpReceiver: contentLen=0, process request\r\n");		
		return 0;
	}

	/*find content start position within the recvBuf; content comes after header; separation is one empty line!
	*/

	/*positionHeaderContentLenEnd - holds the EOL position for header line "Content-Length". Before the 
	seprator (empty) line, there may be more header lines*/
	/*TODO: detect the exact position of the content start!!!*/

	/*Assume that the content start just after the above header line!*/
	r->contentPosition = positionHeaderContentLenEnd + 1;
 	{
		/*search for a first empty line after the content header line -> empty line
		is the separator between the header and the content*/
		int state = 0;
		int idx = r->contentPosition;
		char c;
		while(1){
			c = pSession->pRecvBuf[idx];
			switch(state){
				case 0:
					if( c == '\n' ){
						state = 1;
					}else					
					if( c == 0){
						return 1;
					}
					idx++;
				break;
				case 1:
					if( c == '\n' ){
						/*found! two consecutive \n !*/
						r->contentPosition = idx + 1;
						return 1;
					}else					
					if( c == '\r'){
						/*ignore \r */
					}else
					if( c == 0){
						return 1;
					}else{
						/*continue searching for the empty line*/
						state = 0;
					}
					idx++;
				break;
			}
		}
	}

}

/*
####################################################################################################
####################################################################################################
####################################################################################################
*/

 int bgHttpServerProcessRequest(bgTcpSocketSession_typ* pSession)
{
/*
This function parses and analyzes standard HTTP request (HTTP/0.9, HTTP/1.0, HTTP1.1).
As a result, it fills-in elemetns in the HTTP protocol data structure (httpMessage_typ)

Following is the typical HTTP/1.1 request:

GET /file1.html HTTP/1.1\r\n                    <-- line 0: HTTP command line
Content-Type: text/html\r\n                     <-- line 1 (optional): HTTP Header, 1st Header Line (if present)
Connection: keep-alive\r\n                      <-- line 2 (optional): HTTP Header, 2nd Header Line (if present) (followed by as many header lines as needed) 
\r\n                                            <-- line x (optional): Empty line (Marks separation between HTTPHeader (if present) and HTTP Content (if present)
<head>\r\n                                      <-- line (x+1) (optional): HTTP Content (if present), 1st Content line (if present)
<title>HTTP Content in HTML format</title>\r\n  <-- line (x+2, etc) (optional): Additional HTTP Content lines (if present)
</head>\r\n     
<body>\r\n
<h1>My Header 1</h1>\r\n
<p>Simple text in HTML format.\r\n	
</body>\r\n
\r\n


This function uses/sets following data (using the above sample request):
pSession->pRecvBuf                              <-- tcp rx buffer; holds the entire HTTP request content

(httpMessage_typ)r = pSession->pProtocol        <-- ptr to the httpMessage structure; structure resides within the pSession->pAppBuf memory area
r->listHeaderLines.itemCount = 3;               <-- http msg header has 3 lines
r->listHeaderLines.items[]                      <-- array of ptrs to each of the 3 lines
r->command = "GET";                             <-- command ptr points to "GET"
r->commandId = BGHTTP_REQUEST_COMMAND_GET;
r->url = "/file1.html"                          <-- url ptr points to "/file1.html"
r->versionString = "HTTP/1.1";
r->version = 11;

*/
httpMessage_typ* r;
int i;
int ret = 1;                                    /*mark request as failure*/
    
    /*session/application/protocol specific data structure*/
    r = (httpMessage_typ*)pSession->pProtocol;

	/*check if http content is present*/
	if( r->contentLength ){
		/*content is present; separate header and content by inserting 0 (string termination)
		at the end of the http header*/
		if( r->contentPosition && (r->contentPosition < pSession->rxByteCount) )
			pSession->pRecvBuf[r->contentPosition - 1] = 0;
		else{
			DBGMSG("bgHttpServerProcessRequest: WARNING! content data invalid! conLen=%d, conPos=%d, rxLen=%d\r\n",
				r->contentLength, r->contentPosition, pSession->rxByteCount);
			r->contentLength = 0;
			r->contentPosition = 0;
		}			
	}

	/*parse 1st part of the request; generate list of header lines*/	
	if( bgStringListLinesEx(&r->listHeaderLines, &pSession->pRecvBuf[0], 0, 0) < 0 ){
		DBGMSG("bgHttpServerProcessRequest(): Received header too long! SessionId= %d, len = %d, Error=500\r\n", 
			pSession->ident, pSession->rxByteCount);
		bgHttpSendErrorResponse(pSession, 500);
 		return ret;
    }

	/*empty request? it should not be possible, but...*/
	if( r->listHeaderLines.itemCount == 0 ){
		DBGMSG("bgHttpServerProcessRequest(): Invalid request! SessionId= %d, len = %d, Error=400\r\n", 
			pSession->ident, pSession->rxByteCount);
		bgHttpSendErrorResponse(pSession, 400);
 		return ret;
	}

#if 0
	DBGMSG("bgHttpServerProcessRequest(): Header (lines=%d):\r\n", 
		r->listHeaderLines.itemCount);
	DBGMSG("============================================\r\n");
	for(i=0;i < r->listHeaderLines.itemCount;i++){
		DBGMSG("%s\r\n", r->listHeaderLines.items[i]);
	}
	DBGMSG("============================================\r\n\r\n");
#endif
		
    /*HTTP REQUEST COMMAND:*/
	/*parse first line of the request*/
	bgStringSplitLineEx(&r->list1, ' ', (char*)r->listHeaderLines.items[0], 0, 0);


	/*one or less words in the first line?*/
	if( (r->list1.itemCount <= 1) || (r->list1.itemCount > 3)){
		DBGMSG("bgHttpServerProcessRequest(): Invalid request! SessionId=%d, len =%d, 1stLnWords=%d  Error=400\r\n", 
			pSession->ident, pSession->rxByteCount, r->list1.itemCount);
		bgHttpSendErrorResponse(pSession, 400);
 		return ret;
	}


	/*extract httpVersion & url*/
	r->url = (char*)r->list1.items[1];
	if( r->list1.itemCount == 2){
		DBGMSG("bgHttpServerProcessRequest(): Warning! HTTP request wo. protocol version\r\n");
		r->versionString = "HTTP/0.9";
	}else{
		r->versionString = (char*)r->list1.items[2];
	}


	/*extract http command*/
	if( bgStrFind((char*)r->list1.items[0], "GET", -1, -1) != -1){
		r->command = "GET";
		r->commandId = BGHTTP_REQUEST_COMMAND_GET;
	}else	
	if( bgStrFind((char*)r->list1.items[0], "POST", -1, -1) != -1){
		r->command = "POST";
		r->commandId = BGHTTP_REQUEST_COMMAND_POST;
	}else{
		DBGMSG("bgHttpServerProcessRequest(): Invalid command! SessionId=%d, len =%d, Command=[%s] Error=400\r\n", 
			pSession->ident, pSession->rxByteCount, r->list1.items[0]);
		bgHttpSendErrorResponse(pSession, 400);
 		return ret;
	}			
	
	DBGMSG("bgHttpServerProcessRequest(): request cmd=[%s] url=[%s] ver=[%s]\r\n", 
		r->command, r->url, r->versionString);


	/*mark request as OK!*/
 	ret = 0;

	return ret;
}



static int bgHttpClientProcessResponse(bgTcpSocketSession_typ* pSession)
{
/*
This function parses and analyzes standard HTTP response (HTTP/0.9, HTTP/1.0, HTTP1.1).
As a result, it fills-in elemetns in the HTTP protocol data structure (httpMessage_typ)

Following is the typical HTTP/1.0 response:

HTTP/1.0 200 OK\r\n                    			<-- line 0: HTTP response line
Server: SimpleHTTP/0.6 Python/2.5c2				<-- line 1 (optional): HTTP Header, 1st Header Line (if present)
Date: Mon, 23 Jul 2007 02:26:25 GMT				<-- line 2 (optional): HTTP Header, 2nd Header Line (if present)
Content-Type: text/html\r\n                     <-- line 3 (optional): HTTP Header, 3rd Header Line (if present)
\r\n                                            <-- line x (optional): Empty line (Marks separation between HTTPHeader (if present) and HTTP Content (if present)
<head>\r\n                                      <-- line (x+1) (optional): HTTP Content (if present), 1st Content line (if present)
<title>HTTP Content in HTML format</title>\r\n  <-- line (x+2, etc) (optional): Additional HTTP Content lines (if present)
</head>\r\n     
<body>\r\n
<h1>My Header 1</h1>\r\n
<p>Simple text in HTML format.\r\n	
</body>\r\n
\r\n


This function uses/sets following data (using the above sample request):
pSession->pRecvBuf                              <-- tcp rx buffer; holds the entire HTTP request content

(httpMessage_typ)r = pSession->pProtocol        <-- ptr to the httpMessage structure; structure resides within the pSession->pAppBuf memory area
r->listHeaderLines.itemCount = 3;               <-- response header has 3 lines
r->listHeaderLines.items[]                      <-- array of ptrs to each of the 3 lines
r->command = "OK";                             	<-- command ptr points to "OK" (long response result)
r->commandId = 200;								<-- commandId contains error value, 200
r->versionString = "HTTP/1.0";
r->version = 10;

*/
httpMessage_typ* r;
int i;
int ret = 1;                                    /*mark request as failure*/
    
    /*session/application/protocol specific data structure*/
    r = (httpMessage_typ*)pSession->pProtocol;

	/*check if http content is present*/
	if( r->contentLength ){
		/*content is present; separate header and content by inserting 0 (string termination)
		at the end of the http header*/
		if( r->contentPosition && (r->contentPosition < pSession->rxByteCount) )
			pSession->pRecvBuf[r->contentPosition - 1] = 0;
		else{
			DBGMSG("bgHttpServerProcessRequest: WARNING! content data invalid! conLen=%d, conPos=%d, rxLen=%d\r\n",
				r->contentLength, r->contentPosition, pSession->rxByteCount);
			r->contentLength = 0;
			r->contentPosition = 0;
		}			
	}

	/*parse 1st part of the response; generate list of header lines*/	
	if( bgStringListLinesEx(&r->listHeaderLines, &pSession->pRecvBuf[0], 0, 0) < 0 ){
		DBGMSG("bgHttpClientProcessResponse: Received header too long! SessionId= %d, len = %d, Error=500\r\n", 
			pSession->ident, pSession->rxByteCount);
 		return ret;
    }

	/*empty request? it should not be possible, but...*/
	if( r->listHeaderLines.itemCount == 0 ){
		DBGMSG("bgHttpClientProcessResponse(): Invalid response! SessionId= %d, len = %d, Error=400\r\n", 
			pSession->ident, pSession->rxByteCount);
 		return ret;
	}

#if 0
	DBGMSG("bgHttpClientProcessResponse(): Header (lines=%d):\r\n", 
		r->listHeaderLines.itemCount);
	DBGMSG("============================================\r\n");
	for(i=0;i < r->listHeaderLines.itemCount;i++){
		DBGMSG("%s\r\n", r->listHeaderLines.items[i]);
	}
	DBGMSG("============================================\r\n\r\n");
#endif
		
    /*HTTP RESPONSE:*/
	/*parse first line of the response*/
	bgStringSplitLineEx(&r->list1, ' ', (char*)r->listHeaderLines.items[0], 0, 0);


	/*one or less words in the first line?*/
	if( (r->list1.itemCount <= 1) || (r->list1.itemCount > 3)){
		DBGMSG("bgHttpClientProcessResponse(): Invalid response! SessionId=%d, len =%d, 1stLnWords=%d  Error=400\r\n", 
			pSession->ident, pSession->rxByteCount, r->list1.itemCount);
 		return ret;
	}


	/*extract data from the 1st line in response*/
	r->versionString = (char*)r->list1.items[0];
	bgsscanf((char*)r->list1.items[1], "%d", &r->commandId);
	r->command = (char*)r->list1.items[2];
	

	
	DBGMSG("bgHttpClientProcessResponse(): response ver=[%s] err=[%d] msg=[%s]\r\n", 
		r->versionString, r->commandId, r->command);


	/*mark response as OK!*/
 	ret = 0;

 	return ret;
}


/*
####################################################################################################
####################################################################################################
####################################################################################################
*/
static void bgHttpSendBufReset(bgTcpSocketSession_typ* pSession)
{
/*session/application/protocol specific data structure*/
httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;
char* pDst = pSession->pSendBuf;

	/*clr send buffer*/
	*pDst = 0;

	/*reset current ptr into the send buffer; prepare for incremental write*/
	pSession->curIdxSendBuf = 0;


	/*reset length of sections within the send buffer*/
	r->sendHeaderLength = 0;
	r->sendContentLength = 0;
	r->sendTotalLength = 0;
}

static void bgHttpSendBufApndHeaderEnd(bgTcpSocketSession_typ* pSession)
{
/*session/application/protocol specific data structure*/
httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;
    
	addLine(pSession,"");
	r->sendTotalLength = strlen(pSession->pSendBuf);
	r->sendHeaderLength = r->sendTotalLength;
}

static void bgHttpSendBufApndHeaderLine(bgTcpSocketSession_typ* pSession, char* headerLine)
{
	addLine(pSession, headerLine);
}

static void bgHttpSendBufApndHeaderLineAndFormat(bgTcpSocketSession_typ* pSession, char* headerKeyword, char* value)
{
char txt[255];
    bgsprintf(txt, "%s: %s", headerKeyword, value);
	addLine(pSession, txt);
}

static void bgHttpSendBufApndContentLine(bgTcpSocketSession_typ* pSession, char* contentLine)
{	
	addLine(pSession, contentLine);
}


static int bgHttpSendBufMonitorOverflow(bgTcpSocketSession_typ* pSession, int newLineSize)
{
 /*as content may be very long, monitor for send buffer overflow!*/
httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;

int sendBufTotalSize = pSession->sendBufLength;
int sendBuffUsedSize = r->sendTotalLength;
	
	if(sendBufTotalSize <= (sendBuffUsedSize + newLineSize + 10)){
		/*trigger error in  bgXXXApplication() (i.e. bgWebServerApplication() )*/
		r->sendTotalLength = pSession->sendBufLength;
		return 1;
	}

	return 0;	
}

static void bgHttpSendBufApndContentEnd(bgTcpSocketSession_typ* pSession)
{
httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;

	addLine(pSession, "");
	
	r->sendContentLength = strlen(pSession->pSendBuf) - r->sendHeaderLength;	

	bgHttpSendBufFillContentSize(pSession);
}


static void bgHttpSendBufFillContentSize(bgTcpSocketSession_typ* pSession)
{
/*session/application/protocol specific data structure*/
httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;
char* pDst = pSession->pSendBuf;
int headerEnd = r->sendHeaderLength;
int lineStart = 0;
int lineEnd = 0;
int separatorStart = 0;
int i;
char txtBuf[32];
/*
This function searches trough the header section of the reply (send) buffer.
When it finds keyword "Content-Length" it sets the value part correctly
(i.e. "Content-Length: 342")
*/

	/*search for keyword within the reply header*/
	lineStart = bgStrFind(pDst, BGHTTP_HEADER_KEY_CONLEN, 
		0,
		headerEnd);

	if( lineStart < 0){
		DBGCRT("bgHttpSendBufFillContentSize(): can't find keyword %s\r\n", BGHTTP_HEADER_KEY_CONLEN);
		return;
	}			

	/*search for key / value separator*/
	separatorStart = bgStrFind(pDst, ":", 
		lineStart,
		headerEnd);

	if( separatorStart < 0){
		DBGCRT("bgHttpSendBufFillContentSize(): can't find separator\r\n");
		return;
	}			

	/*search for end of the line with the above keyword*/
	lineEnd = bgStrFind(pDst, BGHTTP_CONST_STRING_CRLF, 
		separatorStart,
		headerEnd);

	if( lineEnd < 0){
		DBGCRT("bgHttpSendBufFillContentSize(): can't find EOL\r\n");
		return;
	}			

	/*DBGMSG("headerStart=%d, headerEnd=%d, lineStart=%d, sepStart=%d, lineEnd=%d\r\n", headerStart, headerEnd, lineStart, separatorStart, lineEnd);*/

	/*erase existing value by filling spaces*/
	for(i=(separatorStart + 1); i<lineEnd; i++){
		pDst[i] = ' ';
	}

	/*write new value*/
	bgsprintf(&txtBuf[0], "%d", r->sendContentLength);    
	memcpy(&pDst[separatorStart + 2], &txtBuf[0], strlen(&txtBuf[0]));    

}

void bgHttpSendErrorResponse(bgTcpSocketSession_typ* pSession, int responseCode)
{
/*
Example of error response:

HTTP/1.0 404 Resource Not Found
Server: bgHTTPServer/0.50
Connection: close
Content-Type: text/html
Content-Length: 123 

<head>
<title>Error response</title>
</head>
<body>
<h1>Error response</h1>
<p>Error code 404
<p>Message: Resource Not Found.
</body>
*/
char textBuffer[255]; 

	bgHttpServerSendResponseBeg(pSession, responseCode, "text/html");


	addLine(pSession,"<head>" );
	addLine(pSession,"<title>Error response</title>");
	addLine(pSession,"</head>");
	addLine(pSession,"<body>");
	addLine(pSession,"<h1>Error response</h1>");

	bgsprintf(textBuffer, "<p>Error code %d.", responseCode);	
	addLine(pSession,textBuffer);

	bgsprintf(textBuffer, "<p>Message: %s.", bgHttpGetResponseMsg(responseCode) );	
	addLine(pSession,textBuffer);

	addLine(pSession,"</body>");

	bgHttpServerSendResponseEnd(pSession);
}

 char* bgHttpGetResponseMsg(int responseCode)
{

	switch(responseCode){
		case 200: return "OK"; break;
		case 400: return "Bad Request"; break;
		case 401: return "Unauthorized"; break;
		case 403: return "Forbidden"; break;
		case 404: return "Resource Not Found"; break;
		case 405: return "Method Not Allowed"; break;
		case 413: return "Requested Entity Too Large"; break;
		case 500: return "Internal Server Error"; break;
		case 501: return "Not Implemented"; break;
		case 505: return "HTTP Version Not Supported"; break;
		default: return "unknown error"; break;
	}
}

void bgHttpClientSendRequestSetHeader(bgTcpSocketSession_typ* pSession, char* pCmd, int contentSize, char* contentType)
{
char txtBuf[255];

    bgsprintf(txtBuf, "%s %s", pCmd, BGHTTP_CONST_STRING_VERSION);
	bgHttpSendBufApndHeaderLine(pSession, txtBuf);

	bgHttpSendBufApndHeaderLine(pSession, "Connection: close");

    if(contentType){
		bgsprintf(txtBuf, "Content-Type: %s", contentType);
		bgHttpSendBufApndHeaderLine(pSession, txtBuf);
}
    else
		bgHttpSendBufApndHeaderLine(pSession, "Content-Type: text/html");

	bgsprintf(txtBuf, "%s: %d", BGHTTP_HEADER_KEY_CONLEN, contentSize);
	bgHttpSendBufApndHeaderLine(pSession, txtBuf);

    /*append empty line - separates header from content & calculate header size*/
	bgHttpSendBufApndHeaderEnd(pSession);
}


static void bgHttpServerSendResponseSetHeader(bgTcpSocketSession_typ* pSession, int responseCode, int contentSize, char* contentType)
{
char txtBuf[255];

    bgsprintf(txtBuf, "%s %d %s", BGHTTP_CONST_STRING_VERSION, responseCode, bgHttpGetResponseMsg(responseCode));
	bgHttpSendBufApndHeaderLine(pSession, txtBuf);

	bgHttpSendBufApndHeaderLine(pSession, "Connection: close");

    if(contentType){
		bgsprintf(txtBuf, "Content-Type: %s", contentType);
		bgHttpSendBufApndHeaderLine(pSession, txtBuf);
}
    else
		bgHttpSendBufApndHeaderLine(pSession, "Content-Type: text/html");

	bgsprintf(txtBuf, "%s: %d", BGHTTP_HEADER_KEY_CONLEN, contentSize);
	bgHttpSendBufApndHeaderLine(pSession, txtBuf);

    /*append empty line - separates header from content & calculate header size*/
    bgHttpSendBufApndHeaderEnd(pSession);
}


void bgHttpSendData(bgTcpSocketSession_typ* pSession)
{
httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;
int len = strlen(pSession->pSendBuf);

	if( len !=  r->sendTotalLength){
		DBGMSG("bgTcpSocketServerSessionSendData: Warning! l=%d, t=%d\r\n", len, r->sendTotalLength);
	}

	pSession->txByteCountTotal = len;
	pSession->txByteCount = 0;

	pSession->pSendBufHandler = bgHttpSendBufCallback;
	
	/*set flag for session layer to send data*/
	pSession->bSendData = 1;

	if(pSession->dbgDumpSendBuf)
		bgHttpSendBufDebug(pSession);
} 

static void bgHttpSendBufCallback(bgTcpSocketSession_typ* pSession)
{
	/*incomplete data send*/
	pSession->txByteCount += pSession->tcpSend.sentlen;

	/*adjust send ptr & len*/
	pSession->tcpSend.pData = (UDINT)(&pSession->pSendBuf[pSession->txByteCount]);

	if(pSession->txByteCountTotal > pSession->txByteCount)
		pSession->tcpSend.datalen = pSession->txByteCountTotal - pSession->txByteCount;

}


/*
Dump send buffer.
*/
static void bgHttpSendBufDebug(bgTcpSocketSession_typ* pSession)
{
int i;
int j;
int crFound;
char text[90];
int len = pSession->txByteCountTotal;

	DBGMSG("bgHttpSendBuf dump:(%d bytes)\r\n", len);
	DBGMSG("=================================================================\r\n");
	crFound = 0;
	for(i=0, j=0; i<len; i++){		
		text[j] = pSession->pSendBuf[i];

		if( text[j] == '\r' ){
			crFound = 1;
		}

		if( text[j] == '\n'){
			if(!crFound || (crFound > 2) ){
				j++;
				text[j] = '\r';
			}
			crFound = 0;
		}

		if( (j >= 80) || (i >= (len - 1) ) ){
			text[j+1] = 0;
			j = 0;
			DBGMSG("%s", &text[0]);
		}else{
			j++;
		}		

		if( crFound )
			crFound++;
		
	}
	DBGMSG("\r\n=================================================================\r\n");
}	

/*
Dump recv buffer.
*/
static void bgHttpRecvBufDebug(bgTcpSocketSession_typ* pSession)
{
int i;
int j;
int crFound;
char text[90];
int len = pSession->rxByteCount;

	DBGMSG("bgHttpRecvBuf dump: (%d bytes)\r\n", len);
	DBGMSG("=================================================================\r\n");
	crFound = 0;
	for(i=0, j=0;i<len; i++){		
		text[j] = pSession->pRecvBuf[i];

		if( text[j] == '\r' ){
			crFound = 1;
		}

		if( text[j] == '\n'){
			if(!crFound || (crFound > 2) ){
				j++;
				text[j] = '\r';
			}
			crFound = 0;
		}

		if( (j >= 80) || (i >= (len - 1) ) ){
			text[j+1] = 0;
			j = 0;
			DBGMSG("%s", &text[0]);
		}else{
			j++;
		}		

		if( crFound )
			crFound++;
		
	}
	DBGMSG("\r\n=================================================================\r\n");
}	
	
char* bgHttpGetContentStartAddr(bgTcpSocketSession_typ* pSession)
{
	/*session/application/protocol specific data structure*/
	httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;

	return &pSession->pRecvBuf[r->contentPosition];
}
int bgHttpGetContentLength(bgTcpSocketSession_typ* pSession)
{
	/*session/application/protocol specific data structure*/
	httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;

	return r->contentLength;
}
