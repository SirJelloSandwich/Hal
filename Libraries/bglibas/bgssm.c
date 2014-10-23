/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Library: bglibas
 * File: bglibas.c
 * Author: admin
 * Created: March 07, 2012
 ********************************************************************
 * Implementation of library bglibas
 ********************************************************************/

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

	#include "bglibas.h"

#ifdef __cplusplus
	};
#endif

#include <string.h>

/* Init state machine. MUST be called once, before any other bgSSM function call. */
unsigned long bgSSMInit(struct bgSSM_typ* p, unsigned long pFunGetStateName, unsigned long pFunCyclicCallback)
{
	/*TODO: Add your code here*/
	memset(p, 0, sizeof(struct bgSSM_typ));
	
	p->pfunGetStateName = (UDINT)pFunGetStateName;
	p->pfunCyclicCallback = (UDINT)pFunCyclicCallback;
	p->stateInit = 1;
	return(0);
}
/* State machine cyclic operation. */
unsigned long bgSSMCyclic(struct bgSSM_typ* p)
{
	/*TODO: Add your code here*/
	int ii;
	
	//operate all timers
	for(ii=0; ii<BGSSM_TIMER_COUNT;ii++){
		bgTimerCyclic(&p->timer_x1ms[ii]);
	}
	bgTimerCyclic(&p->dlytimer_x1ms);
	
	//operate dlyCycle counter
	if(p->dlyCycles){
		p->dlyCycles--;
	}
	
	//call cyclic callback function if defined
	if(p->pfunCyclicCallback){
		void(*callback)(bgSSM_typ*) = ( void (*) (bgSSM_typ*) ) p->pfunCyclicCallback;
		(*callback)(p);
	}
	
	//clear single cycle flag every scan
	p->stateInit = 0;
	
	//check if state change requested
	if(p->state != p->stateNext){
		
		//if state change w. delay > wait for delay timer to expire or dlyCycles == 0
		if(!bgTimerIsTimeOut(&p->dlytimer_x1ms) || p->dlyCycles){
			return 1;
		}
		
		p->state = p->stateNext;
		//generate stateInit sigle scan flag
		p->stateInit = 1;
		
		//define state name if exteranl function for state names is defined
		if( p->pfunGetStateName ){
			char*(*getStateName)(UDINT) = ( char* (*) (UDINT) )p->pfunGetStateName;
			strcpy(p->stateText, (*getStateName)(p->state));
		}else{
			strcpy(p->stateText, "???");		
		}		
	}
	return(0);
}
/* Change to next state. */
unsigned long bgSSMStateNext(struct bgSSM_typ* p, unsigned long stateNext)
{
	/*TODO: Add your code here*/
	p->stateNext = stateNext;	
	return(p->stateNext);
}

/* Change to next state w. timer delay. */
unsigned long bgSSMStateNextTimeDelay(struct bgSSM_typ* p, unsigned long stateNext, unsigned long delayTimeX1ms)
{
	/*TODO: Add your code here*/
	p->stateNext = stateNext;
	bgTimerStart(&p->dlytimer_x1ms, delayTimeX1ms);
	return(p->stateNext);
}

/* Change to next state w. cycles delay. */
unsigned long bgSSMStateNextCycleDelay(struct bgSSM_typ* p, unsigned long stateNext, unsigned long delayCycles)
{
	/*TODO: Add your code here*/
	p->stateNext = stateNext;
	p->dlyCycles = delayCycles;
	return(p->stateNext);
}
/* Check if first scan for the state. */
plcbit bgSSMIsStateInit(struct bgSSM_typ* p)
{
	/*TODO: Add your code here*/
	return(p->stateInit);
}
//EOF

/* Get current state */
unsigned long bgSSMGetState(struct bgSSM_typ* p)
{
	/*TODO: Add your code here*/
	return(p->state);
}

/* Get current state text*/
plcstring* bgSSMGetStateText(struct bgSSM_typ* p)
{
	/*TODO: Add your code here*/
	return(&p->stateText[0]);
}
