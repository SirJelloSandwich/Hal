/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: HydOilTemp
 * File: HydOilTemp.c
 * Author: ISM
 * Created: March 27, 2013
 ********************************************************************
 * Implementation of program HydOilTemp
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

void analoglimit(INT* px)
{
	INT x = *px;
	if (x < 0)  //Set lower limit to 0 volts
	{
		x=0;
	}
	
	if (x > 16383) //Set upper limit to 5 volts
	{
		x = 16383;
	}
	*px = x;
}
	
 void updateOilTemp()
{
	//Convert rHydTemp to deg F
	
	rHydOilTempf = (PanelOut.rHydOilTemp * 1.8) + 32.0;
	
//	x = (int) (PanelOut.rHydOilTempf * 10.0);
	
/*	if (x < 0)  //Set lower limit to 0 volts
	{
		x=0;
	}
	
	if (x > 16383) //Set upper limit to 5 volts
	{
		x = 16383;
	}*/
	
	analoglimit(&x);  //Bound the data between 0 and 5 volts
	
//	CurrentHydOilTemp.iHydOilTemp = x; 
	
	CurrentHydOilTemp.iHydOilTemp = 8200;
}
		
void _INIT HydOilTempINIT(void)
{
	/* TODO: Add code here */
	int rHydOilTempf = 0.0;
}

void _CYCLIC HydOilTempCYCLIC(void)
{
	if (!IOSstatusData.Freeze)  //Stop Temp meter if system set to freeze
	{
		updateOilTemp();
	}
}

void _EXIT HydOilTempEXIT(void)
{
	/* TODO: Add code here */
}
