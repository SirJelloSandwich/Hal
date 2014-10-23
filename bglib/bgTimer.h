#ifndef _BG_TIMER_H
#define _BG_TIMER_H

#include <bur/plc.h>
#include <bur/plctypes.h>
#include <standard.h>

#define BG_TIMERTIME_1SEC		100					/*100 x 10 ms = 1 sec*/
#define BG_TIMERTIME_1MIN		(60 * BG_TIMERTIME_1SEC) 	/*60 x 1s = 1 min*/
#define BG_TIMERTIME_1HOUR	(60 * BG_TIMERTIME_1MIN) 	/*60 x 1min = 1 hour*/

void bgTimerExecute(TON_10ms_typ *timer);
void bgTimerStart(TON_10ms_typ *timer, int timeoutTime);
void bgTimerStop(TON_10ms_typ *timer);
void bgTimerReset(TON_10ms_typ *timer);
int	bgTimerIsTimeOut(TON_10ms_typ *timer);
int	bgTimerTimeRemaining(TON_10ms_typ *timer);

#endif
