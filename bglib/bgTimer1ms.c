/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Package: bglib
 * File: bgTimer1ms.c
 * Author: goran
 * Created: October 05, 2011
 *******************************************************************/

#include <bur/plctypes.h>
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

//#include <standard.h>


#ifndef _BG_TIMER_x1MS_C
#define _BG_TIMER_x1MS_C

void bgTimerExecute(TON_typ *timer){
	TON(timer);	
}

void bgTimerStart(TON_typ *timer, int timeoutTime){
	timer->IN = 0;
	TON(timer);	
	timer->IN = 1;
	timer->PT = timeoutTime;
	TON(timer );	
}

void bgTimerStop(TON_typ *timer){
	timer->IN = 0;
	TON(timer);	
}

BOOL  bgTimerIsTimeOut(TON_typ *timer){
	return timer->Q;
}

UDINT bgTimerTimeRemaining(TON_typ *timer){
	return (timer->PT - timer->ET);
}

UDINT bgTimerTimeElapsed( TON_typ *timer ) {
	return (timer->ET);
}

#endif //_BG_TIMER_x1MS_C

