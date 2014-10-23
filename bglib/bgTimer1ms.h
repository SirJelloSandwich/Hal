#ifndef _BG_TIMER1MS_H
#define _BG_TIMER1MS_H

#include <bur/plc.h>
#include <bur/plctypes.h>
#include <standard.h>


void bgTimerExecute(TON_typ *timer);
void bgTimerStart(TON_typ *timer, int timeoutTime);
void bgTimerStop(TON_typ *timer);
void bgTimerReset(TON_typ *timer);
int	bgTimerIsTimeOut(TON_typ *timer);
int	bgTimerTimeRemaining(TON_typ *timer);

#endif
