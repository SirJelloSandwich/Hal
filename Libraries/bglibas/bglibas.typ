(********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Library: bglibas
 * File: bglibas.typ
 * Author: admin
 * Created: March 07, 2012
 ********************************************************************
 * Data types of library bglibas
 ********************************************************************)

TYPE
	BGSSM_CONST_XYZ : 
		(
		BGSSM_FLAG_COUNT := 5,
		BGSSM_EVENT_COUNT := 5,
		BGSSM_COUNTER_COUNT := 5,
		BGSSM_TIMER_COUNT := 5,
		BGSSM_REGISTER_COUNT := 5
		);
	bgFIFO_typ : 	STRUCT 
		iPtrPushData : UDINT;
		cfgEntrySize : UDINT;
		cfgMaxEntries : UDINT;
		oPtrPullData : UDINT;
		oFIFOCount : UDINT;
		oFIFOFull : UDINT;
		oPushIdx : UDINT;
		oPullIdx : UDINT;
		prvFIFOMemory : UDINT;
	END_STRUCT;
	bgSSM_typ : 	STRUCT 
		state : UDINT;
		stateNext : UDINT;
		stateRet : UDINT;
		stateOld : UDINT;
		stateText : STRING[32];
		pfunGetStateName : UDINT;
		pfunCyclicCallback : UDINT;
		stateInit : BOOL;
		error : UDINT;
		errorText : STRING[32];
		event : ARRAY[0..BGSSM_EVENT_COUNT]OF BOOL;
		flag : ARRAY[0..BGSSM_FLAG_COUNT]OF BOOL;
		counter : ARRAY[0..BGSSM_COUNTER_COUNT]OF UDINT;
		value : ARRAY[0..BGSSM_REGISTER_COUNT]OF UDINT;
		timer_x1ms : ARRAY[0..BGSSM_TIMER_COUNT]OF TON;
		dlytimer_x1ms : TON;
		dlyCycles : UDINT;
	END_STRUCT;
END_TYPE
