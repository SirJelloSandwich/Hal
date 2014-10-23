#ifndef _bgHttpProtocol_h_
#define _bgHttpProtocol_h_

#include "bgTcpSocketClientServer.h"

#define APNDCL(line)	bgHttpSendBufApndContentLine(pSession, line)

void bgHttpServerSessionHandler(bgTcpSocketSession_typ* pSession);
void bgHttpClientSessionHandler(bgTcpSocketSession_typ* pSession);


void bgHttpSendErrorResponse(bgTcpSocketSession_typ* p, int responseCode);
void bgHttpServerSendResponse(bgTcpSocketSession_typ* pSession, int responseCode, char* pContent , int contentSize, char* contentType);
void bgHttpServerSendResponseBeg(bgTcpSocketSession_typ* pSession, int responseCode, char* contentType);
void bgHttpServerSendResponseEnd(bgTcpSocketSession_typ* pSession);


void bgHttpSendData(bgTcpSocketSession_typ* pSession);

char* bgHttpGetContentStartAddr(bgTcpSocketSession_typ* p);
int bgHttpGetContentLength(bgTcpSocketSession_typ* p);


void bgHttpClientSetConnection(bgTcpSocketClient_typ* pClient, char* serverIpAddr, int serverPortNumber);
void bgHttpClientSendRequest(bgTcpSocketClient_typ* pClient, char* pCmd, char* pContent , int contentSize, char* contentType);
void bgHttpClientSendRequestSetHeader(bgTcpSocketSession_typ* p, char* pCmd, int contentSize, char* contentType);


#endif
