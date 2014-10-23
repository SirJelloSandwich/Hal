
#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

int abs (int);
int key ;

 static int wasAlarmAcknowledged()
{
	if ( 0 < EncoderCtrl.CableDepth < 5)
	{
		if (RigUpModule.bduAlarmAck && !key )
		{
			key = 1;
			return 1;
		}
		return 0;
	}
}

static void areTwoDepthsEqual_RU(void)
{
	
	if (EncoderSimSpool43.rLineSpeed > 6)
	{

		if ( ((EncoderCtrl.CableDepth + 5 < J1939WirelineDepthRx.depth) || (EncoderCtrl.CableDepth - 5 > J1939WirelineDepthRx.depth)) & !RigUpModule.firstpassRU)
		{
			RigUpModule.firstpassRU = 1;
			RigUpModule.equalDepthsFail++;
		}
	
		if ( ((EncoderCtrl.CableDepth + 5 > J1939WirelineDepthRx.depth) || (EncoderCtrl.CableDepth - 5 < J1939WirelineDepthRx.depth)) & !RigUpModule.firstpassRU)
		{
			RigUpModule.firstpassRU = 1;
			RigUpModule.equalDepthsPass++;
		}
	}
}

static void TooFast()
{
	if (((EncoderSimSpool43.rLineSpeed > 60) || (EncoderSimSpool43.rLineSpeed < -60)) & !RigUpModule.onetime)// This is the speed POLICE!  Don't go over 10ft/min, if so you fail.
	{						
		RigUpModule.TooFast = 1;
		RigUpModule.onetime = 1;
	}
	if (((EncoderSimSpool43.rLineSpeed < 3 && EncoderSimSpool43.rLineSpeed > 0) 
		|| (EncoderSimSpool43.rLineSpeed < 0 && EncoderSimSpool43.rLineSpeed > -3))  & RigUpModule.onetime)
		//After speeding and then slowing down we reset the TooFast flag.
	{
		RigUpModule.onetime = 0;
		RigUpModule.TooFast = 0;
	}
}

static void RigUpScore()
{
	RigUpModule.Denominator =
		(RigUpModule.CatwalkTargetPass + 
		RigUpModule.RigFloorTargetPass +
		RigUpModule.equalDepthsPass + 
		RigUpModule.equalDepthsFail +
		RigUpModule.FailBySpeed + 
		RigUpModule.FailByTime 
		 );
				
	if (RigUpModule.Denominator > 0)
	{	
			
		RigUpModule.TrainingScoreSum =  
			(((REAL) RigUpModule.CatwalkTargetPass + 
			 (REAL) RigUpModule.RigFloorTargetPass +
			 (REAL) RigUpModule.equalDepthsPass) / 
			 (REAL) RigUpModule.Denominator) * 100;
	}
	
}

static void StartTimer1()
{	
	RigUpModule.timer1.IN = 1;
	RigUpModule.timer1.PT = 300000;// 5 minutes
					
	TON(&RigUpModule.timer1);
					
	gHmi_Time.output1 = RigUpModule.timer1.Q;
	gHmi_Time.elapsed_time1 = RigUpModule.timer1.ET;
}	

static void StartTimer2()
{
				
	RigUpModule.timer2.IN = 1;
	RigUpModule.timer2.PT = 300000;
				
	TON(&RigUpModule.timer2);
				
	gHmi_Time.output2 = RigUpModule.timer2.Q;
	gHmi_Time.elapsed_time2 = RigUpModule.timer2.ET;
						
	
}

static void RigUpInitialize()//current_state 0
{	
	RigUpModule.begin0 = 0;
	RigUpModule.begin1 = 0;
	
	RigUpModule.TrainingScoreSum = 0;
	RigUpModule.Denominator = 0;
	RigUpModule.CatwalkTargetPass = 0;
	RigUpModule.RigFloorTargetPass = 0;
	RigUpModule.equalDepthsPass = 0;
	RigUpModule.equalDepthsFail = 0;
	RigUpModule.FailBySpeed = 0;
	RigUpModule.FailByTime = 0;
	
	if (RigUpModule.firstpass) //load IC set 1 when training class one is selected
	{
		gHmi.locValue.selectedInitialCondition = 0;
		gHmi.key.mom.ICreset = 1;
		gHmi.prv.pageChange = 8;
		//RigUpModule.firstpass = 0;
	}
			
	if (IOScmdData.CurrentMode == 3)  //Delay timer so that the IG can finish ICing
	{
		RigUpModule.timer3.IN = 1;
		RigUpModule.timer3.PT = 4000;
		TON(&RigUpModule.timer3);
		gHmi_Time.output3 = RigUpModule.timer3.Q;
		gHmi_Time.elaspsed_time3 = RigUpModule.timer3.ET;
	}
	
	if (RigUpModule.timer3.Q)
	{
		RigUpModule.current_state = TWENTY_FOOT_TARGET;
	}
	
}
	
static void twentyFootTarget()//current_state 1
{
	areTwoDepthsEqual_RU();
	
	RigUpModule.ColorDatapoint1 = 7;
	
	if( abs((USINT)EncoderSimSpool43.rLineSpeed) > 1)
	{
		RigUpModule.begin0 = 1;
	}
	
	if(RigUpModule.begin0)
	{
		StartTimer1();
	}
	
	if( abs(((USINT)EncoderSimSpool43.rLineSpeed) > 1) & RigUpModule.firstpass)
		{
				
			RigUpModule.firstpass = 0;
		}		
	
	
	if ( abs((USINT)EncoderSimSpool43.rLineSpeed) < 1 
		&& !gHmi_Time.output1 							
		&& !RigUpModule.PassSwitch1	
		&& !RigUpModule.TooFast		
		&& EncoderCtrl.CableDepth > 18.5 
		&& EncoderCtrl.CableDepth < 21.5 
		&& gHmi_Time.output3)
		
			{			
				RigUpModule.timer4.IN = 1;
				RigUpModule.timer4.PT = 10000;
				TON(&RigUpModule.timer4);
				
					if (RigUpModule.timer4.Q)
					{
						RigUpModule.PassSwitch1 = 1;//Passed objective 1; wire line has stopped in the depth 
													//window within 5 min and stayed there for 10s 
													//and speed of 60ft/min was NOT exceeded
						RigUpModule.PassSwitch2 = 0;						
						
						//RigUpModule.ColorDatapoint1 = 10;//Obj 1 Green 
								
						RigUpModule.CatwalkTargetPass++;
								
						RigUpModule.timer1.IN = 0;
						RigUpModule.timer1.Q = 0;
						TON(&RigUpModule.timer1);
								
						RigUpModule.timer4.IN = 0;
						RigUpModule.timer4.Q = 0;
						TON(&RigUpModule.timer4);
			
						RigUpModule.begin1 = 0;
						
						RigUpModule.current_state = FOUR_FOOT_TARGET;
							
					}
												
			}
	
	if((EncoderCtrl.CableDepth < 18.5 || EncoderCtrl.CableDepth > 21.5) && RigUpModule.TooFast) //Fail by speed
		{																													
			RigUpModule.FailBySpeed++;			
			RigUpModule.ColorDatapoint1 = 51;// Red  Fail
			RigUpModule.TooFast = 0;
		}
	
	if((EncoderCtrl.CableDepth < 18.5 || EncoderCtrl.CableDepth > 21.5) && RigUpModule.timer1.Q) //Fail by time
		{
			RigUpModule.ColorDatapoint1 = 51;// Red  Fail
			RigUpModule.FailByTime++;
			RigUpModule.timer1.IN = 0;			
			RigUpModule.timer1.Q = 0;
			TON(&RigUpModule.timer1);
			
			StartTimer1();
				
		}
}

static void fourFootTarget()
{
	//RigUpModule.ColorDatapoint1 = 7;
	if( abs((USINT)EncoderSimSpool43.rLineSpeed) > 1)
	{
		RigUpModule.begin1 = 1;
	}
	
	if(RigUpModule.begin1)
	{
		StartTimer2();
	}
			
	
	if ( abs( (USINT) EncoderSimSpool43.rLineSpeed) < 1 
		&& !gHmi_Time.output2	
		&& EncoderCtrl.CableDepth > 3.25 
		&& EncoderCtrl.CableDepth < 4.75 
		&& !RigUpModule.PassSwitch2 
		&& !RigUpModule.TooFast ) 	
				
			{											
				RigUpModule.timer4.IN = 1;
				RigUpModule.timer4.PT = 10000;
				TON(&RigUpModule.timer4);
							
					if (RigUpModule.timer4.Q)
						{
													//passed objective 2 (Rig Floor)
													//wire line has stopped in the depth window within 5 min and stayed there for 10s
													// and speed of 10ft/min was NOT exceeded
							RigUpModule.RigFloorTargetPass++;
							
							RigUpModule.timer2.IN = 0;
							RigUpModule.timer2.Q = 0;
							TON(&RigUpModule.timer2);
												
							RigUpModule.timer4.IN = 0;
							RigUpModule.timer4.Q = 0;
							TON(&RigUpModule.timer4);
									
							RigUpModule.PassSwitch2 = 1;
							RigUpModule.PassSwitch1 = 0;							
							
							RigUpModule.ColorDatapoint2 = 10;//Green
			
							RigUpModule.begin0 = 0;
			
							RigUpModule.current_state = TWENTY_FOOT_TARGET;
			
							//RigUpModule.FailSwitch1 = 0;
			
						}
									
			}
	
	if((EncoderCtrl.CableDepth < 3.25 || EncoderCtrl.CableDepth > 4.75) && RigUpModule.TooFast) //Fail by speed
	{																													
		RigUpModule.FailBySpeed++;
		RigUpModule.ColorDatapoint2 = 51;// Red  Fail
		RigUpModule.TooFast = 0;
	}
	
	if (((EncoderCtrl.CableDepth < 3.25 || EncoderCtrl.CableDepth > 4.75) && RigUpModule.timer2.Q) /*|| RigUpModule.TooFast*/) // Fail by Time
	{
		RigUpModule.ColorDatapoint2 = 51;
		RigUpModule.FailByTime++;
		RigUpModule.timer2.IN = 0;
		RigUpModule.timer2.Q = 0;
		TON(&RigUpModule.timer2);
	
		StartTimer2();
	
	}

}


static void RigUpTest()
{
	
	if (wasAlarmAcknowledged())
	{
		RigUpModule.bduAlarmPass = 1;
	}
	
	TooFast();

	RigUpScore();
	
	switch(RigUpModule.current_state)  //Enter each training class
	{
		case(RIGUPINITIALIZE):
			RigUpInitialize();
			break;
		case(TWENTY_FOOT_TARGET):
			twentyFootTarget();
			break;
		case(FOUR_FOOT_TARGET):
			fourFootTarget();
			break;
		//case(4):
		//break;
	}
}
	/*static void (*state_table1[])() = 
			{
				RigUpInitialize,
				twentyFootTarget,
				fourFootTarget
				//Gtet,
				//Dsnt,
				//Sdlt,
				//Acrt
			};
	
		state_table1[RigUpModule.current_state]();
		
		enum{
				RIGUPINITIALIZE = 0,
				TWENTY_FOOT_TARGET,
				FOUR_FOOT_TARGET,
				GTET,
				DSNT,
				SDLT,
				ACRT
				
		}State_Type2;*/
