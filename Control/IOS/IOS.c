  /********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: IOS
 * File: IOS.c
 * Author: ISM
 * Created: March 27, 2013
 ********************************************************************
 * Implementation of program IOS
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#define TRUE  1
#define FALSE 0

static void delaystart()
{
	if (delay > 10001)
	{
		delay = 0;
	}
	
	if ((delay == 10000) & (startXsim))
	{
		gUdpAudioMsg.run[1] = 1;
		startXsim = FALSE;
	}
	else
	{
		delay++;
	}
	
	if ((delay == 2500)& (stopXsim))
	{
		gUdpAudioMsg.run[0] = 0;
		gUdpAudioMsg.run[1] = 0;
		gUdpAudioMsg.run[2] = 0;
		gUdpAudioMsg.run[3] = 0;
		stopXsim = FALSE;
	}
}

static void startIG()
{
	gUdpAudioMsg.run[0] = 1;	
	startXsim = TRUE;
	delay = 0;
}

static void stopIG()
{
	gUdpAudioMsg.run[0] = 2;
	gUdpAudioMsg.run[1] = 2;
	gUdpAudioMsg.run[2] = 1;
	gUdpAudioMsg.run[3] = 1;
	stopXsim = TRUE;
	delay = 0;

		
}

static void updateIOCalibration()
{
	switch (gHmi.locValue.selectedCalibrationValue)
	{
		case 0:  //Winch full Up is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iWinchFullUp;  	   //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iWinchCtrl;                   	   //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                         	   //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iWinchFullUp = gHmi.locStatus.realTimeIOValue;     	   //write the new value
			}
			break;
		case 1:  //Winch full Down is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iWinchFullDn;  	   //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iWinchCtrl;                   	   //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                         	   //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iWinchFullDn = gHmi.locStatus.realTimeIOValue;     	   //write the new value
			}
			break;
		case 2:  //Winch up collar position is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iWinchUpCollar;    //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iWinchCtrl;                   	   //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                         	   //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iWinchUpCollar = gHmi.locStatus.realTimeIOValue;       //write the new value
			}
			break;
		case 3:  //Winch down collar position is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iWinchDnCollar;    //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iWinchCtrl;                       //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                             //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iWinchDnCollar = gHmi.locStatus.realTimeIOValue;       //write the new value
			}
			break;
		case 4:  //Spooler deadband up is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iSpoolerUpDead;    //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iSpooler_UD_Crtl;                 //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                             //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iSpoolerUpDead = gHmi.locStatus.realTimeIOValue;       //write the new value
			}
			break;
		case 5:  //Spooler deadband down is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iSpoolerDnDead;    //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iSpooler_UD_Crtl;                 //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                             //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iSpoolerDnDead = gHmi.locStatus.realTimeIOValue;       //write the new value
			}
			break;
		case 6:  //Spooler left deadband is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iSpoolerLtDead;    //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iSpooler_LR_Ctrl;                 //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                             //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iSpoolerLtDead = gHmi.locStatus.realTimeIOValue;       //write the new value
			}
			break;
		case 7:  //Spooler right deadband is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iSpoolerRtDead;    //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iSpooler_LR_Ctrl;                 //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                             //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iSpoolerRtDead = gHmi.locStatus.realTimeIOValue;       //write the new value
			}
			break;
		case 8:  //Line Tension is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iLineTensionStop;  //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iLineTensionCtrl;                 //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                             //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iLineTensionStop = gHmi.locStatus.realTimeIOValue;     //write the new value
			}
			break;
		case 9:  //Line Tenison Max is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iLineTensionMax;   //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iLineTensionCtrl;                 //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                             //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iLineTensionMax = gHmi.locStatus.realTimeIOValue;      //write the new value
			}
			break;
		case 10:  //Caliper Brake is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iCaliperBrakeStop;  //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iCaliperBrakeCtrl;                 //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                              //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iCaliperBrakeStop = gHmi.locStatus.realTimeIOValue;     //write the new value
			}
			break;
		case 11:  //Caliper Brake Max is active on the IOS calibration page.
			gHmi.locValue.currentCalibrationValue = CalibrationCtrl.iCaliperBrakeMax;   //Current cal value is the value currently in memory
			gHmi.locStatus.realTimeIOValue = PanelIn.iCaliperBrakeCtrl;                 //The realtime vlaue is displayed based on the winch control position
			if(gHmi.key.mom.resetCalValue)                                              //If the set variable button is pressed, then write the new cal value
			{
				CalibrationCtrl.iCaliperBrakeMax = gHmi.locStatus.realTimeIOValue;      //write the new value
			}
			break;

		default:
			gHmi.locValue.currentCalibrationValue = 0;
			gHmi.locStatus.realTimeIOValue = 0;
	}
}

static void updateIOS()
{
	//Handle crash state

	if (IOScmdData.CurrentMode == 5)
		//if (IOSstatusData.test)
	{
		gHmi.key.radioBtnFreezeRun.status = 1;
	}
	
	if ((IOScmdData.CurrentMode == 5) & (loop > 0) & (gHmi.key.crashResetBtn))
	{
		IOSstatusData.CurrentMode = 3;
		loop --;
		
	}
	else
	{
		IOSstatusData.CurrentMode = gHmi.key.radioBtnFreezeRun.status;
		//IOSstatusData.crashactive = FALSE;
		gHmi.key.crashResetBtn = FALSE;
		loop = 100;
	}

	if (IOSstatusData.CurrentMode == 1 || setTODNight == 1) //freeze if the current mode is freeze or a video has been called
	{
		IOSstatusData.Freeze = 1;
	}
	else
	{
		IOSstatusData.Freeze = 0;
	}

	IOSstatusData.ICsetnum = IOScmdData.ICsetnum;
	IOSstatusData.ICscenarioIdx = IOScmdData.ICscenarioIdx;
	IOSstatusData.ICreset = IOScmdData.ICreset;



	// Power the correct trianing panel (43 or 46) based on button on IOS
	
	if (gHmi.key.radioBtnPanel0or1.status == 1)
	{
		IOSstatus43.bIOSenabled43 = 1;
		IOSstatus46.bIOSenabled46 = 0;
	}
	else if (gHmi.key.radioBtnPanel0or1.status == 2)
	{
		IOSstatus43.bIOSenabled43 = 0;
		IOSstatus46.bIOSenabled46 = 1;
	}
	
	
	// Handle IC sets and Malfunctions
	
	IOSstatusData.ICsetnum = gHmi.locValue.selectedInitialCondition;
	
	IOSstatusData.ICreset = gHmi.key.mom.ICreset;
	
	IOSstatusData.ICscenarioIdx = gHmi.locValue.selectedMalfunction;
	
	// Handle Lighting
	if (setTODNight == TRUE) //set environment to night if a video is called to play
	{
		IOSstatusData.CurrentTOD = 3;
	}
	else
	{
		IOSstatusData.CurrentTOD = gHmi.key.radioBtnEnvTOD.status;
	}
	
	//Load training videos on IG pc using VLC.
	iCurrentVideo = gHmi.key.tgl.selectedVideo1 * 1 
		+ gHmi.key.tgl.selectedVideo2 * 2
		+ gHmi.key.tgl.selectedVideo3 * 3
		+ gHmi.key.tgl.selectedVideo4 * 4
		+ gHmi.key.tgl.selectedVideo5 * 5
		+ gHmi.key.tgl.selectedVideo6 * 6
		+ gHmi.key.tgl.selectedVideo7 * 7
		+ gHmi.key.tgl.selectedVideo8 * 8
		+ gHmi.key.tgl.selectedVideo9 * 9
		+ gHmi.key.tgl.selectedVideo10 * 10
		+ gHmi.key.tgl.selectedVideo11 * 11
		+ gHmi.key.tgl.selectedVideo12 * 12;
	
	switch(iCurrentVideo)
	{
		case (1):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 One.avi");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (2):
			strcpy(gUdpAudioMsg.Args, "--width=850 --height=567 --video-x=450 --video-y=200 Two.VOB");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (3):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 Three.avi");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (4):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 --loop Four.m4v");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (5):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 Five.avi");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (6):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 --loop Six.avi");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (7):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 Seven.avi");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (8):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 Eight.avi");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (9):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 Nine.avi");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (10):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 Ten.avi");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (11):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 Eleven.avi");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		case (12):
			strcpy(gUdpAudioMsg.Args, "--width=800 --height=800 --video-x=420 --video-y=200 Twelve.avi");
			gUdpAudioMsg.run[4] = 4;
			setTODNight = TRUE;
			break;
		default:
			gUdpAudioMsg.run[4] = 2;
			setTODNight = FALSE;
	}
	

	
	
	// Start and Stop the IG from the IOS
	if (gHmi.key.mom.igStart)
	{
		startIG();
	}
	
	if (gHmi.key.mom.igStop)
	{
		stopIG();
	}
	
	// Restart and Shutdown the IG Computer from the IOS
	
	if (gHmi.key.mom.igReboot || gHmi.key.mom.igShutdown)
	{
		if(gHmi.key.mom.igShutdown)
		{
			gUdpAudioMsg.COMMAND = 1;
		}
		if (gHmi.key.mom.igReboot)
		{
			gUdpAudioMsg.COMMAND = 2;
		}
	}
	else
	{
		gUdpAudioMsg.COMMAND = 0;
	}
	

}


void _INIT IOSINIT(void)
{
	delay = 0;
	loop = 0;
	gHmi.key.crashResetBtn = FALSE;
	gUdpAudioMsg.run[0] = 0;
	gUdpAudioMsg.run[1] = 0;
	//WellDriving.current_state = 0;
	//gHmi.locValue.selectedInitialCondition = 0;
}

void _CYCLIC IOSCYCLIC(void)
{
	updateIOS();
	delaystart();
	updateIOCalibration();
 
}

void _EXIT IOSEXIT(void)
{
	/* TODO: Add code here */
}
