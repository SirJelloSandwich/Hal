/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Package: bglib
 * File: bgTimer1ms.c
 * Author: goran
 * Created: October 05, 2011
 *******************************************************************/
#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "bglibas.h"

#ifdef __cplusplus
	};
#endif
#include <standard.h>


/* Timer cyclic function. Must be called cyclicly for corret timer operation. */
unsigned long  bgTimerCyclic(struct TON *timer){
	TON(timer);	
	return 0;
}

/* Start timer for a defined amount of time [ms]. IMPORTANT: function uses TON library call. Same restrictions apply! */
unsigned long  bgTimerStart(struct TON *timer, unsigned long timeoutTime){
	timer->IN = 0;
	TON(timer);	
	timer->IN = 1;
	timer->PT = timeoutTime;
	TON(timer );	
	return 0;
}

/* Stop the timer.*/
unsigned long  bgTimerStop(struct TON *timer){
	timer->IN = 0;
	TON(timer);	
	return 0;
}

/* Check if timer has expired.*/
BOOL  bgTimerIsTimeOut(struct TON *timer){
	if( bgTimerIsEnabled(timer) ){
		return (timer->Q);
	}else{
		return (1);	
	}
}

/* Check if timer is enabled.*/
BOOL  bgTimerIsEnabled(struct TON *timer){
	return (timer->IN);
}

/* Retrive the remainig timer time [ms].*/
unsigned long  bgTimerTimeRemaining(struct TON *timer){
	return (timer->PT - timer->ET);
}

/* Retrive the elapsed timer time [ms].*/
unsigned long  bgTimerTimeElapsed(struct TON *timer) {
	return (timer->ET);
}

//EOF

