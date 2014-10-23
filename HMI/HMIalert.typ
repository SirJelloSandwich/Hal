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
(*HMI object - ALERT*)
(*########################################################################*)

TYPE
	hmiAlert_typ : 	STRUCT 
		message : STRING[512];
		header : STRING[128];
		type : UINT; (*0-info; 1-warning; 2-error*)
		status : UINT; (*Status for common dialog layer*)
		tmr1RemainingTimeSec : UDINT; (*Timer remaining time (sec)*)
		tmr1 : TON; (*Dialog control timer*)
	END_STRUCT;
END_TYPE
