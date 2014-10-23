/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: Input_Output
 * File: Input_Output.c
 * Author: ISM
 * Created: March 20, 2013
 ********************************************************************
 * Implementation of program Input_Output
 * Read/Write data to/from control switches and indicators on the main operators panel
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#define TRUE  1
#define FALSE 0
REAL fLocalholder = 0.0;


static void panelLogic()
{

	//Normalize and deadband analog control stick data to be sent via UDP
	
	/*	if (PanelIn.iPivotCtrl < 8320)
	{
	gAnalogCtrl.fPivotCtrl   = (REAL) (PanelIn.iPivotCtrl/Pivot_Ctrl_max - 0.615) * -2.579 ;
	}
	else if (PanelIn.iPivotCtrl > 8460)
	{
	gAnalogCtrl.fPivotCtrl   = (REAL) (PanelIn.iPivotCtrl/Pivot_Ctrl_max - 0.615) * -2.579 ;
	}
	else
	{*/
	gAnalogCtrl.fPivotCtrl = 0.0;  // Pivot is set to neutral until the skid view  
//}
	
	
	if (PanelIn.iSpooler_LR_Ctrl < CalibrationCtrl.iSpoolerLtDead)
	{
		gAnalogCtrl.fSpoolerLR        = (REAL) (PanelIn.iSpooler_LR_Ctrl/Spooler_LR_max -0.61) * -3.0;
	}
	else if (PanelIn.iSpooler_LR_Ctrl > CalibrationCtrl.iSpoolerRtDead)
	{
		gAnalogCtrl.fSpoolerLR        = (REAL) (PanelIn.iSpooler_LR_Ctrl/Spooler_LR_max -0.61) * -3.0;
	}
	else
	{
		gAnalogCtrl.fSpoolerLR = 0.0;
	}
	
	if (PanelIn.iSpooler_UD_Crtl < CalibrationCtrl.iSpoolerUpDead)
	{
		gAnalogCtrl.fSpoolerUD        = (REAL) ((PanelIn.iSpooler_UD_Crtl/Spooler_UD_max -0.61)*3.0);
	}
	else if (PanelIn.iSpooler_UD_Crtl > CalibrationCtrl.iSpoolerDnDead)
	{
		gAnalogCtrl.fSpoolerUD        = (REAL) ((PanelIn.iSpooler_UD_Crtl/Spooler_UD_max -0.61)*3.0);
	}
	else
	{
		gAnalogCtrl.fSpoolerUD = 0.0;
	}

	if (CalibrationCtrl.iCaliperBrakeMax == 0)  //Do not allow a divide by zero
	{
		CalibrationCtrl.iCaliperBrakeMax = 13600; //Set to large normalization vlaue if iCaliperBrakeMax is not yet set
	}
	if (PanelIn.iCaliperBrakeCtrl < CalibrationCtrl.iCaliperBrakeStop)
	{
		gAnalogCtrl.fCaliperBrakeKnob = 0.0;
	}
	else
	{
		//gAnalogCtrl.fCaliperBrakeKnob = (REAL) (((PanelIn.iCaliperBrakeCtrl / CalibrationCtrl.iCaliperBrakeMax)) - 0.62) * 2.5;
		fLocalholder  = (REAL) PanelIn.iCaliperBrakeCtrl / CalibrationCtrl.iCaliperBrakeMax;
		gAnalogCtrl.fCaliperBrakeKnob = (fLocalholder - 0.62) * 2.7;
	}
	
	if (CalibrationCtrl.iLineTensionMax == 0)  //Do not allow a divide by zero
	{
		CalibrationCtrl.iLineTensionMax = 13600; //Set to large normalization vlaue if iCaliperBrakeMax is not yet set
	}
	if (PanelIn.iLineTensionCtrl < CalibrationCtrl.iLineTensionStop)
	{
		gAnalogCtrl.fLineTensionKnob = 0.0;
	}
	else
	{
		//gAnalogCtrl.fLineTensionKnob  = (REAL) (((PanelIn.iLineTensionCtrl / CalibrationCtrl.iLineTensionMax) - 0.62)*2.7);
		fLocalholder  = (REAL) PanelIn.iLineTensionCtrl / CalibrationCtrl.iLineTensionMax;
		gAnalogCtrl.fLineTensionKnob = (fLocalholder - 0.62) * 2.7;
	}
	
	
	
	if (PanelIn.iSuperSlowSpdCtrl < 3)
	{
		
		gAnalogCtrl.fSuperSlowSpd     = 0.0;
	}
	else
	{
		gAnalogCtrl.fSuperSlowSpd     = (REAL) (PanelIn.iSuperSlowSpdCtrl/Super_Slow_Spd_Ctrl_max);
	}
	
	
//set deadband and handle winch control handle input.  Define the speed control adjustment from the HMI panel.  Multiply the
// effectivness of the winch control by the HMI speed control.
	if (J1939Rex481.fishingSpeedCtrl == 0)
	{
		rHMIspeedControl = 1.0;
		gAnalogCtrl.fWinchUD = rWinchInput * rHMIspeedControl;
	}
	else
	{
		rHMIspeedControl = J1939Rex481.fishingSpeedCtrl / 100.00;
		gAnalogCtrl.fWinchUD = rWinchInput * rHMIspeedControl;
	}
	
//Winch positions with variables that are set from the calibration HMI page
	if ((PanelIn.iWinchCtrl < CalibrationCtrl.iWinchUpCollar) && (PanelIn.iWinchCtrl > CalibrationCtrl.iWinchFullUp))  //Winch handle moving up
	{
		if (PanelIn.iWinchCtrl < CalibrationCtrl.iWinchUpCollar - 125)
		{
			rWinchInput = (REAL) ((PanelIn.iWinchCtrl * -0.00023) + 1.8448);
		}
		else if ((PanelIn.iWinchCtrl < lastWinch - 175) || (PanelIn.iWinchCtrl > lastWinch + 175)) //filter noise about the first 10%
		{
			rWinchInput = (REAL) ((PanelIn.iWinchCtrl * -0.00023) + 1.8448);
			lastWinch = PanelIn.iWinchCtrl;
		}
	}
	
	if (PanelIn.iWinchCtrl < CalibrationCtrl.iWinchFullUp) // Capture winch handle at ~90%
	{
		if ((PanelIn.iWinchCtrl < lastWinch - 150) || (PanelIn.iWinchCtrl > lastWinch + 150)) //filter noise about the last 10%
		{
			rWinchInput = (REAL) ((PanelIn.iWinchCtrl * -0.00023) + 1.8448);
			lastWinch = PanelIn.iWinchCtrl;
		}
	}
	if ((PanelIn.iWinchCtrl > CalibrationCtrl.iWinchDnCollar) && (PanelIn.iWinchCtrl < CalibrationCtrl.iWinchFullDn)) //Winch handle moving down
	{
		if (PanelIn.iWinchCtrl < CalibrationCtrl.iWinchDnCollar - 125)
		{
			rWinchInput = (REAL) ((PanelIn.iWinchCtrl * -0.00024) + 2.158019);
		}
		else if ((PanelIn.iWinchCtrl < lastWinch - 75) || (PanelIn.iWinchCtrl > lastWinch + 75)) //filter noise about the first 10%
		{
			rWinchInput = (REAL) ((PanelIn.iWinchCtrl * -0.00024) + 2.158019);
			lastWinch = PanelIn.iWinchCtrl;
		}
	}
	if (PanelIn.iWinchCtrl >CalibrationCtrl.iWinchFullDn)
	{
		if ((PanelIn.iWinchCtrl < lastWinch - 225) || (PanelIn.iWinchCtrl > lastWinch + 225)) //filter noise about the first 10%
		{
			rWinchInput = (REAL) ((PanelIn.iWinchCtrl * -0.00024) + 2.158019);
			lastWinch = PanelIn.iWinchCtrl;
		}
	}

	
	if ((PanelIn.iWinchCtrl < CalibrationCtrl.iWinchDnCollar) && (PanelIn.iWinchCtrl > CalibrationCtrl.iWinchUpCollar) && (indeadband)) //Winch handle in dead band
	{
		rWinchInput = 0.0; // in deadband
		indeadband = FALSE;
	}
	
	if ((PanelIn.iWinchCtrl < CalibrationCtrl.iWinchDnCollar - 200) && (PanelIn.iWinchCtrl > CalibrationCtrl.iWinchUpCollar + 200))
	{
		rWinchInput = 0.0;
		indeadband = TRUE; //reset deadband
	}
	
	
		//Regen model... Currently there is no model for regen.  Turn on lights with switch is set.  Will be defined in phase 2
	
		/*	if (PanelIn.bRegen)
		{
		PanelOut.bDFN1 = TRUE;
		PanelOut.bDFN2 = TRUE;
		PanelOut.bHEST = TRUE;
		}
		else
		{
		PanelOut.bDFN1 = FALSE;
		PanelOut.bDFN2 = FALSE;
		PanelOut.bHEST = FALSE;
		}*/

	
	}
	

 void setBit ()
{
	n[0] = PanelIn.bStopEmer;
	n[1] = PanelIn.bRegen;
	n[2] = PanelIn.bSpoolerDisable;
	n[3] = 0; //Place holder for Spooler Button 2
	n[4] = PanelIn.bSlowSpd;
	n[5] = PanelIn.bSuperSlowSpd;
	n[6] = PanelIn.bWinchDisable;
	n[7] = 0; //Place holder for Winch Button 2
	n[8] = PanelIn.bGearbox;
	n[9] = PanelIn.bBrake;
	n[10] = 1;
//	n[10] = gbusPower;
	n[11] = IOSstatusData.ICreset;
	n[12] = IOSstatusData.Freeze;
	n[13] = Shutdown.bReelshutdown;
	n[14] = gHmi.key.lightsDrum;
	n[15] = gHmi.key.lightsRig;
	n[16] = WellDriving.bNoReIC;
	
	for (i=0; i<iPanelIn_buffer; i++) //Set bit in gStream for max variables of PanelIn.  gstream is also written in other modules.
	{
		gStream |= n[i] << i;// |= bitwise inclusive OR assignment. result is 1 if either bit is 1 and 0 only when both bits are 0. | is called a pipe.
		// << is a left shift.   It shifts each bit in its left-hand operand to the left by the number of positions indicated by the right-hand operand
	}
	
}


 void clearBit ()  //Invert all inputs inorder to leave bit set high when true
{
	n[0] = !PanelIn.bStopEmer;
	n[1] = !PanelIn.bRegen;
	n[2] = !PanelIn.bSpoolerDisable;
	n[3] = 0; //Place holder for Spooler Button 2 or future expansion
	n[4] = !PanelIn.bSlowSpd;
	n[5] = !PanelIn.bSuperSlowSpd;
	n[6] = !PanelIn.bWinchDisable;
	n[7] = 0; //Place holder for Winch Button 2 or future expansion
	n[8] = !PanelIn.bGearbox;
	n[9] = !PanelIn.bBrake;
	n[10] = 0;
//	n[10] = !gbusPower;
	n[11] = !IOSstatusData.ICreset;
	n[12] = !IOSstatusData.Freeze;
	n[13] = !Shutdown.bReelshutdown;
	n[14] = !gHmi.key.lightsDrum;
	n[15] = !gHmi.key.lightsRig;
	n[16] = !WellDriving.bNoReIC;

	
	for (i=0; i<iPanelIn_buffer; i++) //Set bit in gStream for max variables of PanelIn.  gstream is also written in other modules.
	{
		gStream &= ~(n[i] << i);
	}
	
}
void _INIT Input_OutputINIT(void)
{
	iPanelIn_buffer = 17;			    //Number of digital inputs on the main operators panel
	Pivot_Ctrl_max = 13200.00; 		    //Normalize fPivotCtrl to send via UPD
	Spooler_LR_max = 13600.00; 			//Normalize fSpoolerLR to send via UPD
	Spooler_UD_max = 13600.00; 			//Normalize fSpoolerUD to send via UPD
//	Caliper_Brake_Ctrl_max = 13415.00; 	//Normalize fCaliperBrake to send via UPD
//	Line_Tension_Ctrl_max = 13600.00; 	//Normalize fLineTension to send via UPD
	Super_Slow_Spd_Ctrl_max = 32767.00; //Normalize fSuperSlowSpd to send via UPD
	lastWinch = PanelIn.iWinchCtrl;
	
}

void _CYCLIC Input_OutputCYCLIC(void)
{
	
	setBit();     //Set ndi integer to send out via UDP
	clearBit();   //Cleat ndi integer to send out via UDP
	panelLogic(); //Set globals based on data from operators panel
	
}

void _EXIT Input_OutputEXIT(void)
{
	/* TODO: Add code here */
}
