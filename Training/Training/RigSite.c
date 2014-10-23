/********************************************************************
 * COPYRIGHT -- Microsoft
 ********************************************************************
 * Program: Training
 * File: RigSite.c
 * Author: palmerk
 * Created: October 14, 2014
 *******************************************************************/

#include <bur/plctypes.h>
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

static void RSInit();
static void RSquestions();

#define NUM_OF_QUES 10
#define ANSWER_LENGTH 10
#define QUES_MAX_LENGTH 1000
//#define TRUE 1 
//#define FALSE 0

_GLOBAL STRING answer[NUM_OF_QUES][ANSWER_LENGTH];
_GLOBAL STRING ques[NUM_OF_QUES][QUES_MAX_LENGTH];
_GLOBAL USINT rsState _VAR_INIT(0);
_GLOBAL USINT rsKey[10] _VAR_INIT (0);
_GLOBAL	DINT CHECK[10] _VAR_INIT (0);
_GLOBAL STRING TESTSTR[5] _VAR_INIT ("FALSE");

enum{
	RSINIT = 0,
	RSQUESTIONS
};

static void RigSiteMain()
{		
	switch(rsState)  //Enter each training class
	{
		case(RSINIT):
			RSInit();
			break;
		case(RSQUESTIONS):
			RSquestions();
			break;
	}
}
 
static void RSInit()
{
	USINT ll;
	gHmi.prv.pageChange = 15;
	rsState = RSQUESTIONS;
}

static void RSquestions()
{
	REAL testing;
	//USINT mmm;
//	testkey[0] =  answer[0];//,answer[1] ,answer[2] ,answer[3] ,answer[4] ,answer[5] , answer[6],answer[7] ,answer[8] ,answer[9] };	
//	USINT rsKey[] = {0};
	
	
	if(Questions.buttonTrue)
	{		
		Questions.answer[Questions.ii] = 1;
		Questions.ii += 1;
		Questions.buttonTrue = 0;
		//move to next ques.
	}  
	
	if (Questions.buttonFalse)
	{	
		Questions.answer[Questions.ii] = 0;
		Questions.ii += 1;
		Questions.buttonFalse = 0;
		//move to next ques.
	}
		
	//If the contents of pString1 < contents of pString2, then result < 0
	//If contents of pString1 = contents of pString2, then result = 0
	//If contents of pString1 > contents of pString2, then result > 0 

	//result = strcmp(adr(String1),adr(String2))

	//if they are the same then strcmp returns a 0, otherwise see above
		CHECK[0] = strcmp( answer[0],TESTSTR);//TRUE
		CHECK[1] = strcmp( answer[1],TESTSTR);
		CHECK[2] = strcmp( answer[2],TESTSTR);
		CHECK[3] = strcmp( answer[3],TESTSTR);
		CHECK[4] = strcmp( answer[4],TESTSTR);
		CHECK[5] = strcmp( answer[5],TESTSTR);
		CHECK[6] = strcmp( answer[6],TESTSTR);
		CHECK[7] = strcmp( answer[7],TESTSTR);//FALSE
		CHECK[8] = strcmp( answer[8],TESTSTR);//FALSE
		CHECK[9] = strcmp( answer[9],TESTSTR);//FALSE
	
	//when CHECK is not zero it means that answer[x] is TRUE bc TESTSTR is "FALSE"
	
	/*for (mmm = 0; mmm < 10; mmm++)
	{	
		if (CHECK[mmm] != 0)
		{
			rsKey[mmm] = 1;
		}
	}*/
	
	if (CHECK[0] != 0)
	{
		rsKey[0] = 1;
	}
	
	if (CHECK[1] != 0)
	{
		rsKey[1] = 1;
	}
	
	if (CHECK[2] != 0)
	{
		rsKey[2] = 1;
	}
	
	if (CHECK[3] != 0)
	{
		rsKey[3] = 1;
	}
	
	if (CHECK[4] != 0)
	{
		rsKey[4] = 1;
	}
	
	if (CHECK[5] != 0)
	{
		rsKey[5] = 1;
	}
	
	if (CHECK[6] != 0)
	{
		rsKey[6] = 1;
	}
	
	if (CHECK[7] != 0)
	{
		rsKey[7] = 1;
	}
	
	if (CHECK[8] != 0)
	{
		rsKey[8] = 1;
	}
	
	if (CHECK[9] != 0)
	{
		rsKey[9] = 1;
	}
	

	
	switch(Questions.ii)
	{
		case (0):
			strcpy(Questions.quesString, ques[0]);
			break;
		case (1):
			strcpy(Questions.quesString,ques[1]);
			break;
		case (2):
			strcpy(Questions.quesString,ques[2] );
			break;
		case (3):
			strcpy(Questions.quesString, ques[3]);
			break;
		case (4):
			strcpy(Questions.quesString,ques[4]);
			break;
		case (5):
			strcpy(Questions.quesString,ques[5] );
			break;                            
		case (6):
			strcpy(Questions.quesString, ques[6]);
			break;
		case (7):
			strcpy(Questions.quesString,ques[7] );
			break;
		case (8):
			strcpy(Questions.quesString, ques[8]);
			break;
		case (9):
			strcpy(Questions.quesString,ques[9] );
			break;
		case (10):
			
			for(/* Questions.jj*/ ; Questions.jj < (Questions.ii - 1); Questions.jj++)//iterate through
			{
				if (Questions.answer[Questions.jj] == rsKey[Questions.jj])
				{
					Questions.total++;
				}
			}
			
			testing = (REAL) Questions.total / 9.0;
			Questions.rigUpQuesScoreSum = testing * 100;
			
			strcpy(Questions.quesString,"SCORE:");
			Questions.showRigUpQuesScore = 0;
			break;
		
		default:
			break;
	}
}

