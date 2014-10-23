/********************************************************************
 * COPYRIGHT -- HP
 ********************************************************************
 * Program: simmonly
 * File: simmonly.c
 * Author: goran
 * Created: March 21, 2013
 ********************************************************************
 * Implementation of program simmonly
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

void _INIT simonlyINIT(void)
{
	/* TODO: Add code here */
	
	
	//Overrid UDP Client operation settings (Use HMI if needed)
	//gUdpSystemCfg.udpClientMode = 1;//continues mode
	gUdpSystemCfg.udpClientUpdateRate = 5;//[Hz]
	strcpy(gUdpSystemCfg.udpRemoteHostIpAddress.text, "127.0.0.1");
	gUdpClient.clientIf.iTaskCycleTimeXms = 10;//udp client runs under 10ms cycle time
	
}

void _CYCLIC simonlyCYCLIC(void)
{
	/* TODO: Add code here */
		
}
