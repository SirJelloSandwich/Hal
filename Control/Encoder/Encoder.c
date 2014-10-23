/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: Encoder
 * File: Encoder.c10
 * Author: ISM
 * Created: March 20, 2013
 ********************************************************************
 * Implementation of program Encoder
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#define TRUE 1;
#define FALSE 0;

static void UpdateEncoder()
{

	/* Control encoder wheel speed and direction based on wireline depth data from IG 
	
	if (EncoderCtrl.CableDepth > FeedBackSim.rLineDepthFeedback)  //winch moving down
	{
		EncoderSimSpool.iEncSpeed = (EncoderSimSpool.rLineSpeed * 4444.44) ;
	}
	else if (EncoderCtrl.CableDepth < FeedBackSim.rLineDepthFeedback) // winch moving up
	{
		EncoderSimSpool.iEncSpeed = (EncoderSimSpool.rLineSpeed * -4444.44) ;
	}
	else
	{
		EncoderSimSpool.iEncSpeed = 0 ;
	}*/
	//internal loadcell sensor for the 43 and 46 panel are different.  We simulate one encoder for each panel.
	
	//Piecewise model for encoder speed
	
	EncoderSimSpool43.iEncSpeed = (EncoderSimSpool43.rLineSpeed * 1340414.041); //encoder for 43 panel 
	EncoderSimSpool46.iEncoder_46 = (EncoderSimSpool43.rLineSpeed * 859885.753); //encoder for 46 panel
	EncoderSimMag.iReelSpeed = (-EncoderSimSpool43.rLineSpeed * 1100.00); //encoder for BackUp Depth panel
	BackUpDepthReel.iReelEncoder = (-EncoderSimSpool43.rLineSpeed * 2222.22); //encoder for BUD rib counter

	
}
	

void _INIT EncoderINIT(void)
{
	/* TODO: Add code here */
}

void _CYCLIC EncoderCYCLIC(void)
{
	if (!IOSstatusData.Freeze)  //Stop Encoder if system set to freeze
	{
		EncoderSimSpool43.bEncEnable = TRUE;
		EncoderSimMag.bReelEnable = TRUE;
		UpdateEncoder();
	}
	else
	{
		EncoderSimSpool43.bEncEnable = FALSE;
		EncoderSimMag.bReelEnable = FALSE;
	}
	


}


void _EXIT EncoderEXIT(void)
{
	/* TODO: Add code here */
}
