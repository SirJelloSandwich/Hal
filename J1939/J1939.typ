(********************************************************************
 * COPYRIGHT -- HP
 ********************************************************************
 * Package: J1939
 * File: J1939.typ
 * Author: goran
 * Created: February 20, 2013
 ********************************************************************
 * Data types of package J1939
 ********************************************************************)

TYPE
	paiCAN_enums : 
		(
		paiCAN_msgByteSize := 8,
		paiCAN_msgByteLastIndex := 7,
		paiCAN_MaxIDCount := 20,
		paiJ1939PGN_EngineTemp := 16#FEEE,
		paiJ1939PGN_EEC1 := 16#F004,
		paiJ1939PGN_FluidPressure := 16#FEEF,
		paiJ1939PGN_ElectricalPower := 16#FEF7,
		paiJ1939PGN_WirelineDepth := 16#102,
		paiJ1939PGN_RexrothHMI181 := 16#181,
		paiJ1939PGN_RexrothHMI281 := 16#281,
		paiJ1939PGN_RexrothHMI381 := 16#381,
		paiJ1939PGN_RexrothHMI481 := 16#481,
		paiJ1939PGN_RexrothHMI581 := 16#581
		);
END_TYPE

(*==============================================*)
(*J1939 Generic Support*)
(*==============================================*)

TYPE
	paiCanMsgRxTx_typ : 	STRUCT 
		canRxId : UDINT;
		canRxData : ARRAY[0..paiCAN_msgByteLastIndex]OF USINT;
		canTxId : UDINT;
		canTxData : ARRAY[0..paiCAN_msgByteLastIndex]OF USINT;
	END_STRUCT;
	paiJ1939Private_typ : 	STRUCT 
		rxEvent : BOOL;
		rxCountError : UDINT;
		rxCount : UDINT;
		rxStatus : UDINT;
		txEvent : BOOL;
		txCountError : UDINT;
		txCount : UDINT;
		txStatus : UDINT;
		txTimer : TON;
	END_STRUCT;
	paiJ1939PGN_typ : 	STRUCT 
		PGN : UDINT; (*PGN (i.e. 0xFEEE (65262) )*)
		address : UDINT;
		tx : UDINT; (*0-PGN rx; 1-PGN tx*)
		txIntervalXms : UDINT;
		canIdentifier : UDINT; (*29 bit CAN identifier*)
		prv : paiJ1939Private_typ;
	END_STRUCT;
	paiJ1939DriverFBKs_typ : 	STRUCT 
		canOpen : CANopen;
		canWrite : CANwrite;
		canRead : CANread;
	END_STRUCT;
	paiJ1939Driver_typ : 	STRUCT 
		iCANIFString : STRING[63];
		prvCANRxTxData : paiCanMsgRxTx_typ;
		prvCANlib : paiJ1939DriverFBKs_typ;
	END_STRUCT;
END_TYPE

(*==============================================*)
(*Individual PGN items*)
(*==============================================*)

TYPE
	paiJ1939EngineTempData_typ : 	STRUCT  (*PGN=0xFEEE (65262)*)
		engineTemp : USINT; (*Resolution: 1degC/bit, Offset=-40degC Range: -40..+210, *)
		fuelTemp : USINT;
		engineOilTemp : UINT;
		turboOilTemp : UINT;
		engineIntercoolTemp : USINT;
		engineIntercoolThermoOpening : USINT;
	END_STRUCT;
	paiJ1939EngineTemp_typ : 	STRUCT 
		pgn : paiJ1939PGN_typ;
		data : paiJ1939EngineTempData_typ;
		engineTemp : REAL;
		engineOilTemp : REAL;
	END_STRUCT;
	paiJ1939EEC1Data_typ : 	STRUCT  (*PGN=0xF004 (61444)*)
		byte1NotUsed : SINT;
		byte2NotUsed : SINT;
		enginePercentTorque : USINT; (*Resolution: 1% Range: -125%..+125%, *)
		engineSpeedRPM : UINT; (*Resolution: 0.125 RPM/bit Range: 0..8031PRM *)
		byte6NotUsed : SINT;
		byte7NotUsed : SINT;
		byte8NotUsed : SINT;
	END_STRUCT;
	paiJ1939EEC1_typ : 	STRUCT 
		pgn : paiJ1939PGN_typ;
		data : paiJ1939EEC1Data_typ;
		rpm : REAL; (*RPM value as float*)
	END_STRUCT;
	paiJ1939FluidPressureData_typ : 	STRUCT  (*PGN=0xFEEF (65263)*)
		fuelDeliveryPressure : USINT; (*Resolution: 4kPa/bit, Offset:0  Range: 0..1000kPa*)
		extCrankcasePressure : USINT; (*Resolution: 0.05kPa/bit, Offset:0  Range: 0..12.5kPa*)
		engineOilLevel : USINT; (*Resolution: 0.4%/bit, Offset:0  Range: 1..100%*)
		engineOilPressure : USINT; (*Resolution: 4kPa/bit, Offset:0  Range: 0..1000kPa*)
		crankcasePressure : UINT; (*Resolution: 0.0078kPa/bit, Offset:-250kPa  Range:-250..251.99kPa*)
		coolantPressure : USINT; (*Resolution: 2kPa/bit, Offset:0  Range: 0..500kPa*)
		coolantLevel : USINT; (*Resolution: 0.4%/bit, Offset:0  Range: 0..100%*)
	END_STRUCT;
	paiJ1939FluidPressure_typ : 	STRUCT 
		pgn : paiJ1939PGN_typ;
		data : paiJ1939FluidPressureData_typ;
		engineOilPressure_psi : REAL;
	END_STRUCT;
	paiJ1939ElectricPowerData_typ : 	STRUCT  (*PGN=0xFEEF (65263)*)
		byte0NotUsed : SINT;
		byte1NotUsed : SINT;
		alternatorVoltage : UINT; (*Resolution: 0.05V/bit, Offset:0  Range: 0..3212V*)
		electricalVoltage : UINT; (*Resolution: 0.05V/bit, Offset:0  Range: 0..3212V*)
		batteryVoltage : UINT; (*Resolution: 0.05V/bit, Offset:0  Range: 0..3212V*)
	END_STRUCT;
	paiJ1939ElectricPower_typ : 	STRUCT 
		pgn : paiJ1939PGN_typ;
		data : paiJ1939ElectricPowerData_typ;
		batteryVoltage_V : REAL;
	END_STRUCT;
	paiJ1939WirelineDepthData_typ : 	STRUCT  (*PGN=0x102 *)
		depth : UDINT; (*Resolution: 0.1 feet/bit Range: +/-2^30 feet*)
		tension : UINT; (*Resolution: 1 Lb/bit Range: +/-32768Lbs *)
		lineSpeed : UINT; (*Resolution: 0.1 m/min/bit Range: +/-3276 feet/min *)
	END_STRUCT;
	paiJ1939RexrothHMI181Data_typ : 	STRUCT  (*PGN=0x181*)
		bottomHoleDepth : UDINT;
		endOfCasingDepth : UDINT;
	END_STRUCT;
	paiJ1939RexrothHMI281Data_typ : 	STRUCT  (*PGN=0x281*)
		slowSpeed : UDINT;
		lastChanceDepth : DINT;
	END_STRUCT;
	paiJ1939RexrothHMI381Data_typ : 	STRUCT  (*PGN=0x381*)
		maxOutofHolePosition : DINT;
	END_STRUCT;
	paiJ1939RexrothHMI481Data_typ : 	STRUCT  (*PGN=0x481*)
		cruiseCtrlSpd : UINT;
		fishingSpeedCtrl : UINT;
	END_STRUCT;
	paiJ1939RexrothHMI581Data_typ : 	STRUCT  (*PGN=0x581*)
		upperShevePosition : DINT;
		kellyBushingPosition : DINT;
	END_STRUCT;
	paiJ1939RexrothHMI181_typ : 	STRUCT 
		pgn : paiJ1939PGN_typ;
		data : paiJ1939RexrothHMI181Data_typ;
		bottomHoleDepth : REAL;
		endOfCasingDepth : REAL; (*depth value as float*)
	END_STRUCT;
	paiJ1939RexrothHMI281_typ : 	STRUCT 
		pgn : paiJ1939PGN_typ;
		data : paiJ1939RexrothHMI281Data_typ;
		slowSpeed : REAL;
		lastChanceDepth : REAL;
	END_STRUCT;
	paiJ1939RexrothHMI381_typ : 	STRUCT 
		pgn : paiJ1939PGN_typ;
		data : paiJ1939RexrothHMI381Data_typ;
		maxOutofHoleDepth : REAL;
	END_STRUCT;
	paiJ1939RexrothHMI481_typ : 	STRUCT 
		pgn : paiJ1939PGN_typ;
		data : paiJ1939RexrothHMI481Data_typ;
		cruiseCtrlSpd : REAL;
		fishingSpeedCtrl : REAL;
	END_STRUCT;
	paiJ1939RexrothHMI581_typ : 	STRUCT 
		pgn : paiJ1939PGN_typ;
		data : paiJ1939RexrothHMI581Data_typ;
		upperShevePosition : REAL;
		kellyBushingPostion : REAL;
	END_STRUCT;
	paiJ1939WirelineDepth_typ : 	STRUCT 
		pgn : paiJ1939PGN_typ;
		data : paiJ1939WirelineDepthData_typ;
		depth : REAL; (*depth value as float*)
		tension : REAL; (*tension value as float*)
		lineSpeed : REAL; (*line speed value as float*)
	END_STRUCT;
END_TYPE
