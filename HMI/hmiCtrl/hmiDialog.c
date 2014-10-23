/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Program: hmilogic
 * File: hmiDialog.c
 * Author: goran
 * Created: September 05, 2013
 *******************************************************************/

#include <bur/plctypes.h>
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

/** 
Bit manipulaiton utility functions.
 */
//#include <../../bglib/bgBitUtilities.h>

/** 
String manipulaiton utility functions.
 */
//#include <../../bglib/bgsprintf.h>
//#include <../../bglib/bgstring.h>

/** 
 Timer control utility functions.
 */
//#include <../../bglib/bgTimer1ms.h>


/*
###############################################################################
Simple dialog support w. OK and Cancle response
###############################################################################
*/


/**
Find if HMI dialog is active (visible) or not
\param p prt to hmiDialog_typ structure
\return 0 not active (not visible)
\return 1 active (visible)
*/
BOOL hmiDialogIsActive(hmiDialog_typ* p)
{
	//hide the dialog
	if(p->status & 0x1){
		return 0; //not visible > not active
	}
	
	return 1; //visible > active
}

/**
Hide HMI dialog (make it not visible)
\param p prt to hmiDialog_typ structure
*/
void hmiDialogHide(hmiDialog_typ* p)
{
	//hide the dialog
	bgBitSetUINT(&p->status, 0, 1);

	//define the message
	strcpy(p->message, "");
}

/**
Hide HMI dialog (make it not visible)
\param p prt to hmiDialog_typ structure
\param msg text message that will be displayed in a dialog
\param timeout_ms time duration to show the dialog. Upon timer expiration, dialog will automatically generate either ok or cancel (configured, see timeoutConfig)
\param resultForOk value returned in case user response is OK (if 0, default is used = 1)
\param resultForCancel value returned in case user response is Cancel ((if 0, default is used = 2)
\param timeoutConfig 0-timeout wil return value for Cancel, 1-timeout will return value for OK
*/
 void hmiDialogShow(hmiDialog_typ* p, char* msg, UINT timeout_ms, UINT resultForOk, UINT resultForCancel, UINT timeoutConfig)
{
	//show the dialog
	bgBitSetUINT(&p->status, 0, 0);

	//define the message
	bgStrCpyN(p->message, msg, sizeof(p->message));

	//start the timer
	bgTimerStart(&p->tmr1, timeout_ms);
	
	//set the result to 0 (not complete)
	p->result = 0;

	//configuration of the timeout
	p->timeoutConfig = timeoutConfig;
	
	p->resultForOk = resultForOk;
	if(!p->resultForOk)
		p->resultForOk = 1;
	
	p->resultForCancel = resultForCancel;
	if(!p->resultForCancel)
		p->resultForCancel = 2;
		
}


/**
Hide HMI dialog cyclic routine 
\param p prt to hmiDialog_typ structure
*/
UINT hmiDialogCyclic(hmiDialog_typ* p)
{
	//operate timer always
	bgTimerCyclic(&p->tmr1);
	
	p->tmr1RemainingTimeSec = (p->tmr1.PT - p->tmr1.ET) / 1000;
	
	//if dialog not visible > nothing to do
	if(p->status)
		return 0;
		
	if( bgTimerIsTimeOut(&p->tmr1) ){
		//hide the dialog
		hmiDialogHide(p);
		//check how is the timeout configured and generate such result
		if(!p->timeoutConfig){
			p->result = p->resultForCancel;
			return p->resultForCancel; //return cancel
		}
		p->result = p->resultForOk;
		return p->resultForOk;//return OK		
	}

	//keep checking if the result value will get set by the HMI
	if( p->result ){
		//hide the dialog
		hmiDialogHide(p);

		//predefined values in HMI are:
		// 1=OK
		// 2=Cancel
		// Override the above and return what's configured 
		if(p->result == 1){
			p->result = p->resultForOk;
		}else{
			p->result = p->resultForCancel;
		}
		
		return p->result; //return user's choice (1=ok, 2=cancel > see the HMI setup)
	}
}
//EOF

