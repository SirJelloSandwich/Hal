/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Program: udpserv
 * File: udpserv.c
 * Author: goran
 * Created: October 05, 2011
 ********************************************************************
 * Implementation of program udpserv
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif



enum{
	udpServZero = 0,
	udpServEnable,
	udpServInit,
	udpServIdle,
	udpServRxData,
	udpServClose,
	udpServError,
	udpServLast
}STATEMACHINE;

static const char* udpServStateName(int state)
{
static const char* stateNames[] = {
"Zero",
"Enable",
"Init",
"Idle",
"RxData",
"Close",
"Error",
};

	if( state >= udpServLast)
		return "unknow";
	
	return stateNames[state];
};


/*
###############################################################################
Utilities
###############################################################################
*/

/**
 Simple logging
 Log goes to an array of PV strings > see _LOCAL STRING log[][] variable below
 Log output is visible via Automation Studio watch window
 Example of a log statement used within the code is: 
 DBG(lgr, "This is log example. %d ", value1);
 To complety disable logging from this source file, change next line to: #if 0
*/
#if 1
#include <../../../bglib/bgPVLogger.c>
#define LOG_ENTRY_COUNT 20	//max number of log entries
#define LOG_ENTRY_LENGTH  32 //max length of individual log message. Longer msg are truncated
_LOCAL STRING log[LOG_ENTRY_COUNT][LOG_ENTRY_LENGTH]; //log output visible from Automation Studio!
static bgPVLogger_typ lgr;	//PV logger object
#else
#define DBG(loggerInstance, format, args...) ;
#define DBGINIT(loggerInstance, logoutput, logEntryCount, logEntrySize)  ;
#endif



void bitSet(UDINT* dest,  UINT bitIndex, BOOL bitValue)
{
	UDINT mask = 0x1;
	UDINT current = *dest;

	mask <<= bitIndex;


	if(!(bitValue && 0x1) ){
		mask = ~(mask);
		current &= mask;
	}else{
		current |= mask;
	}

	*dest = current;
}


/*
###############################################################################
Functions
###############################################################################
*/

static void udpServerSMCyclic(udpServer_typ *p)
{
	bgSSM_typ* pSm = &p->sm;


	//operate timer(s)
	bgTimerCyclic(&pSm->timer_x1ms[0]);
	bgTimerCyclic(&pSm->timer_x1ms[1]);//rx timeout timer
	bgTimerCyclic(&pSm->timer_x1ms[2]);//1 sec tick timer

	p->serverIf.oRxIsTimeout = bgTimerIsTimeOut(&pSm->timer_x1ms[1]);
		
	//stats timers
	if( bgTimerIsTimeOut(&pSm->timer_x1ms[2]) )
	{
		bgTimerStart(&pSm->timer_x1ms[2], 1000);

		//values for cummulative (per min) calculations for datagrams
		p->rxStats.datagramPerSec = pSm->counter[0];
		pSm->counter[0] = 0;
		if(p->rxStats.datagramPerSec > p->rxStats.datagramPerSecMax)
		{
			p->rxStats.datagramPerSecMax = p->rxStats.datagramPerSec;
		}
		
		//values for cummulative (per min) calculations for bytes
		p->rxStats.bytesPerSec = pSm->counter[1];
		pSm->counter[1] = 0;
		if(p->rxStats.bytesPerSec > p->rxStats.bytesPerSecMax)
		{
			p->rxStats.bytesPerSecMax = p->rxStats.bytesPerSec;
		}
	}


	/*execute control state machine cyclic handler*/		
	bgSSMCyclic(pSm);
	//show change of state in a log
	if( bgSSMIsStateInit(pSm) )
	{
		DBG(lgr, "sm=%s(%d)", bgSSMGetStateText(pSm), bgSSMGetState(pSm));
	}
		
	switch(bgSSMGetState(pSm))
	{
		//========================================
		case udpServZero:
			bgSSMStateNext(pSm, udpServEnable);
			p->serverIf.oError = 0;
			p->serverIf.oRxReady = 0;
			
		break;
		//========================================
		case udpServEnable:
			if(p->serverIf.iEnable)
			{
				bgSSMStateNext(pSm, udpServInit);
			}
		break;
		//========================================
		case udpServInit:

			if( bgSSMIsStateInit(pSm) )
			{
				
				p->server.open.enable = 1;
				p->server.open.port = p->serverIf.iUdpServerPort;
				p->server.open.pIfAddr = 0;
//IMPORTANT:
//in order for the following conditional compilation to work properly, PLC object in the Configuration view
//must contain following defintion:
//Folder: Build
//Field: Additional Build Options
//Value: -D $(AS_CONFIGURATION)
//Using the above, name of the current AS configuration (i.e. realhw1 or testsim or else) becomes 
//a defined preprocessor name. As a result, it's possible to destinguish between different configurations! 
#ifdef testsim
				//In case of testsim (ARsim) target, don't use socket options
				p->server.open.options = 0;
#warning "Project - UDP - ARsim target!"
#else
				//For all other (real) targets, use socket options
				p->server.open.options = udpOPT_REUSEPORT;
#warning "Project - UDP - NOT an ARsim target!"
#endif
				return;
			}			

			if(p->server.open.status == ERR_FUB_BUSY)
				return;

			p->server.open.enable = 0;
			
			if(p->server.open.status == ERR_OK)
			{
				p->server.recv.ident = p->server.open.ident;
				p->server.send.ident = p->server.open.ident;
				p->server.close.ident = p->server.open.ident;
				p->server.connect.ident = p->server.open.ident;
				p->server.disconnect.ident = p->server.open.ident;
				p->server.ioctl.ident = p->server.open.ident;
				bgSSMStateNext(pSm, udpServIdle);
				return;
			}

			DBG(lgr,"Err udp.open=%d", p->server.open.status);
			bgSSMStateNext(pSm, udpServError);
			
		break;
		//========================================
		case udpServIdle:

			if( bgSSMIsStateInit(pSm) )
			{
				p->server.recv.enable = 1;
				p->server.recv.pData = (UDINT)&p->rxbuf[0];
				p->server.recv.datamax = UDP_SERVER_RXBUF_SIZE;
				p->server.recv.pIpAddr = (UDINT)&p->rmtIPAddr[0];
				p->server.recv.flags = 0;

				bgTimerStart(&pSm->timer_x1ms[1], p->serverIf.iRxTimeoutXms);

				return;
			}

			//Disabling UDP server currently not supported. Disabling (i.e. change port) requires reset
			//if(!p->serverIf.iEnable){
			//	stateNext(pSm, udpServClose);
			//}

			
			if((p->server.recv.status == ERR_FUB_BUSY) || (p->server.recv.status == udpERR_NO_DATA))
			{
				return;
			}
			
			p->server.recv.enable = 0;

			if(p->server.recv.status == ERR_OK)
			{

				DBG(lgr,"udp.recv OK!");
				DBG(lgr,"rmtIP=%s", p->rmtIPAddr);
				DBG(lgr,"rmtPort=%d", p->server.recv.port);
				DBG(lgr,"recvlen=%d", p->server.recv.recvlen);

				//stats
				p->rxStats.datagramCountTotal++;
				p->rxStats.bytesTotal += p->server.recv.recvlen;

				//values for cummulative (per min) calculations
				pSm->counter[0]++;
				pSm->counter[1] +=p->server.recv.recvlen;
				

				
				bgSSMStateNext(pSm, udpServRxData);
				return;
			}

			DBG(lgr,"Err udp.recv=%d", p->server.recv.status);
			bgSSMStateNext(pSm, udpServError);

			
		break;
		//========================================
		case udpServRxData:

			if( bgSSMIsStateInit(pSm) )
			{				
				DBG(lgr,"udp.Rx event!");
				
				p->serverIf.oRxReady = 1;
				p->serverIf.oRxDataLen = p->server.recv.recvlen;
				p->serverIf.oRxDataAdr = (UDINT)p->rxbuf;
				p->serverIf.oRxRmtPort = p->server.recv.port;
				strcpy(p->serverIf.oRxRmtIP, p->rmtIPAddr);

				//execute rx callback to enable app to process received data
				if(p->serverIf.iRxCallback)
				{
					void (*rxCallback)(UDINT pBuf, UDINT rxLen) =  ( void(*) (UDINT, UDINT) ) p->serverIf.iRxCallback;
					(*rxCallback)( (UDINT)p->rxbuf, p->server.recv.recvlen);
				}

				DBG(lgr,"rmtIP=%s", p->serverIf.oRxRmtIP);
				DBG(lgr,"rmtPort=%d", p->serverIf.oRxRmtPort);
				DBG(lgr,"recvlen=%d", p->serverIf.oRxDataLen);				
			}

			bgSSMStateNext(pSm, udpServIdle);
			
		break;
		//========================================
		case udpServClose:

			if( bgSSMIsStateInit(pSm) )
			{				
				DBG(lgr,"UDP Server close");
				p->server.close.enable = 1;				
				p->server.recv.enable = 0;
				return;
			}


			if(p->server.close.status == ERR_FUB_BUSY)
			{
				return;
			}

			if(p->server.close.status)
			{
				DBG(lgr,"Err udp.close=%d", p->server.close.status);
			}

			p->server.close.enable = 0;				

			bgSSMStateNext(pSm, udpServZero);	
			
		break;
		//========================================
		case udpServError:

			if( bgSSMIsStateInit(pSm) )
			{
				DBG(lgr,"UDP Server Error State");
				
				//start error recovery timer
				bgTimerStart(&pSm->timer_x1ms[0], 5000);
			}


			if( bgTimerIsTimeOut(&pSm->timer_x1ms[0]) )
			{
				//after timer exp > try to close the port and see if it recovers
				bgSSMStateNext(pSm, udpServClose);			
			}
			
		break;
		//========================================
		default:
		break;
	}



}


static void udpServerInit(udpServer_typ *p)
{
	bgSSM_typ* pSm = &p->sm;
	
	memset(p, 0, sizeof(udpServer_typ));
	
	bgTimerStart(&p->sm.timer_x1ms[2], 1000);
	
	bgSSMInit(pSm, (UDINT)udpServStateName, 0);
	
}


static void udpServerCyclic(udpServer_typ *p)
{
	//udp server state machine cyclic
	udpServerSMCyclic(p);

	//udp server fbk-s
	UdpOpen(&p->server.open);
	UdpClose(&p->server.close);
	UdpConnect(&p->server.connect);
	UdpDisconnect(&p->server.disconnect);
	UdpSend(&p->server.send);
	UdpRecv(&p->server.recv);
	UdpIoctl(&p->server.ioctl);

}


/*
###############################################################################
Main entry: Init & Cyclic Functions
###############################################################################
*/
void _INIT udpservINIT(void)
{
	/* TODO: Add code here */

	//init logger object
	DBGINIT(lgr, log, LOG_ENTRY_COUNT, LOG_ENTRY_LENGTH);

	//udp server init	
	udpServerInit(&gUdpServer);

	//application specific udp server init	
	gUdpServer.serverIf.iEnable = 1;

	//check config value (stored in remanent memory, configured via HMI) and set the UDP port number of the server
	if(!gUdpSystemCfg.udpLocalServerPortNumber)
	{
		gUdpSystemCfg.udpLocalServerPortNumber = gUdpServer_DefaultPort;
	}
	
	gUdpServer.serverIf.iUdpServerPort = gUdpSystemCfg.udpLocalServerPortNumber;

}

void _CYCLIC udpservCYCLIC(void)
{
	/* TODO: Add code here */

	//udp server cyclic
	udpServerCyclic(&gUdpServer);
}
