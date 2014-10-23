(********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Package: Training2
 * File: Training2.typ
 * Author: ISMLaptop
 * Created: April 11, 2014
 ********************************************************************
 * Data types of package Training2
 ********************************************************************)

TYPE
	Main_typ : 	STRUCT 
		acknowledgeButton : USINT;
		disclaimerDisplay : USINT;
		ackButtonDisplay : USINT;
	END_STRUCT;
	Questions_typ : 	STRUCT 
		current_state : USINT;
		buttonTrue : USINT;
		buttonFalse : USINT;
		ii : USINT;
		jj : USINT;
		ques : ARRAY[0..30]OF USINT;
		quesString : STRING[650];
		answer : ARRAY[0..30]OF USINT;
		total : USINT;
		iii : USINT;
		rigUpQuesScoreSum : REAL;
		showRigUpQuesScore : UINT;
	END_STRUCT;
	State_Type3 : 
		(
		QUESTIONS_INITIALIZE := 0,
		QUESTIONS := 1
		);
	State_Type2 : 
		(
		RIGUPINITIALIZE := 0,
		TWENTY_FOOT_TARGET := 1,
		FOUR_FOOT_TARGET := 2
		);
	State_Type : 
		(
		INITIALIZE := 0,
		DOWNHOLE_FREEWHEEL := 1,
		DOWNHOLE_BRIDGE_OUT := 2,
		UPHOLE_TOOL_PULLOFF := 3,
		UPHOLE_SLOUGHING := 4,
		BLANK := 5
		);
	WellDriving_typ : 	STRUCT 
		CurrentCaliperBrakeKnob : USINT;
		CaliperKnobTurned : USINT;
		ReactedToFreewheeling : USINT;
		timer5 : TON;
		timer6 : TON;
		BridgeOutFail : USINT;
		ReactedToToolPulloff : USINT;
		firstpass2 : USINT;
		current_state : USINT;
		ToolPullOffIsOccurring : USINT;
		ToolPullOffReaction : USINT;
		FreewheelPass : USINT;
		BridgeOutPass : USINT;
		ToolPullOffPass : USINT;
		CasingShoePass : USINT;
		CasingShoeFail : USINT;
		CasingShoePassKey : USINT;
		CasingShoeFailKey : USINT;
		Auto300FailKey : USINT;
		Auto300Fail : USINT;
		Auto300PassKey : USINT;
		Auto300Pass : USINT;
		Auto150FailKey : USINT;
		Auto150Fail : USINT;
		Auto150PassKey : USINT;
		Auto150Pass : USINT;
		WeCanNowGoUphole : USINT := 0;
		UpholeSloughingFail : USINT;
		UpholeSloughingPass : USINT := 0;
		FreewheelFail : USINT;
		ToolPullOffFail : USINT;
		counter : UINT;
		counter2 : USINT;
		freewheelStart : REAL;
		freewheelEnd : REAL;
		bridgeoutStart : REAL;
		bridgeoutEnd : REAL;
		toolPulloffStart : REAL;
		sloughingStart : REAL;
		casingBottom : REAL;
		equalDepthsFail : USINT;
		equalDepthsPass : USINT;
		denominator : USINT;
		trainingScoreSum : USINT;
		firstpassWD : USINT;
		spoolFlange : REAL;
		arrayDepth : ARRAY[0..19]OF DINT;
		arrayDepthColor : ARRAY[0..19]OF USINT;
		arrayTensionUp : ARRAY[0..19]OF DINT;
		arrayTensionUpColor : ARRAY[0..19]OF USINT;
		arrayTensionDown : ARRAY[0..19]OF DINT;
		arrayTensionDownColor : ARRAY[0..19]OF USINT;
		autoArrayTensionDown : ARRAY[0..19]OF DINT;
		autoArrayTensionUp : ARRAY[0..19]OF DINT;
		autoArrayDepth : ARRAY[0..19]OF DINT;
		jj : USINT;
		kk : USINT;
		mm : USINT;
		stationaryCheck : USINT;
		buttonDoneWithZChart : USINT;
		totalTensionDownScore : REAL;
		totalTensionUpScore : REAL;
		totalDepthScore : REAL;
		totalZChartScore : REAL;
		bNoReIC : BOOL;
		tensionUpBoxNotEmpty : USINT;
		begin : USINT;
		zChartDenom : USINT;
		kkMinusOne : USINT;
	END_STRUCT;
	RigUpModule_typ : 	STRUCT 
		PassSwitch1 : USINT;
		onetime : USINT;
		firstpass : USINT;
		FailSwitch2 : USINT;
		FailSwitch1 : USINT;
		DifferentialTensionAlarm : UDINT;
		Denominator : UINT;
		CatwalkScore : USINT;
		CatwalkTargetFail : USINT;
		CatwalkTargetPass : USINT;
		ColorDatapoint1 : USINT;
		ColorDatapoint2 : USINT;
		PassSwitch2 : USINT;
		RigFloorTargetFail : USINT;
		RigFloorTargetPass : USINT;
		TensionAlarm : UDINT;
		TooFast : USINT;
		timer1 : TON;
		timer2 : TON;
		timer3 : TON;
		timer4 : TON;
		gAppDisapp1 : USINT;
		gAppDisapp2 : USINT;
		FailBySpeed : USINT;
		FailByTime : USINT;
		current_state : USINT;
		equalDepthsFail : USINT;
		equalDepthsPass : USINT;
		bduAlarmAck : USINT;
		TrainingScoreSum : USINT;
		firstpassRU : USINT;
		bduAlarmPass : USINT;
		begin0 : USINT;
		begin1 : USINT;
	END_STRUCT;
END_TYPE
