/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Program: appUDP
 * File: appUDP.c
 * Author: goran
 * Created: Jan 25, 2013
 ********************************************************************
 * Implementation of program appUDP
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

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
IMPORTANT:
==========
DBGINIT() statement MUST BE PLACED as a first statement within the task _INIT function!
Example:
void _INIT myTaskINIT(void)
{
	//TODO: Add code here
	//init logger object
	DBGINIT(lgr, log, LOG_ENTRY_COUNT, LOG_ENTRY_LENGTH);
}
*/
#if 1
#include <../../bglib/bgPVLogger.c>
#define LOG_ENTRY_COUNT 20	//max number of log entries
#define LOG_ENTRY_LENGTH  32 //max length of individual log message. Longer msg are truncated
_LOCAL STRING log[LOG_ENTRY_COUNT][LOG_ENTRY_LENGTH]; //log output visible from Automation Studio!
static bgPVLogger_typ lgr;	//PV logger object
#else
#define DBG(loggerInstance, format, args...) ;
#define DBGINIT(loggerInstance, logoutput, logEntryCount, logEntrySize)  ;
#endif


//UDP Server Receive function
static void udpServerCallback(UDINT pRxBuffer, UDINT lenRxData)
{
	//process received udp data
	ctrlInterfaceCmd_typ* pMsgCommand = (ctrlInterfaceCmd_typ*)pRxBuffer;

	if( lenRxData > sizeof(ctrlInterfaceCmd_typ) )
	{
		DBG(lgr, "lenRxData(%d) > maxSize(%d)", lenRxData, sizeof(ctrlInterfaceCmd_typ));	
		return;
	}
	
	if( pMsgCommand->nPacketID != eXSIM_TO_IOS )
	{
		return;
	}
		
	memcpy(&gUdpCmdMsg, pMsgCommand, sizeof(ctrlInterfaceCmd_typ) );	
}


//UDP Server Receive function
static void udpServerCallbackAfterReceive(UDINT pRxBuffer, UDINT lenRxData)
{
	udpServerCallback(pRxBuffer, lenRxData);
}


//UDP Client Send Callback function
static void udpClientCallback(UDINT pTxBuffer, UDINT lenTxBuffer, UDINT* pTxDataLen)
{
		
	ctrlInterfaceStatus_typ* pMsgStatus = (ctrlInterfaceStatus_typ*)pTxBuffer;

	//check the size of the tx buffer
	if((sizeof(ctrlInterfaceStatus_typ) > lenTxBuffer) || !pTxBuffer )
	{
		return;
	}
	
	//application specific stuff
	gUdpStatusMsg.nFrameCnt++;
	gUdpStatusMsg.nPacketSize = sizeof(ctrlInterfaceStatus_typ);
	
	memcpy(pMsgStatus, &gUdpStatusMsg, sizeof(ctrlInterfaceStatus_typ) );

	//define a total lenght of the message
	(*pTxDataLen) = sizeof(ctrlInterfaceStatus_typ);
}

//UDP Client Send Callback function
static void udpClientCallbackBeforeSend(UDINT pTxBuffer, UDINT lenTxBuffer, UDINT* pTxDataLen)
{
	udpClientCallback(pTxBuffer, lenTxBuffer, pTxDataLen);
}


static void udpClientAudioCallback(UDINT pTxBuffer, UDINT lenTxBuffer, UDINT* pTxDataLen)
{
		
	ctrlInterfaceStatusAudio_typ* pMsgStatus = (ctrlInterfaceStatusAudio_typ*)pTxBuffer;

	//check the size of the tx buffer
	if((sizeof(ctrlInterfaceStatusAudio_typ) > lenTxBuffer) || !pTxBuffer )
	{
		return;
	}
	
	//msg specific stuff that does not change
	gUdpAudioMsg.header = 0x1234ABCD;
//	gUdpAudioMsg.header = 0xCDAB3412;
	gUdpAudioMsg.trailer = 0x1E1E1E1E;
	
	memcpy(pMsgStatus, &gUdpAudioMsg, sizeof(ctrlInterfaceStatusAudio_typ) );
	
	//define a total lenght of the message
	(*pTxDataLen) = sizeof(ctrlInterfaceStatusAudio_typ);
}

static void udpClientAudioCallbackBeforeSend(UDINT pTxBuffer, UDINT lenTxBuffer, UDINT* pTxDataLen)
{
	udpClientAudioCallback(pTxBuffer, lenTxBuffer, pTxDataLen);
}
//application specific - map UDP commands to application PV-s
static void udpMsgMappingCmdtoApp()
{
	IOScmdData.CurrentMode = gUdpCmdMsg.nXsimStatus;
	PanelOut.rEngRPM = gUdpCmdMsg.fEngSpd;
	PanelOut.rEngOilTemp = gUdpCmdMsg.fEngOilTemp;
	PanelOut.rEngOilPress = gUdpCmdMsg.fEngOilPress;
	PanelOut.rEngCoolTemp = gUdpCmdMsg.fEngCoolTemp;
	PanelOut.rEngVoltage = gUdpCmdMsg.fEngECM;
	PanelOut.rHydOilTemp = gUdpCmdMsg.fHydOilTemp;
	EncoderCtrl.CableDepth = gUdpCmdMsg.fCableDepth;
	LoadcellSim43.rUdpLCTension = gUdpCmdMsg.fLineTension;
	EncoderSimSpool43.rLineSpeed = gUdpCmdMsg.fLineSpeed;
	WinchSim.iReelSpeed = gUdpCmdMsg.fWinchSpd;
	SysStatusTruck.rSysPressure = gUdpCmdMsg.fSysPress;
	SysStatusTruck.rChargePressure = gUdpCmdMsg.fChargePress;
	SysStatusTruck.rGenCharge = gUdpCmdMsg.fGenCharge;
	WellDriving.spoolFlange = gUdpCmdMsg.fCableDrumPos;
}		

//application specific - map application PV-s to UDP status
static void udpMsgMappingApptoStatus()
{
	gUdpStatusMsg.nPacketID = eIOS_TO_XSIM;
	gUdpStatusMsg.nIOSStatus = IOSstatusData.CurrentMode;
	gUdpStatusMsg.ndi = gStream;
	gUdpStatusMsg.nICSetIndex = IOSstatusData.ICsetnum;    
	gUdpStatusMsg.nScenarioIndex = IOSstatusData.ICscenarioIdx;
	gUdpStatusMsg.nTOD = IOSstatusData.CurrentTOD - 1;
	gUdpStatusMsg.fThrottleCtrl = 0;//Place holder for throttle control rpm
	gUdpStatusMsg.fPivotCtrl = gAnalogCtrl.fPivotCtrl;
	gUdpStatusMsg.fSpoolerLR = gAnalogCtrl.fSpoolerLR;
	gUdpStatusMsg.fSpoolerUD = gAnalogCtrl.fSpoolerUD;
	gUdpStatusMsg.fCaliperBrakeKnob = gAnalogCtrl.fCaliperBrakeKnob;
	gUdpStatusMsg.fLineTensionKnob = gAnalogCtrl.fLineTensionKnob;
	gUdpStatusMsg.fSuperSlowSpd = gAnalogCtrl.fSuperSlowSpd;
	
	 
	gUdpStatusMsg.fWinchUD = gAnalogCtrl.fWinchUD;
	gUdpStatusMsg.fBottomHoleSet = J1939Rex181.bottomHoleDepth;
	gUdpStatusMsg.fCasingHoleSet = J1939Rex181.endOfCasingDepth;
	gUdpStatusMsg.fLastChanceSet = J1939Rex281.lastChanceDepth;
	gUdpStatusMsg.fMaxOutHoleSet = J1939Rex381.maxOutofHoleDepth;
	gUdpStatusMsg.fCruiseCtrl = J1939Rex481.cruiseCtrlSpd;
	gUdpStatusMsg.fUpperSheave = J1939Rex581.upperShevePosition;
	gUdpStatusMsg.fKellyBushing = J1939Rex581.kellyBushingPostion;	

}

static void	udpTimeOutHandler()
{
	//application logic - an example of handling a UDP receive timeout event 
	static BOOL oldRxTimeout;
	if( gUdpServer.serverIf.oRxIsTimeout && !oldRxTimeout){
		DBG(lgr, "udp server rx timeout");	
	}
	if( !gUdpServer.serverIf.oRxIsTimeout && oldRxTimeout){
		DBG(lgr, "udp server rx ok");	
	}	
	oldRxTimeout = gUdpServer.serverIf.oRxIsTimeout;
}

/*
###############################################################################
Task execution entry points: Init & Cyclic Functions
###############################################################################
*/
void _INIT udpAppINIT(void)
{
	/* TODO: Add code here */
	//init logger object
	DBGINIT(lgr, log, LOG_ENTRY_COUNT, LOG_ENTRY_LENGTH);

	//Define application specific UDP client tx callback function 
	gUdpClient.clientIf.iTxCallback = (UDINT)udpClientCallbackBeforeSend;

	gUdpClientAudio.clientIf.iTxCallback = (UDINT)udpClientAudioCallbackBeforeSend;
	
	//Define application specific UDP server rx callback function 
	gUdpServer.serverIf.iRxCallback = (UDINT)udpServerCallbackAfterReceive;
	gUdpServer.serverIf.oRxDataLenMax = (UDINT)sizeof(ctrlInterfaceCmd_typ);
	
	//Adjust UDP client/server settings according to the application needs (i.e. use HMI if needed)
	//For other defaults see:
	//paiUDPService.var (i.e. defined UDP related const)
	//udpserv.c/udpservINIT() and udpclient.c/udpclientINIT()
	gUdpSystemCfg.udpClientMode = 0;//echo mode
	gUdpSystemCfg.udpClientUpdateRate = 60;//[Hz]
	
	//strcpy(gUdpSystemCfg.udpRemoteHostIpAddress.text, "192.168.100.31");
	//strcpy(gUdpSystemCfg.udpRemoteHostIpAddress.text, "192.168.100.171");
	//strcpy(gUdpSystemCfg.udpRemoteHostIpAddress.text, "192.168.100.255");
	strcpy(gUdpSystemCfg.udpRemoteHostIpAddress.text, "192.168.2.100");
	gUdpClient.clientIf.iTaskCycleTimeXms = 2;//udp client runs under 10ms cycle time
	gUdpClient.clientIf.iUpdateRate = 60;//60Hz update rate
	gUdpServer.serverIf.iRxTimeoutXms = 4000;//udp server rx timeout time

	gUdpClientAudio.clientIf.iTaskCycleTimeXms = 2;//udp client runs under 10ms cycle time
	gUdpClientAudio.clientIf.iUpdateRate = 60;//60Hz update rate
		
	//Application specific UDP messaging 
	gUdpStatusMsg.nFrameCnt = 0;
	gUdpCmdMsg.nFrameCnt = 0;	
	
}

void _CYCLIC udpAppCYCLIC(void)
{
	/* TODO: Add code here */
		
	//application logic - on send complete, copy status into last statusSent
	if(gUdpClient.clientIf.oSendDone)
	{
		gUdpClient.clientIf.oSendDone = 0;		
		//save last transmitted data
	}


	if(gUdpClientAudio.clientIf.oSendDone)
	{
		gUdpClientAudio.clientIf.oSendDone = 0;		
		//save last transmitted data
	}
	
	//application logic - en example of handling a UDP rx event	
	if(gUdpServer.serverIf.oRxReady)
	{
		gUdpServer.serverIf.oRxReady = 0;
		
		//use server rx event to trigger udp send to host
		gUdpClient.clientIf.iSendEcho = 1;
		strcpy(gUdpClient.clientIf.iSendEchoRmtIpAddr, gUdpServer.serverIf.oRxRmtIP);
		
		//map command message into application specific variables
		udpMsgMappingCmdtoApp();		
	}
	
	
	//map application specific variables into status message
	udpMsgMappingApptoStatus();		
	
	//handle a UDP rx timeout
	udpTimeOutHandler();
}
//EOF

