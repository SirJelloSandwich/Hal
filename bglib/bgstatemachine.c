#ifndef _BG_STATEMACHINE_C
#define _BG_STATEMACHINE_C
/*
File: 
bgStateMachine.c

About:
------
- state machine support 

B&R System Dependencies:
------------------------
- none

History:
--------
05/05/2005
-first version

Test and example code:
----------------------
- to run this file standalone:
--- enable following code section (remove or comment out #if 0 / #endif) 
--- create cyclic object (task) from C Language and add this file
*/
#include "bgstatemachine.h"


#include "bgdebug.h"


void _bgSMCopyString(char *src, char *dst, int maxLen);
void _bgSMCharToString(char *dst, char value);
void _bgSMDbg(bgStateMachine_typ *p);

int _bgSMCyclic(bgStateMachine_typ *p, char *srcFunction)
{
int i;

	/*this is the last resort to print output state change debug msg*/
	_bgSMDbg(p);

/*clear init (1 cycle) flags*/
	p->init = 0;
	p->errorRaised = 0;		
	p->doneRaised = 0;		
	
/*Reset state = 0 - starting state, application must call bgSMReset before starting executing SM*/
	if(!p->state){	
		p->nextState = 1;
		
		/*clear state history*/		
		for(i=0; i<BGSM_STATEHISTORY;i++){
			p->stateHistory[i] = 0;
			p->stateCyclesHistory[i] = 0;
		}

#if BGSM_DEBUG != 0 		
		/*clear txt history*/
		p->stateHistoryTxt0[0] = 0;
		p->stateHistoryTxt1[0] = 0;
		p->stateHistoryTxt2[0] = 0;
		p->stateHistoryTxt3[0] = 0;
		p->stateHistoryTxt4[0] = 0;
		p->stateHistoryTxt5[0] = 0;
#endif	
		/*clear state cyclic timer1*/
		p->stateCyclicTimer1 = 0;

		/*clear error handling info*/
		p->errorRaised = 0;		
		p->errorId = 0;		
		p->errorCode = 0;		
		p->errorText[0] = 0;

		/*clear done handling info*/
		p->doneRaised = 0;		
		p->doneId = 0;		

		/*define current function name*/
		_bgSMCopyString(&p->functionName[0], srcFunction, BGSM_STATENAMEMAXLEN);
	}

/*check for a change of state request*/
	if(p->state != p->nextState){		
		char txtBuff[10];
			
		p->state = p->nextState;
		p->init = 1;
		p->dbgOutput = 1;

		/*generate state & timer history*/
		for(i=(BGSM_STATEHISTORY-1);i>0;i--){
			p->stateHistory[i] = p->stateHistory[i-1];
			p->stateCyclesHistory[i] = p->stateCyclesHistory[i-1];
		}
		
		p->stateHistory[0] = p->state;


		/*clear state cyclic timer1*/
		p->stateCyclicTimer1 = 0;

		/*clear error handling info*/
		p->errorRaised = 0;		
		p->errorId = 0;		
		p->errorCode = 0;		
		p->errorText[0] = 0;

		/*clear done handling info*/
		p->doneRaised = 0;		
		p->doneId = 0;		

#if BGSM_DEBUG != 0 		
		/*generate state txt history*/
		if(p->debug){				
			_bgSMCopyString(&p->stateHistoryTxt5[0], &p->stateHistoryTxt4[0], BGSM_STATENAMEMAXLEN);
			_bgSMCopyString(&p->stateHistoryTxt4[0], &p->stateHistoryTxt3[0], BGSM_STATENAMEMAXLEN);
			_bgSMCopyString(&p->stateHistoryTxt3[0], &p->stateHistoryTxt2[0], BGSM_STATENAMEMAXLEN);
			_bgSMCopyString(&p->stateHistoryTxt2[0], &p->stateHistoryTxt1[0], BGSM_STATENAMEMAXLEN);
			_bgSMCopyString(&p->stateHistoryTxt1[0], &p->stateHistoryTxt0[0], BGSM_STATENAMEMAXLEN);
		}
#endif

		/*define state default textual name*/
		_bgSMCharToString(&txtBuff[0], (char)p->state);
		_bgSMCopyString(&p->stateName[0], &txtBuff[0], BGSM_STATENAMEMAXLEN);
#if BGSM_DEBUG != 0 		
		_bgSMCopyString(&p->stateHistoryTxt0[0], &p->stateName[0], BGSM_STATENAMEMAXLEN);
#endif

	}	

	/*increment state cyclic timer1 & store it in history*/
	p->stateCyclesHistory[0] = p->stateCyclicTimer1++;	
	return p->state;
}

void _bgSMDbg(bgStateMachine_typ *p)
{
/*
- debug messages to telnet client
NOTE:
->debug value is used to adjust output formatting (can display state machine hierarchy)
*/
char *identation = "";
	switch(p->debug){
		case 2: identation = "   "; break;
		case 3: identation = "       "; break;
		case 4: identation = "          "; break;
	}			

	if( p->dbgOutput && p->debug){
		/*function called in multiple places-> this flag guranteies that this part 
		(state change msg) is executed only once!!!*/
		p->dbgOutput = 0;	
		DBG("%s %s\r\n", identation, &p->stateName[0]);
	}

	if(p->dbgOutputError && p->errorRaised){
		p->dbgOutputError = 0;
		DBG("SM ERROR: %s-%s-%s-%d-%d\r\n", &p->functionName[0], &p->stateName[0], &p->errorText[0], p->errorId, p->errorCode);
	}
}

int bgSMIsStateInit(bgStateMachine_typ *p)
{
	return p->init;
}

int bgSMIsStateTimer1Done(bgStateMachine_typ *p, int timerTime)
{
	if(p->stateCyclicTimer1 > timerTime)
		return 1;
	else
		return 0;
}

int	bgSMGetState(bgStateMachine_typ *p)
{
	return p->state;
}

int bgSMSetNextState(bgStateMachine_typ *p, int state)
{
	return p->nextState = state;

}

void bgSMRaiseError(bgStateMachine_typ *p, char *errorText, int errorId, int errorCode)
{
	p->errorRaised = 1;
	p->dbgOutputError = 1;
	p->errorId = errorId;		
	p->errorCode = errorCode;
	_bgSMCopyString(&p->errorText[0], errorText, BGSM_STATENAMEMAXLEN);

}

int bgSMOnError(bgStateMachine_typ *p)
{
	_bgSMDbg(p);
	return p->errorRaised;
}

int bgSMGetErrorId(bgStateMachine_typ *p)
{
	return p->errorId;
}

void bgSMRaiseDone(bgStateMachine_typ *p, int doneId)
{
	p->doneRaised = 1;		
	p->doneId = doneId;		
}

int bgSMOnDone(bgStateMachine_typ *p)
{
	return p->doneRaised;
}


int bgSMGetDoneId(bgStateMachine_typ *p)
{
	return p->doneId;
}

			
void bgSMReset(bgStateMachine_typ *p)
{
/*set reset state*/
	p->state = 0;
	
/*clear init (1 cycle) flags*/
	p->init = 0;
	p->errorRaised = 0;		
	p->doneRaised = 0;		
}

void bgSMSetStateName(bgStateMachine_typ *p, char *stateName)
{
/*do this only on state init*/
	if(!p->init)
		return;
	
	 p->stateName[3] = '-';
	_bgSMCopyString(&p->stateName[4], stateName, BGSM_STATENAMEMAXLEN - 4);
#if BGSM_DEBUG != 0 		
	_bgSMCopyString(&p->stateHistoryTxt0[0], &p->stateName[0], BGSM_STATENAMEMAXLEN);
#endif

	/*if application uses this method, then the change state string will be written here
	if not, then the dbg output will be written in bgSMCyclic()
	*/
	_bgSMDbg(p);

}


void _bgSMCopyString(char *dst, char *src, int maxLen)
{
int 	i;
char 	c;
char	k = 1;	

	for(i=0;i<maxLen;i++){
		if( k ){
			c = *(src + i);
			*(dst + i) = c;
			if(!c) k = 0;
		}
		else{
			*(dst + i) = 0;
		}
	}
	
/*terminate string if larger then max*/	
	*(dst + maxLen - 1) = 0;	
}

void _bgSMCharToString(char *dst, char value)
{
int v100;
int v100rem;
int v10;
int v1;

char c100;
char c10;
char c1;


	v100 	= (int)value / 100;
	v100rem = (int)value % 100;	
	v10 	= v100rem / 10;
	v1	 	= v100rem % 10;

	c100	= '0' + (char)v100;
	c10		= '0' + (char)v10;
	c1		= '0' + (char)v1;

	*(dst + 0) = c100;
	*(dst + 1) = c10;
	*(dst + 2) = c1;
	*(dst + 3) = 0;	
}
#endif
/*EOF*/
