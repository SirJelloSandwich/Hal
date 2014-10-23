/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: LoadCell
 * File: LoadCell.c
 * Author: ISM
 * Created: March 20, 2013
 ********************************************************************
 * Implementation of program LoadCell
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

_LOCAL REAL lastTension;

static void updateLoadcell()
{
	//CtrlTension.crtl_Tension = CtrlTension.CtrlTensionRaw ;
/*	if(lastTension < LoadcellSim.rUdpLCTension - 10.0)  //if the next command frame is 10 units larger than the last ramp up the loadcell
	{
		LoadcellSim.rUdpLCTension ++;
		
	}
	else if(lastTension > LoadcellSim.rUdpLCTension + 10.0) //if the next command frame is 10 units smaller than the last ramp down the loadcell
	{
		LoadcellSim.rUdpLCTension --;
	}
	
	lastTension = LoadcellSim.rUdpLCTension;
*/
/*	if (LoadcellSim.rUdpLCTension > 11000)
	{
		lastTension = 11000;
	}
	else
	{
		lastTension = LoadcellSim.rUdpLCTension;
	}*/
	LoadcellSim43.iPanelLCTension43 = (INT) (LoadcellSim43.rUdpLCTension * 1.6437) + 313; //scale factor to drive the loadcell emulator from 0 to 10 volts
	LoadcellSim46.iPanelLCTension46 = (INT) (LoadcellSim43.rUdpLCTension * 1.50575); //scale factor for cased hole panel
}

void _INIT LoadCellINIT(void)
{
	/* TODO: Add code here */
}

void _CYCLIC LoadCellCYCLIC(void)
{
	updateLoadcell();
	/* Control Tension Readout based on toggle buttons on IOS */	
	

}

void _EXIT LoadCellEXIT(void)
{
	/* TODO: Add code here */
}
