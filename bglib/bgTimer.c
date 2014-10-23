#include "bgTimer.h"


void bgTimerExecute(TON_10ms_typ *timer){
	TON_10ms(timer);	
}

void bgTimerStart(TON_10ms_typ *timer, int timeoutTime){
	timer->IN = 0;
	TON_10ms(timer);	
	timer->IN = 1;
	timer->PT = timeoutTime;
	TON_10ms(timer );	
}

void bgTimerStop(TON_10ms_typ *timer){
	timer->IN = 0;
	TON_10ms(timer);	
}
int  bgTimerIsTimeOut(TON_10ms_typ *timer){
	return timer->Q;
}

int bgTimerTimeRemaining(TON_10ms_typ *timer){
	return (timer->PT - timer->ET);
}
