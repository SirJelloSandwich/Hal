/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: Training
 * File: WinchmansWellDrivingTest.c
 * Author: ISMLaptop
 * Created: April 11, 2014
 *******************************************************************/

#include <bur/plctypes.h>


#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#define TRUE 1
#define FALSE 0

//This colors the numeric input boxes green as a spool flange is hit.
//This signifies to the user that they must input data into the numeric box.
static void updateTextBoxColor()
{
	int ttt = 0;
	
	for( ttt; ttt < WellDriving.kk ; ttt++)
	{
		WellDriving.arrayDepthColor[ttt] = 10;
		
		WellDriving.arrayTensionDownColor[ttt] = 10;
		
		WellDriving.arrayTensionUpColor[ttt] = 10;
		
	}
}

 static void zChart()
{
	USINT LL;
	USINT JJ;
	WellDriving.kkMinusOne = (WellDriving.kk - 1);
	 updateTextBoxColor();
	
	//The below is the left side of the spool.  
	if( (WellDriving.spoolFlange > 0.0000 && WellDriving.spoolFlange < 0.0080 ) && !WellDriving.stationaryCheck)
	{
		if(!WellDriving.WeCanNowGoUphole)
		{
			WellDriving.autoArrayTensionDown[WellDriving.kk] = LoadcellSim43.rUdpLCTension;
			WellDriving.autoArrayDepth[WellDriving.kk] = EncoderCtrl.CableDepth;
			WellDriving.stationaryCheck = 1;
			WellDriving.kk++;
			WellDriving.jj = WellDriving.kk;
			
		}
		
		/*if (WellDriving.WeCanNowGoUphole)
		{
			WellDriving.autoArrayTensionUp[WellDriving.mm] = LoadcellSim43.rUdpLCTension;
			WellDriving.mm ++;
		}*/
		
		if (WellDriving.WeCanNowGoUphole)
		{
			WellDriving.autoArrayTensionUp[WellDriving.kkMinusOne] = LoadcellSim43.rUdpLCTension;
			WellDriving.stationaryCheck = 1;
			
			if (WellDriving.kk > 0)
			{
				WellDriving.kk --;
			}
		}
			
	}
	
	if( (WellDriving.spoolFlange > 0.990 && WellDriving.spoolFlange < 0.999) && !WellDriving.stationaryCheck)
	{
		if(!WellDriving.WeCanNowGoUphole)
		{
			WellDriving.autoArrayTensionDown[WellDriving.kk] = LoadcellSim43.rUdpLCTension;
			WellDriving.autoArrayDepth[WellDriving.kk] = EncoderCtrl.CableDepth;
			WellDriving.stationaryCheck = 1;
			WellDriving.kk++;
			WellDriving.jj = WellDriving.kk;
		}
		
		/*if (WellDriving.WeCanNowGoUphole)
		{
			WellDriving.autoArrayTensionUp[WellDriving.mm] = LoadcellSim43.rUdpLCTension;
			WellDriving.mm ++;
		}*/
		
		if (WellDriving.WeCanNowGoUphole)
		{
			WellDriving.autoArrayTensionUp[WellDriving.kkMinusOne] = LoadcellSim43.rUdpLCTension;
			WellDriving.stationaryCheck = 1;
			
			if (WellDriving.kk > 0)
			{
				WellDriving.kk --;
			}
		}
		
	}
	
	if (WellDriving.stationaryCheck)
	{
		if(  WellDriving.spoolFlange > 0.100 && WellDriving.spoolFlange < 0.900 )//make larger window in case spool never makes it across
		{
			WellDriving.stationaryCheck = 0;
		}
	}

	if (WellDriving.buttonDoneWithZChart)
	{
		for( LL = 0; LL < 20; LL++)//iterate through 0-19
		{
			/*if ((WellDriving.autoArrayTensionDown[LL] - 500 < WellDriving.arrayTensionDown[LL]) || (WellDriving.autoArrayTensionDown[LL] + 500 > WellDriving.arrayTensionDown[LL]))//are they equal?
			{
				if ( (WellDriving.autoArrayTensionDown[LL] + WellDriving.arrayTensionDown[LL]) != 0)//are they not zero?
				{
					WellDriving.totalTensionDownScore++;
				}
			}
			
			if  ((WellDriving.autoArrayTensionUp[LL] - 500 < WellDriving.arrayTensionUp[LL]) || (WellDriving.autoArrayTensionUp[LL] + 500 > WellDriving.arrayTensionUp[LL]))
			{
				if ( (WellDriving.autoArrayTensionUp[LL] + WellDriving.arrayTensionUp[LL]) != 0)
				{
					WellDriving.totalTensionUpScore++;
				}
			}
			
			
			if  ((WellDriving.autoArrayDepth[LL] - 500 < WellDriving.arrayDepth[LL]) || (WellDriving.autoArrayDepth[LL] + 500 > WellDriving.arrayDepth[LL]))
			{
				if ( (WellDriving.autoArrayDepth[LL] + WellDriving.arrayDepth[LL]) != 0)
				{
					WellDriving.totalDepthScore++;
				}
			}*/
			
			if ( WellDriving.arrayTensionDown[LL] > (WellDriving.autoArrayTensionDown[LL] - 500) && (WellDriving.arrayTensionDown[LL] < (WellDriving.autoArrayTensionDown[LL] + 500)))// are they within a 1000 unit range
			{
				if (( WellDriving.arrayTensionDown[LL] + WellDriving.autoArrayTensionDown[LL]) != 0)//Not zero
				{
					WellDriving.totalTensionDownScore++;
				}
			}
			
			if ( WellDriving.arrayTensionUp[LL] > (WellDriving.autoArrayTensionUp[LL] - 500) && (WellDriving.arrayTensionUp[LL] < (WellDriving.autoArrayTensionUp[LL] + 500)))// are they within a 1000 unit range
			{
				if (( WellDriving.arrayTensionUp[LL] + WellDriving.autoArrayTensionUp[LL]) != 0)//Not zero
				{
					WellDriving.totalTensionUpScore++;
				}
			}
			
			if ( WellDriving.arrayDepth[LL] > (WellDriving.autoArrayDepth[LL] - 500) && (WellDriving.arrayDepth[LL] < (WellDriving.autoArrayDepth[LL] + 500)))// are they within a 1000 unit range
			{
				if (( WellDriving.arrayDepth[LL] + WellDriving.autoArrayDepth[LL]) != 0)//Not zero
				{
					WellDriving.totalDepthScore++;
				}
			}
			WellDriving.zChartDenom = (WellDriving.jj*3);
		
				if (WellDriving.zChartDenom >0)
			{
				WellDriving.totalZChartScore = 
					((WellDriving.totalTensionDownScore + WellDriving.totalTensionUpScore + WellDriving.totalDepthScore)
					/ 	(WellDriving.zChartDenom) )* 100;
					}
			gHmi.prv.pageChange = 7;
			gHmi.key.tgl.training = 0;
			WellDriving.buttonDoneWithZChart = 0;
			
			
		}
	
	}
}

  static void wellDrivingScore()
{
	WellDriving.denominator =  (WellDriving.equalDepthsPass +
								WellDriving.equalDepthsFail +
								WellDriving.FreewheelPass +
								WellDriving.FreewheelFail +
								WellDriving.BridgeOutPass +
								WellDriving.BridgeOutFail +
								WellDriving.ToolPullOffPass +
								WellDriving.ToolPullOffFail +
								WellDriving.UpholeSloughingPass +
								WellDriving.UpholeSloughingFail +
								WellDriving.Auto300Pass +
								WellDriving.Auto300Fail +
								WellDriving.Auto150Pass +
								WellDriving.Auto150Fail +
								WellDriving.CasingShoePass +
								WellDriving.CasingShoeFail);
				
	if (WellDriving.denominator > 0)
	{	
		WellDriving.trainingScoreSum =  
			(((REAL)  WellDriving.equalDepthsPass + 
			  (REAL) WellDriving.FreewheelPass + 
			  (REAL) WellDriving.BridgeOutPass +
			  (REAL) WellDriving.ToolPullOffPass +
			  (REAL) WellDriving.UpholeSloughingPass +
			  (REAL) WellDriving.Auto300Pass +
			  (REAL) WellDriving.Auto150Pass +
			  (REAL) WellDriving.CasingShoePass 
		     ) / 
			  (REAL) WellDriving.denominator) * 100;
	}
}

  static void areTwoDepthsEqual_WD(void)
{
	
	if (EncoderSimSpool43.rLineSpeed > 6)
	{

		if ( ((EncoderCtrl.CableDepth + 5 < J1939WirelineDepthRx.depth) || (EncoderCtrl.CableDepth - 5 > J1939WirelineDepthRx.depth)) & !WellDriving.firstpassWD)
		{
			WellDriving.firstpassWD = 1;
			WellDriving.equalDepthsFail++;
		}
	
		if ( ((EncoderCtrl.CableDepth + 5 > J1939WirelineDepthRx.depth) || (EncoderCtrl.CableDepth - 5 < J1939WirelineDepthRx.depth)) & !WellDriving.firstpassWD)
		{
			WellDriving.firstpassWD = 1;
			WellDriving.equalDepthsPass++;
		}
	}
}

 static void Initialize() //current_state 0
{
	int iiii;
	int jjj = 0;
	int mmm = 0;
	
	for( mmm; mmm < 20 ; mmm++)// This colors the numeric input boxes white
	{
		WellDriving.arrayDepthColor[mmm] = 15;
		
		WellDriving.arrayTensionDownColor[mmm] = 15;
		
		WellDriving.arrayTensionUpColor[mmm] = 15;		
	}
	
	WellDriving.kk = 0;
	WellDriving.mm = 0;
	//WellDriving.tensionUpBoxNotEmpty = 0;
	
	WellDriving.stationaryCheck = 0;
	
	WellDriving.totalZChartScore = 0;
	WellDriving.totalTensionDownScore = 0;
	WellDriving.totalTensionUpScore = 0;
	WellDriving.totalDepthScore = 0;
	
	for (iiii = 0; iiii < 20; iiii++)
	{
		WellDriving.autoArrayTensionDown[iiii] = 0;
		WellDriving.autoArrayTensionUp[iiii] = 0;
		WellDriving.autoArrayDepth[iiii] = 0;
		
		WellDriving.arrayTensionDown[iiii] = 0;
		WellDriving.arrayTensionUp[iiii] = 0;
		WellDriving.arrayDepth[iiii] = 0;
		
	}
	
	WellDriving.trainingScoreSum = 0;
	WellDriving.denominator = 0;
	WellDriving.equalDepthsPass = 0;
	WellDriving.equalDepthsFail = 0;
	WellDriving.FreewheelPass = 0;
	WellDriving.FreewheelFail = 0;
	WellDriving.BridgeOutPass = 0;
	WellDriving.BridgeOutFail = 0;
	WellDriving.ToolPullOffPass = 0;
	WellDriving.ToolPullOffFail = 0;
	WellDriving.UpholeSloughingPass = 0;
	WellDriving.UpholeSloughingFail = 0;
	WellDriving.Auto300Pass = 0;
	WellDriving.Auto300Fail = 0;
	WellDriving.Auto150Pass = 0;
	WellDriving.Auto150Fail = 0;
	WellDriving.CasingShoePass = 0;
	WellDriving.CasingShoeFail = 0;
	
	gHmi.prv.pageChange = 12;
	
	
	
	//set well parameter based on current total depth
	WellDriving.casingBottom     = (gConfig.active.trainer.totalBottom * 0.2041);
	WellDriving.freewheelStart   = (gConfig.active.trainer.totalBottom * 0.4489);
	WellDriving.freewheelEnd     = (gConfig.active.trainer.totalBottom * 0.5306);
	WellDriving.bridgeoutStart   = (gConfig.active.trainer.totalBottom * 0.5612);
	WellDriving.bridgeoutEnd     = (gConfig.active.trainer.totalBottom * 0.6122);
	WellDriving.toolPulloffStart = (gConfig.active.trainer.totalBottom * 0.9387);
	WellDriving.sloughingStart   = (gConfig.active.trainer.totalBottom * 0.7959);
		
	gHmi.locValue.selectedInitialCondition = 0;
	gHmi.key.mom.ICreset = 1; 
	
	WellDriving.kkMinusOne = 0;
	
	//WellDriving.current_state = DOWNHOLE_FREEWHEEL;//current_state 1
	
	if (IOScmdData.CurrentMode == 3)  //Delay timer so that the IG can finish ICing
	{
		WellDriving.timer6.IN = 1;
		WellDriving.timer6.PT = 4000;
		TON(&WellDriving.timer6);
	} 
	
	if (WellDriving.timer6.Q)
	{
		WellDriving.current_state = DOWNHOLE_FREEWHEEL;//current_state 1
	}
}

 static void DownholeFreewheel()//current_state 1
{	
		//We want to set autoArrayTensionDown[0] and autoArrayDepth[0] when we first start and begin training.
	//int jjj; 
	
	/*if(!jjj)
	{
		WellDriving.autoArrayTensionDown[0] = LoadcellSim43.rUdpLCTension;
		WellDriving.autoArrayDepth[0] = EncoderCtrl.CableDepth;
		jjj = 1;
		WellDriving.kk++;
	}*/
	
	
	areTwoDepthsEqual_WD();
	
	WellDriving.bNoReIC = TRUE;
	
	if((WellDriving.CurrentCaliperBrakeKnob < gAnalogCtrl.fCaliperBrakeKnob - .25) || (WellDriving.CurrentCaliperBrakeKnob > gAnalogCtrl.fCaliperBrakeKnob + .25))
	{
		WellDriving.CurrentCaliperBrakeKnob = gAnalogCtrl.fCaliperBrakeKnob;
		WellDriving.CaliperKnobTurned = 1;
	}
			
	if (((EncoderCtrl.CableDepth > WellDriving.freewheelStart) & (EncoderCtrl.CableDepth < WellDriving.freewheelEnd) )  & !WellDriving.ReactedToFreewheeling) //RAB was 2200 ft
	{
		gHmi.locValue.selectedMalfunction = 1;//Freewheeling Malfunction
			
		if (WellDriving.CaliperKnobTurned & !WellDriving.firstpass2)
		{
			WellDriving.firstpass2 = 1;
			WellDriving.CaliperKnobTurned = 0;
		}
			
		if (WellDriving.CaliperKnobTurned & (EncoderSimSpool43.rLineSpeed < 1))
		{
			gHmi.locValue.selectedMalfunction = 0;
			WellDriving.ReactedToFreewheeling = 1;
			WellDriving.CaliperKnobTurned = 0;
			WellDriving.FreewheelPass++;
			WellDriving.current_state = DOWNHOLE_BRIDGE_OUT;//current_state 2
		}
		
	}
	
	if ((EncoderCtrl.CableDepth > WellDriving.freewheelEnd) & !WellDriving.ReactedToFreewheeling)
	{
		gHmi.locValue.selectedMalfunction = 0;
		WellDriving.FreewheelFail++;
		WellDriving.current_state = DOWNHOLE_BRIDGE_OUT;
	}
				
	if (WellDriving.ReactedToFreewheeling & (EncoderCtrl.CableDepth > (WellDriving.freewheelEnd + 5)))   //Reset malfunction trigger when cable is greater than 2600 ft.
	{
		WellDriving.ReactedToFreewheeling = 0;
		//WellDriving.current_state = DOWNHOLE_BRIDGE_OUT;
		gHmi.locValue.selectedMalfunction = 0;
	}
}

 static void DownholeBridgeOut()//current_state 2
{	
	//Downhole Malfunction 2 "Bridge Out"
	
	if ((EncoderCtrl.CableDepth > WellDriving.bridgeoutStart) & (EncoderCtrl.CableDepth < WellDriving.bridgeoutEnd) )
	{
		gHmi.locValue.selectedMalfunction = 2;//Bridge Out Malfunction
		
		WellDriving.timer5.IN = 1;
		WellDriving.timer5.PT = 13000; // Give the operator 13 seconds to react.
		TON(&WellDriving.timer5);
		
		if ( (EncoderSimSpool43.rLineSpeed < 1) & !WellDriving.timer5.Q)// stopped
		{
			
			if ((EncoderSimSpool43.rLineSpeed < -10) & !WellDriving.timer5.Q)//pulls slowly back up
			{
				gHmi.locValue.selectedMalfunction = 0;//Clear Malfunction
			
				WellDriving.timer5.IN = 0;
				WellDriving.timer5.Q = 0; 
				TON(&WellDriving.timer5);
			
				WellDriving.BridgeOutPass++;
			
				WellDriving.current_state = UPHOLE_TOOL_PULLOFF;//current_state 3
			}
		}
	}
		
	
	if( WellDriving.timer5.Q)
	{
		WellDriving.BridgeOutFail++;
		gHmi.locValue.selectedMalfunction = 0;//Clear Malfunction
		WellDriving.timer5.IN = 0;
		WellDriving.timer5.Q = 0; 
		TON(&WellDriving.timer5);
		
		WellDriving.current_state = UPHOLE_TOOL_PULLOFF;
	}
	
	if (EncoderCtrl.CableDepth >= (gConfig.active.trainer.totalBottom - 10))
	{
		gHmi.locValue.selectedMalfunction = 0;//Clear Malfunction
		WellDriving.BridgeOutFail++;
		WellDriving.current_state = UPHOLE_TOOL_PULLOFF;
	}
	
	
}

 static void UpholeToolPulloff()//current_state 3
{	
	//Uphole Malfunction 1 "Tool Pulloff"
	
	
	//Uphole is negative number. Downhole is a Pos number.	
	if ((EncoderCtrl.CableDepth < (WellDriving.toolPulloffStart)) & WellDriving.WeCanNowGoUphole & !WellDriving.ToolPullOffFail)
	{
		gHmi.locValue.selectedMalfunction = 14;//Tool Pulloff malfunction
		WellDriving.ToolPullOffIsOccurring = 1;
	}

	if ((EncoderSimSpool43.rLineSpeed > -6) & WellDriving.ToolPullOffIsOccurring)//operator slowing down.  line speed was > -6
	{
		WellDriving.ToolPullOffReaction = 1;
	}
	
	if(WellDriving.ToolPullOffReaction & (EncoderSimSpool43.rLineSpeed > 6))// operator stopped and reversed direction
	{
		gHmi.locValue.selectedMalfunction = 0;       //Clear Tool Pulloff malfunction
		WellDriving.ToolPullOffReaction = 0;
		WellDriving.ToolPullOffIsOccurring = 0;
		WellDriving.ToolPullOffPass++;               //If this scenario passes, then IC does not reset to last IC
		WellDriving.current_state = UPHOLE_SLOUGHING;//current_state 4
	} 
	
	if ( (IOSstatusData.CurrentMode == 1) & (IOScmdData.CurrentMode == 5))
	{
		WellDriving.counter++;
		
		if (WellDriving.counter >= 5000)
		{
			WellDriving.ToolPullOffFail++;           
			gHmi.key.crashResetBtn = 1;              
			gHmi.locValue.selectedMalfunction = 0;
			WellDriving.counter = 0;
			
		}
	}
	
	if ((IOSstatusData.CurrentMode == 1) & (IOScmdData.CurrentMode == 2) & (WellDriving.ToolPullOffFail >= 1) )
	{
		gHmi.locValue.selectedMalfunction = 0;
		gHmi.key.radioBtnFreezeRun.status = 2;//RUN!
		WellDriving.current_state = UPHOLE_SLOUGHING;
	}
}

 static void UpholeSloughing()//current_state 4
{	
	//Uphole Malfunction 2 "Sloughing"
	if (!gHmi.key.radioBtnFreezeRun.status == 2	)
	{
		gHmi.key.radioBtnFreezeRun.status = 2;//RUN!
	}
	
	if (EncoderCtrl.CableDepth < (WellDriving.sloughingStart))//This is a trouble spot bc of re-IC after failure above.  URS
	{
		gHmi.locValue.selectedMalfunction = 4;//Sloughing Malfunction
	}
	
	if ((gHmi.locValue.selectedMalfunction ==  4) & (EncoderSimSpool43.rLineSpeed > 10) )//Uphole speed is negative number. Downhole speed is a Pos number.
	{
		gHmi.locValue.selectedMalfunction = 0;
		
		WellDriving.UpholeSloughingPass++;
		
		WellDriving.current_state = BLANK;//Advance to next state current_state = 5.  BLANK state does nothing.  Its empty.
	}
	
	if ((IOSstatusData.CurrentMode == 1) & (IOScmdData.CurrentMode == 5))//Crash State
	{
		WellDriving.counter++;
		
		if (WellDriving.counter >= 5000)
		{
			WellDriving.UpholeSloughingFail++;
			gHmi.key.crashResetBtn = 1;
			gHmi.locValue.selectedMalfunction = 0;
			WellDriving.counter = 0;
		}
	}
	
	if ((IOSstatusData.CurrentMode == 1) & (IOScmdData.CurrentMode == 2) & (WellDriving.UpholeSloughingFail >= 1) )
	{
		gHmi.key.radioBtnFreezeRun.status = 2;//RUN!
		gHmi.locValue.selectedMalfunction = 0;
		WellDriving.current_state = BLANK;				
	}	
}	

 static void blank()//current_state = 5
{
	//In here doing nothing
}


 static void Uphole300FtAutoSlowDown()
{

	//Uphole Malfunction 4 "300ft Auto Slow Down"
if (((EncoderCtrl.CableDepth < 350) & (EncoderCtrl.CableDepth > 250)) & PanelIn.bSlowSpd & !WellDriving.Auto300PassKey & WellDriving.WeCanNowGoUphole)
	{
		WellDriving.Auto300PassKey = 1;
		WellDriving.Auto300Pass++;
	}

if (((EncoderCtrl.CableDepth < 350) & (EncoderCtrl.CableDepth > 250)) & !PanelIn.bSlowSpd & !WellDriving.Auto300FailKey & WellDriving.WeCanNowGoUphole)
	{
		WellDriving.Auto300FailKey = 1;
		WellDriving.Auto300Fail++;
	}

}	

 static void Uphole150FtAutoStop ()
{	
	//Uphole Malfunction 5 "150ft Auto Stop"

	if (((EncoderCtrl.CableDepth < 200) & (EncoderCtrl.CableDepth > 100)) & PanelIn.bSlowSpd & !WellDriving.Auto150PassKey & WellDriving.WeCanNowGoUphole)
	{
		WellDriving.Auto150PassKey = 1;
		WellDriving.Auto150Pass++;
	}
	
	if (((EncoderCtrl.CableDepth < 200) & (EncoderCtrl.CableDepth > 100)) & !PanelIn.bSlowSpd & !WellDriving.Auto300FailKey & WellDriving.WeCanNowGoUphole)
	{
		WellDriving.Auto150FailKey = 1;
		WellDriving.Auto150Fail++;
	}
}	

/*static void UpholeLineTension()
{	
	//Uphole Malfunction 6 "Line Tension"
	//In order to do this we need Data from URS
	//This is currently being worked out. 4/21/2014
}

static void UpholeZChart()
{	
	//Uphole Malfunction 7 "Z Chart"
}

static void UpholeDepthAlarmBackupDepth()
{

	//Uphole Malfunction 8 "Depth Alarm Back Up Depth"
}*/

 static void UpholeCasingShoe()//WAS current_state 5, now it runs in WinchmansWellDrivingTest()
{	
	//Uphole Malfunction 3 "Casing Shoe"

		if ((EncoderCtrl.CableDepth > (WellDriving.casingBottom - 50)) && (EncoderCtrl.CableDepth < (WellDriving.casingBottom + 50)))
		{
			if( ((EncoderSimSpool43.rLineSpeed > -10) && (EncoderSimSpool43.rLineSpeed < 10)) && !WellDriving.CasingShoePassKey)//Uphole speed is negative number. Downhole speed is a Pos number.
			{
				WellDriving.CasingShoePassKey = 1;
				WellDriving.CasingShoePass++;
			}
			
			if (((EncoderSimSpool43.rLineSpeed < -10) && (EncoderSimSpool43.rLineSpeed > 10) )&& !WellDriving.CasingShoeFailKey)
			{
				WellDriving.CasingShoeFailKey = 1;
				WellDriving.CasingShoeFail++;
			}
				
		}
	
	}

 static void WinchmansWellDrivingTest()
{
	switch(WellDriving.current_state)  //Enter each training class
	{
		case(INITIALIZE):
			Initialize();
			break;
		case(DOWNHOLE_FREEWHEEL):
			DownholeFreewheel();
			break;
		case(DOWNHOLE_BRIDGE_OUT):
			DownholeBridgeOut();
			break;
		case(UPHOLE_TOOL_PULLOFF):
			UpholeToolPulloff();
			break;
		case(UPHOLE_SLOUGHING):
			UpholeSloughing();
			break;
		case(BLANK):
			blank();
			break;
	}
	
		
	zChart();
	
	wellDrivingScore();
	
	if(WellDriving.timer6.Q)
	{
		UpholeCasingShoe();
	}
	
	Uphole300FtAutoSlowDown();
	
	Uphole150FtAutoStop ();
	
	if (EncoderCtrl.CableDepth > (WellDriving.toolPulloffStart))
	{	
		WellDriving.WeCanNowGoUphole = 1;
	}

}


/*void(*state_table2[])() = 
		{
		Initialize,
		DownholeFreewheel,
		DownholeBridgeOut,
		UpholeToolPulloff,
		UpholeSloughing,
		blank
		//UpholeCasingShoe,
		//Uphole300FtAutoSlowDown,
		//Uphole150FtAutoStop,
		//UpholeLineTension,
		//UpholeZChart,
		//UpholeDepthAlarmBackupDepth
		};

	state_table2[WellDriving.current_state]();

		enum {        This enum is found in Training::Training.typ
					INITIALIZE = 0,	
					DOWNHOLE_FREEWHEEL,
					DOWNHOLE_BRIDGE_OUT,
					UPHOLE_TOOL_PULLOFF,
					UPHOLE_SLOUGHING,
					BLANK
					//UPHOLE_CASING_SHOE,
					//UPHOLE_300FT_AUTO_SLOW_DOWN,
					//UPHOLE_150FT_AUTO_STOP,
					//UPHOLE_LINE_TENSION,
					//UPHOLE_Z_CHART,
					//UPHOLE_DEPTH_ALARM_BACKUP_DEPTH
				}   State_Type;*/

