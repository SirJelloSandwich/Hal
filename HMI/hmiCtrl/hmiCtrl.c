/********************************************************************
 * COPYRIGHT -- PAI
 ********************************************************************
 * Program: hmiCtrl
 * File: hmiCtrl.c
 * Author: goran
 * Created: March 26, 2013
 ********************************************************************
 * Implementation of program hmiCtrl
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#include <../../Logical/bglib/bgsprintf.c>
#include <../../bglib/bgstring.c>
#include <../../bglib/bgBitUtilities.c>

#include "hmiDialog.h"


/*
###############################################################################
Includes, Consts, Typedefs
###############################################################################
*/
enum{
	scrInit = 0,
	scrMain,
	scrOEMSetup,
	scrTrend,
	scrNetworkSetup,
	scrLast,	
}HMI_SCREEN_INDEX;


#define DBG(format, args...)   ;


static const char* stateName(int state)
{
static const char* stateNames[] = {
"Init",
"Main",
"OEMConfig",
"Trend",
"NetworkSetup",
};

	if( state >= scrLast)
		return "unknow";
	
	return stateNames[state];
};



/*
###############################################################################
Utilities
###############################################################################
*/
/** 
 State control utility functions.
 */

/*
###############################################################################
Utility Functions
###############################################################################
*/

static void convertIPDigitsToText(udpIPAddress_typ* p)
{
	bgsprintf(p->text, "%d.%d.%d.%d", p->digit3, p->digit2, p->digit1, p->digit0);
}

static void convertIPTextToDigits(udpIPAddress_typ* p)
{
char txtbuffer[64];
bgStringList_typ list;

	strcpy(txtbuffer, p->text);
	list.src = txtbuffer;


//	DBG(lgr, "iptxt: %s", p->text);
//	return;
	bgStringSplitLine(&list, '.');
	if(list.itemCount != 4){
		p->digit3 = 0;
		p->digit2 = 0;
		p->digit1 = 0;
		p->digit0 = 0;
		return;
	}

	p->digit3 = (USINT)atoi(list.items[0]);
	p->digit2 = (USINT)atoi(list.items[1]);
	p->digit1 = (USINT)atoi(list.items[2]);
	p->digit0 = (USINT)atoi(list.items[3]);
	
}

/*
###############################################################################
HMI Alert Functions
###############################################################################
*/
static void alertShow(hmiAlertDialog_typ* p, char* alertText, UDINT alertDuration)
{
	//control visibility of the layer > make it visible
	bgBitSetUINT(&p->dialogStatus, 0, 0);
	bgStrCpyN(p->text, alertText, 240);
	bgTimerStart(&p->timer, alertDuration);		
}
static void alertInit(hmiAlertDialog_typ* p)
{
	//set status to invisible
	bgBitSetUINT(&p->dialogStatus, 0, 1);
}

static void alertCyclic(hmiAlertDialog_typ* p)
{
	bgTimerCyclic(&p->timer);

	if( !(p->dialogStatus&0x1) && bgTimerIsTimeOut(&p->timer) ){
		bgBitSetUINT(&p->dialogStatus, 0, 1);
	}
}

/*
###############################################################################
Recipe and config support cyclic logic. Monitors recipe and config changes 
(a change indication provided by recipe and config subsystem) 
and controls visibility of the global buttons that allows user to trigger recipe and config save actions.
Prompts the user to make a decession about saving changes using an HMI dialog. Based on the user input,
it than notifies the recipe and config subsystem to either accept or discard changes in active recipe or config.
NOTE: uses global variables
*/
static void recipeSaveCyclic()
{
	UINT ret;

	//control visibility: save cfg button
	bgBitSetUINT(&gHmi.locStatus.visibleSaveCfg, 0, (gConfig.prv.isChanged)?0:1);


	//NOTE: visibility of this button is controlled! This action is only possible when the button is visible!
	if(gHmi.key.btnSaveCfg){
		DBG(lgr, "save cfg request");
		gHmi.key.btnSaveCfg = 0;
		hmiDialogShow(&gHmi.dialogMsg, "Save Machine Config Changes?", 10000, 1, 2, 0);		
	}


	ret = hmiDialogCyclic(&gHmi.dialogMsg);	
	if(ret == 1){
		//save config accepted
		DBG(lgr, "save cfg accepted!");
		alertShow(&gHmi.locValue.alertDialog,"Machine Config changes saved.", 2000);		
		gConfig.prv.isChangeAccepted = 1;

	}
	if(ret == 2){
		//save config canceled
		DBG(lgr, "save cfg rejected!");
		alertShow(&gHmi.locValue.alertDialog,"Machine Config changes NOT saved.\nValues restored.", 3000);		
		gConfig.prv.isChangeRejected = 1;
	}
}


/*
###############################################################################
HMI Password dialog Functions
###############################################################################
*/
static void dialogPwdShow(hmiPwdDialog_typ* p)
{
	//control visibility of the layer > make it visible
	bgBitSetUINT(&p->dialogStatus, 0, 0);
}
static void dialogPwdInit(hmiPwdDialog_typ* p)
{
	//set status to invisible
	bgBitSetUINT(&p->dialogStatus, 0, 1);
}

static void dialogPwdCyclic(hmiPwdDialog_typ* p)
{
}
/*
###############################################################################
HMI Radio Button Functions
###############################################################################
*/
static void radioBtnInit(hmiButtonsRadio_typ* pRadioBtn, UDINT initValue)
{
	//set status to invisible
	pRadioBtn->status = initValue;
	pRadioBtn->statusOld = pRadioBtn->status;
}

static void radioBtnCyclic(hmiButtonsRadio_typ* pRadioBtn)
{
	//if the same radio button is pressed twice, it will be turned off > don't allow that to take place
	//i.e. one and only one radio button MUST be on at any time
	if(!pRadioBtn->status && pRadioBtn->statusOld){
		pRadioBtn->status = pRadioBtn->statusOld;
	}
	pRadioBtn->statusOld = pRadioBtn->status;
}
/*
###############################################################################
HMI Screen support specific functions
###############################################################################
*/
static void screenMain(hmi_typ* pHmi)
{

}


static void screenOEMSetup(hmi_typ* pHmi)
{


}

static void screenNetworkSetup(hmi_typ* pHmi)
{

	if(pHmi->locValue.remoteUDPHostIpAddrComplete){
		pHmi->locValue.remoteUDPHostIpAddrComplete = 0;	
		convertIPDigitsToText(&gUdpSystemCfg.udpRemoteHostIpAddress);
	}
	

	if(pHmi->key.mom.netCfgNew && (pHmi->locValue.pwdDialog.dialogStatus & 0x1)){
		pHmi->key.mom.netCfgNew = 0;
		
		//set network data button pressed > display password dialog
		//factoryCfgAction = 2;
		//control visibility: pwd dialog - show
		bgBitSetUINT(&pHmi->locValue.pwdDialog.dialogStatus, 0, 0);
		pHmi->locValue.pwdDialog.pwdLevel = 0;
	}


	if(pHmi->locValue.pwdDialog.btnOk && !(pHmi->locValue.pwdDialog.dialogStatus & 0x1)){
		//button OK pressed on a pwd dialog > check if password correct
		//DBG("pwd level=%d modify", p->pHmi->statusloc.pwdDialogLevel);
		pHmi->locValue.pwdDialog.btnOk = 0;
		if(pHmi->locValue.pwdDialog.pwdLevel >= 1){
			//pwd OK! > accept changes 
			alertShow(&pHmi->locValue.alertDialog, 
"System will reboot in 5 sec!\n\
PLEASE ADJUST NETWORK CLIENTS THAT ACCESS THIS STATION!\n\n\
SETTING STATION's <NodeNum>=0xEE AND REBOOTING\n\
WILL RESTORE NETWORKING SETTINGS TO FACTORY DEFAULTS!\n\
[IP=192.168.2.10]", 10000);	

			pHmi->key.mom.netCfgNewApply = 1;

		}else{
			alertShow(&pHmi->locValue.alertDialog, "New network configuration NOT set.\n\nEntered PASSWORD is not valid!", 3000);
		}


		//control visibility: pwd dialog - hide
		bgBitSetUINT(&pHmi->locValue.pwdDialog.dialogStatus, 0, 1);
	}


	if(pHmi->locValue.pwdDialog.btnCancel ){
		pHmi->locValue.pwdDialog.btnCancel = 0;
		alertShow(&pHmi->locValue.alertDialog, "New network configuration NOT set.\n\nRequest CANCELED.", 3000);
		//DBG("pwd cancel Modify()");
		//factoryCfgAction = 0;
		//control visibility: pwd dialog - hide
		bgBitSetUINT(&pHmi->locValue.pwdDialog.dialogStatus, 0, 1);
	}
}

/*
###############################################################################
CYCLIC Functions
###############################################################################
*/
static void visibilityControl(hmi_typ* pHmi)
{


}

static void globalHMILogic(hmi_typ* pHmi)
{
	alertCyclic(&pHmi->locValue.alertDialog);

	visibilityControl(pHmi);

	radioBtnCyclic(&pHmi->key.radioBtnFreezeRun);

	radioBtnCyclic(&pHmi->key.radioBtnPanel0or1);

	recipeSaveCyclic();
	
}

void screenLogic(hmi_typ* pHmi)
{
bgSSM_typ* pSm = &pHmi->prv.sm;

	//operate timer(s)
	bgTimerCyclic(&pSm->timer_x1ms[0]);
		
	globalHMILogic(pHmi);
	
	switch(pHmi->prv.pageCurrent){
	case scrInit:
		break;
	case scrMain:
		screenMain(pHmi);
		break;
	case scrOEMSetup:
		screenOEMSetup(pHmi);
		break;
	case scrNetworkSetup:
		screenNetworkSetup(pHmi);
		break;
	default:
		break;
	}
}

/*
###############################################################################
INIT Functions
###############################################################################
*/


static void screenLogicInit(hmi_typ* pHmi)
{	
	alertInit(&pHmi->locValue.alertDialog);

	dialogPwdInit(&pHmi->locValue.pwdDialog);

	radioBtnInit(&pHmi->key.radioBtnFreezeRun, 1);
	
	radioBtnInit(&pHmi->key.radioBtnEnvTOD, 1);

	radioBtnInit(&pHmi->key.radioBtnPanel0or1, 1);
	
}

/*
###############################################################################
Main entry points (global scope): init & cyclic

IMPORTANT: 
-note that all other (above defined) functions within this file are declared 
as "static" (have only local scope) 
###############################################################################
*/
void _INIT hmiCtrlINIT(void)
{
	
	int ii;
	//Must reference PV-s at least once within the source code!
	int dummy=0;
	dummy = strlen(gVersion_Project);
	dummy = strlen(gVersion_Date);
	dummy = strlen(gVersion_Automation_Studio);

	screenLogicInit(&gHmi);
}

void _CYCLIC hmiCtrlCYCLIC(void)
{
	/* TODO: Add code here */
	
	
	screenLogic(&gHmi);
}
//EOF
