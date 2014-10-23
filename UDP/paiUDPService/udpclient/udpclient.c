/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Program: udpclient
 * File: udpclient.c
 * Author: goran
 * Created: January 23, 2013
 ********************************************************************
 * Implementation of program udpclient
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif



#include <../../../bglib/bgstring.c>

enum{
	udpCliZero = 0,
	udpCliInit,
	udpCliOpen,
	udpCliConnect,
	udpCliIdle,
	udpCliTxData,
	udpCliDisconnect,
	udpCliClose,
	udpCliError,
	udpCliLast
}STATEMACHINE;

static const char* udpCliStateName(int state)
{
static const char* stateNames[] = {
"Zero",
"Init",
"Open",
"Connect",
"Idle",
"TxData",
"Disconnect",
"Close",
"Error",
};

	if( state >= udpCliLast)
		return "unknow";
	
	return stateNames[state];
};


/*
###############################################################################
Utilities
###############################################################################
*/

static void convertIPTextToDigits(udpIPAddress_typ* p)
{
unsigned char txtbuffer[64];
bgStringList_typ list;

	strcpy((void*)txtbuffer, (void*)(p->text));
	list.src = txtbuffer;

	bgStringSplitLine(&list, '.');
	if(list.itemCount != 4){
		p->digit3 = 0;
		p->digit2 = 0;
		p->digit1 = 0;
		p->digit0 = 0;
		return;
	}
	p->digit3 = (USINT)atoi((UDINT)list.items[0]);
	p->digit2 = (USINT)atoi((UDINT)list.items[1]);
	p->digit1 = (USINT)atoi((UDINT)list.items[2]);
	p->digit0 = (USINT)atoi((UDINT)list.items[3]);
}


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



/*
###############################################################################
Functions
###############################################################################
*/

static void udpClientSMCyclic(udpClient_typ *p)
{
	bgSSM_typ* pSm = &p->sm;


	//operate timer(s)
	bgTimerCyclic(&pSm->timer_x1ms[0]); // delay timer

	//TimerExecute(&pSm->timer2x1ms); // update rate timer (in continues mode)
	//IMPORANT: for high speed, can't use TON block > rather, use cycle time
	if(pSm->counter[2] )
		pSm->counter[2]--;
	
	bgTimerCyclic(&pSm->timer_x1ms[2]); // 1sec stats timer

	//stats timers
	if( bgTimerIsTimeOut(&pSm->timer_x1ms[2]) )
	{
		bgTimerStart(&pSm->timer_x1ms[2], 1000);
		DBG(lgr, "==========1sec tick");

		//values for cummulative (per min) calculations for datagrams
		p->txStats.datagramPerSec = pSm->counter[0];
		pSm->counter[0] = 0;
		
		if(p->txStats.datagramPerSec > p->txStats.datagramPerSecMax)
		{
			p->txStats.datagramPerSecMax = p->txStats.datagramPerSec;
		}
		
		//values for cummulative (per min) calculations for bytes
		p->txStats.bytesPerSec = pSm->counter[1];
		pSm->counter[1] = 0;
		
		if(p->txStats.bytesPerSec > p->txStats.bytesPerSecMax)
		{
			p->txStats.bytesPerSecMax = p->txStats.bytesPerSec;
		}
	}



	/*execute control state machine cyclic handler*/		
	bgSSMCyclic(pSm);
	
	//show change of state in a log
	if( bgSSMIsStateInit(pSm) )
	{
		DBG(lgr, "sm=%s(%d)", bgSSMGetStateText(pSm), bgSSMGetState(pSm));
	}
		
	switch( bgSSMGetState(pSm) )
	{
		//========================================
		case udpCliZero:
			bgSSMStateNext(pSm, udpCliInit);
		break;
		//========================================
		case udpCliInit:
			if(p->clientIf.iConnect)
			{
				bgSSMStateNext(pSm, udpCliOpen);
			}			
		break;
		//========================================
		case udpCliOpen:

			if( bgSSMIsStateInit(pSm) )
			{				
				p->client.open.enable = 1;
				p->client.open.port = 0;
				p->client.open.pIfAddr = 0;
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
				p->client.open.options = 0;
#warning "ISM Project - ARsim target!"
#else
				//For all other (real) targets, use socket options
				p->client.open.options = udpOPT_REUSEPORT | udpOPT_BROADCAST;
#warning "ISM Project - real target!"
#endif
				return;
			}			

			if(p->client.open.status == ERR_FUB_BUSY)
				return;

			p->client.open.enable = 0;
			
			if(p->client.open.status == ERR_OK)
			{
				p->client.recv.ident = p->client.open.ident;
				p->client.send.ident = p->client.open.ident;
				p->client.close.ident = p->client.open.ident;
				p->client.connect.ident = p->client.open.ident;
				p->client.disconnect.ident = p->client.open.ident;
				p->client.ioctl.ident = p->client.open.ident;
				bgSSMStateNext(pSm, udpCliConnect);
				return;
			}

			DBG(lgr,"Err udp.open=%d", p->client.open.status);
			bgSSMStateNext(pSm, udpCliError);
			
		break;
		//========================================
		case udpCliConnect:

			if( bgSSMIsStateInit(pSm) )
			{				
				p->client.connect.enable = 1;
				p->client.connect.port = p->clientIf.iRmtUdpPort;
				p->client.connect.pHost = (UDINT)p->clientIf.iRmtIPAddr;
				return;
			}			

			if(p->client.connect.status == ERR_FUB_BUSY)
				return;

			//p->client.connect.enable = 0;
			
			if(p->client.connect.status == ERR_OK)
			{
				strcpy(p->clientIf.oRmtIpAddr, p->clientIf.iRmtIPAddr);
				p->clientIf.oRmtUdpPort = p->clientIf.iRmtUdpPort;
				
				bgSSMStateNext(pSm, udpCliIdle);
				return;
			}

			DBG(lgr,"Err udp.connect=%d", p->client.connect.status);
			bgSSMStateNext(pSm, udpCliError);
			
		break;
		//========================================
		case udpCliIdle:

			if( bgSSMIsStateInit(pSm) )
			{				
				
			}			


			//udp client in echo mode!? 
			//check for iSendEcho set by the server upon it receives a message > client sends out to the same host
			if(!p->clientIf.iMode && p->clientIf.iSendEcho)
			{
				p->clientIf.iSendEcho = 0;
				p->clientIf.iSend = 1;
				strcpy(p->clientIf.iRmtIPAddr, p->clientIf.iSendEchoRmtIpAddr);
			}
			
			//udp client in continues mode!? 
			//trigger send update at a configured rate to a defined (sipplied to the inputs) remote host/port
			if(p->clientIf.iMode)
			{

				if(!pSm->counter[2])
				{
					int updateTimeMs;
					
					if( p->clientIf.iUpdateRate )
					{
						updateTimeMs = 1000 /  p->clientIf.iUpdateRate;
					}
					else
					{
						updateTimeMs = 1000;
					}

					if(updateTimeMs < 10)
						updateTimeMs = 12;
					
					if(updateTimeMs > 1000)
						updateTimeMs = 1000;
					
					if(p->clientIf.iTaskCycleTimeXms)
					{
						pSm->counter[2]= updateTimeMs / p->clientIf.iTaskCycleTimeXms;
					}
					else
					{
						pSm->counter[2]= updateTimeMs / gUdpClient_TaskCycleTimeXms; // devide by task clas time to find out increments					
					}
						
					p->clientIf.iSend = 1;
				}


			}


			//monitor for change in remote host settings
			//in case of a change > reconnect (disconncet then connect)
			if((p->clientIf.oRmtUdpPort != p->clientIf.iRmtUdpPort) ||	strcmp(p->clientIf.oRmtIpAddr, p->clientIf.iRmtIPAddr))
			{
				DBG(lgr,"rmt settings changed. Triggering reconnect.");
				bgSSMStateNext(pSm, udpCliDisconnect);
				return;
			}

			//monitor request to send 
			if(p->clientIf.iSend)
			{
				p->clientIf.iSend = 0;


				if(p->clientIf.iTxDataLen > UDP_CLIENT_TXBUF_SIZE)
				{
					p->clientIf.iTxDataLen = UDP_CLIENT_TXBUF_SIZE;
				}

				if(!p->clientIf.iTxDataAdr && !p->clientIf.iTxCallback)
				{
					DBG(lgr,"udp.send nothing to tx");
					return;
				}

				
				//execute memcpy if datasource is defined
				if(p->clientIf.iTxDataAdr)
				{
					memcpy((void*)(p->txbuf), (void*)(p->clientIf.iTxDataAdr), p->clientIf.iTxDataLen);
				}

				//execute callback to enable app to populate data before sending
				//NOTE: callback will overwrite effect of the above memcpy if both a defined!!!
				if(p->clientIf.iTxCallback)
				{
					void (*txCallback)(UDINT pBuf, UDINT lenTxBuf, UDINT* pLenData) =  ( void(*) (UDINT, UDINT, UDINT*) ) p->clientIf.iTxCallback;
					(*txCallback)( (UDINT)&p->txbuf[0], UDP_CLIENT_TXBUF_SIZE, &p->clientIf.iTxDataLen);
				}

				p->client.send.enable = 1;
				p->client.send.pData = (UDINT)&p->txbuf[0];
				p->client.send.datalen = p->clientIf.iTxDataLen;

				//stats
				p->txStats.datagramCountTotal++;
				p->txStats.bytesTotal += p->client.send.datalen;

				//values for cummulative (per min) calculations
				pSm->counter[0]++;
				pSm->counter[1] +=p->client.send.datalen;


				
				bgSSMStateNext(pSm, udpCliTxData);
				return;
			}
			
		break;
		//========================================
		case udpCliTxData:
			if( bgSSMIsStateInit(pSm) )
			{				
				
			}			

			if(p->client.send.status == ERR_FUB_BUSY)
				return;

			p->client.send.enable = 0;
			
			if(p->client.send.status == ERR_OK)
			{
				p->clientIf.oSendDone = 1;
				DBG(lgr,"udp.send OK");
				bgSSMStateNext(pSm, udpCliIdle);
				return;
			}
		

			DBG(lgr,"Err udp.send=%d", p->client.send.status);
			bgSSMStateNext(pSm, udpCliError);
			
		break;
		//========================================
		case udpCliDisconnect:

			if( bgSSMIsStateInit(pSm) )
			{				
				p->client.connect.enable = 0;
				p->client.disconnect.enable = 1;
				return;				
			}			

			if(p->client.disconnect.status == ERR_FUB_BUSY)
				return;

			p->client.disconnect.enable = 0;
			
			if(p->client.disconnect.status == ERR_OK)
			{
				bgSSMStateNext(pSm, udpCliClose);
				return;
			}


			DBG(lgr,"Err udp.disconnect=%d", p->client.disconnect.status);
			bgSSMStateNext(pSm, udpCliClose);
			
		break;
		//========================================
		case udpCliClose:
			
			if( bgSSMIsStateInit(pSm) )
			{				
				DBG(lgr,"UDP Client close");
				p->client.close.enable = 1;				
				return;
			}


			if(p->client.close.status == ERR_FUB_BUSY)
				return;

			if(p->client.close.status)
			{
				DBG(lgr,"Err udp.close=%d", p->client.close.status);
			}

			p->client.close.enable = 0;				

			bgSSMStateNext(pSm, udpCliZero);			
		break;
		//========================================
		case udpCliError:


			if( bgSSMIsStateInit(pSm) )
			{
				DBG(lgr,"UDP Client Error State");
				
				//start error recovery timer
				bgTimerStart(&pSm->timer_x1ms[0], 5000);
			}


			if( bgTimerIsTimeOut(&pSm->timer_x1ms[0]) )
		{
				//after timer exp > try to close the port and see if it recovers
				bgSSMStateNext(pSm, udpCliDisconnect);			
			}
			
		break;
		//========================================
		default:
		break;
	}
	
}


static void udpClientInit(udpClient_typ *p)
{
	bgSSM_typ* pSm = &p->sm;
	
	bgTimerStart(&p->sm.timer_x1ms[2], 1000);
	
	bgSSMInit(pSm, (UDINT)udpCliStateName, 0);
	
}


static void udpClientCyclicExt(udpClient_typ *p)
{

	//udp client state machine
	udpClientSMCyclic(p);


	//udp client fbk-s
	UdpOpen(&p->client.open);
	UdpClose(&p->client.close);
	UdpConnect(&p->client.connect);
	UdpDisconnect(&p->client.disconnect);
	UdpSend(&p->client.send);
	UdpRecv(&p->client.recv);
	UdpIoctl(&p->client.ioctl);
}


/*
###############################################################################
Main entry: Init & Cyclic Functions
###############################################################################
*/


void _INIT udpclientINIT(void)
{
	/* TODO: Add code here */
	
	//init logger object
	DBGINIT(lgr, log, LOG_ENTRY_COUNT, LOG_ENTRY_LENGTH);

	udpClientInit(&gUdpClient);

	udpClientInit(&gUdpClientAudio);

	//upd client application specific init
	gUdpClient.clientIf.iConnect = 1;
	gUdpClientAudio.clientIf.iConnect = 1;


	//RESTORE SETTINGS FROM THE REMANENT STORAGE
	//sanity check for remote host ip address
	convertIPTextToDigits(&gUdpSystemCfg.udpRemoteHostIpAddress);
	
	if(!gUdpSystemCfg.udpRemoteHostIpAddress.digit0)
	{
		strcpy(gUdpSystemCfg.udpRemoteHostIpAddress.text, gUdpClient_DefaultHostAdr);
		convertIPTextToDigits(&gUdpSystemCfg.udpRemoteHostIpAddress);		
	}
	strcpy(gUdpClient.clientIf.iRmtIPAddr, gUdpSystemCfg.udpRemoteHostIpAddress.text);

	strcpy(gUdpClientAudio.clientIf.iRmtIPAddr, gUdpSystemCfg.udpRemoteHostIpAddress.text);
	
	
	//sanity check for remote host UDP port number
	if(!gUdpSystemCfg.udpRemoteHostPortNumber){
		gUdpSystemCfg.udpRemoteHostPortNumber = gUdpClient_DefaultPort;
	}
	gUdpClient.clientIf.iRmtUdpPort = gUdpSystemCfg.udpRemoteHostPortNumber;

	gUdpClientAudio.clientIf.iRmtUdpPort = 5000;

	// udpClientMode
	// 0-echo mode (send status after receiving a command from the host. Uses received IP address and UDP port number configured here.
	// 1-continues mode (send status independantly to configured 	remoteIpAddress. remoteUDP port at the configure UDP rate
	if(gUdpSystemCfg.udpClientMode > 1){
		gUdpSystemCfg.udpClientMode = 0;
	}
	gUdpClient.clientIf.iMode = gUdpSystemCfg.udpClientMode;

	gUdpClientAudio.clientIf.iMode = 1;

	// udpClientUpdateRate
	// rate to send outgoing status to remote host in case of UDP client mode 1
	// 
	if(gUdpSystemCfg.udpClientUpdateRate < 1){
		gUdpSystemCfg.udpClientUpdateRate  = 1;
	}
	if(gUdpSystemCfg.udpClientUpdateRate > 100){
		gUdpSystemCfg.udpClientUpdateRate  = 100;
	}
	gUdpClient.clientIf.iUpdateRate = gUdpSystemCfg.udpClientUpdateRate;

	gUdpClientAudio.clientIf.iUpdateRate = gUdpSystemCfg.udpClientUpdateRate;
	
	gUdpClient.clientIf.iSend = 0;

	gUdpClientAudio.clientIf.iSend = 0;

}

void _CYCLIC udpclientCYCLIC(void)
{
	/* TODO: Add code here */

	//udp client cyclic
	udpClientCyclicExt(&gUdpClient);

	udpClientCyclicExt(&gUdpClientAudio);
	
	//in continues client mode, use localy configured settings!
	if(gUdpSystemCfg.udpClientMode &&
		((gUdpSystemCfg.udpClientMode != gUdpClient.clientIf.iMode) ||
		(gUdpClient.clientIf.iUpdateRate != gUdpSystemCfg.udpClientUpdateRate) ||
		strcmp(gUdpClient.clientIf.iRmtIPAddr, gUdpSystemCfg.udpRemoteHostIpAddress.text))
	){
		gUdpClient.clientIf.iMode = gUdpSystemCfg.udpClientMode;
		gUdpClient.clientIf.iUpdateRate = gUdpSystemCfg.udpClientUpdateRate;
 		strcpy(gUdpClient.clientIf.iRmtIPAddr, gUdpSystemCfg.udpRemoteHostIpAddress.text);
	}

	//in echo mode, just monitor for iMode change
	if(!gUdpSystemCfg.udpClientMode &&
		((gUdpSystemCfg.udpClientMode != gUdpClient.clientIf.iMode) )
	){
		gUdpClient.clientIf.iMode = gUdpSystemCfg.udpClientMode;
	}


	//in both modes use configured port number for the remote UDP host
	gUdpClient.clientIf.iRmtUdpPort = gUdpSystemCfg.udpRemoteHostPortNumber;

}
