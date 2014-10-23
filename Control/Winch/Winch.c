/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: Winch
 * File: Winch.c
 * Author: ISM
 * Created: March 20, 2013
 ********************************************************************
 * Implementation of program Winch
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
//	#include <stdlib.h>
#endif

static void updateWinchAudio()
{
	
	if (IOSstatusData.Freeze)
	{
		gUdpAudioMsg.control[1] = 4;

	}
	else
	{
		gUdpAudioMsg.control[1] = 2;
		gUdpAudioMsg.volume[1] = (abs(WinchSim.iReelSpeed)) * 0.025;
		gUdpAudioMsg.freq[1] = (abs(WinchSim.iReelSpeed)) * 0.01 + 0.35;
	}
}

void _INIT WinchINIT(void)
{
	/* TODO: Add code here */
}

void _CYCLIC WinchCYCLIC(void)
{
	/* Read Winch control postion */
//	ilwinchinput = CtrlWinch.CtrlWinchRaw ;
	
	
	if (!IOSstatusData.Freeze)  //Stop Encoder if system set to freeze
	{
		updateWinchAudio();
	}
	else
	{
		gUdpAudioMsg.control[1] = 4; //Mute audio if in freeze
	}
	
#if 0	
	/* Control encoder wheel speed and direction based on wire line data from IG */
	
	if (ilwinchinput < 7850)  //winch moving down
	{
		EncoderSim.mov_Speed = (3900/ilwinchinput) * 10240000 ;
	}
	else if (ilwinchinput > 9400) // winch moving up
	{
		EncoderSim.mov_Speed = (ilwinchinput/14000) * -10240000 ;

	}
	else
	{
		EncoderSim.mov_Speed = 0 ;
	}
	
#endif
}

void _EXIT WinchEXIT(void)
{
	/* TODO: Add code here */
}
