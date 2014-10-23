(********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Library: bglibas
 * File: bglibas.fun
 * Author: admin
 * Created: March 07, 2012
 ********************************************************************
 * Functions and function blocks of library bglibas
 ********************************************************************)
(*=========================================================================*)
(*SUPPORT FOR SIMPLE STATE MACHINE (SSM)*)
(*=========================================================================*)

FUNCTION bgSSMInit : UDINT (*Init state machine. MUST be called once, before any other bgSSM function call.*)
	VAR_INPUT
		p : bgSSM_typ;
		pfunGetStateName : UDINT;
		pfunCyclicCallback : UDINT;
	END_VAR
END_FUNCTION

FUNCTION bgSSMCyclic : UDINT (*State machine cyclic operation.*)
	VAR_INPUT
		p : bgSSM_typ;
	END_VAR
END_FUNCTION

FUNCTION bgSSMStateNextTimeDelay : UDINT (*Set SM next state w. timer delay*)
	VAR_INPUT
		p : bgSSM_typ;
		stateNext : UDINT;
		timeDelayX1ms : UDINT;
	END_VAR
END_FUNCTION

FUNCTION bgSSMStateNextCycleDelay : UDINT (*Set SM next state w. cycles delay*)
	VAR_INPUT
		p : bgSSM_typ;
		stateNext : UDINT;
		delayCycles : UDINT;
	END_VAR
END_FUNCTION

FUNCTION bgSSMStateNext : UDINT (*Set SM next state.*)
	VAR_INPUT
		p : bgSSM_typ;
		stateNext : UDINT;
	END_VAR
END_FUNCTION

FUNCTION bgSSMIsStateInit : BOOL (*Check if first scan within the current state.*)
	VAR_INPUT
		p : bgSSM_typ;
	END_VAR
END_FUNCTION

FUNCTION bgSSMGetState : UDINT (*Get current SM state*)
	VAR_INPUT
		p : bgSSM_typ;
	END_VAR
END_FUNCTION

FUNCTION bgSSMGetStateText : STRING[32] (*Get current SM state name*)
	VAR_INPUT
		p : bgSSM_typ;
	END_VAR
END_FUNCTION
(*=========================================================================*)
(*SUPPORT FOR FIFOs*)
(*=========================================================================*)

FUNCTION bgFIFOInit : UDINT (*Init simple FIFO. Stores upto (entryTotalCount) number of elements. Elements MUST be of the same size.*)
	VAR_INPUT
		p : bgFIFO_typ;
		entrySize : UDINT;
		entryTotalCount : UDINT;
		pFIFOmem : UDINT;
	END_VAR
END_FUNCTION

FUNCTION bgFIFOPush : UDINT (*Push data to FIFO. Content from pSrc is copied into the available FIFO memory. In case of error return value is != 0*)
	VAR_INPUT
		p : bgFIFO_typ;
		pSrc : UDINT;
		srcSize : UDINT;
	END_VAR
END_FUNCTION

FUNCTION bgFIFOPull : UDINT (*Pull (retrive) data from FIFO. Oldest entry from FIFO memory is copied into the provided pDst space. In case of error return value is != 0*)
	VAR_INPUT
		p : bgFIFO_typ;
		pDst : UDINT;
		dstSize : UDINT;
	END_VAR
END_FUNCTION

FUNCTION bgFIFOGetCount : UDINT (*Get number of entries stored in the FIFO*)
	VAR_INPUT
		p : bgFIFO_typ;
	END_VAR
END_FUNCTION

FUNCTION bgFIFOIsFull : UDINT (*Check if FIFO is full*)
	VAR_INPUT
		p : bgFIFO_typ;
	END_VAR
END_FUNCTION
(*=========================================================================*)
(*Support for TIMERs*)
(*=========================================================================*)

FUNCTION bgTimerCyclic : UDINT (*Timer cyclic function. Must be called cyclicly for corret timer operation.*)
	VAR_INPUT
		pTON : TON;
	END_VAR
END_FUNCTION

FUNCTION bgTimerStart : UDINT (*Start timer for a defined amount of time [ms]. IMPORTANT: function uses TON library call. Same restrictions apply!*)
	VAR_INPUT
		pTON : TON;
		timeoutTimeX1ms : UDINT;
	END_VAR
END_FUNCTION

FUNCTION bgTimerStop : UDINT (*Stop the timer.*)
	VAR_INPUT
		pTON : TON;
	END_VAR
END_FUNCTION

FUNCTION bgTimerIsEnabled : BOOL (*Check if timer is enabled.*)
	VAR_INPUT
		pTON : TON;
	END_VAR
END_FUNCTION

FUNCTION bgTimerIsTimeOut : BOOL (*Check if timer has expired.*)
	VAR_INPUT
		pTON : TON;
	END_VAR
END_FUNCTION

FUNCTION bgTimerTimeElapsed : UDINT (*Retrive the elapsed timer time [ms]*)
	VAR_INPUT
		pTON : TON;
	END_VAR
END_FUNCTION

FUNCTION bgTimerTimeRemaining : UDINT (*Retrive the remainig timer time [ms]*)
	VAR_INPUT
		pTON : TON;
	END_VAR
END_FUNCTION
