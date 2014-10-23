/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Program: hmilogic
 * File: hmiAlert.c
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

//Control on/off a variable that control the visibility of the alarm alert display
void hmiAlertHide(hmiAlert_typ* p)
{
	//control visibility of the layer > make it invisible
	bgBitSetUINT(&p->status, 0, 1);
}

// Is alert message active (visible)?
BOOL hmiAlertIsActive(hmiAlert_typ* p)
{
	if(bgTimerIsTimeOut(&p->tmr1))
		return 0;
	else
		return 1;
}

// Show the alert with a specified text and for a selected amount of time
void hmiAlertShow(hmiAlert_typ* p, char* alertText, UDINT alertDuration)
{
	//control visibility of the layer > make it visible
	bgBitSetUINT(&p->status, 0, 0);
	bgStrCpyN(p->message, alertText, sizeof(p->message));
	bgTimerStart(&p->tmr1, alertDuration);		
}

// Cyclic operation of the alert box.
void hmiAlertCyclic(hmiAlert_typ* p)
{
	bgTimerCyclic(&p->tmr1);			
	if( !(p->status & 0x1) && bgTimerIsTimeOut(&p->tmr1) ){
		hmiAlertHide(p);
	}
}
//eof
