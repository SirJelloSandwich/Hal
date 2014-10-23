/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: Training
 * File: Questions.c
 * Author: Kevin
 * Created: May 4, 2014
 *******************************************************************/

#include <bur/plctypes.h>
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

 char * strcpy ( char *,  const char *);


 static void questionsInitialize()
{	
	gHmi.prv.pageChange = 10;
	Questions.current_state = QUESTIONS;
}

 static void questions()
{
	REAL testing;
	
	USINT rigUpKey[] = {1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1};

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
	
	
	switch(	Questions.ii)
	{
	
		case (0):
			strcpy(Questions.quesString, "1. Before performing any task at the wellsite you must conduct a pre-job safety meeting and discuss any safety concerns in detail.");
			break;
		case (1):
			strcpy(Questions.quesString, "2. Minimum personnel: non-essential personnel should be removed from the logging unit so that the field professional and winchman can fully focus on the task at hand.  It is the field professional's responsibility to minimize the people present in the logging unit.");
			break;
		case (2):
			strcpy(Questions.quesString, "3. After locating the T Bar, lock the elevator swivel.  Lock the elevator handles. Secure the elevator handles using rope or bungee cord.");
			break;
		case (3):
			strcpy(Questions.quesString, "4. The reel operator should control speed during operation, slow down when nearing surface, and slow down near any down hole restriction or anything that could potentially hang up the tool or create a hazard.");
			break;
		case (4):
			strcpy(Questions.quesString, "5. Before rig up the operator should change his depth to bypass the safety features built in the RC controller to avoid having to depress the safety override button on top of the joystick.");
			break;
		case (5):
			strcpy(Questions.quesString, "6. While running the wireline reel during rig up you must always have the line tension mooring valve backed off just where normal operations begin and the reel slows.");
			break;
		case (6):
			strcpy(Questions.quesString, "7. With the mooring valve backed off properly the reel operator can prevent tool pull offs at the surface.");
			break;
		case (7):
			strcpy(Questions.quesString, "8. When using the joystick for reel up and down control, the operator is responsible for acceleration and deceleration of the reel. If he moves the joystick too fast out of neutral the reel will accelerate at too quickly. If this occurs, the operator should ease the joystick out of neutral and control the speed of the reel using the joystick controls which provide proportional output to the pump solenoids.");
			break;
		case (8):
			strcpy(Questions.quesString, "9. Use of a tag line to control the tools and rig up equipment up the pipe ramp to the rig floor is prohibited.");
			break;
		case (9):
			strcpy(Questions.quesString, "10. The operator on the catwalk must always be in control of the tools or rig up equipment being lifted up the pipe ramp via the tagline. Once the tools or equipment make it to the rig floor they can not swing loosely and possibly hit a crew member or rig personnel.");
			break;
		case (10):
			strcpy(Questions.quesString, "11. The rig up chain once secured to the frame of the rig for the bottom sheave must be tied in a knot to ensure that it is secure.");
			break;
		case (11):
			strcpy(Questions.quesString, "12. Use of a bottom sheave stand is not recommended. The sheave wheel will stand on its own.");
			break;
		case (12):
			strcpy(Questions.quesString, "13. When unloading tools from the truck the operators should use proper lifting techniques in order to avoid injuries.");
			break;
		case (13):
			strcpy(Questions.quesString, "14. If the catwalk is cluttered with items from the rig, the operator has the right to issue a stop work authority (SWA) and get the rig hands to clear the items and make a clear work area.");
			break;
		case (14):
			strcpy(Questions.quesString, "15. When rigging up tools at the surface, if the reel gearbox is a 2 speed model, you should rig up in High Gear.");
			break;
		case (15):
			strcpy(Questions.quesString, "16. The winch operator or engineer running the reel should have full attention on the task at hand. He should be aware of all personnel on the catwalk and rig floor involved with moving the tools off the catwalk, up the pipe ramp and to the rig floor. He or she should not be looking at the computer, talking on the phone or distracted by the company man. If the company man is persistent, place the reel in brake mode, stop, use your SWA, explain to him you have to watch and concentrate specifically on the task at hand. Your crew members and rig personnel are dependent on the operator to focus on the task as they are in harms way.");
			break;
		case (16):
			strcpy(Questions.quesString, "17. The logging unit truck or skid should be spotted or centered as best as possible from the reel center to the wellhead to enable a good smooth spooling of the cable without much intervention from the operator at all.");
			break;
		case (17):
			strcpy(Questions.quesString, "18. If the spotting of the unit truck or skid is off a few degrees the operator will need to adjust the spooler using the line tension relief valve on the console.");
			break;
		case (18):
			strcpy(Questions.quesString, "19. When the operator is making up the tools on the rig floor they use hand signals to relay what he wants the winchman operator to do, the winchman operator should proceed with caution and do as he was signaled.");
			break;
		case (19):
			strcpy(Questions.quesString, "20. While rigging up tools, if the wireline becomes caught on pipe or some other obstruction the reel operator must use the SWA and correct the situation immediately.");
			break;
		case (20):
			strcpy(Questions.quesString, "21. Z charts are mandatory. They may help prevent a tool pulloff. You should record the depth and tension each time the wireline gets within 2 or 3 wraps from the flange.");
			break;
		case (21):
			strcpy(Questions.quesString, "22. A Z chart should be created for every run in the well.");
			break;
		case (22):
			strcpy(Questions.quesString, "23. Picking up tools on the surface - The line tension valve should be backed out all the way (counter clockwise) then screwed in only a few turns clockwise prior to picking up the individual tools on the surface. The worst case scenario after performing this is something hangs up during rig up and there will not be a pull off. The line tension valve may have to be adjusted in (clockwise) a turn at a time as the tool string grows in length in order to safely pick up the string off of the makeup plate. ");
			break;
		case (23):
			strcpy(Questions.quesString, "24. Bad line spooling can change the Z chart reference depths. Therefore, create a Z chart for each run. A bad spooling from a previous well or run can be corrected on the way out of the hole.");
			break;
		case (24):
			strcpy(Questions.quesString, "25. The hoist operator can be fatigued, exhausted or too sleepy to operate the winch. If anyone is to safely operate the winch he must be alert, awake, and qualified.  The person operating the hoist must be able to focus on the task and to devote full undivided attention to the tasks of running the hoist, properly controlling speeds, monitoring tension, and adjusting the line tension valve at each layer while properly spooling the wireline back on the drum.");
			break;
		case (25):
			strcpy(Questions.quesString, "26. Back up depth and drum revolution panel operation: zero backup depth and drum rev : When the top of tool (cablehead) is at the reference depth (rotary floor or packoff head) the logging software zero depth will be referenced to the bottom of the logging tool assembly; however, the backup depth panel displays the depth when the top of the tool leaves and returns to the rotary floor or the packoff head. If not  then zero point equals rig floor.");
			break;
		case (26):
			strcpy(Questions.quesString, "27. When the top of the tool reaches twenty five (25) revolutions from the reference depth, then the backup depth and drum revolution panel sounds an alarm for the remaining distance until it is zero.");
			break;
		case (27):
			strcpy(Questions.quesString, "28. Are all of the below considered general procedures: Concentrate on the task at hand. Do not get distracted.  Understand the job, the risks, the procedures and the objectives. Have a clear line of sight and a means of communication.  Hoist operation shall be conducted by competent personnel. Control speed during operation. Slow down when nearing surface, any downhole restriction, or anything that could potentially hang up the tool or create a hazard. Follow all local and global HMS procedures. If all of the above is true, answer true.  Otherwise, answer false.");
			break;
		case (28):
			strcpy(Questions.quesString, "29. Halliburton employees trained and qualified to operate the hoist are permitted to do so. Even competent third party or non-Halliburton personnel may operate the hoist. ");
			break;
		case (29):
			strcpy(Questions.quesString, "30. At the depth where the top of tool is at the lubricator reference (packoff head) or the rotary floor depending on the condition, capture the depth by:  Placing a strip of colored tape on the hoist operator window. Mark an arrow and record the reference depth using an erasable marker on the hoist operator window or erasable white board and on the Z Chart. Count wraps of wireline on the top wrap to the reference depth and record on the window and Z chart.");
			break;
		case (30):
				
			for( /*Questions.jj */; Questions.jj < (Questions.ii - 1); Questions.jj++)//iterate through 0-29
			{
				if (Questions.answer[Questions.jj] == rigUpKey[Questions.jj])
				{
					Questions.total++;
				}
			}
			testing = (REAL) Questions.total / 29.0;
			Questions.rigUpQuesScoreSum = testing * 100;
			
			strcpy(Questions.quesString,"SCORE:");
			Questions.showRigUpQuesScore = 0;
			
			break;
		default:
			break;
	}
	
}
 


 static void RigUpQuestions()
{
	switch(Questions.current_state)  //Enter each training class
	{
		case(QUESTIONS_INITIALIZE):
			questionsInitialize();
			break;
		case(QUESTIONS):
			questions();
			break;
	}
	
	/*void(*state_table3[])() = 
		{
		questionsInitialize,
		questions		
		};

	state_table3[Questions.current_state]();
	
	 //This below enum is found in Training::Training.typ
	
		enum {       
					QUESTIONS_INITIALIZE = 0,	
					QUESTIONS
									
				}   State_Type;*/
}

