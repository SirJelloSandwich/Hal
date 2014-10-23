/*
File: 
bgStateMachine.h

About:
------
- state machine support
*/
#ifndef _BG_STATEMACHINE_H
#define _BG_STATEMACHINE_H

/*
Support for an extensive SM debugging (conditional compilation):
0 - off; 1 - on
*/
#define BGSM_DEBUG				0

/*
Max size of the state name:
- to set state name use bgSMSetStateName()
- if given state name is longer (good for documentation), only a portion
will be visible in PV monitor
*/
#define BGSM_STATENAMEMAXLEN	20


/*
Depth of the history buffer (states and state cycles)
*/
#define BGSM_STATEHISTORY		20


/*
Standard msgId-s
*/
#define BGSM_MSGID_CYCLIC	0
#define BGSM_MSGID_INIT		1


typedef struct bgStateMachine_typ{
	/*current state*/
	int state;
	
	/*next state*/
	int nextState;

	/*state init flag (state entered for the first time); 0-no;1-yes*/
	char init;		

	/*sm error raised flag; 0-no error;1-yes error*/
	char errorRaised;		
	int errorId;		
	int errorCode;		
	char errorText[BGSM_STATENAMEMAXLEN];

	/*sm done raised flag; 0-not done;1-done*/
	char doneRaised;		
	int doneId;		

	/*debug flag*/	
	char debug;
	/*INTERNAL USE ONLY: special flag to take care of the correct output to telnet ringbuffer*/
	char dbgOutput;		
	char dbgOutputError;
	
	/*current state name (debugging)*/
	char stateName[BGSM_STATENAMEMAXLEN];
	/*current function name (debugging)*/
	char functionName[BGSM_STATENAMEMAXLEN];

	/*history arrays:
	item[0]=sample(t) 
	item[1]=sample(t-1)
	item[2]=sample(t-2) etc.
	*/
	int stateHistory[BGSM_STATEHISTORY];
	int stateCyclesHistory[BGSM_STATEHISTORY];

	/*cyclic timer; incremented every cycle; reseted (0) on state change*/
	int stateCyclicTimer1;


#if BGSM_DEBUG != 0 		
	char stateHistoryTxt0[BGSM_STATENAMEMAXLEN];
	char stateHistoryTxt1[BGSM_STATENAMEMAXLEN];
	char stateHistoryTxt2[BGSM_STATENAMEMAXLEN];
	char stateHistoryTxt3[BGSM_STATENAMEMAXLEN];
	char stateHistoryTxt4[BGSM_STATENAMEMAXLEN];
	char stateHistoryTxt5[BGSM_STATENAMEMAXLEN];	
#endif	
}bgStateMachine_typ;

#define bgSMCyclic(psm)	_bgSMCyclic(psm, __FUNCTION__)
	
int bgSMIsStateInit(bgStateMachine_typ *p);
int bgSMIsStateTimer1Done(bgStateMachine_typ *p, int timerTime);

int _bgSMCyclic(bgStateMachine_typ *p, char *srcFunction);
int	bgSMGetState(bgStateMachine_typ *p);
int bgSMSetNextState(bgStateMachine_typ *p, int nextState);
void bgSMReset(bgStateMachine_typ *p);
	
void bgSMSetState(bgStateMachine_typ *p, int state);
void bgSMSetStateName(bgStateMachine_typ *p, char *stateName);

void bgSMRaiseError(bgStateMachine_typ *p, char *errorText, int errorId, int errorCode);
int bgSMOnError(bgStateMachine_typ *p);
int bgSMGetErrorId(bgStateMachine_typ *p);
void bgSMRaiseDone(bgStateMachine_typ *p, int doneId);
int bgSMOnDone(bgStateMachine_typ *p);
int bgSMGetDoneId(bgStateMachine_typ *p);

#endif
/*EOF*/
