 
#include <bur/plctypes.h>
#include "RigUpTest.c"
#include "WinchmansWellDrivingTest.c"
#include "RigUpQuestions.c"
#include "LT_MVQuestions.c"
#include "RigSite.c"


#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#define TRUE 1
#define FALSE 0

_GLOBAL USINT iiii _VAR_INIT(0);

	


void TrainingCyclicResetAll()
{
	//Clean up flags
	
				
	RigUpModule.firstpass = 1;
				
	RigUpModule.timer1.IN = 0;
	RigUpModule.timer2.IN = 0;
	RigUpModule.timer3.IN = 0;
	RigUpModule.timer4.IN = 0;
	TON(&RigUpModule.timer1);
	TON(&RigUpModule.timer2);
	TON(&RigUpModule.timer3);
	TON(&RigUpModule.timer4);
				
	RigUpModule.ColorDatapoint1 = 7;
	RigUpModule.ColorDatapoint2 = 7;
			
	gHmi.key.mom.ICreset = 0;
				
	RigUpModule.PassSwitch1 = 0;
	RigUpModule.PassSwitch2 = 0;
	RigUpModule.FailSwitch1 = 0;
	RigUpModule.FailSwitch2 = 0;
				
	RigUpModule.current_state = 0;
	RigUpModule.firstpassRU = 0;
	
	RigUpModule.bduAlarmAck = 0;
	RigUpModule.bduAlarmPass = 0;
	
	/**********************************************************************************/
	//Clean up flags for Case 2
	//CalBrakeChange = 0;
	WellDriving.ReactedToFreewheeling = 0;
	WellDriving.CaliperKnobTurned = 0;
	WellDriving.firstpass2 = 0;
	WellDriving.current_state = 0;
	WellDriving.ToolPullOffIsOccurring = 0;
	WellDriving.ToolPullOffReaction = 0;
	WellDriving.WeCanNowGoUphole = 0;
	
	WellDriving.timer5.IN = 0;
	WellDriving.timer5.Q = 0;
	TON(&WellDriving.timer5);
	WellDriving.timer6.IN = 0;
	WellDriving.timer6.Q = 0;
	TON(&WellDriving.timer6);
		
	WellDriving.CasingShoePassKey = 0;
	WellDriving.CasingShoeFailKey = 0;
	
	WellDriving.Auto300PassKey = 0;
	WellDriving.Auto300FailKey = 0;
	WellDriving.Auto150PassKey = 0;
	WellDriving.Auto150FailKey = 0;
		
	WellDriving.firstpassWD = 0;
		
	WellDriving.stationaryCheck = 0;
	
	WellDriving.bNoReIC = FALSE;
		
	/********************************************************/

	
	Questions.current_state = QUESTIONS_INITIALIZE;
	
	Questions.jj = 0;
	
	Questions.total = 0;
	
	Questions.ii = 0;//First question
	
	Questions.showRigUpQuesScore = 1;
	Questions.rigUpQuesScoreSum = 0;
	
	/***********************************************************/
	//RigSite.c cleanup
	
	//clears the answer key when the Rig Site button is unselected
	for(iiii = 0; iiii < 32; iiii++)
	{
		Questions.answer[iiii] = 0;
	}
	
	//resets the state machine
	rsState = 0;
		
}

 static void mainSwitch()
{
	/*if(gHmi.prv.pageCurrent == 12)
	{
		gHmi.key.tgl.training = 2;
	}
	else
	{*/
		switch(gHmi.key.tgl.training)  //Enter each training class
		{
			case(1):
				RigUpTest();
				break;
			case(2):
				WinchmansWellDrivingTest();
				break;
			case(3):
				RigUpQuestions();
				break;
			case(4):
				LT_MVQuestionsMain();//Line tension and mooring valve ques.
				break;
			case(5):
				RigSiteMain();//Stop work authority ques written 10/14/2014
				break;
		/*case(6):
			break;
		case(7):
			break;
		case(8):
			break;
		case(9):
			break;
		case(10):
			break;
		case(11):
			break;
		case(12):
			break;
		case(13):
			break;
		case(14):
			break;
		case(15):
			break;
		case(16):
			break;
		case(17):
			break;
		case(18):
			break;
		case(19):
			break;
		case(20):
			break;*/
			default:
				TrainingCyclicResetAll();
				break;
		
	}
}



void _CYCLIC TrainingCyclic(void)
{
	
	if(gHmi.prv.pageCurrent > 6)
	{
		mainSwitch();
		
		if (WellDriving.begin)
		{
			gHmi.key.tgl.training = 2;
			
		}
			
	}

}




	
	
	
	

