/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: Panel_I_O
 * File: Panel_I_O.c
 * Author: ISM
 * Created: March 20, 2013
 ********************************************************************
 * Implementation of program Panel_I_O
 * Read Data feedback from individule winchman control panels
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#define TRUE  1
#define FALSE 0

static void readPanelIO()
{
	if ((SafetySim.iStopREEL_SD_46 < 18000) || (FeedBackSim.iStopREEL_SD_43 < 18000))
	{
		Shutdown.bReelshutdown = TRUE;
	}
	else
	{
		Shutdown.bReelshutdown = FALSE;
	}

}

void _INIT Panel_I_OINIT(void)
{
	/* TODO: Add code here */
}

void _CYCLIC Panel_I_OCYCLIC(void)
{
		readPanelIO();
	
		
}

void _EXIT Panel_I_OEXIT(void)
{
	/* TODO: Add code here */
}
