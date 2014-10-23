(********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Package: HMI
 * File: HMI.typ
 * Author: goran
 * Created: August 28, 2013
 ********************************************************************
 * Data types of package HMI
 ********************************************************************)
(*########################################################################*)
(*HMI object - DIALOG (OK, Cancel)*)
(*########################################################################*)

TYPE
	hmiDialog_typ : 	STRUCT 
		message : STRING[512];
		result : UINT; (*value returned when button pressed (0-busy, button not pressed, dialog active, OK pressed returns resultForOk (eg.1); Cancel pressed returns resultForCancel (eg.2)*)
		resultForOk : UINT; (*value to return when Ok pressed (if 0 return value is 1)*)
		resultForCancel : UINT; (*value to return when Cancel pressed (if 0 return value is 2)*)
		timeoutConfig : UINT; (*configuration input that defines meaning of the timeout. If 0 > timeout means cancel; if 1> timeout means ok*)
		status : UINT; (*Status for common dialog layer*)
		tmr1RemainingTimeSec : UDINT; (*Timer remaining time (sec)*)
		tmr1 : TON; (*Dialog control timer*)
	END_STRUCT;
END_TYPE
