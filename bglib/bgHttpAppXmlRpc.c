/**\file bgHttpAppXmlRpc.c
*/

#ifndef WIN32
/*
B&R components:
===============================================================================
*/
#include <bur/plc.h>
#include <bur/plctypes.h>
#include <astcp.h>
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
#include "bgHttpAppXmlRpc.h"
#include "bgDataStructures.h"
#include "bgBufMemAlloc.h"
#include "bgXmlRpcParser.h"

static void apndXmlBlockOpen(bgTcpSocketSession_typ* pSession, char* keywName);
static void apndXmlBlockClos(bgTcpSocketSession_typ* pSession, char* keywName);



/*
XmlRpc Server
######################################################################################################
######################################################################################################
######################################################################################################
######################################################################################################
*/
void bgXmlRpcServerApplication(bgTcpSocketSession_typ* pSession)
{
char* pMem;
int sizeMem;
char* pMemTree;
int sizeMemTree;
char* pMemStr;
int sizeMemStr;

httpAppXmlRpc_typ* pXmlRpcAppData;
bgXmlRpcParser_typ* pXmlRpc;
bgTree4C_typ* pTree;
bgTree4CNode_typ* pNode;
bgStringHeap_typ* pStrHeap;

int i;

	
	if( bgXmlRpcServerAppCommon(pSession) ){
		/*common XMLRPC check(s) have failed > exit (& send automatic error response)*/
		return;
	}

/*App Specific Buf MemAlloc*/
	/*1) application specific data structure*/
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, sizeof(httpAppXmlRpc_typ) + sizeof(char*), &pMem, &sizeMem) )
	{
    	DBGMSG("bgXmlRpcServerApplication(): FATAL ERROR (1); No memory\r\n");
		return;
	}
	
	pXmlRpcAppData = (httpAppXmlRpc_typ*)pMem;
	pXmlRpc = &pXmlRpcAppData->xmlRpcData;
	pTree = &pXmlRpc->xmlTree;
	pStrHeap = &pXmlRpc->strHeap;
	
	/*2) set storage for xmlrpc node tree*/
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, 1500, &pMemTree, &sizeMemTree) )
	{
    	DBGMSG("bgXmlRpcServerApplication(): FATAL ERROR (2); No memory\r\n");
		return;
	}

	/*3) set storage for xmlrpc string heap (rest of the available mem)*/
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, 0, &pMemStr, &sizeMemStr) )
	{
    	DBGMSG("bgXmlRpcServerApplication(): FATAL ERROR (3); No memory\r\n");
		return;
	}

	DBGMSG("bgXmlRpcServerApplication: memTotal=%d, memData=%d, memTree=%d, memStr=%d\n", 
		sizeMem + sizeMemStr + sizeMemTree, sizeMem, sizeMemTree, sizeMemStr);


    bgXmlRpcParserInit(pXmlRpc, bgHttpGetContentStartAddr(pSession), pMemTree, sizeMemTree, pMemStr, sizeMemStr);

    bgXmlRpcParserParse(pXmlRpc);


	if(pXmlRpc->error){
		DBGMSG("xml parsing error=%d; keyword=[%s], line=%d, col=%d)\n",
			pXmlRpc->error,
			bgXmlRpcGetKeywordStr(pXmlRpc->keywId),
			pXmlRpc->keywLineNo,
			pXmlRpc->keywColNo);
/*		
			bgXmlRpcServerSendFaultResponse(pSession, 100, "This is fault!");
			rerurn;
*/			
	}

    DBGMSG("xmlRpc tree:\n");
    DBGMSG("---------------------------------------------------------------------\n");

    bgTree4CPrint(pTree);

    DBGMSG("---------------------------------------------------------------------\n");


    DBGMSG("xmlRpc list of values:\n");
    DBGMSG("---------------------------------------------------------------------\n");

	for(i=0; i < pTree->nodeCountUsed; i++){
		
		pNode = &pTree->nodes[i];

		if(!pNode || !pNode->used)
			continue;


		switch(pNode->dataType){
			case XMLRPC_ID_METHOD_NAME:
				DBGMSG("MethodName:[%s]\n", pNode->data ? pNode->data : "unknown");
			break;        
			case XMLRPC_ID_NAME:
				DBGMSG("\nParamName:[%s]\n", pNode->data ? pNode->data : "unknown");
			break;
			case XMLRPC_ID_I4:
			case XMLRPC_ID_INT:
			case XMLRPC_ID_BOOL:
				DBGMSG("Value:[%d]\n", pNode->dataValue );
		
		
			break;
			case XMLRPC_ID_STRING:
			case XMLRPC_ID_TIME:
				DBGMSG("Value:[%s]\n", pNode->data ? pNode->data : "unknown" );		
			break;
			case XMLRPC_ID_DOUBLE:
				DBGMSG("Value:[%f]\n", (float)pNode->dataValue);		
			break;
			default:
			break;
		}

	}
    DBGMSG("---------------------------------------------------------------------\n");



	DBGMSG("StringHeap space: total=%d, free=%d\n", pStrHeap->memSize, pStrHeap->memFree );

	bgXmlRpcServerSendFaultResponse(pSession, 100, "This is fault!");
}


int bgXmlRpcServerAppCommon(bgTcpSocketSession_typ* pSession)
{
    /*session/application/protocol specific data structure*/
    httpMessage_typ* r = (httpMessage_typ*)pSession->pProtocol;

	if( r->commandId != BGHTTP_REQUEST_COMMAND_POST){
		DBGMSG("bgXmlRpcServerApplication(): command (%s) not supported!\r\n", r->command);			
		bgHttpSendErrorResponse(pSession, 405);
 		return 1;
	} 

	return 0;	
}	

int bgXmlRpcServerSendFaultResponse(bgTcpSocketSession_typ* p, int faultCode, char* faultString)
{
/*
XML-RPC Fault Response Example:

HTTP/1.1 200 OK
Connection: close
Content-Length: 426
Content-Type: text/xml
Date: Fri, 17 Jul 1998 19:55:02 GMT
Server: UserLand Frontier/5.1.2-WinNT

<?xml version="1.0"?> 
<methodResponse> 
	<fault>
		<value>
			<struct>
				<member> 
					<name>faultCode</name> 
					<value><int>4</int></value> 
				</member> 
				<member> 
					<name>faultString</name> 
					<value><string>Too many parameters.</string></value> 
				</member> 
			</struct> 
		</value> 
	</fault>
</methodResponse> 
*/
	bgHttpServerSendResponseBeg(p, 200, "text/xml");

	apndXmlBlockOpen(p, XMLRPC_STR_XMLHEADLINE );

	apndXmlBlockOpen(p, XMLRPC_STR_METHOD_RSP);
		apndXmlBlockOpen(p, XMLRPC_STR_FAULT);
			apndXmlBlockOpen(p, XMLRPC_STR_VALUE);
				apndXmlBlockOpen(p, XMLRPC_STR_STRUCT);
					apndXmlBlockOpen(p, XMLRPC_STR_MEMBER);
					{
						char txtBuffer[255];
						bgsprintf(&txtBuffer[0], "<%s>%s</%s>", XMLRPC_STR_NAME, "faultCode", XMLRPC_STR_NAME);
						bgHttpSendBufApndContentLine(p, &txtBuffer[0]);


						bgsprintf(&txtBuffer[0], "<%s><%s>%d</%s></%s>", XMLRPC_STR_VALUE, XMLRPC_STR_INT, faultCode, XMLRPC_STR_INT, XMLRPC_STR_VALUE);
						bgHttpSendBufApndContentLine(p, &txtBuffer[0]);

					}
					apndXmlBlockClos(p, XMLRPC_STR_MEMBER);
					apndXmlBlockOpen(p, XMLRPC_STR_MEMBER);
					{
						char txtBuffer[255];
						bgsprintf(&txtBuffer[0], "<%s>%s</%s>", XMLRPC_STR_NAME, "faultString", XMLRPC_STR_NAME);
						bgHttpSendBufApndContentLine(p, &txtBuffer[0]);


						bgsprintf(&txtBuffer[0], "<%s><%s>%s</%s></%s>", XMLRPC_STR_VALUE, XMLRPC_STR_STRING, faultString, XMLRPC_STR_STRING, XMLRPC_STR_VALUE);
						bgHttpSendBufApndContentLine(p, &txtBuffer[0]);

					}
					apndXmlBlockClos(p, XMLRPC_STR_MEMBER);
				apndXmlBlockClos(p, XMLRPC_STR_STRUCT);
			apndXmlBlockClos(p, XMLRPC_STR_VALUE);
		apndXmlBlockClos(p, XMLRPC_STR_FAULT);
	apndXmlBlockClos(p, XMLRPC_STR_METHOD_RSP );

	bgHttpServerSendResponseEnd(p);
	return 0;
}


int bgXmlRpcServerSendResponse(bgTcpSocketSession_typ* pSession, char* pContent, int contentSize)
{
    bgHttpServerSendResponse(pSession, 200, pContent, contentSize, "text/xml");
	return 0;
}


/*
XmlRpc Client
######################################################################################################
######################################################################################################
######################################################################################################
######################################################################################################
*/
void bgXmlRpcClientSetConnection(bgTcpSocketClient_typ* pClient, char* serverIpAddr, int serverPortNumber)
{
	bgHttpClientSetConnection(pClient, serverIpAddr, serverPortNumber);
}

void bgXmlRpcClientSendRequest(bgTcpSocketClient_typ* pClient, char* pContent , int contentSize)
{
	bgHttpClientSendRequest(pClient, "POST /RPC2 ", pContent , contentSize, "text/xml");
}


void bgXmlRpcClientApplication(bgTcpSocketSession_typ* pSession)
{
int i;
char* pMem;
int sizeMem;
httpAppWeb_typ* pWebData;
int sizeMemAppTotal = 0;

/*App Specific Buf MemAlloc*/
	/*1) application specific data structure*/
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, sizeof(httpAppWeb_typ) + sizeof(char*), &pMem, &sizeMem) )
	{
    	DBGMSG("bgXmlRpcClientApplication(): FATAL ERROR (1); No memory\r\n");
		return;
	}
	pWebData = (httpAppWeb_typ*)pMem;
	sizeMemAppTotal += sizeMem;

	/*2) set storage for line list (long list of items, i.e. several hundred items, depending on the memory)*/
	if( bgBufMemAllocCreateBlock(&pSession->sessionMem, 0, &pMem, &sizeMem) )
	{
    	DBGMSG("bgXmlRpcClientApplication(): FATAL ERROR (4); No memory\r\n");
		return;
	}
    bgStringListExMemSet(&pWebData->listContentLines, pMem, sizeMem);
	sizeMemAppTotal += sizeMem;
	DBGMSG("bgXmlRpcClientApplication: memTotal=%d, memList=%d\n", sizeMemAppTotal, sizeMem);



	/*if http response does not contain content, request processing is ready*/
	if( !bgHttpGetContentLength(pSession)){
		DBGMSG("bgXmlRpcClientApplication: received response with no content!\n");
		return;
	}

	/*parse content part of the request; generate list of content lines*/	
	if( bgStringListLinesEx(&pWebData->listContentLines, bgHttpGetContentStartAddr(pSession), 0, 0) < 0 ){
		DBGMSG("bgXmlRpcClientApplication(): Received content too long! SessionId= %d, len = %d, Error=500\r\n", 
			pSession->ident, pSession->rxByteCount);
 		return;
    }

	DBGMSG("bgXmlRpcClientApplication(): Content list (lines=%d):\r\n", 
		pWebData->listContentLines.itemCount);
	DBGMSG("============================================\r\n");
	for(i=0;i < pWebData->listContentLines.itemCount;i++){
		DBGMSG("%s\r\n", pWebData->listContentLines.items[i]);
	}
	DBGMSG("============================================\r\n");

 	return;
}
/*
############################################################################################################
*/



static void apndXmlBlockOpen(bgTcpSocketSession_typ* pSession, char* keywName)
{
char txt[255];
	bgsprintf(txt, "<%s>", keywName);
	bgHttpSendBufApndContentLine(pSession, txt);
}

static void apndXmlBlockClos(bgTcpSocketSession_typ* pSession, char* keywName)
{
char txt[255];
	bgsprintf(txt, "</%s>", keywName);
	bgHttpSendBufApndContentLine(pSession, txt);
}

