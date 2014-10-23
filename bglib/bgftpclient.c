#include <bur/plc.h>
#include <bur/plctypes.h>
#include <ethsock.h>
#include <astcp.h>
#include <asstring.h>
#include <brsystem.h>
#include <sys_lib.h>

#include "bgsprintf.h"
#include "bgstatemachine.h"
#include "bgstring.h"
#include "bgFtpClient.h"

#include "bgdebug.h"


#include "bgstatemachine.c"
#include "bgsprintf.c"
#include "bgstring.c"


/*
History:
7/22/2007
- updated to a new, AsTcp library


9/11/2005 
- added support for receiving large files over ftp
- added error event for user ftp commands:
int bgFtpClientCmd<CmdName>OnError(bgFtpClient_typ *p);


*/

void ftpClientConnect(bgFtpClient_typ *p, int msgId);
int ftpClientConnectOnDone(bgFtpClient_typ *p);
int ftpClientConnectOnError(bgFtpClient_typ *p);


void ftpClientLogin(bgFtpClient_typ *p, int msgId);
int ftpClientLoginOnDone(bgFtpClient_typ *p);
int ftpClientLoginOnError(bgFtpClient_typ *p);


void ftpClientLogout(bgFtpClient_typ *p, int msgId);
int ftpClientLogoutOnDone(bgFtpClient_typ *p);
int ftpClientLogoutOnError(bgFtpClient_typ *p);
	

void ftpClientCmdGetFile(bgFtpClient_typ *p);

void ftpClientCmdList(bgFtpClient_typ *p);

void ftpClientCmdPwd(bgFtpClient_typ *p);

void ftpClientCmdCwd(bgFtpClient_typ *p);


void ftpClientDataServer(bgFtpClient_typ *p, int msgId);
int ftpClientDataServerOnError(bgFtpClient_typ *p);
int ftpClientDataServerOnDone(bgFtpClient_typ *p);


void ftpClientSingleCmdInit(bgFtpClient_typ *p, char *cmd, char *expectedServerResponse);
void ftpClientSingleCmd(bgFtpClient_typ *p);
int ftpClientSingleCmdOnError(bgFtpClient_typ *p);
int ftpClientSingleCmdOnDone(bgFtpClient_typ *p);

void ftpClientExecuteUserCmd_cyclic(bgFtpClient_typ *p);


int ftpClientCmdMultiOnDone(bgFtpClient_typ *p, int cmdId);
int ftpClientMultiCmdOnError(bgFtpClient_typ *p, int cmdId);
int ftpClientCmdCheckServerResponseCode(bgFtpClient_typ *p);

void ftpClientInitDataPort(bgFtpClient_typ *p);
void swapBytesInShort(unsigned short *value);
void setSocketAddressData(ETH_sockaddr_typ *adrData, char *hostIpAddressString, unsigned short hostPort);
void ftpClientGetResponse(bgFtpClient_typ *p);

void ftpClientCmdTextPort(bgFtpClient_typ *p, char *textBuffer);
void ftpClientMutliStateCmdErrorLog(bgFtpClient_typ *p);

int ftpClientIncrDataPort(bgFtpClient_typ *p);

/*
########################################################################################
========================================================================================
- FTP client protocol main state machine
- must be executed as a cyclic task (<=100ms)
- protocol implemenation uses nested state machines (several levels deep)
- FTP functions such as: connect, disconnect, login, logout and different FTP commands
are all implemented as state machines 
*/
void bgFtpClientCyclic(bgFtpClient_typ *p, int msgId)
{
bgStateMachine_typ *psm = &p->smFtp;

	/*FTP client top level state machine*/
	switch( bgSMCyclic(psm) ){
		case 1:
          	/*idle; monitor user connection request flag*/
			bgSMSetStateName(psm, "Idle");
			
          	if(p->user.connectRequest){
	            p->user.connectRequest = 0;
	            p->user.disconnectRequest = 0;
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
          	}
		break;	
		case 2:
			/*connect to FTP server*/
			bgSMSetStateName(psm, "Connect");

			if(bgSMIsStateInit(psm)){			
				ftpClientConnect(p, BGSM_MSGID_INIT);
			}
			
			ftpClientConnect(p, BGSM_MSGID_CYCLIC);

			/*check if we are connected to FTP server*/			
			if( p->isConnected ){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}
			
			if(	ftpClientConnectOnError(p)){
                ERRxwarning(USER_ERROR_FTP, 1, "ftpClient-Err-socket()");
				bgSMRaiseError(psm, "error in lower level SM", 1, 0);
            }
		break;	
		case 3:
			/*login*/
			bgSMSetStateName(psm, "Login");

			if(bgSMIsStateInit(psm)){			
				ftpClientLogin(p, BGSM_MSGID_INIT);
			}
			
			ftpClientLogin(p, BGSM_MSGID_CYCLIC);

			/*check if we are looged-in*/			
			if(p->isLoggedIn){
				p->userState = 0;
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}	
			if(ftpClientLoginOnError(p)){
                ERRxwarning(USER_ERROR_FTP, 1, "ftpClient-Err-login()");
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
            }
		break;	
		case 4:
            /*connected & logged-in -> normal operational state*/
			bgSMSetStateName(psm, "Ready");

			/*monitor requests & execute FTP commands*/            
			ftpClientExecuteUserCmd_cyclic(p);

			/*terminate if socket error detected*/
			if(p->cmdError == ERR_SOCKET){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}

			if(p->test){
				p->test = 0;
				ftpClientGetResponse(p);
			}

            /*monitor user disconnect request*/            
            /*NOTE: upon receiving request, make sure to wait that FTP command in progress completes!*/

			/*disconnect allowed only after command completes*/
            if(p->user.disconnectRequest && !p->userState){
                p->user.connectRequest = 0;
                p->user.disconnectRequest = 0;
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
            }
		break;
		case 5:
            /*logout*/
			bgSMSetStateName(psm, "Logout");

			if(bgSMIsStateInit(psm)){			
				ftpClientLogout(p, BGSM_MSGID_INIT);
			}	
			
			ftpClientLogout(p, BGSM_MSGID_CYCLIC);

			/*chech if we are logged out*/			
			if(!p->isLoggedIn){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}	
			
			if(ftpClientLogoutOnError(p)){
                ERRxwarning(USER_ERROR_FTP, 1, "ftpClient-Err-socket()");
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
            }
		break;
		case 6:
			bgSMSetStateName(psm, "Close CMD socket");
            /*normal operation; close socket*/
			bgSMRaiseDone(psm, 0);
		break;	
		default:
			/*unhandled state*/
			bgSMRaiseError(psm, "unknown state", 0, bgSMGetState(psm));
		break;
	}	


	if(bgSMOnDone(psm)){
		/*error state; socket closing is needed*/
		close(p->sock);

		p->isConnected = 0;
		
		bgSMSetNextState(psm, 1);
	}

	if(bgSMOnError(psm)){
		/*check if socket close needed or not?*/	
		if( !bgSMGetErrorId(psm) )
			close(p->sock);
		
		p->isConnected = 0;
		p->cmdError = 1;
		bgSMSetNextState(psm, 1);
	}
}
/*
########################################################################################
	 INDIVIDUAL FTP COMMANDS
- each individual FTP command has following USER level interface methods:
	ftpClientCmd<Name>Init()
		- start command execution; pass-in user parameters; init cmd state machine
	ftpClientCmd<Name>OnDone()
		- monitor cmd completion (0-not complete; 1-complete)

- each command also has core implemenation (not used or called by the user)
	ftpClientCmd<Name>()

- FTP commands such as: PWD, CWD, PUT, GET, DIR are multi-level commands: it usually takes
several primitive FTP commands (i.e. PORT, TYPE, RETR etc) to complete one FTP command

########################################################################################
*/

/*
========================================================================================
- state machine that handles execution of individual, multi-level FTP commands 
- FTP client executes one Multi-level command at the time
*/
void ftpClientExecuteUserCmd_cyclic(bgFtpClient_typ *p)
{
	/*execute user commands*/
	switch(p->userState){
	/*idle*/
        case 0:
        break;            
        case 1:
        break;
        case 2:
            ftpClientCmdList(p);
        break;
        case 4:
            ftpClientCmdPwd(p);
        break;
        case 6:
            ftpClientCmdGetFile(p);
        break;
        case 8:
            ftpClientCmdCwd(p);
        break;
        default:
            p->userState = 0;
        break;            
	}

}

/*
========================================================================================
FTP Command Implementation: 
GetFile
*/
void bgFtpClientCmdGetFileInit(bgFtpClient_typ *p, char *fileName, char* buffer, int bufferLength)
{
    /*select command to execute*/
	p->userState = 6;

	/*pass command parameters*/
	p->user.dataBuffer = buffer;
	p->user.dataBufferLength = bufferLength;
	strcpy(&p->user.fileName[0], fileName);

    /*start command execution*/
	bgSMReset(&p->smFtpMultiCmd);
}

int bgFtpClientCmdGetFileOnDone(bgFtpClient_typ *p)
{
	return ftpClientCmdMultiOnDone(p, 6);
}

int bgFtpClientCmdGetFileOnError(bgFtpClient_typ *p)
{
	return ftpClientMultiCmdOnError(p, 6);
}

void ftpClientCmdGetFile(bgFtpClient_typ *p)
{
char cmdText[256];
bgStateMachine_typ *psm = &p->smFtpMultiCmd;

	/*FTP client multi level command state machine*/
	switch( bgSMCyclic(psm) ){
		case 1:
          	/*idle - monitor user connection request flag*/
			bgSMSetStateName(psm, "GetFile-InitDataSocket");

			if( bgSMIsStateInit(psm) ){
				/*generate port number to use*/
				ftpClientIncrDataPort(p);
				/*init Data transport socket*/
	            ftpClientDataServer(p, BGSM_MSGID_INIT);
			}

			/*wait for ready msg from the DataServer*/
            if( p->isDataPortReady ){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
            }
/*TODO: timeout*/			
		break;	
		case 2:
    		/*PORT command: "PORT 192,168,1,6,0,20<CR><LF>"*/
			bgSMSetStateName(psm, "GetFile-PORT");
			if(bgSMIsStateInit(psm)){			
	    		/*send command*/
	            ftpClientCmdTextPort(p, &cmdText[0]);

	    		ftpClientSingleCmdInit(p, &cmdText[0], "2??");
			}
			
    		/*recv cmd response*/
    		ftpClientSingleCmd(p);
			if(ftpClientSingleCmdOnDone(p)){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}
			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
			}			
		break;	
		case 3:
    		/*TYPE command: "TYPE I<CR><LF>"*/
			bgSMSetStateName(psm, "GetFile-TYPE");
			if(bgSMIsStateInit(psm)){			
	    		/*send command*/
	    		ftpClientSingleCmdInit(p, "TYPE I", "2??");
			}
			
    		/*recv cmd response*/
    		ftpClientSingleCmd(p);
			if(ftpClientSingleCmdOnDone(p)){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}

			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
			}
		break;	
		case 4:
    		/*RETR command: "RETR<CR><LF>""*/
			bgSMSetStateName(psm, "GetFile-RETR");
			if(bgSMIsStateInit(psm)){			
	    		/*send command*/
	    		/*send command*/
	    		strcpy(&cmdText[0], "RETR ");
	    		strcat(&cmdText[0], &p->user.fileName[0]);
	    		ftpClientSingleCmdInit(p, &cmdText[0], "2??");
			}
			
    		/*recv cmd response*/
    		ftpClientSingleCmd(p);
			if(ftpClientSingleCmdOnDone(p)){
				/*notify data port server that the response is comming*/
				ftpClientDataServer(p, 2);
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}

			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
				/*close data server port*/
				ftpClientDataServer(p, 3);
			}
		break;	
        case 5:
			bgSMSetStateName(psm, "GetFile-DATASRVR active");
        break;            
		default:
			/*unhandled state*/
			bgSMRaiseError(psm, "unknown state", 0, bgSMGetState(psm));
		break;	
	}


	if(bgSMOnError(psm)){
		p->isErrorMultiCmd = p->userState;
		p->userState = 0;
		return;
	}

	ftpClientDataServer(p, BGSM_MSGID_CYCLIC);

	if(ftpClientDataServerOnDone(p)){			
		p->isDoneMultiCmd = p->userState;
		p->userState = 0;
	}

	if(ftpClientDataServerOnError(p)){
		p->isErrorMultiCmd = p->userState;
		p->userState = 0;
	}

}
/*
========================================================================================
FTP Command Implementation: 
List (dir)
*/
void bgFtpClientCmdListInit(bgFtpClient_typ *p, char *buffer, int bufferLength)
{
    /*select command to execute*/
	p->userState = 2;

	/*pass command parameters*/
	p->user.dataBuffer = buffer;
	p->user.dataBufferLength = bufferLength;

    /*start command execution*/
	bgSMReset(&p->smFtpMultiCmd);
}

int bgFtpClientCmdListOnDone(bgFtpClient_typ *p)
{
	return ftpClientCmdMultiOnDone(p, 2);
}

int bgFtpClientCmdListOnError(bgFtpClient_typ *p)
{
	return ftpClientMultiCmdOnError(p, 2);

}

void ftpClientCmdList(bgFtpClient_typ *p)
{
char cmdText[64];
bgStateMachine_typ *psm = &p->smFtpMultiCmd;

	/*FTP client multi level command state machine*/
	switch( bgSMCyclic(psm) ){
		case 1:
          	/*idle - monitor user connection request flag*/
			bgSMSetStateName(psm, "List-InitDataSocket");

			if( bgSMIsStateInit(psm) ){
				/*generate port number to use*/
				ftpClientIncrDataPort(p);
				/*init Data transport socket*/
	            ftpClientDataServer(p, BGSM_MSGID_INIT);
			}

			/*wait for ready msg from the DataServer*/
            if( p->isDataPortReady ){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
            }
/*TODO: timeout*/			
		break;	
		case 2:
    		/*PORT command: "PORT 192,168,1,6,0,20<CR><LF>"*/
			bgSMSetStateName(psm, "List-PORT");
			if(bgSMIsStateInit(psm)){			
	    		/*send command*/
	            ftpClientCmdTextPort(p, &cmdText[0]);

	    		ftpClientSingleCmdInit(p, &cmdText[0], "2??");
			}
			
    		/*recv cmd response*/
    		ftpClientSingleCmd(p);
			if(ftpClientSingleCmdOnDone(p)){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}
			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
			}			
		break;	
		case 3:
    		/*TYPE command: "TYPE I<CR><LF>"*/
			bgSMSetStateName(psm, "List-TYPE");
			if(bgSMIsStateInit(psm)){			
	    		/*send command*/
	    		ftpClientSingleCmdInit(p, "TYPE I", "2??");
			}
			
    		/*recv cmd response*/
    		ftpClientSingleCmd(p);
			if(ftpClientSingleCmdOnDone(p)){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}

			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
			}
		break;	
		case 4:
    		/*LIST command: "LIST<CR><LF>""*/
			bgSMSetStateName(psm, "List-List");
			if(bgSMIsStateInit(psm)){			
	    		/*send command*/
	    		ftpClientSingleCmdInit(p, "LIST", "2??");
			}
			
    		/*recv cmd response*/
    		ftpClientSingleCmd(p);
			if(ftpClientSingleCmdOnDone(p)){
				/*notify data port server that the response is comming*/
				ftpClientDataServer(p, 2);
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}

			if(ftpClientSingleCmdOnError(p)){
				/*close data server port*/
				ftpClientDataServer(p, 3);
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
			}
		break;	
        case 5:
			bgSMSetStateName(psm, "List-DATASRVR active");
        break;            
		default:
			/*unhandled state*/
			bgSMRaiseError(psm, "unknown state", 0, bgSMGetState(psm));
		break;	
	}


	if(bgSMOnError(psm)){
		p->isErrorMultiCmd = p->userState;
		p->userState = 0;
		return;
	}

	ftpClientDataServer(p, BGSM_MSGID_CYCLIC);

	if(ftpClientDataServerOnDone(p)){			
		p->isDoneMultiCmd = p->userState;
		p->userState = 0;
	}

	if(ftpClientDataServerOnError(p)){
		p->isErrorMultiCmd = p->userState;
		p->userState = 0;
	}
}
/*
========================================================================================
FTP Command Implementation: 
Pwd (Print Working Directory)
*/
void bgFtpClientCmdPwdInit(bgFtpClient_typ *p, char *buffer, int bufferLength)
{
    /*select command to execute*/
	p->userState = 4;

	/*pass command parameters*/
	p->user.dataBuffer = buffer;
	p->user.dataBufferLength = bufferLength;

    /*start command execution*/
	bgSMReset(&p->smFtpMultiCmd);
}

int bgFtpClientCmdPwdOnDone(bgFtpClient_typ *p)
{
	return ftpClientCmdMultiOnDone(p, 4);
}

int bgFtpClientCmdPwdOnError(bgFtpClient_typ *p)
{
	return ftpClientMultiCmdOnError(p, 4);

}

void ftpClientCmdPwd(bgFtpClient_typ *p)
{
bgStateMachine_typ *psm = &p->smFtpMultiCmd;

	/*FTP client multi level command state machine*/
	switch( bgSMCyclic(psm) ){
		case 1:
    		/*XPWD command: "XPWD<CR><LF>"*/
			bgSMSetStateName(psm, "PWD");
			if(bgSMIsStateInit(psm)){			
	    		/*send command*/
	    		ftpClientSingleCmdInit(p, "PWD", "2??");
			}
			
    		/*recv cmd response*/
    		ftpClientSingleCmd(p);
			
			if(ftpClientSingleCmdOnDone(p)){
				/*copy response strig into the user supplied buffer*/
				strcpy(p->user.dataBuffer, p->responseList.items[0]);
				bgSMRaiseDone(psm, 0);
			}

			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "got lower SM error", 0, 0);
			}
		break;	
		default:
			/*unhandled state*/
			bgSMRaiseError(psm, "unknown state", 0, bgSMGetState(psm));
		break;	
	}

	if(bgSMOnError(psm)){
		p->userState = 0;
	}

	if(bgSMOnDone(psm)){
		p->isDoneMultiCmd = p->userState;
		p->userState = 0;
	}

}
/*
========================================================================================
FTP Command Implementation: 
Cwd (Change Working Directory)
*/
void bgFtpClientCmdCwdInit(bgFtpClient_typ *p, char *fileName, char *buffer, int bufferLength)
{
    /*select command to execute*/
	p->userState = 8;

	/*pass command parameters*/
	p->user.dataBuffer = buffer;
	p->user.dataBufferLength = bufferLength;
	strcpy(&p->user.fileName[0], fileName);

    /*start command execution*/
	bgSMReset(&p->smFtpMultiCmd);

}

int bgFtpClientCmdCwdOnDone(bgFtpClient_typ *p)
{
	return ftpClientCmdMultiOnDone(p, 8);
}


int bgFtpClientCmdCwdOnError(bgFtpClient_typ *p)
{
	return ftpClientMultiCmdOnError(p, 8);

}

void ftpClientCmdCwd(bgFtpClient_typ *p)
{
char cmdText[255];

bgStateMachine_typ *psm = &p->smFtpMultiCmd;

	/*FTP client multi level command state machine*/
	switch( bgSMCyclic(psm) ){
		case 1:
    		/*XPWD command: "CWD <fileName><CR><LF>"*/
			bgSMSetStateName(psm, "CWD");
			if(bgSMIsStateInit(psm)){			
	    		/*send command: "CWD <fileName><CR><LF>"*/
	    		strcpy(&cmdText[0], "CWD ");
	    		strcat(&cmdText[0], &p->user.fileName[0]);
	    		ftpClientSingleCmdInit(p, &cmdText[0], "2??");
			}
			
    		/*recv cmd response*/
    		ftpClientSingleCmd(p);

			if(ftpClientSingleCmdOnDone(p)){
				/*copy response strig into the user supplied buffer*/
				strcpy(p->user.dataBuffer, p->responseList.items[0]);
				bgSMRaiseDone(psm, 0);
			}

			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "got lower SM error", 0, 0);
			}
		break;	
	}

	if(bgSMOnError(psm)){
		p->isErrorMultiCmd = p->userState;
		p->userState = 0;
		return;
	}

	if(bgSMOnDone(psm)){
		p->isDoneMultiCmd = p->userState;
		p->userState = 0;
	}
}
/*
########################################################################################
========================================================================================
FTP Connect command state machine:
*/
int ftpClientConnectOnDone(bgFtpClient_typ *p){
	return bgSMOnDone(&p->smCmdSocket);
}

int ftpClientConnectOnError(bgFtpClient_typ *p){
	return bgSMOnError(&p->smCmdSocket);
}

void ftpClientConnect(bgFtpClient_typ *p, int msgId)
{
DINT ret;
bgStateMachine_typ *psm = & p->smCmdSocket;

	if( msgId == BGSM_MSGID_INIT){
		bgSMReset(psm);
	}
	
    switch(bgSMCyclic(psm)){    
        case 1:
			bgSMSetStateName(psm, "Connect-CreateSocket");

			if( bgSMIsStateInit(psm) ){
				p->cmdError = 0;
				p->cmdErrorCode = 0;
				p->isConnected = 0;
				/*create socket*/
				p->sock = socket(ETH_AF_INET, ETH_SOCK_STREAM, 0);

				if( p->sock == ERR_SOCKET ){
					bgSMRaiseError(psm, "socket create err", 0, EthGetError());
					break;
				}

				/*set non-blocking socket operation*/
				{
					UDINT arg = 1;
					ioctl(p->sock, ETH_FIONBIO, (UDINT)&arg);
				}

				/*reset select flags*/			
				fd_zero((UDINT)&p->fdR);
				fd_zero((UDINT)&p->fdW);

				/*set time-out to 10ms*/
				p->timeOut.tv_sec = 0;
				p->timeOut.tv_usec = 1000 * 10;

				/*adjust my own timeout counters*/
				p->sockData_TimerCounter = 0;
				p->sockData_TimerTime = 10;
#if 0
/*bind not necessary*/
				/*bind client socket*/
				ret = bind(p->sock, (UDINT)&p->ftpClientAdrData, sizeof(p->ftpClientAdrData));
				if(ret == ERR_SOCKET){
					bgSMRaiseError(psm, "socket bind err", 0, EthGetError());
					break;
				}
#endif				
			}

			/*connect to remote host*/
			ret = connect(p->sock, (UDINT)&p->ftpServerAdrCmd, sizeof(p->ftpServerAdrCmd) );
			
			if(ret == ERR_SOCKET){
				UINT errCode = EthGetError();
				if(errCode == ERR_ETH_INPROGRESS){
					/*next: wait connect to complete*/
					bgSMSetNextState(psm, bgSMGetState(psm) + 1);
				    break;
				}else
				if(errCode == ERR_ETH_ISCONN){
					/*it's connected -> done*/
					ERRxwarning(USER_ERROR_FTP, errCode, "connect()-alreadyOK!");
					bgSMRaiseDone(psm, 0);
					break;
				}
				else{
					bgSMRaiseError(psm, "socket connect err", 0, EthGetError());
					break;
				}
			}
			/*connected!*/
			ERRxwarning(USER_ERROR_FTP, ret, "connect()-OK-Quick!");
			bgSMRaiseDone(psm, 0);
		break;	
		case 2:
			bgSMSetStateName(psm, "Connect-Connect");
			
			/*select waits for connection to complete*/
			fd_set(p->sock, (UDINT)&p->fdR);
			fd_set(p->sock, (UDINT)&p->fdW);
			ret = sock_select(32, (UDINT)&p->fdR, (UDINT)&p->fdW, 0L, (UDINT)&p->timeOut);
			
			if(ret == ERR_SOCKET){
				bgSMRaiseError(psm, "socket connect err", 0, EthGetError());
				break;
			}
/*
			DBG("connect: ret=%d, error=%d\n\r", ret, EthGetError());
*/
			if(ret == 0){
				if( p->sockData_TimerCounter >= p->sockData_TimerTime ){
					/*NOTES: 
					- trying to figure out how does the select timeout works in B&R!
					- using EthGetError() here is not needed, just to see what comes back
					*/
					bgSMRaiseError(psm, "connect timeout", 0, EthGetError());
				}else{
					p->sockData_TimerCounter++;
				}
			}else{
				if( fd_isset(p->sock, (UDINT)&p->fdW) ){					
					bgSMRaiseDone(psm, 0);
				}
			}
		break;
		default:
			/*unhandled state*/
			bgSMRaiseError(psm, "unknown state", 0, bgSMGetState(psm));
		break;	
    }


	if( bgSMOnDone(psm)){
		ERRxwarning(USER_ERROR_FTP, 0, "connect()-OK!");
		p->isConnected = 1;
	}

	if( bgSMOnError(psm)){

	}
}
/*
========================================================================================
*/
void ftpClientMutliStateCmdErrorLog(bgFtpClient_typ *p){
char text[64];
	/*log error*/
	strcpy(&text[0], "Err: ");
	strcat(&text[0], &p->cmdSendBuf[0]);
    ERRxwarning(USER_ERROR_FTP, p->cmdErrorCode, &text[0]);
}
/*
########################################################################################
========================================================================================
FTP Login command state machine:
-return values:
*/
int ftpClientLoginOnDone(bgFtpClient_typ *p){
	return bgSMOnDone(&p->smFtpMultiCmd);
}

int ftpClientLoginOnError(bgFtpClient_typ *p){
	return bgSMOnError(&p->smFtpMultiCmd);
}
void ftpClientLogin(bgFtpClient_typ *p, int msgId)
{
bgStateMachine_typ *psm = &p->smFtpMultiCmd;

	if( msgId == BGSM_MSGID_INIT){
		bgSMReset(psm);
	}
	
    switch(bgSMCyclic(psm)){    
        case 1:
			/*send USER name*/
			bgSMSetStateName(psm, "Login-Welcome");

			if( bgSMIsStateInit(psm) ){
				/*send request*/
				/*ftpClientSingleCmdInit(p, 0, ">name");*/
				ftpClientSingleCmdInit(p, 0, ">FTP");
			}
			
			/*recv response*/
			ftpClientSingleCmd(p);

			if(ftpClientSingleCmdOnDone(p)){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}

			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
			}
        break;
        case 2:
			/*send USER name*/
			bgSMSetStateName(psm, "Login-UserName");

			if( bgSMIsStateInit(psm) ){
				/*send request*/
				ftpClientSingleCmdInit(p, "USER  anonymous", "331");
			}
			
			/*recv response*/
			ftpClientSingleCmd(p);

			if(ftpClientSingleCmdOnDone(p)){
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}

			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
			}
        break;
        case 3:
			/*send PASSWORD */
			bgSMSetStateName(psm, "Login-Password");

			if( bgSMIsStateInit(psm) ){
				/*send request*/
				ftpClientSingleCmdInit(p, "PASS  anonymous@", "230");
			}

			/*recv response*/
			ftpClientSingleCmd(p);

			if(ftpClientSingleCmdOnDone(p)){
				bgSMRaiseDone(psm, 0);
			}

			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
			}
        break;
        default:
			/*unhandled state*/
			bgSMRaiseError(psm, "unknown state", 0, bgSMGetState(psm));
        break;
    }    	

	if( bgSMOnDone(psm) ){
		p->isLoggedIn = 1;
	}

	if( bgSMOnError(psm) ){
		ftpClientMutliStateCmdErrorLog(p);
	}

}
/*
########################################################################################
========================================================================================
FTP Logout command state machine:
-return values:
*/
int ftpClientLogoutOnDone(bgFtpClient_typ *p){
	return bgSMOnDone(&p->smFtpMultiCmd);
}

int ftpClientLogoutOnError(bgFtpClient_typ *p){
	return bgSMOnError(&p->smFtpMultiCmd);
}

void ftpClientLogout(bgFtpClient_typ *p, int msgId)
{
bgStateMachine_typ *psm = &p->smFtpMultiCmd;

	if( msgId == BGSM_MSGID_INIT){
		bgSMReset(psm);
	}
	
    switch(bgSMCyclic(psm)){    
        case 1:
			/*send USER name*/
			bgSMSetStateName(psm, "Logout-Quit");

			if( bgSMIsStateInit(psm) ){
				/*send request*/
				ftpClientSingleCmdInit(p, "QUIT", "2??");
			}
			
			/*recv response*/
			ftpClientSingleCmd(p);

			if(ftpClientSingleCmdOnDone(p)){
				bgSMRaiseDone(psm, 0);
			}

			if(ftpClientSingleCmdOnError(p)){
				bgSMRaiseError(psm, "error in lower level SM", 0, 0);
			}
        break;
        default:
			/*unhandled state*/
			bgSMRaiseError(psm, "unknown state", 0, bgSMGetState(psm));
        break;
    }


	if(bgSMOnDone(psm)){
		p->isLoggedIn = 0;
	}
	
	if(bgSMOnError(psm)){
		p->isLoggedIn = 0;
		ftpClientMutliStateCmdErrorLog(p);
	}
}
/*
########################################################################################
========================================================================================
Lowest level state machine handles execution of a single FTP request/response messaging

ftpClientSingleCmdInit();
	- selects command to execute; init command state machine
	
ftpClientSingleCmd();
	- command execution state machine

ftpClientCmdCheckServerResponseCode()
	- command response check and verification

*/
void ftpClientSingleCmdInit(bgFtpClient_typ *p, char *cmd, char *expectedServerResponse){
int i;

	/*copy command string into send buffer*/
	p->cmdSendBuf[0]=0;
	if(cmd){
		strcpy(&p->cmdSendBuf[0], cmd);
		/*append string termination -> CRLF*/
		strcat(&p->cmdSendBuf[0], "\r\n");
	}

	/*define expected server response string?*/
	p->cmdExpectedServerResponse[0] = 0;	/*don't care*/
	if(expectedServerResponse){
		/*is expected server response code or string?*/
		if( *expectedServerResponse == '>'){
			i = 1;
			p->cmdExpectedServerResponseCodeOrString = 1;	/*string*/
		}else{
			i = 0;
			p->cmdExpectedServerResponseCodeOrString = 0;	/*code*/
		}
		bgStrCpyN(&p->cmdExpectedServerResponse[0], expectedServerResponse+i, FTPCLIENT_RESPONSECODE_LEN);
	}


	/*clr rcv buffer string*/
	memset(&p->cmdRecvBuf[0], 0, FTPCLIENT_CMDBUFSIZE);
#if 0
/*clear response lines*/
	/*init ptrs*/
	p->responseList.maxCount = CMD_RESPONSE_MAXLINES;
	p->responseList.lines[0] = &p->responseList.line0[0];	
	p->responseList.lines[1] = &p->responseList.line1[0];	
	p->responseList.lines[2] = &p->responseList.line2[0];	
	p->responseList.lines[3] = &p->responseList.line3[0];	
	p->responseList.lines[4] = &p->responseList.line4[0];	

	/*clear all response lines*/
	for(i=0;i<CMD_RESPONSE_MAXLINES; i++){
		memset(p->responseList.lines[i], 0, CMD_RESPONSE_MAXLINELEN);
	}
	/*number of lines retrieved from the cmd buffer*/
	p->responseList.count = 0; 
#endif	
	/*number of lines retrieved from the cmd buffer*/
	p->responseList.itemCount = 0; 

	
	/*reset command execution state machine*/
	bgSMReset(&p->smFtpCmd);
}

void ftpClientSingleCmd(bgFtpClient_typ *p){
DINT len;
DINT ret;
bgStateMachine_typ *psm = &p->smFtpCmd;

	/*state machine states*/
	switch( bgSMCyclic(psm) ){
		case 1:
          	/*send command request*/
			bgSMSetStateName(psm, "ftpSingleCmd");

			if(bgSMIsStateInit(psm)){
				/*check input conditions and paramters*/
				if( !p->isConnected ){
					bgSMRaiseError(psm, "not connected", 0, 0);
					break;
				}	
				if( !p->cmdSendBuf[0]){
					/*nothing to send! receive only (i.e. welcome message just after connect()*/
					break;
				}
				
				/*send command to server*/
	         	len = strlen(&p->cmdSendBuf[0]);
	        	ret = send(p->sock, (UDINT)&p->cmdSendBuf[0], len, 0L);			
	        		
	        	if(ret == ERR_SOCKET){
					bgSMRaiseError(psm, "send() error", 0, EthGetError());
					break;
	        	}   	
				
				/*remember the send length*/
				DBG("Single CMD send();Bytes=%d;[%s]\r\n", (int)ret, &p->cmdSendBuf[0]);
				p->sendLen = ret;				
				return;
			}	
			
	       	/*call receive (NON BLOCKING MODE!)*/
        	ret = recv(p->sock, (UDINT)&p->cmdRecvBuf[0],  FTPCLIENT_CMDBUFSIZE, 0L);			
        	
        	if(ret == ERR_SOCKET){
        		UINT errCode = EthGetError();
        		/*none blocking receive return this following value if data is not available yet*/
        		if( errCode == ERR_ETH_WOULDBLOCK ){
        			/*wait response*/
					return;
        		}
				
				/*recv error*/
				bgSMRaiseError(psm, "recv() error", 0, EthGetError());
				break;				
        	}

			if(ret == 0){
				bgSMRaiseError(psm, "rx len=0", 0, 0);
				return;
			}	
			
			/*receive is complete; store receive length & terminate rx buffer*/
			p->cmdRecvBuf[ret] = 0;
			DBG("Single CMD recv();Bytes=%d;[%s]\r\n", (int)ret, &p->cmdRecvBuf[0]);
			p->recvLen = ret;

			ret = ftpClientCmdCheckServerResponseCode(p);

			if( ret == 1)
				bgSMRaiseDone(psm, 0);

			if( ret == 2)
				bgSMRaiseError(psm, "server error response", 0, 0);
				
		break;	
		default:
			/*unhandled state*/
			bgSMRaiseError(psm, "unknown state", 0, bgSMGetState(psm));
		break;	
	}

	/*error event handler*/
	if(bgSMOnError(psm)){
		DBG("ftpClientSingleCmd - ERROR.\r\n");
	}

	/*done event handler*/
	if(bgSMOnDone(psm)){
		DBG("ftpClientSingleCmd - DONE.\r\n");
	}
}

int ftpClientSingleCmdOnError(bgFtpClient_typ *p)
{
	return bgSMOnError(&p->smFtpCmd);
}	

int ftpClientSingleCmdOnDone(bgFtpClient_typ *p)
{
	return bgSMOnDone(&p->smFtpCmd);
}

/*
========================================================================================
FTP command response code is an ASCII line that looks like:
"xyz-response text<CR><LF>"
where x,y,z are numeric chars <0-9> followed by a '-' and the response text
- multiple lines per response are possible
- this function retrieves the response from the server, formats it into a list (array) of lines
and then searches each line for the user supplied code (res1,res2,res3)
- if res<x> is given as 0, it means don't care (see ftp spec)

*/
int ftpClientCmdCheckServerResponseCode(bgFtpClient_typ *p)
{
char serverResponseCode[3];
char *line;
int result;
int i;
	
	/*get server's response
	IMPORTANT:
	- note that data for ftpClientGetResponse() method is initilized within ftpClientSingleCmdInit()!
	- order of execution is like:
		init:
			ftpClientSingleCmdInit();
		cyclic:
			if( ftpClientSingleCmd() == DONE )
				if( ftpClientCmdCheckServerResponseCode() == OK )
					nextState;

	- therfore, the ftpClientGetResponse() shall be designed to operate in an incremenal mode: every time
	ftpClientSingleCmd() completes (data is available in the receive buffer) we should just add to the existing lines
	and not delete what we've received till that moment
	*/	
	ftpClientGetResponse(p);



	/*final result of the ftpClientCmdCheckServerResponseCode() method:
	0 - response not found (not available -> wait)
	1 - response found (and it's ok; match with expected response)
	2 - response found (and it's not ok; error response from server)	
	*/
	p->serverResponseCode = 0;

	/*anything to process!?*/	
	if(!p->responseList.itemCount)
		return 0;

	/*check all received response lines every time (not necessary but it's easy to implement and it does not metter*/	
	for(i=0; i<p->responseList.itemCount; i++){
		line = p->responseList.items[i];

		if(!p->cmdExpectedServerResponseCodeOrString){
			serverResponseCode[0]=*(line+0);		
			serverResponseCode[1]=*(line+1);		
			serverResponseCode[2]=*(line+2);		

			result = 0;	
			if( p->cmdExpectedServerResponse[0] != '?'){
				if(p->cmdExpectedServerResponse[0] == serverResponseCode[0])
					result++;	
			}else
				result++;

			if( p->cmdExpectedServerResponse[1] != '?'){
				if(p->cmdExpectedServerResponse[1] == serverResponseCode[1])
					result++;	
			}else
				result++;

			if( p->cmdExpectedServerResponse[2] != '?'){
				if(p->cmdExpectedServerResponse[2] == serverResponseCode[2])
					result++;	
			}else
				result++;

				
			if( result >= 3)
				return 1;

			if( (serverResponseCode[0] == '4') || (serverResponseCode[0] == '5'))
				return 2;		
		}else{
			result = bgStrFind(line, &p->cmdExpectedServerResponse[0], -1, -1);

			if(result >= 0)
				return 1;
		}			
	}

	return 0;
}
/*
========================================================================================
*/
void ftpClientGetResponse(bgFtpClient_typ *p){
int i;
char *src;

	/*ptr to rcv buffer*/
	src = &p->cmdRecvBuf[0];
	p->responseList.src = src;	
	bgStringListLines(&p->responseList);

	DBG("Received Cmd Response: lines=%d\r\n", p->responseList.itemCount);
	for(i=0;i<p->responseList.itemCount;i++){
		DBG("Line[%d]=%s\r\n", i, p->responseList.items[i]);
	}

}

int readline(char *src, char *dest, int maxLength){
/*
return: >= 0 (length of the line including CRLF)
return: -1 (no line read)
*/
int lineLen;
char *cPtr;

	lineLen = 0;
	cPtr = src;
	while(maxLength--){
	
		if( (*cPtr == 0x0D) || ( *cPtr == 0x0A) ){
			
			lineLen++;
			cPtr++;
			
			if( (*cPtr == 0x0D) || ( *cPtr == 0x0A) ){
				lineLen++;
			}	
			
			memcpy(dest, src, lineLen);
			*(dest+lineLen+1)= 0;
			return lineLen;		
		}
		
		if( *cPtr == 0 )
			return -1;			
		
		lineLen++;
		cPtr++;  
	}
			
	return -1;
}

/*
========================================================================================
*/
int ftpClientCmdMultiOnDone(bgFtpClient_typ *p, int cmdId)
{
/*generate CmdIsDone signal (1 shoot)*/
	if( p->isDoneMultiCmd == cmdId ){
		p->isDoneMultiCmd = 0;
		return 1;
	}else
		return 0;
}

int ftpClientMultiCmdOnError(bgFtpClient_typ *p, int cmdId){
/*generate CmdIsError signal (1 shoot)*/
	if( p->isErrorMultiCmd == cmdId ){
		p->isErrorMultiCmd = 0;
		return 1;
	}else
		return 0;
}

/*
========================================================================================
*/
void bgFtpClientConnectAndLogin(bgFtpClient_typ *p, char *ethernetDevice, char *serverIp)
{
unsigned long ipAddrClient;

    /*set server ip address and ports to use*/
    /*  21 - standard FTP command port
        20 - standard FTP data port
    */
	setSocketAddressData(&p->ftpServerAdrCmd, serverIp, 21);    
	setSocketAddressData(&p->ftpClientAdrData, "", ETH_INADDR_ANY);

	/*retrieve own ip address*/
	p->CfgIPAddr_1.enable = 1;
	p->CfgIPAddr_1.pDevice = (UDINT)ethernetDevice;
	p->CfgIPAddr_1.pIPAddr = (UDINT)&p->CfgIPAddrString[0];
	p->CfgIPAddr_1.Len = 16;
	CfgGetIPAddr(&p->CfgIPAddr_1);

	ipAddrClient = inet_addr( (unsigned long) &p->CfgIPAddrString[0]);

	/*store ip address in an easy format for later use; see PORT command*/	
	p->ipAddr[3]= (ipAddrClient>>24)&0xFF;
	p->ipAddr[2]= (ipAddrClient>>16)&0xFF;
	p->ipAddr[1]= (ipAddrClient>>8)&0xFF;
	p->ipAddr[0]= (ipAddrClient)&0xFF;


	p->cmdError = 0;
	p->cmdErrorCode = 0;
	p->isConnected = 0;
    p->user.connectRequest = 1;
}
/*
========================================================================================
*/
int bgFtpClientIsLoggedIn(bgFtpClient_typ *p)
{
    return p->isLoggedIn;
}

/*
========================================================================================
*/
void bgFtpClientDisconnect(bgFtpClient_typ *p)
{
    p->user.disconnectRequest = 1;
}
/*
========================================================================================
*/
int bgFtpClientIsConnected(bgFtpClient_typ *p)
{
    return p->isConnected;
}   
/*
========================================================================================
*/
int bgFtpClientIsError(bgFtpClient_typ *p)
{
    return p->cmdError;
}

int ftpClientIncrDataPort(bgFtpClient_typ *p){

	if( !p->dataPort){
		p->dataPort = 190;
	}

	if(p->dataPort>=190)
		p->dataPort++;
		
		
	if(p->dataPort>=200)
		p->dataPort = 190;
	
	return p->dataPort;
}
/*
========================================================================================
*/
void ftpClientCmdTextPort(bgFtpClient_typ *p, char *textBuffer)
{
/*
	bgsprintf(textBuffer, "PORT 192,168,1,6,4,%d", p->dataPort);
*/
	bgsprintf(textBuffer, "PORT %d,%d,%d,%d,4,%d", p->ipAddr[0], p->ipAddr[1], p->ipAddr[2], p->ipAddr[3], p->dataPort);
}
/*
========================================================================================
*/
int ftpClientDataServerOnError(bgFtpClient_typ *p)
{
	return bgSMOnError(&p->smDataSocket);
}

int ftpClientDataServerOnDone(bgFtpClient_typ *p)
{
	return bgSMOnDone(&p->smDataSocket);
}

void ftpClientDataServer(bgFtpClient_typ *p, int msgId)
{   
bgStateMachine_typ *psm = &p->smDataSocket;

    /*init data socket (listen)*/
	if( (msgId == BGSM_MSGID_INIT) ){
		bgSMReset(psm);
		p->sockData_TimerTime = 5;
		p->sockData_TimerCounter = 0;
		p->recvLenData = 0;
		p->sendLenData = 0;
		return;
	}

    /*close data socket (listen stop)*/
	if( msgId == 2){
		/*command socket is telling us to receive data from server
		after this event, keep receiving until we don't get any data
		for a certain period of time or the remote host closes the
		connection
		*/
		p->sockData_Timer = 1;
		p->sockData_TimerTime = 10;
		p->sockData_TimerCounter = 0;
		return;
	}

    /*close data socket now!*/
	if( msgId == 3){
		/*command socket is telling us to immediately close data socket
		*/
		bgSMSetNextState(psm, 3);
		return;
	}
	
	switch(bgSMCyclic(psm)){
		case 1:
			bgSMSetStateName(psm, "DataPort-Open");
			if(bgSMIsStateInit(psm)){

				p->TCPopen_1.enable = 1;
				p->TCPopen_1.port = 4*256 + p->dataPort;
				p->TCPopen_1.pIfAddr = 0;
				p->TCPopen_1.options = 0;
				DBG("data port open: init\r\n");
				break;

			}	
			
    		TcpOpen(&p->TCPopen_1);					/*open a listening socket*/

			/*server connection accepted*/
			if(p->TCPopen_1.status == 0){
				/*NOTE: data port is opened; next: listen for remote connection*/
				DBG("datasocket - open ok\r\n");
 				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
				break;
			}
			
			/*waiting for connection || pNA is busy*/
			if( (p->TCPopen_1.status == 0xFFFF) ){
				DBG("datasocket - waiting to open socket\r\n");
				break;
			}

			DBG("datasocket - error=%d\r\n", p->TCPopen_1.status);
			
			bgSMRaiseError(psm, "TCPopen() error", 0, p->TCPopen_1.status);
		break;
		case 2:
			bgSMSetStateName(psm, "DataPort-Init");
			if(bgSMIsStateInit(psm)){
				p->sockData_Timer = 0;

				p->TCPserv_1.enable = 1;            		/* enables the FUB */
				p->TCPserv_1.ident = p->TCPopen_1.ident;    /* ident from open */

	    		p->TCPrecv_1.enable = 1;           	/* enables the FUB */
				p->rxDataBuff = 0;
				p->recvLenData = 0;
				p->TCPrecv_1.datamax = 10000;		/* size of the buffer should be > 8192*/		
    			TMP_alloc (p->TCPrecv_1.datamax, (void**)&p->rxDataBuff);   /*allocate memory for data rx*/
	    		p->TCPrecv_1.pData = (UDINT)p->rxDataBuff; /* address of the buffer */
	    		p->TCPclose_1.enable = 1;                  /* enables the FUB */
				
				DBG("data port open\r\n");
				break;

			}	
			
    		TcpServer(&p->TCPserv_1);					/*listen for a server connection request*/

			/*server connection accepted*/
			if(p->TCPserv_1.status == 0){
				/*NOTE: data port is ready if either connection is accepted or if waiting for connection*/
				DBG("datasocket - connected\r\n");
				p->TCPrecv_1.ident = p->TCPserv_1.identclnt;
				p->TCPclose_1.ident = p->TCPserv_1.identclnt;
				p->isDataPortReady = 1; 
 				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
				break;
			}
			
			/*waiting for connection || pNA is busy*/
			if( (p->TCPserv_1.status == 0xFFFF) ){
				DBG("datasocket - waiting connection\r\n");
				p->isDataPortReady = 1; 
				break;
			}

			DBG("datasocket - error=%d\r\n", p->TCPserv_1.status);
			
			bgSMRaiseError(psm, "TCPserv() error", 0, p->TCPserv_1.status);
		break;
		case 3:
			bgSMSetStateName(psm, "DataPort-Recv");
			/* recv in progress*/
           	TcpRecv(&p->TCPrecv_1);          

			/*recv timer*/
            if(p->sockData_Timer && (p->TCPrecv_1.status == 0xFFFF)){
    			p->sockData_TimerCounter++;
				DBG("data port recv() busy; counter=%d\r\n", p->sockData_TimerCounter);
            }

            if(p->TCPrecv_1.status == 0){
				/*test only; inc receive counter*/
				p->counter++;
			
				/*in case data arrives->copy data to user buffer*/
				if(p->TCPrecv_1.recvlen != 0){

					if(p->user.dataBufferLength < (p->recvLenData + p->TCPrecv_1.recvlen) ){
						DBG("!!!! rxBufferOverflow rxLen=%d > rxBufferSize=%d!!!!\r\n", (int)(p->recvLenData + p->TCPrecv_1.recvlen), (int)p->user.dataBufferLength);
						DBG("!!!! INCREASE USER RX BUFFER SIZE !!!!\r\n");
						/*user supplied buffer is too small!!!*/
						bgSMRaiseError(psm, "Data Buffer Overflow!", 0, bgSMGetState(psm));
						break;			
					}
					
					memcpy( (char*)(p->user.dataBuffer + p->recvLenData), p->rxDataBuff, p->TCPrecv_1.recvlen);
					p->recvLenData += p->TCPrecv_1.recvlen;
					
					p->sockData_TimerCounter = 0;
				}	
				else{
					/*Status == 0 && rxLengt == 0 -> remote peer has closed the connection!!!*/
					bgSMSetNextState(psm, bgSMGetState(psm) + 1);
				}
            }
			else 
			if(p->sockData_TimerCounter >= p->sockData_TimerTime){
				/*if receive timer expires, close data socket*/
				bgSMSetNextState(psm, bgSMGetState(psm) + 1);
			}	

			DBG("data recv(); rxLen=%d, rxStatus=%d, timer=%d \r\n", 
				(int)p->TCPrecv_1.recvlen, (int)p->TCPrecv_1.status, (int)p->sockData_TimerCounter);
		break;
		case 4:
			bgSMSetStateName(psm, "DataPort-Close");
            TcpClose(&p->TCPclose_1);          /* set the socket free */

			if(p->TCPclose_1.status == 0){
				p->isDataPortReady = 0; 
				p->sockData_Timer = 0;
				
				/*terminate rx buffer with 0 -> for strings*/	
				p->user.dataBuffer[p->recvLenData] = 0;
				
				bgSMRaiseDone(psm, 0);
			}	
			
			if( p->rxDataBuff ){
				p->rxDataBuff = 0;
				TMP_free (p->TCPrecv_1.datamax, (void**)p->rxDataBuff);
			}			
		break;
		default:
			/*unhandled state*/
			bgSMRaiseError(psm, "unknown state", 0, bgSMGetState(psm));
		break;	
	}

	if(bgSMOnError(psm)){
		p->TCPclose_1.ident = p->TCPserv_1.identclnt;    /* copy the  Ident number */
	    TcpClose(&p->TCPclose_1);          				/* set the socket free */
		p->isDataPortReady = 0; 

		if( p->rxDataBuff ){
			p->rxDataBuff = 0;
			TMP_free (p->TCPrecv_1.datamax, (void**)p->rxDataBuff);
		}
		/*raise error again for cmd level state machine*/
		bgSMRaiseError(psm, "Data Server Error", 0, 0);
 	}

	if(bgSMOnDone(psm)){
		
	}
}
/*
========================================================================================
*/
void swapBytesInShort(unsigned short *value)
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
/*
========================================================================================
*/
void setSocketAddressData(ETH_sockaddr_typ *adrData, char *hostIpAddressString, unsigned short hostPort)
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
/*
========================================================================================
========================================================================================
========================================================================================
========================================================================================
*/
/*EOF*/
