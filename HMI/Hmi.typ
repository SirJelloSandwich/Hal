(********************************************************************
 * COPYRIGHT -- HP
 ********************************************************************
 * Package: HMI
 * File: Hmi.typ
 * Author: goran
 * Created: March 28, 2013
 ********************************************************************
 * Data types of package HMI
 ********************************************************************)

TYPE
	hmiNetworkParametar_typ : 	STRUCT 
		digit3 : USINT;
		digit2 : USINT;
		digit1 : USINT;
		digit0 : USINT;
		text : STRING[20];
	END_STRUCT;
	hmiAlertDialog_typ : 	STRUCT 
		dialogStatus : UINT;
		text : STRING[240];
		timer : TON;
	END_STRUCT;
	hmiPwdDialog_typ : 	STRUCT 
		dialogStatus : UINT;
		btnOk : BOOL;
		btnCancel : BOOL;
		pwdLevel : USINT;
	END_STRUCT;
	hmiButtonsMomentary_typ : 	STRUCT 
		netCfgNew : BOOL; (*Button to trigger password for saving of new networking settings*)
		netCfgNewApply : BOOL; (*Button to trigger saving of new networking settings*)
		ICreset : BOOL;
		netCfgRestoreFactory : BOOL; (*Button to trigger restore of networking settings to factory defaults*)
		igStart : BOOL;
		igStop : BOOL;
		igReboot : BOOL;
		igShutdown : BOOL;
		resetCalValue : BOOL;
	END_STRUCT;
	hmiButtonsRadio_typ : 	STRUCT 
		status : UDINT;
		statusOld : UDINT;
	END_STRUCT;
	hmiButtonsTogle_typ : 	STRUCT 
		selectedVideo1 : BOOL;
		selectedVideo2 : BOOL;
		selectedVideo3 : BOOL;
		selectedVideo4 : BOOL;
		selectedVideo5 : BOOL;
		selectedVideo6 : BOOL;
		selectedVideo7 : BOOL;
		selectedVideo8 : BOOL;
		selectedVideo9 : BOOL;
		selectedVideo10 : BOOL;
		selectedVideo11 : BOOL;
		selectedVideo12 : BOOL;
		training : USINT;
	END_STRUCT;
	hmiButtons_typ : 	STRUCT 
		radioBtnFreezeRun : hmiButtonsRadio_typ;
		radioBtnPanel0or1 : hmiButtonsRadio_typ;
		mom : hmiButtonsMomentary_typ;
		crashResetBtn : BOOL;
		tgl : hmiButtonsTogle_typ;
		lightsDrum : BOOL;
		radioBtnEnvTOD : hmiButtonsRadio_typ;
		btnSaveCfg : USINT;
		lightsRig : BOOL;
	END_STRUCT;
	hmiNavigationAndControl_typ : 	STRUCT 
		pageCurrent : USINT;
		pageChange : USINT;
		sm : bgSSM_typ;
	END_STRUCT;
	hmiLocalStatus_typ : 	STRUCT 
		realTimeIOValue : INT;
		visibleSaveCfg : USINT;
		New_Member : USINT;
		status1 : USINT;
	END_STRUCT;
	hmiLocalValue_typ : 	STRUCT 
		selectedMalfunction : UDINT;
		selectedInitialCondition : UDINT;
		pwdDialog : hmiPwdDialog_typ;
		alertDialog : hmiAlertDialog_typ;
		remoteUDPHostIpAddrComplete : BOOL;
		remoteUDPHostIPAddress : hmiNetworkParametar_typ;
		netIPAddress : hmiNetworkParametar_typ;
		netGateway : hmiNetworkParametar_typ;
		netSubnetMask : hmiNetworkParametar_typ;
		currentCalibrationValue : INT;
		selectedCalibrationValue : INT;
	END_STRUCT;
	hmi_typ : 	STRUCT  (*HMI interface data*)
		prv : hmiNavigationAndControl_typ;
		key : hmiButtons_typ;
		alertMsg : hmiAlert_typ;
		dialogMsg : hmiDialog_typ;
		locStatus : hmiLocalStatus_typ;
		locValue : hmiLocalValue_typ;
	END_STRUCT;
	hmi_Timer : 	STRUCT 
		output1 : BOOL;
		elapsed_time1 : TIME;
		elapsed_time2 : TIME;
		output2 : BOOL;
		output3 : BOOL;
		elaspsed_time3 : TIME;
	END_STRUCT;
END_TYPE
