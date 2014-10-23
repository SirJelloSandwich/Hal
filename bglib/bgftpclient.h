/*
File: 
bgFtpClient.h

About:
------
- FTP (File Transfer Protocol) Client implementation for B&R PCC
*/
#ifndef _BG_FTPCLIENT_H
#define _BG_FTPCLIENT_H

#include <bur/plctypes.h>
#include <AsARCfg.h>
#include <ethsock.h>
#include <astcp.h>
#include "bgstatemachine.h"
#include "bgstring.h"

/*
Public data types:
-----------------
*/

/*User Private Library Error Numbers (value assigned as suggested in the B&R help file)*/
#define USER_ERROR_MIN  0xC350
#define USER_ERROR_MAX  0xEA5F
#define USER_ERROR_FTP  USER_ERROR_MIN + 10

#define CMD_DONE	0
#define CMD_BUSY	1
#define CMD_ERROR	2

typedef struct ftpClientUser_typ{
   	DINT cmd;
   	int dataType; /*asciiOrBinary*/
	char fileName[128];
	char *dataBuffer;
	int dataBufferLength;
    int disconnectRequest;
    int connectRequest;
}ftpClientUser_typ;


#define FTPCLIENT_RESPONSECODE_LEN 16

#define FTPCLIENT_CMDBUFSIZE	256

typedef struct bgFtpClient_typ{
	bgStateMachine_typ smFtp;
	bgStateMachine_typ smFtpCmd;
	bgStateMachine_typ smFtpMultiCmd;
	bgStateMachine_typ smCmdSocket;
	bgStateMachine_typ smDataSocket;

	unsigned int userState; 	/*state of the user command execution*/

	int	cmdError;
	int	cmdErrorCode;

	char isConnected;
	char isLoggedIn;
	char isDataPortReady;
	char isDoneMultiCmd;
	char isErrorMultiCmd;

	char sockData_Timer;
	int sockData_TimerTime;
	int sockData_TimerCounter;

	char serverResponseCode;

	DINT sock;			/*command socket*/
	DINT sockData;		/*data socket - listen*/
	DINT sockData01;	/*data socket - send/recv*/
	 
	ETH_sockaddr_typ ftpServerAdrCmd;
	ETH_sockaddr_typ ftpServerAdrData;
	ETH_sockaddr_typ ftpClientAdrData;

	/*non-blocking I/O control*/
	ETH_fd_set_typ fdR;
	ETH_fd_set_typ fdW;
	ETH_timeval_typ timeOut;

	char cmdSendBuf[FTPCLIENT_CMDBUFSIZE];
	char cmdRecvBuf[FTPCLIENT_CMDBUFSIZE];
#if 0	
	char dataSendBuf[1024];
	char dataRecvBuf[1024];
#endif	
	char cmdExpectedServerResponse[FTPCLIENT_RESPONSECODE_LEN];
	char cmdExpectedServerResponseCodeOrString; /*0-code,1-string*/

	DINT sendLen;
	DINT recvLen;
	DINT sendLenData;
	DINT recvLenData;
	
	TcpOpen_typ TCPopen_1;
	TcpServer_typ TCPserv_1;
	TcpSend_typ TCPsend_1;
	TcpRecv_typ TCPrecv_1;
	TcpClose_typ TCPclose_1;
	CfgGetIPAddr_typ CfgIPAddr_1;
	char CfgIPAddrString[20];
	unsigned int ipAddr[4];	/*client's own ip address; i.e. 192.168.1.6 -> ipAddr[0]=192, ipAddr[1]=168, etc*/
	
	int counter;
	int dataPort;
	bgStringList_typ responseList;
	int test;	
	ftpClientUser_typ user;
	char 	*rxDataBuff;
}bgFtpClient_typ;



/*
Public functions:
-----------------
*/
void bgFtpClientCyclic(bgFtpClient_typ *p, int msgId);

void bgFtpClientDisconnect(bgFtpClient_typ *p);

void bgFtpClientConnectAndLogin(bgFtpClient_typ *p, char *ethernetInterface, char *serverIp);
int bgFtpClientIsLoggedIn(bgFtpClient_typ *p);
int bgFtpClientIsConnected(bgFtpClient_typ *p);
int bgFtpClientIsError(bgFtpClient_typ *p);

void bgFtpClientCmdCwdInit(bgFtpClient_typ *p, char *fileName, char *buffer, int bufferLength);
int bgFtpClientCmdCwdOnDone(bgFtpClient_typ *p);
int bgFtpClientCmdCwdOnError(bgFtpClient_typ *p);

void bgFtpClientCmdGetFileInit(bgFtpClient_typ *p, char *fileName, char* buffer, int bufferLength);
int bgFtpClientCmdGetFileOnDone(bgFtpClient_typ *p);
int bgFtpClientCmdGetFileOnError(bgFtpClient_typ *p);

void bgFtpClientCmdListInit(bgFtpClient_typ *p, char *buffer, int bufferLength);
int bgFtpClientCmdListOnDone(bgFtpClient_typ *p);
int bgFtpClientCmdListOnError(bgFtpClient_typ *p);

void bgFtpClientCmdPwdInit(bgFtpClient_typ *p, char *buffer, int bufferLength);
int bgFtpClientCmdPwdOnDone(bgFtpClient_typ *p);
int bgFtpClientCmdPwdOnError(bgFtpClient_typ *p);


#endif
