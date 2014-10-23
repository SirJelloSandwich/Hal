(********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * File: Global.typ
 * Author: Rob Byrne ISM
 * Created: March 20, 2013
 ********************************************************************
 * Global data types of project Winchmans_Simulator
 ********************************************************************)

TYPE
	OperatorsPanel_typ : 	STRUCT 
		bStopEmer : BOOL;
		bSlowSpd : BOOL;
		bSuperSlowSpd : BOOL;
		bWinchDisable : BOOL;
		bGearbox : BOOL;
		iWinchdead : INT;
		bBrake : BOOL;
		bRegen : BOOL;
		bDFN1 : BOOL;
		bDFN2 : BOOL;
		bHEST : BOOL;
		bSpoolerDisable : BOOL;
		iPivotCtrl : INT;
		iSpooler_UD_Crtl : INT;
		iSpooler_LR_Ctrl : INT;
		iCaliperBrakeCtrl : INT;
		iLineTensionCtrl : INT;
		iSuperSlowSpdCtrl : INT;
		iWinchCtrl : INT;
		rHydOilTemp : REAL;
		rEngVoltage : REAL;
		rEngCoolTemp : REAL;
		rEngOilPress : REAL;
		rEngRPM : REAL;
		rEngOilTemp : REAL;
	END_STRUCT;
	System_typ : 	STRUCT 
		rSysPressure : REAL;
		rChargePressure : REAL;
		rGenCharge : REAL;
		bReelshutdown : BOOL;
	END_STRUCT;
	BD_Panel_typ : 	STRUCT 
		iReelEncoder : DINT;
		iHydOilTemp : INT;
		bReelEnable : BOOL;
		iReelSpeed : DINT;
		iReelPosition : DINT;
	END_STRUCT;
	CasedHolePanel_typ : 	STRUCT 
		iStopREEL_SD_46 : INT;
		rCCL_46 : BOOL;
		bIOSenabled43 : BOOL;
		iPanelLCTension46 : INT;
		iEncoder_46 : DINT;
	END_STRUCT;
	OpenHolePanel_typ : 	STRUCT 
		bEncEnable : BOOL;
		iEncSpeed : DINT;
		iEncPosition : DINT;
		iStopREEL_SD_43 : INT;
		iTensionFeedback_43 : INT;
		rUdpLCTension : REAL;
		bIOSenabled46 : BOOL;
		rLineSpeed : REAL;
		rLineDepthFeedback : REAL;
		iPanelLCTension43 : INT;
	END_STRUCT;
	PowerHandle_typ : 	STRUCT 
		bSafetyPanel : BOOL;
		b43Panel : BOOL;
		b46Panel : BOOL;
		b12VDC : BOOL;
	END_STRUCT;
	CtrlIOScmd_typ : 	STRUCT 
		Freeze : INT;
		Running : INT;
		ICsetnum : INT;
		ICscenarioIdx : INT;
		ICreset : INT;
		CurrentMode : UDINT;
	END_STRUCT;
	CtrlIOSstatus_typ : 	STRUCT 
		CurrentTOD : INT;
		Freeze : INT;
		Running : INT;
		ICsetnum : INT;
		ICscenarioIdx : INT;
		ICreset : INT;
		CurrentMode : INT;
		crashactive : BOOL;
	END_STRUCT;
	CalCtrl_typ : 	STRUCT 
		iLineTensionStop : INT;
		iCaliperBrakeStop : INT;
		iSpoolerRtDead : INT;
		iSpoolerLtDead : INT;
		iSpoolerDnDead : INT;
		iSpoolerUpDead : INT;
		iWinchUpCollar : INT;
		iWinchFullDn : INT;
		iWinchDnCollar : INT;
		iWinchFullUp : INT;
		iCaliperBrakeMax : INT;
		iLineTensionMax : INT;
	END_STRUCT;
END_TYPE
