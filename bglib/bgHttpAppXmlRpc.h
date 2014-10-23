#ifndef _bgHttpAppXmlRpc_h_
#define _bgHttpAppXmlRpc_h_

#include "bgTcpSocketClientServer.h"
#include "bgXmlRpcParser.h"

typedef struct httpAppXmlRpc_typ{
/*Note: following object(s) require(s) external ("allocated") memory!*/

bgXmlRpcParser_typ xmlRpcData;


}httpAppXmlRpc_typ;

void bgXmlRpcServerApplication(bgTcpSocketSession_typ* pSession);
int bgXmlRpcServerAppCommon(bgTcpSocketSession_typ* pSession);
int bgXmlRpcServerSendFaultResponse(bgTcpSocketSession_typ* pSession, int faultCode, char* faultString);
int bgXmlRpcServerSendResponse(bgTcpSocketSession_typ* pSession, char* pContent, int contentSize);

void bgXmlRpcClientSetConnection(bgTcpSocketClient_typ* pClient, char* serverIpAddr, int serverPortNumber);
void bgXmlRpcClientSendRequest(bgTcpSocketClient_typ* pClient, char* pContent , int contentSize);
void bgXmlRpcClientApplication(bgTcpSocketSession_typ* pSession);


#endif
