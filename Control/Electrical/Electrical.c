/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: Electrical
 * File: Electrical.c
 * Author: ISM
 * Created: March 20, 2013
 ********************************************************************
 * Implementation of program Electrical
 ********************************************************************/

#include <bur/plctypes.h>
#include <stdbool.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

static void BusPower()
{
	PowerSim.b12VDC       = gbusPower;
	PowerSim.b43Panel     = !IOSstatus43.bIOSenabled43;
	PowerSim.b46Panel     = !IOSstatus46.bIOSenabled46;
	PowerSim.bSafetyPanel = gbusPower;	
}

//Move Engine to its own thread once the engine model is invoked
static void updateEngine()
{
	//since no engine model is currently required set the engine rpm audio to constant 1500 rpm
	if (IOSstatusData.Freeze)
	{
		gUdpAudioMsg.control[0] = 4;
	}
	else
	{
		gUdpAudioMsg.control[0] = 2;
		gUdpAudioMsg.volume[0] = 0.90;
		gUdpAudioMsg.freq[0] = 0.65;
	}
}	

void _INIT ElectricalINIT(void)
{
	/* TODO: Add code here */
}

void _CYCLIC ElectricalCYCLIC(void)
{
 
	if (!IOSstatusData.Freeze)  //Stop Encoder if system set to freeze
	{
   		BusPower();
		updateEngine();
	}
	else
	{
		gUdpAudioMsg.control[0] = 4; //Mute audio if in freeze
	}
}

void _EXIT ElectricalEXIT(void)
{
	/* TODO: Add code here */
}
