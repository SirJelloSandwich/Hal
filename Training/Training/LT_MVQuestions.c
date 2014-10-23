/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: Training
 * File: LTandMVQuestions.c
 * Author: ISM
 * Created: May 05, 2014
 *******************************************************************/

#include <bur/plctypes.h>
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

char * strcpy ( char *,  const char *);
//_GLOBAL USINT lt_MVKey[12] _VAR_INIT  {1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1 };

 static void LT_MVquestionsInitialize()
{
	gHmi.prv.pageChange = 11;
	Questions.current_state = QUESTIONS;
}

static void LT_MVquestions()
{	
	
	REAL testing;
	
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
	
	switch(Questions.ii)
	{
		case (0):
			strcpy(Questions.quesString, "1. With all of the new safety features used to alarm and alert the operator about dropped objects or tool pull offs, the line tension valve is the one true line of defense that every unit operator has to avoid tools pull offs. In reality it is the only safety feature needed if it is operated correctly and adjusted at every point of the operation.");
			break;
		case (1):
			strcpy(Questions.quesString, "2. Picking up tools on the surface - The line tension valve should be backed out all the way (counter clockwise)until the reel just slows and then screwed in only a 1/4 turn clockwise prior to picking up the individual tools on the surface. The worst occurrence that could happen is if something hangs up during rig up there will not be a pull off. The line tension valve may have to be adjusted in (clockwise) a turn at a time as the tool string grows in length in order to safely pick up the string off of the make up plate. ");
			break;
		case (2):
			strcpy(Questions.quesString, "3. The line tension adjustment on the console controls a direct acting relief valve which affects the mooring valve which is also a direct acting relief valve. The line tension adjustment is used to limit the line tension for the mooring control. The mooring control valve controls the pump, de-strokes the pump if an obstruction is hit and stops the Wireline reel from turning.");
			break;
		case (3):
			strcpy(Questions.quesString, "4. Should the operator adjust the line tension adjustment every time the wireline reaches a flange? Answer True for Yes and False for No.");
			break;
		case (4):
			strcpy(Questions.quesString, "5. Reel speed is adjusted by the joystick or the line tension valve?  Answer True for joystick and False for line tension valve.");//Answer Joystick TRUE
			break;
		case (5):
			strcpy(Questions.quesString, "6. The hydraulic fluid in the system heats up if you use the line tension valve to control speed.");//Answer Heats Up or TRUE
			break;                            
		case (6):
			strcpy(Questions.quesString, "7. Continuous flow through a hydraulic relief valve causes excessive fluid heat.");//Answer Excessive Fluid Heat TRUE
			break;
		case (7):
			strcpy(Questions.quesString, "8. When backing off the line tension valve the system pressure gauge will increase.");
			break;
		case (8):
			strcpy(Questions.quesString, "9. Lowering tools strung from the surface - The line tension valve must not continue to be used or adjusted each time we pick up a tool until the tool string has cleared the wellhead. This will help to increase unintentional pull offs while working at surface ");
			break;
		case (9):
			strcpy(Questions.quesString, "10. The line tension valve setting must be readjusted (checked) every time stopping and starting the tool upward – whether to check the line tension and torque, or after a sudden tool stoppage or when coming off bottom. (This is not a “set it and forget operation”!)");
			break;
		case (10):
			strcpy(Questions.quesString, "11. If the drum has stopped due to line tension - check mooring valve settings, then to recover, slowly increase driving pressure by slight clockwise turn of the tension adjustment valve knob. If the tool is does not come free, then it may be necessary to stop the hoist, close any caliper tool, and go back down hole to try and free the tool.");
			break;
		//case (11):
			//strcpy(Questions.quesString, "12. What are the color markings on the line?");
			//break;
		case (11):
			
			for(/* Questions.jj*/ ; Questions.jj < (Questions.ii - 1); Questions.jj++)//iterate through
			{
				if (Questions.answer[Questions.jj] == lt_MVKey[Questions.jj])
				{
					Questions.total++;
				}
			}
			
			testing = (REAL) Questions.total / 10.0;
			Questions.rigUpQuesScoreSum = testing * 100;
			
			strcpy(Questions.quesString,"SCORE:");
			Questions.showRigUpQuesScore = 0;
			break;
		default:
			break;
	}
}

 static void LT_MVQuestionsMain()
{
	switch(Questions.current_state)  //Enter each training class
	{
		case(QUESTIONS_INITIALIZE):
			LT_MVquestionsInitialize();
			break;
		case(QUESTIONS):
			LT_MVquestions();
			break;
	}
	
	/*void(*state_table4[])() = 
		{
		LT_MVquestionsInitialize
		LT_MVquestions
		};

	state_table4[Questions.current_state]();
	
	//This below enum is found in Training::Training.typ
	
	enum {       
			QUESTIONS_INITIALIZE = 0,	
			QUESTIONS 
					
			}   State_Type;*/
}

