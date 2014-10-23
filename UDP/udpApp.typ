(********************************************************************
 * COPYRIGHT -- HP
 ********************************************************************
 * File: Global.typ
 * Author: goran
 * Created: January 23, 2013
 ********************************************************************
 * Global data types of project Wireline
 ********************************************************************)

TYPE
	applicationGlobal_enums : 
		(
		eXSIM_TO_IOS := 1,
		eIOS_TO_XSIM := 101,
		eXSIM_OFFLINE := 0,
		eXSIM_RUNNING,
		eXSIM_SIM_FREEZE,
		eXSIM_IC_RESET,
		eXSIM_IG_OFFLINE,
		eIOS_OFFLINE := 0,
		eIOS_RUNNING,
		eIOS_SIM_FREEZE,
		eSC_NONE := 0,
		eSC_Freewheeling,
		eSC_Bridgeout,
		eSC_Lubricators,
		eSC_Sluffing,
		eSC_Accel_Top_Sheave,
		eSC_Accel_Top_Depth,
		eSC_Hole_Deviation,
		eSC_Loss_Of_12V,
		eSC_Loss_Of_Hyd,
		eSC_Loss_Of_Elect,
		eSC_Caliper_Brk_Fail,
		eSC_Gearbox_Brk_Fail,
		eSC_Backlash,
		eSC_Loss_Of_Tension,
		eSC_Reel_Shutdown,
		eSC_Tool_Push,
		eSC_Pump_Down_Tool,
		eSC_NUM_Scenarios
		);
END_TYPE

(*==========================*)
(*==== UDP Command Message ====*)
(*==========================*)

TYPE
	ctrlInterfaceCmd_typ : 	STRUCT 
		nPacketSize : INT; (*Packet size*)
		nPacketID : INT; (*Packet ID (see enum above)*)
		nFrameCnt : DINT; (*Frame count*)
		nXsimStatus : DINT; (*IOS status*)
		fEngSpd : REAL; (*Throttle control*)
		fEngOilTemp : REAL; (*Pivot control*)
		fEngOilPress : REAL;
		fEngCoolTemp : REAL;
		fEngECM : REAL;
		fHydOilTemp : REAL; (*Spoole left/right control*)
		fCableDepth : REAL; (*Spooler up/down control*)
		fLineTension : REAL; (*Caliper brake control*)
		fLineSpeed : REAL;
		fWinchSpd : REAL; (*Line tension control*)
		fSysPress : REAL; (*Super slow spped control*)
		fChargePress : REAL;
		fGenCharge : REAL;
		fCableDrumPos : REAL;
	END_STRUCT;
END_TYPE

(*==========================*)
(*==== UDP Status Message ====*)
(*==========================*)

TYPE
	ctrlInterfaceStatusAudio_typ : 	STRUCT 
		header : UDINT; (*Packet size*)
		COMMAND : UDINT;
		control : ARRAY[0..3]OF UDINT; (*Packet ID (see enum above)*)
		run : ARRAY[0..5]OF UDINT; (*Packet ID (see enum above)*)
		Args : STRING[99];
		volume : ARRAY[0..3]OF REAL; (*Frame count*)
		pan : ARRAY[0..3]OF REAL; (*IOS status*)
		freq : ARRAY[0..3]OF REAL; (*Bit field*)
		trailer : UDINT; (*IC Set number*)
	END_STRUCT;
	ctrlInterfaceStatus_typ : 	STRUCT 
		nPacketSize : INT; (*Packet size*)
		nPacketID : INT; (*Packet ID (see enum above)*)
		nFrameCnt : DINT; (*Frame count*)
		nIOSStatus : DINT; (*IOS status*)
		ndi : DINT; (*Bit field*)
		nICSetIndex : INT; (*IC Set number*)
		nScenarioIndex : INT; (*Scenario number*)
		nTOD : INT;
		fThrottleCtrl : REAL; (*Throttle control*)
		fPivotCtrl : REAL; (*Pivot control*)
		fSpoolerLR : REAL; (*Spoole left/right control*)
		fSpoolerUD : REAL; (*Spooler up/down control*)
		fCaliperBrakeKnob : REAL; (*Caliper brake control*)
		fLineTensionKnob : REAL; (*Line tension control*)
		fSuperSlowSpd : REAL; (*Super slow spped control*)
		fWinchUD : REAL; (*Winch up/down control*)
		fBottomHoleSet : REAL; (*value of Bottom of Hole from HMI*)
		fCasingHoleSet : REAL; (*value of Bottom of Casing from HMI*)
		fLastChanceSet : REAL; (*value of Last Chance from HMI*)
		fMaxOutHoleSet : REAL; (*value of Out of Hole max from HMI*)
		fCruiseCtrl : REAL; (*value of Cruise Ctrl from HMI*)
		fUpperSheave : REAL; (*value of Upper Sheave from HMI*)
		fKellyBushing : REAL; (*value of Kelly Bushing from HMI*)
	END_STRUCT;
END_TYPE
