/********************************************************************
 * COPYRIGHT -- piedmont
 ********************************************************************
 * Program: cfgFS
 * File: cfgFS.c
 * Author: admin
 * Created: Jan 20, 2014
 ********************************************************************
 * Implementation of program cfgFS
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
 #include <AsDefault.h>
#endif

/*
###############################################################################
Default machine configuration utility functions.
###############################################################################
*/
static TIME paiTimeSetDHMS(DINT day, DINT hour, DINT min, DINT sec)
{
	TIMEStructure prvTimeStruct;
	TIME timeValue;
	
	prvTimeStruct.day = day;
	prvTimeStruct.hour = hour;
	prvTimeStruct.minute = min;
	prvTimeStruct.second = sec;
	prvTimeStruct.millisec = 0;
	prvTimeStruct.microsec = 0;
	
	timeValue = TIMEStructure_TO_TIME((UDINT)&prvTimeStruct);
	return timeValue;
}
	
void cfgFSSetConfigDefaults(cfgMachineConfig_typ* p)
{
	int ii;
	
	//system
	strcpy(p->system.infoText, "File: ISM Winchmans Phase2");
	strcpy(p->system.author, "n.a.");
	strcpy(p->system.dateLastSaved, "n.a");	

	//diagnostics
	p->diag.diagMode = 0;
	p->diag.suppressAlarmIOModule = 0;
	
	
	//hmi
	p->hmi.splashScreenTimeout = 5.0;
	p->hmi.dialogBoxTimeout = 10.0;
	p->hmi.loginTimeout = 10 * 60.0;//10 minutes
	bgStrCpyN(p->hmi.passwordSupervisor, "1111", sizeof(p->hmi.passwordSupervisor));
	bgStrCpyN(p->hmi.passwordAdmin, "2222", sizeof(p->hmi.passwordAdmin));
	
	//trainer
//	p->trainer.lineDepth = 0.5;
//	p->trainer.casingBottom = 1000.25;
//	p->trainer.freewheelStart = 2400; //Hole depth to start freewheeling
	p->trainer.totalBottom = 3000;
	 
}

/*
###############################################################################
Default recipe utility functions.
###############################################################################
*/
void cfgFSRecipeFactory0(cfgMachineRecipe_typ* p)
{
	//Defaults
	strcpy(p->name, "Hello");
	strcpy(p->author, "n.d.");
	strcpy(p->dateLastSaved, "n.d.");
	
	p->settings.value1 = 1.0;
	p->settings.value2 = 2.0;
}
//EOF
