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

/**
 Simple logging
 Log goes to an array of PV strings > see _LOCAL STRING log[][] variable below
 Log output is visible via Automation Studio watch window
 Example of a log statement used within the code is: 
 DBG("This is log example. %d ", value1);
 To complety disable logging from this source file, change next line to: #if 0
*/
#if 1
#include <../../bglib/bgPVLogger.c>
#define LOG_ENTRY_COUNT 30	//max number of log entries
#define LOG_ENTRY_LENGTH  200//max length of individual log message. Longer msg are truncated
_LOCAL STRING log[LOG_ENTRY_COUNT][LOG_ENTRY_LENGTH]; //log output visible from Automation Studio!
static bgPVLogger_typ lgr;	//PV logger object
#else
#define DBG(format, args...) ;
#define DBGINIT()  ;
#endif

//Support for PV serialization/deserialization
#include <../../bglib/bgPVSerialize.c>

/*
###############################################################################
PV-s
###############################################################################
*/

_LOCAL char cfgFilename[64]; 	//name of the app's config file
_LOCAL char cfgFilenameFactory[64]; 	//name of the app's config file with factory defaults
_LOCAL char recFilename[64];	//name of app's recipe file
_LOCAL char recDirPath[64];	//recipe directory path
_LOCAL char textBuffer[512];	//text buffer

_LOCAL int readcounterok ;	
_LOCAL int readcountererr ;

_LOCAL int writecounterok ;
_LOCAL int writecountererr ;

_LOCAL bgSSM_typ bgssm; //file service state machine

#define APP_FILE_SIZE_MAX  16000 //max file size for any recipe or config file
static char fileRWBuffer[APP_FILE_SIZE_MAX]; //file io read/write buffer

_LOCAL cfgMachineConfig_typ tempCfg; //temporary machine config structure
_LOCAL cfgMachineRecipe_typ tempRec; //temporary recipe structure

/*
NOTE: Using C: disk is not the best solution considering that system files are there. 
However, using disk other then C: (i.e. D:, etc) for application stuff creates
issue with the ability to view/copy/access application files from windows (multiple partitions not supported, etc)

File structure used in this application is following:

Directory:
C://ABC
is used for application specific stuff. 
This directory holds following files:
machine configuration file - "appcfg.txt"
machine factory deafult configuration file - "appcfgfactory.txt");

a subdirectory (REC) of the above directory is used for storing recipes.
C://ABC/REC

*/

#define APP_DIR_NAME "Data" //root app directory C://Data
#define APP_REC_DIR_NAME "Rec" // recipes will be stgored at "C://Data/Rec"

#define APP_FILENAME_CFG "appcfg.txt"
#define APP_FILENAME_CFGDEFAULT "appcfgfactory.txt"
#define APP_FILENAME_CURRECIPE "apprec.txt"


/*component states*/
enum{
	cfgFsZero = 0,
	cfgFsInit,
	cfgFsDirCreate,
	cfgFsDirExistRec,
	cfgFsDirCreateRec,
	cfgFsCfgRead,
	cfgFsRecRead,
	cfgFsIdle,
	cfgFsReadCfgFactory,
	cfgFsBusyFileWrite,
	cfgFsBusyFileRead,
	cfgFsBusyRecipeList,
	cfgFsBusyRecipeList1,
	cfgFsBusyRecipeListNames,
	cfgFsBusyRecipeListNames1,
	cfgFsBusyRecipeDel,
	cfgFsBusyRecipeSaveAs,
	cfgFsBusyRecipeSetCurrent,
	cfgFsBusyRecipeSetCurrent1,
	cfgFsLAST
}LOCAL_STATEMACHINE;

const char* getCfgFsStateName(int state)
{
static const char* stateNames[] = {
"Zero",
"Init",
"DirCreate",
"DirRecCheck",
"DirRecCreate",
"RDCfg",
"RDRec",
"Idle",
"RDCfgFac",
"FileWriting",
"FileReading",
"RecList",
"RecList1",
"RecListNames",
"RecListNames1",
"RecDelete",
"RecSaveAs",
"SetCurrent",
"SetCurrent1",
};

	if( state >= cfgFsLAST)
		return "unknow";
	
	return stateNames[state];
};


/*
###############################################################################
Functions
###############################################################################
*/
void	cfgFSDefaultConfig()
{
	//Init both, active and default cfg to default value on a startup
	cfgFSSetConfigDefaults(&gConfig.active);
	cfgFSSetConfigDefaults(&gConfig.cfgDefault);
}


void cfgFSConfigInitDerivedData()
{
	//Triggered after power up, just before config data is marked valid (isValid = 1)
	//Function is used to initilize configuration dependant application settings


}

void	cfgFSDefaultRecipeList()
{
	char text[80];
	int ii;

	//init recipe list on a startup
	//-all recipes unused
	for(ii=0;ii<=APP_RECIPE_MAXID;ii++){
		bgsprintf(text, "%d", ii);
		strcpy(gRecipe.prv.listRecIds[ii], text);
		strcpy(gRecipe.prv.listRecNames[ii], "n.a.");
		gRecipe.prv.listRecUsed[ii] = 0;
	}

	//mark recipe 0 as used after power up
	//this is important only in case when starting for the first time on a brand new, empty system
	gRecipe.prv.listRecUsed[0] = 1;
	strcpy(gRecipe.prv.listRecNames[0], gRecipe.recDefault.name);
	
}

void	cfgFSDefaultRecipe()
{

	cfgFSRecipeFactory0(&gRecipe.recDefault);
	cfgFSRecipeFactory0(&gRecipe.active);
	
	gRecipe.prv.currentRecId= 0;

	cfgFSDefaultRecipeList();
}

void cfgFSRecipeInitDerivedData()
{
//Triggered after power up, just before recipe data is marked valid (isValid = 1)
//Function is used to initilize recipe dependant application settings


}

/*
###############################################################################
Utility function to set directory/file names used in this application.
###############################################################################
*/
static void dirCreateRequest(paiFileService_typ* p, UDINT adrPath)
{
	p->i.request = PAI_FS_CRDIR;
	p->i.adrName = (UDINT)adrPath;
}


static void dirInfoRequest(paiFileService_typ* p, UDINT adrPath)
{
	p->i.request = PAI_FS_INFODIR;
	p->i.adrName = (UDINT)adrPath;
}

static void fileDelRequest(paiFileService_typ* p, UDINT adrName)
{
	p->i.request = PAI_FS_DELFILE;
	p->i.adrName = (UDINT)adrName;
}

static void fileRdRequest(paiFileService_typ* p, UDINT adrName, UDINT adrDst, UDINT sizeDst)
{
	p->i.request = PAI_FS_RDFILE;
	p->i.adrName = (UDINT)adrName;
	p->i.adrReadDst = (UDINT)adrDst;
	p->i.readDstLen = sizeDst;
}


static void fileWrRequest(paiFileService_typ* p, UDINT adrName, UDINT adrSrc, UDINT sizeSrc)
{
	p->i.request= PAI_FS_WRFILE;
	p->i.adrName = (UDINT)adrName;
	p->i.adrWriteSrc = (UDINT)adrSrc;
	p->i.writeSrcLen= sizeSrc;
}


static BOOL fileRequestIsBusy(paiFileService_typ* p)
{
	//wait until request completes
	if(p->o.busy)
		return 1;

	//reset request! IMPORTANT!
	p->i.request = PAI_FS_IDLE;
	return 0;
}

static void fileSetCfgFilePath(char* pCfgFilePath)
{
//Example:
//Output: pRecFilePath = "ABC/appcfg.txt"
	bgsprintf(pCfgFilePath, "%s/%s", APP_DIR_NAME, APP_FILENAME_CFG);
}

static void fileSetDefaultCfgFilePath(char* pCfgFilePath)
{
//Example:
//Output: pRecFilePath = "Data/appcfgfactory.txt"
	bgsprintf(pCfgFilePath, "%s/%s", APP_DIR_NAME, APP_FILENAME_CFGDEFAULT);
}


static void fileSetRecDirPath(char* pRecDirPath)
{
//Example:
//Output: pRecDirPath = "Data/REC"
	bgsprintf(pRecDirPath, "%s/%s", APP_DIR_NAME, APP_REC_DIR_NAME);
}

static void fileSetRecipeFilePath(char* pRecFilePath, char* pRecFileName)
{
//Example:
//Input: pRecFileName = "rec1"
//Output: pRecFilePath = "Data/Rec/rec1.txt"
	bgsprintf(pRecFilePath, "%s/%s/%s.txt", APP_DIR_NAME, APP_REC_DIR_NAME, pRecFileName);
}


static void fileSetCurrentRecipeFilePath(char* pRecFilePath)
{
//Example:
//Output: pRecFilePath = "ABC/REC/apprec.txt"
	bgsprintf(pRecFilePath, "%s/%s/%s", APP_DIR_NAME, APP_REC_DIR_NAME, APP_FILENAME_CURRECIPE);
}


static void fileSetRecipeFilePathFromId(char* pRecFilePath, UINT recipeId)
{
//Example:
//Input: recipeId = "13"
//Output: pRecFilePath = "ABC/REC/rec13.txt"
char temp[32];
	bgsprintf(temp, "rec%d", recipeId);
	fileSetRecipeFilePath(pRecFilePath, temp);
}


static BOOL PVtoTextFileWriteRequest(char* pvName, char* fileName)
{
	DINT charsWritten = bgPVStructSerialize(pvName, (UDINT)&fileRWBuffer[0], APP_FILE_SIZE_MAX);
	if(  charsWritten > 0){
		DBG(lgr,"WRrec %s size=%d", fileName, charsWritten);
		fileWrRequest(&paiFS, (UDINT)fileName, (UDINT)fileRWBuffer, charsWritten);
		//bgSSMStateNext(&bgssm, cfgFsBusyFileWrite);
		return 1;
	}else{
		DBG(lgr,"Can't serialize pv=%s", pvName);
		return 0;
	}
}



/*
###############################################################################
Remanent Memory Handling
- remanent (REM) memory is battery backed SRAM
- unlike files, it has a fast, pointer based access
- in this application, it is used as a first level of storage, as follows

  working memory (DRAM) data   
- valid only while CPU powered
- working memory holds all PV-s and application strucutres

Remanent memory (SRAM battery backed) 
- power fail safe (only WARM restart! COLD restart deletes remanent memory)
- selected structures (eg. machine confiuration, current recipe) are copied into this area as a first level backup
- fast backup via simple memcpy

Files (CF disk)
- power fail safe and permanent
- backup requires use of FileIO access (slower, more complex)

This application combines use of the above mentioned memories.

IMPORTANT:
The REMMEM support was added before realizing that most CPU-s support only tiny (eg. 32 bytes) REM Memory areas.
As a result, the REM memory support is not really working as intended. The intended REMMEM PV-s are stored as
regular PV-s which means that these values are not preserved during power cycle.
###############################################################################
*/

static void cfgRemanentDataSetValid()
{
//writting magic number into battery backed SRAM marks SRAM as valid
//if the battery fails, the magic number will be altered. As a result, the magic number is checked on each power up
//to detect how to proceede after power up (i.e. if the remamenet memory content shall be used or not)
	gRemanentData.magicPrefix = MAGIC_NUMBER;
	gRemanentData.magicPosfix = MAGIC_NUMBER;

	//this REMMEM area (small) is still supported and is power cycle safe!
	//gRemanentDataSmall.magicPrefix = MAGIC_NUMBER;	
}

//check if REM mem is valid
static BOOL cfgRemanentDataIsValid()
{
	if(	(gRemanentData.magicPrefix != MAGIC_NUMBER) ||
		(gRemanentData.magicPosfix != MAGIC_NUMBER)
	){
		return 0;
	}

	return 1;
}

//routines to copy data from/to REM mem
static void cfgRemanentCopyCFGFromRemToMem()
{
	memcpy(&gConfig.active, &gRemanentData.activeCfg, sizeof(cfgMachineConfig_typ));	
}

static void cfgRemanentCopyCFGFromMemToRem()
{
	memcpy(&gRemanentData.activeCfg, &gConfig.active, sizeof(cfgMachineConfig_typ));	
}

static void cfgRemanentCopyRECFromRemToMem()
{
	memcpy(&gRecipe.active, &gRemanentData.activeRec, sizeof(cfgMachineRecipe_typ));
}

static void cfgRemanentCopyRECFromMemToRem()
{
	memcpy(&gRemanentData.activeRec, &gRecipe.active, sizeof(cfgMachineRecipe_typ));
}

static void cfgRemanentDataLogicInit()
{

	if( !cfgRemanentDataIsValid() ){
		//remanent memory does not conatin valid data
		//NOTE: since the change and considering that gRemanentData PV is actually not in REMMEM
		//we'll always use this path
		DBG(lgr,"REMMEM invalid!");
		gRemanentData.dataIsValid = 0;

		//copy valid data into REM memory
		cfgRemanentCopyCFGFromMemToRem();
		cfgRemanentCopyRECFromMemToRem();
		
		//the small REMMEM area is still working
		//if(gRemanentDataSmall.magicPrefix == MAGIC_NUMBER){
		if(gRemanentData.magicPrefix == MAGIC_NUMBER){
			DBG(lgr,"SMALL REMMEM valid!");
			//restore current recipe ID
			//if((gRemanentDataSmall.currentRecId >= 0)  && (gRemanentDataSmall.currentRecId < RECIPE_MAXID) ){		
			if((gRemanentData.currentRecId >= 0)  && (gRemanentData.currentRecId <= APP_RECIPE_MAXID) ){		
				gRecipe.prv.currentRecId = gRemanentData.currentRecId;
			}else{
				gRecipe.prv.currentRecId = 0;
			}	
	
		}else{
			//don't know what curren recipe id is > init to 0
			gRecipe.prv.currentRecId = 0;
			
			//set flag to try recovering recipe ID from the available application logic and files stored on a CF
			gRecipe.prv.requestRestoreRecId = 1;
		}


		//mark REM data as valid
		cfgRemanentDataSetValid();
		
		return;
	}

	//REM mem is valid > just restore machine config and last active (current) recipe and the machine is ready to go!
	DBG(lgr,"REMMEM OK. Restoring data...");
	gRemanentData.dataIsValid = 1;
	
	//restore data from REM memory into main memory structures

	//restore currentRecId from REM  memory into the recipe structure
	//if((gRemanentDataSmall.currentRecId >= 0)  && (gRemanentDataSmall.currentRecId < RECIPE_MAXID) ){		
	if((gRemanentData.currentRecId >= 0)  && (gRemanentData.currentRecId <= APP_RECIPE_MAXID) ){		
		DBG(lgr,"Active recId=%d restored.", gRemanentData.currentRecId);
		gRecipe.prv.currentRecId = gRemanentData.currentRecId;
	}else{
		DBG(lgr,"recId=0 not restored");
		gRecipe.prv.currentRecId = 0;
	}	

	cfgRemanentCopyCFGFromRemToMem();
	cfgRemanentCopyRECFromRemToMem();
	
}

static void cfgRemanentDataLogicCyclic()
{
/*
This routine continounsly compares content of the data structures stored in REM mem vs. coresponding data
structures in the main memory. As a result of the comparison, an xxx.IsChanged flag is set to 0 or 1 (xxx is a certain
memory structure of interest).
The xxx.IsChanged flag is then used by the rest of the application logic to handle requiired support for preserving data 
structures.

Once the xxx.IsChanged flag is set to 1, the application uses xxx.IsChangeAccepted and xxx.IsChangeRejected
to notify this routine how to handle the changes.

*/



	/*keep updating remanent memory location for current recipe id NOTE: rest of the code does not use REM based PV-s!*/
	gRemanentData.currentRecId = gRecipe.prv.currentRecId;


	//keep checking if configuration changes
	if( memcmp(&gRemanentData.activeCfg, &gConfig.active, sizeof(cfgMachineConfig_typ))){
		gConfig.prv.isChanged = 1;
		gConfig.prv.hmiBtnSaveCfgVisible = 0;//btn visible - YES
	}else{
		gConfig.prv.isChanged = 0;
		gConfig.prv.hmiBtnSaveCfgVisible = 1;//btn visible - NO
	}
	
	
	//keep checking if recipe changes
	if( memcmp(&gRemanentData.activeRec, &gRecipe.active, sizeof(cfgMachineRecipe_typ))){
		gRecipe.prv.isChanged = 1;
		gRecipe.prv.hmiBtnSaveRecVisible = 0;//btn visible - YES
	}else{
		gRecipe.prv.isChanged = 0;
		gRecipe.prv.hmiBtnSaveRecVisible = 1;//btn visible - NO
	}


	//keep checking if user accepts configuration changes	
	if(gConfig.prv.isChangeAccepted){
		DBG(lgr,"CFG change accepted");
		gConfig.prv.isChangeAccepted = 0;
		
		//store changes into REM mem
		cfgRemanentCopyCFGFromMemToRem();
		
		//TRIGGER timer that will eventually cause a file update (timer is used to control the max frequency of file updates, in this case set to 2 sec)
		bgTimerStart(&bgssm.timer_x1ms[1], 1000 * 2);
		bgssm.event[1]=1;
		DBG(lgr,"Start CFG update timer(2 sec)");				
	}


	//keep checking if user accepted recipe changes	
	if(gRecipe.prv.isChangeAccepted){
		DBG(lgr,"REC change accepted");
		
		gRecipe.prv.isChangeAccepted = 0;

		//store changes into REM mem
		cfgRemanentCopyRECFromMemToRem();
		
		//TRIGGER timer that will eventually cause a file update (timer is used to control the max frequency of file updates, in this case set to 3 sec)
		bgTimerStart(&bgssm.timer_x1ms[0], 1000 * 3);
		bgssm.event[0]=1;
		DBG(lgr,"Start REC update timer(3 sec)");		
	}


	//keep checking if user rejects configuration changes	
	if(gConfig.prv.isChangeRejected){
		DBG(lgr,"CFG change rejected");
		gConfig.prv.isChangeRejected = 0;
		//restore REM mem to main mem
		cfgRemanentCopyCFGFromRemToMem();
	}


	//keep checking if user rejects recipe changes	
	if(gRecipe.prv.isChangeRejected){
		DBG(lgr,"REC change rejected");
		gRecipe.prv.isChangeRejected = 0;
		//restore REM mem to main mem
		cfgRemanentCopyRECFromRemToMem();
	}

	
}

/*
###############################################################################
Utility function to handle state machine states. Names of the function are given as:
<stateMachineStaete>State
Example:
cfgFsCfgReadState() is used during processing of the cfgFsCfgRead SM state
###############################################################################
*/
static void cfgFsClearResponseFlags()
{
	gConfig.prv.reqError = 0;
	gConfig.prv.reqOk = 0;		
}
static void cfgFsSetResponseOk()
{
	gConfig.prv.reqError = 0;
	gConfig.prv.reqOk = 1;		
}
static void cfgFsSetResponseErr()
{
	gConfig.prv.reqError = 1;
	gConfig.prv.reqOk = 0;		
}

/*
Read machine config file. Executed upon power cycle.
Needs to be robust and handle cases when directory/files don't exist or fileIO errors are reported.
*/
static void cfgFsCfgReadState()
{
DINT errCount;
bgSSM_typ *pssm = &bgssm;

	if( bgSSMIsStateInit(pssm) ){				
		//isue a request to read config file
		//if read is succesful, content of the file will be copied to fileRWBuffer
		fileSetCfgFilePath(cfgFilename);

		//check if cfg file has been restored from remanent (battery backed) memory
		if(gRemanentData.dataIsValid && !gConfig.prv.isPowerupDone){
			DBG(lgr,"cfg from REMMEM; no file RD.");
			bgSSMStateNext(pssm, cfgFsRecRead);
			return;
		}
		
		//file that holds machine configuration	
		//Example: cfgFilename = "ABC/appcfg.txt"
		
		fileRdRequest(&paiFS, (UDINT)cfgFilename, (UDINT)&fileRWBuffer[0], APP_FILE_SIZE_MAX);
		return;
	}

	//wait until request completes
	if( fileRequestIsBusy(&paiFS) )
		return;
				
	
	// TODO: next is to prepopulate tempCfg in case a new value, that is still not saved in the text file, is defined in the project since the last save of the file
	// This case is typical for a new program version (i.e. added new recipe settings) when starting for the first time and using "old" (previosly saved) text file
	// This shall be improved and maybe handled differntly in order to consider other issues (program upgrades, backward compability, etc)
	memcpy(&tempCfg, &gConfig.active, sizeof(cfgMachineConfig_typ));
	DBG(lgr,"cfg set to default");


	//read error?
	if(paiFS.o.status){
		//read error for a config file > file does not exist > use defaults!
		readcountererr++;
		DBG(lgr,"cfg RD error=%d", paiFS.o.status);
		DBG(lgr,"Using cfg defaults");
		// TODO: nofify machine operator
		//bgSSMStateNext(pssm, cfgFsRecRead);
		//return;	
		cfgFsSetResponseErr();	
		goto exit_cfgFsCfgReadState;
	}
	
	DBG(lgr,"cfg fileRD OK");
	readcounterok++;

	//specify a local (this task) variable for reading the cfg text file content (deserilaziation)
	errCount = bgPVStructDeserialize("cfgFS:tempCfg", (UDINT)fileRWBuffer, paiFS.prv.file.readex.bytesread);			
	if( errCount){
		DBG(lgr,"cfg deserialz err(%d); struct change?", errCount);
	}

	// TODO: change the processing logic for deserialization in order to handle:
	// partial setup,
	// version changes,
	// upgrades, etc
	//For now:
	//return value for the above is number of errors > see function
	//as a result, and only if no errors, accept the deserialized values and use them here

	//restore values from the text file
	memcpy(&gConfig.active, &tempCfg, sizeof(cfgMachineConfig_typ));
	//store same values into the REM memory
	cfgRemanentCopyCFGFromMemToRem();
	
	DBG(lgr,"cfg deserialz OK, accepting values");
	cfgFsSetResponseOk();	

//	bgSSMStateNext(pssm, cfgFsRecRead);
exit_cfgFsCfgReadState:
	bgSSMStateNext(pssm, cfgFsIdle);
}


static void cfgFsReadCfgFactoryState()
{
bgSSM_typ *pssm = &bgssm;

	if( bgSSMIsStateInit(pssm) ){
		//isue a request to read config file
		//if read is succesful, content of the file will be copied to fileRWBuffer				
		fileRdRequest(&paiFS, (UDINT)cfgFilenameFactory, (UDINT)&fileRWBuffer[0], APP_FILE_SIZE_MAX);
		return;
	}


	//wait until request completes
	if( fileRequestIsBusy(&paiFS) )
		return;

	//init config file to default, hardcoded values
	//NOTE: if reading factory default config fails, the restored factory config comes from this call. These are hardcoded value.
	//If reading factory default config file succeds, restored factory value would come from that file
	cfgFSDefaultConfig();
	
	//store current config into REM memory
	cfgRemanentCopyCFGFromMemToRem();

	//read error?
	if(paiFS.o.status){
		//read error for a config file > file does not exist > use defaults!
		readcountererr++;
		cfgFsSetResponseErr();	
		DBG(lgr,"cfg RD error=%d", paiFS.o.status);
		DBG(lgr,"Using cfg defaults!");
		// TODO: nofify machine operator
		bgSSMStateNext(pssm, cfgFsIdle);
		return;
	}
	
	DBG(lgr,"cfgFac fileRD OK");
	readcounterok++;
	
	//specify a local (this task) variable for reading the cfg text file content (deserilaziation)
	if( bgPVStructDeserialize("cfgFS:tempCfg", (UDINT)fileRWBuffer, paiFS.prv.file.readex.bytesread) ){
		DBG(lgr,"cfgFac deserialz error; using defaults!");
		// TODO: nofify machine operator
		bgSSMStateNext(pssm, cfgFsIdle);
		cfgFsSetResponseErr();	
		return;
	}

	cfgFsSetResponseOk();	
	
	//restore values from the text file
	memcpy(&gConfig.active, &tempCfg, sizeof(cfgMachineConfig_typ));
	DBG(lgr,"cfgFactory deserialz OK, accepting values");
	
	//store current config into REM memory
	cfgRemanentCopyCFGFromMemToRem();

	bgSSMStateNext(pssm, cfgFsIdle);
}


static void cfgFsRecReadState()
{
DINT errCount;
bgSSM_typ *pssm = &bgssm;

	if( bgSSMIsStateInit(pssm) ){

		//check if active recipe content has been restored from remanent (battery backed) memory
		if(gRemanentData.dataIsValid && !gConfig.prv.isPowerupDone){
			DBG(lgr,"CURREC from REMMEM; no file RD.");
			//on startup > request recipe list read	
			gRecipe.prv.reqList = 1;
			
			bgSSMStateNext(pssm, cfgFsIdle);			
			return;
		}
		
		//isue a request to read recipe  file
		//if read is succesful, content of the file will be copied to fileRWBuffer				
		fileRdRequest(&paiFS, (UDINT)recFilename, (UDINT)&fileRWBuffer[0], APP_FILE_SIZE_MAX);
		return;
	}


	//wait until request completes
	if( fileRequestIsBusy(&paiFS) )
		return;

	//init active rec to default values
	memcpy(&gRecipe.active, &gRecipe.recDefault, sizeof(cfgMachineRecipe_typ));
	

	// TODO: next is to prepopulate tempRec in case a new value, that is still not saved in the text file, is defined in the project since the last save of the file
	// This case is typical for a new program version (i.e. added new recipe settings) when starting for the first time and using "old" (previosly saved) text file
	// This shall be improved and handled in a more elegant fashion
	memcpy(&tempRec, &gRecipe.active, sizeof(cfgMachineRecipe_typ));
	DBG(lgr,"rec set to default");
	
	//read error?
	if(paiFS.o.status){
		//read error for a config file > file does not exist > use defaults!
		readcountererr++;
		DBG(lgr,"rec RD error=%d", paiFS.o.status);
		DBG(lgr,"Using rec defaults!");
		// TODO: nofify machine operator
		//bgSSMStateNext(pssm, cfgFsIdle);
		cfgFsSetResponseErr();	
		goto exit_cfgFsRecReadState;
		//return;
	}


	DBG(lgr,"rec fileRD OK");
	readcounterok++;
	errCount = bgPVStructDeserialize("cfgFS:tempRec", (UDINT)fileRWBuffer, paiFS.prv.file.readex.bytesread);
	//specify a local (this task) variable for reading the cfg text file content (deserilaziation)
	if(errCount >= 5){
		DBG(lgr,"rec deserialz err(%d); using defaults!", errCount);
		// TODO: nofify machine operator
		//bgSSMStateNext(pssm, cfgFsIdle);
		//return;
		cfgFsSetResponseErr();	
		goto exit_cfgFsRecReadState;
	}

	if( errCount){
		DBG(lgr,"rec deserialz err(%d); struct change?", errCount);
	}
						
	// TODO: change the processing logic for deser to handle:
	// partial setup,
	// version changes,
	// upgrades, etc
	//For now:
	//return value for the above is number of errors > see function
	//as a result, and only if no errors, accept the deser values and use them here

	//restore values saved in the text file
	memcpy(&gRecipe.active, &tempRec, sizeof(cfgMachineRecipe_typ));
	//store same values into the REM memory
	cfgRemanentCopyRECFromMemToRem();
	
	DBG(lgr,"rec deserialz OK, accepting values");
	cfgFsSetResponseOk();	

	//on startup > request recipe list read	
	//gRecipe.prv.reqList = 1;
exit_cfgFsRecReadState:			
	bgSSMStateNext(pssm, cfgFsIdle);

}

static void cfgFsBusyRecipeListState()
{
bgSSM_typ *pssm = &bgssm;

	if( bgSSMIsStateInit(pssm) ){
		//isue a request for file info
		fileSetRecipeFilePathFromId(recFilename, gRecipe.prv.counter1);
		paiFS.i.request= PAI_FS_INFOFILE;
		paiFS.i.adrName = (UDINT)recFilename;

		if(!gRecipe.prv.counter1){


		}
		
		return;
	}
			

	if( fileRequestIsBusy(&paiFS) )
		return;
	
	if(paiFS.o.status){
	//error: mark recipe entry as not used/doesn't exist 
	//TODO: this may require a better error handling!?
	//for example, a correct way would be to just use fileNotFount status and then all other error handle in a different fashion 
	//however, this may not be needed!?
		gRecipe.prv.listRecUsed[gRecipe.prv.counter1] = 0;

		//update recipe list entries
		strcpy(gRecipe.prv.listRecNames[gRecipe.prv.counter1], "n.a.");

		//in case of a current currentRecId, active recipe is sufficent as we may not still have recxx.txt file for it
		if(gRecipe.prv.counter1 == gRecipe.prv.currentRecId){
			gRecipe.prv.listRecUsed[gRecipe.prv.counter1] = 1;

			//update recipe list entries
			strcpy(gRecipe.prv.listRecNames[gRecipe.prv.counter1], gRecipe.active.name);
			
			DBG(lgr,"CUR rec RD error=%d", paiFS.o.status);

			//however, in this case trigger a SetCurrent request to acompish two objectives:
			// 1) to save active recipe to an apprec.txt file (file for an active rec)
			// 2) to save active recipe to a recXX.txt (where XX is the current index of the recipe) 
			// NOTE: SetCurrent call is called with the reqIdx that is the same as the current index! Trick!
			gRecipe.prv.reqSetCurrent = 1;
			gRecipe.prv.reqIdx = gRecipe.prv.currentRecId;			
		}
		
	}
	else{
	//ok: mark recipe entry as used/existing
		gRecipe.prv.listRecUsed[gRecipe.prv.counter1] = 1;
		gRecipe.prv.counter2++;
	}
	

	//update recipe list entries
	bgsprintf(gRecipe.prv.listRecIds[gRecipe.prv.counter1], "%d", gRecipe.prv.counter1);
	
	bgSSMStateNext(pssm, cfgFsBusyRecipeList1);

}

static void cfgFsBusyRecipeListNamesState()
{
bgSSM_typ *pssm = &bgssm;

	if( bgSSMIsStateInit(pssm) ){
		//isue a request for recipe file read
		fileSetRecipeFilePathFromId(recFilename, gRecipe.prv.counter1);
		fileRdRequest(&paiFS, (UDINT)recFilename, (UDINT)&fileRWBuffer[0], APP_FILE_SIZE_MAX);
		return;
	}
			

	if( fileRequestIsBusy(&paiFS) )
		return;
	
	//read error?
	if(paiFS.o.status){
		//read error for a config file > file does not exist > use defaults!
		readcountererr++;
		DBG(lgr,"rec RD error=%d", paiFS.o.status);


		//in case of a current currentRecId, active recipe has the info and we may still not have recxx.txt file for it
		if(gRecipe.prv.counter1 == gRecipe.prv.currentRecId){

		}else{
			strcpy(gRecipe.prv.listRecNames[gRecipe.prv.counter1], "rd error!");
		}
		
		bgSSMStateNext(pssm, cfgFsBusyRecipeListNames1);
		return;
	}

	readcounterok++;


	//extract value of the variable "name" from the file
	{
		int offsetEntryStart;
		int offsetEntryEnd;
		int lengthKey = strlen("name=");
		int lengthVal = 0;
				
		offsetEntryStart = bgStrFind(fileRWBuffer,"name=",-1,-1);
		if(offsetEntryStart >= 0){
			offsetEntryEnd = bgStrFindEOL(fileRWBuffer, offsetEntryStart);
			if(offsetEntryEnd > offsetEntryStart){
				lengthVal = offsetEntryEnd - (offsetEntryStart + lengthKey);
				memcpy(textBuffer, fileRWBuffer + offsetEntryStart + lengthKey,  lengthVal);
				textBuffer[lengthVal]=0;
				
				//update recipe list entries
				strcpy(gRecipe.prv.listRecNames[gRecipe.prv.counter1], textBuffer);						
			}
		}
	}
	
	bgSSMStateNext(pssm, cfgFsBusyRecipeListNames1);
}


static void restoreRecIdFromRecNames()
{
//handle a very special case when REM memory was corrupt (eg. bat loss) and try to restore the most recent recId based on the following logic:
//active recipe is restored from the apprec.txt
//at the same time, same recipe exists as a file recXX.txt where XX is the recId
//knowing the above, and once the active rec is restored, just search the list of available recipes for the matching name 
int ii;

	gRecipe.prv.currentRecId = 0;
	for(ii=0; ii<=APP_RECIPE_MAXID; ii++){
		if( !gRecipe.prv.listRecUsed[ii] )
			continue;

		if( !strcmp(gRecipe.prv.listRecNames[ii], gRecipe.active.name) )	{
			gRecipe.prv.currentRecId = ii;
			DBG(lgr,"recId=%d restored from file", gRecipe.prv.currentRecId);
			return;
		}
	}
	
	DBG(lgr,"unable to restore recID from file");
}


static void cfgFsBusyRecipeSetCurrentState()
{
DINT errCount;
bgSSM_typ *pssm = &bgssm;

	if( bgSSMIsStateInit(pssm) ){
		//first, make sure to store the current recipe in a file
		fileSetRecipeFilePathFromId(recFilename, gRecipe.prv.currentRecId);
		PVtoTextFileWriteRequest("gRecipe.active", recFilename);
		return;
	}


	//wait until request completes
	if( fileRequestIsBusy(&paiFS) )
		return;


	//error?
	if(paiFS.o.status){
		//read error for a config file > file does not exist > use defaults!
		readcountererr++;
		DBG(lgr,"rec WR error=%d", paiFS.o.status);
		// TODO: nofify machine operator, log this event
	}


	bgSSMStateNext(&bgssm, cfgFsBusyRecipeSetCurrent1);
}

static void cfgFsBusyRecipeSetCurrent1State()
{
DINT errCount;
bgSSM_typ *pssm = &bgssm;


	if( bgSSMIsStateInit(pssm) ){
		//isue a request for recipe file read
		fileSetRecipeFilePathFromId(recFilename, gRecipe.prv.reqIdx);
		//if read is succesful, content of the file will be copied to fileRWBuffer				
		fileRdRequest(&paiFS, (UDINT)recFilename, (UDINT)&fileRWBuffer[0], APP_FILE_SIZE_MAX);
		return;
	}

	//wait until request completes
	if( fileRequestIsBusy(&paiFS) )
		return;


	// TODO: next is to prepopulate tempRec in case a new value, that is still not saved in the text file, is defined in the project since the last save of the file
	// This case is typical for a new program version (i.e. added new recipe settings) when starting for the first time and using "old" (previosly saved) text file
	// This shall be improved and handled in a more elegant fashion
	memcpy(&tempRec, &gRecipe.recDefault, sizeof(cfgMachineRecipe_typ));
	DBG(lgr,"rec set to default");
	
	//read error?
	if(paiFS.o.status){
		//read error > file does not exist > use defaults!
		readcountererr++;
		DBG(lgr,"rec RD error=%d", paiFS.o.status);
		DBG(lgr,"Current rec not changed!");
		// TODO: nofify machine operator, log the event
		bgSSMStateNext(pssm, cfgFsIdle);
		return;
	}


	DBG(lgr,"rec fileRD OK");
	readcounterok++;
	errCount = bgPVStructDeserialize("cfgFS:tempRec", (UDINT)fileRWBuffer, paiFS.prv.file.readex.bytesread);
	//specify a local (this task) variable for reading the cfg text file content (deserilaziation)
	if(errCount >= 5){
		DBG(lgr,"rec deserialz err(%d); current rec not changed!", errCount);
		// TODO: nofify machine operator
		bgSSMStateNext(pssm, cfgFsIdle);
		return;
	}

	if( errCount){
		DBG(lgr,"rec deserialz err(%d); struct change?", errCount);
	}

	//activate previosly loaded tempRec file as new active/current recipe
	memcpy(&gRecipe.active, &tempRec, sizeof(cfgMachineRecipe_typ));
	//update REMMEM 
	cfgRemanentCopyRECFromMemToRem();

	//set new current recId	
	gRecipe.prv.currentRecId = gRecipe.prv.reqIdx;
	DBG(lgr,"new currec ID#=%d", gRecipe.prv.currentRecId);

	//trigger request for writing an active recipe into the file
	gRecipe.prv.reqWr = 1;
	bgSSMStateNext(pssm, cfgFsIdle);
}

static void cfgFsIdleState()
{
bgSSM_typ *pssm = &bgssm;

	if(!gConfig.prv.isValid){
		cfgFSConfigInitDerivedData();
		gConfig.prv.isValid = 1;
	}
	
	if(!gRecipe.prv.isValid){
		cfgFSRecipeInitDerivedData();
		gRecipe.prv.isValid = 1;
	}

	if(!gConfig.prv.isPowerupDone){
		gConfig.prv.isPowerupDone = 1;//power up cycle is compleet when in this state
	}	

	//Timer driven current recipe write request
	if( bgTimerIsTimeOut(&pssm->timer_x1ms[0])  && (pssm->event[0]==1)){
		pssm->event[0] = 0;//reset event
		DBG(lgr,"REC update timer expired > WR.");
		gRecipe.prv.reqWr = 1;
	}

	//Timer driven current config write request
	if( bgTimerIsTimeOut(&pssm->timer_x1ms[1])  && (pssm->event[1]==1)){
		pssm->event[1] = 0;//reset event
		DBG(lgr,"CFG update timer expired > WR.");
		gConfig.prv.reqConfigWr = 1;
	}

	
	//wait for a config READ request
	if(!paiFS.o.busy && gConfig.prv.reqConfigRd == 1){
		gConfig.prv.reqConfigRd = 0;
		cfgFsClearResponseFlags();
		bgSSMStateNext(pssm, cfgFsCfgRead);
		return;
	}
	
	//wait for a recipe write request (generated by the HMI, after user edits recipes)
	if(!paiFS.o.busy && gRecipe.prv.reqWr){
		gRecipe.prv.reqWr = 0;
		cfgFsClearResponseFlags();
		DBG(lgr,"WrCurRec");

		//store current recipe into REM memory
		cfgRemanentCopyRECFromMemToRem();
		
		//file that holds current / active machine recipe	
		//Example: recFilename = "ABC/xx.txt"
		fileSetCurrentRecipeFilePath(recFilename);		
		PVtoTextFileWriteRequest("gRecipe.active", recFilename);

		//trigger subsequent write request to store this same recipe under a name RecNN where NN is the 
		//index of the currently active REC
		gRecipe.prv.reqWrRec = 1;
		
		bgSSMStateNext(&bgssm, cfgFsBusyFileWrite);
		return;
	}

	//wait for a recipe write request (generated by the HMI, after user edits recipes)
	if(!paiFS.o.busy && gRecipe.prv.reqWrRec){
		gRecipe.prv.reqWrRec = 0;
		cfgFsClearResponseFlags();
		DBG(lgr,"WrCurRecwID=%d", gRecipe.prv.currentRecId);
		
		//file that holds current / active machine recipe	
		//Example: recFilename = "ABC/REC/recNN.txt"
		fileSetRecipeFilePathFromId(recFilename, gRecipe.prv.currentRecId);		
		PVtoTextFileWriteRequest("gRecipe.active", recFilename);
		bgSSMStateNext(&bgssm, cfgFsBusyFileWrite);
		return;
	}

	if(!paiFS.o.busy && gRecipe.prv.reqRd){
		gRecipe.prv.reqRd = 0;
		cfgFsClearResponseFlags();
		DBG(lgr,"LISTrec");
		gRecipe.prv.counter1 = 0;	//used as a file index to create a file names for all recipes		
		gRecipe.prv.counter2 = 0;	//used to counting a number of existing recipe files	
		bgSSMStateNext(pssm, cfgFsRecRead);
		return;
	}

	if(!paiFS.o.busy && gRecipe.prv.reqList){
		gRecipe.prv.reqList = 0;
		cfgFsClearResponseFlags();
		DBG(lgr,"LISTrec");
		gRecipe.prv.counter1 = 0;	//used as a file index to create a file names for all recipes		
		gRecipe.prv.counter2 = 0;	//used to counting a number of existing recipe files	
		bgSSMStateNext(pssm, cfgFsBusyRecipeList);
		return;
	}
	
	if(!paiFS.o.busy && gRecipe.prv.reqDel){
		gRecipe.prv.reqDel = 0;
		cfgFsClearResponseFlags();
	
		if((gRecipe.prv.reqIdx <= APP_RECIPE_MAXID) &&  gRecipe.prv.listRecUsed[gRecipe.prv.reqIdx] ){
			DBG(lgr,"DELrec ID#=%d", gRecipe.prv.reqIdx);
			bgSSMStateNext(pssm, cfgFsBusyRecipeDel);			
		}
		return;
	}


	if(!paiFS.o.busy && gRecipe.prv.reqSetCurrent){
		gRecipe.prv.reqSetCurrent = 0;
		cfgFsClearResponseFlags();
	
		if((gRecipe.prv.reqIdx <= APP_RECIPE_MAXID) &&  gRecipe.prv.listRecUsed[gRecipe.prv.reqIdx] ){
			DBG(lgr,"SETCURrec ID#=%d", gRecipe.prv.reqIdx);
			bgSSMStateNext(pssm, cfgFsBusyRecipeSetCurrent);			
		}
		return;
	}


	if(!paiFS.o.busy && gRecipe.prv.reqSaveAs){
		gRecipe.prv.reqSaveAs = 0;
		cfgFsClearResponseFlags();
	
		if((gRecipe.prv.reqIdx <= APP_RECIPE_MAXID) &&  !gRecipe.prv.listRecUsed[gRecipe.prv.reqIdx] ){
			int len=0;
			
			DBG(lgr,"SaveAs Cur=%d New=%d", gRecipe.prv.currentRecId, gRecipe.prv.reqIdx);
			memcpy(&tempRec, &gRecipe.active, sizeof(cfgMachineRecipe_typ));
			len = strlen(tempRec.name);
			if(len > 32)
				len = 32;			
			bgsprintf(&tempRec.name[len], "(Copy of ID#%d)", gRecipe.prv.currentRecId);
			
			fileSetRecipeFilePathFromId(recFilename, gRecipe.prv.reqIdx);
			PVtoTextFileWriteRequest("cfgFS:tempRec", recFilename);
			bgSSMStateNext(&bgssm, cfgFsBusyFileWrite);


			//update recipe list display
			strcpy(gRecipe.prv.listRecNames[gRecipe.prv.reqIdx], tempRec.name);			
			gRecipe.prv.listRecUsed[gRecipe.prv.reqIdx]=1;
				
			return;
		}
		return;
	}
			
	//wait for a config write request (generated by the HMI, after user edits machine config)
	if(!paiFS.o.busy && gConfig.prv.reqConfigWr == 1){
		gConfig.prv.reqConfigWr = 0;
		cfgFsClearResponseFlags();
		
		DBG(lgr,"WRcfg");
		
		//store config into REM memory
		cfgRemanentCopyCFGFromMemToRem();

		PVtoTextFileWriteRequest("gConfig.active", cfgFilename);
		bgSSMStateNext(&bgssm, cfgFsBusyFileWrite);
		return;		
	}


	//wait for a factory config write request (generated by the HMI, after user request save factory)
	if(!paiFS.o.busy && gConfig.prv.reqConfigWr == 2){
		gConfig.prv.reqConfigWr = 0;
		cfgFsClearResponseFlags();
		
		DBG(lgr,"WRcfgFactory");
		PVtoTextFileWriteRequest("gConfig.active", cfgFilenameFactory);
		bgSSMStateNext(&bgssm, cfgFsBusyFileWrite);
		return;
	}

}


/*
###############################################################################
Init, Cyclic, Exit
###############################################################################
*/
void _INIT cfgFSINIT( void )
{

	
	/* TODO: Add code here */
	DBGINIT(lgr, log, LOG_ENTRY_COUNT, LOG_ENTRY_LENGTH);

	bgSSMInit(&bgssm, (UDINT)getCfgFsStateName, 0);	


	//clear major structures
	memset((UDINT)&gConfig, 0, sizeof(cfgConfig_typ));
	memset((UDINT)&gRecipe, 0, sizeof(cfgRecipe_typ));

	memset((UDINT)&tempCfg, 0, sizeof(cfgMachineConfig_typ));
	memset((UDINT)&tempRec, 0, sizeof(cfgMachineRecipe_typ));

	//init gConfig stucture to default values
	cfgFSDefaultConfig();

	//define a default recipe
	cfgFSDefaultRecipe();

	//remanent data logic
	cfgRemanentDataLogicInit();	
}

//Function called during POWER UP sequence to trigger needed file operations
//1) Read config file
//2) Read recipe file
//3) Read/create recipe list
void cfgFSPowerupSequence(bgSSM_typ *pssm)
{
	//Order of execution is important!
	//Order is defined by the order of statements in the cyclic task (see below) where these flags are evaluated
	//if multiple flags are set at the same, they will be all executed one after another (in the order listed below)
	gConfig.prv.reqConfigRd = 1;
	gRecipe.prv.reqRd = 1; 
	gRecipe.prv.reqList = 1;
	bgSSMStateNext(pssm, cfgFsIdle);
}

void _CYCLIC cfgFSCYCLIC( void )
{
	

	
	/* TODO: Add code here */
bgSSM_typ *pssm = &bgssm;

	/*remanent memory logic - cyclic section*/
	cfgRemanentDataLogicCyclic();
		
	/*execute control state machine cyclic handler*/		
	bgSSMCyclic(pssm);

	/*operate timer always*/		
	bgTimerCyclic(&pssm->timer_x1ms[0]);
	bgTimerCyclic(&pssm->timer_x1ms[1]);
	
	//show change of state in a log
	if( bgSSMIsStateInit(pssm) ){
		UINT curstate = bgSSMGetState(pssm);
		if(	(curstate == cfgFsBusyRecipeList) ||
			(curstate == cfgFsBusyRecipeList1) ||
			(curstate == cfgFsBusyRecipeListNames) ||
			(curstate == cfgFsBusyRecipeListNames1)
		){
			//avoid log entries for some anoying (repetitive) states
		}else{
			DBG(lgr,"sm=%s(%d)", bgSSMGetStateText(pssm), bgSSMGetState(pssm));
		}
	}

	/*sm switch*/
	switch( bgSSMGetState(pssm) ){

		//================================
		//===BEGIN OF POWERUP PROCESSING==
		//================================
		case cfgFsZero:	//init after power cycle - set file names
		
			if(!paiFS.o.initOk)	//can't do much before paiFS completes
				return;

			bgSSMStateNext(pssm, cfgFsInit);

		break;
		//================================
		case cfgFsInit:	//init after power cycle - set file names, check if application directory exists

			if( bgSSMIsStateInit(pssm) ){
				//file that holds machine configuration	
				//Example: cfgFilename = "Data/appcfg.txt"
				fileSetCfgFilePath(cfgFilename);
				
				//file that holds default machine configuration	
				//Example: cfgFilenameFactory = "Data/appcfgfactory.txt"
				fileSetDefaultCfgFilePath(cfgFilenameFactory);
				
				//file that holds current / active machine recipe	
				//Example: recFilename = "Data/Rec/xx.txt"
				fileSetCurrentRecipeFilePath(recFilename);

				//request directory read
				dirInfoRequest(&paiFS, APP_DIR_NAME);	
				break;
			}

			//wait until request completes
			if( fileRequestIsBusy(&paiFS) )
				break;

			//error?
			if(paiFS.o.status){
				DBG(lgr,"dir=%s  InfoError=%d", APP_DIR_NAME, paiFS.o.status);

				if(paiFS.o.status == fiERR_DIR_NOT_EXIST){
					DBG(lgr,"Starting first time...", APP_DIR_NAME);
					DBG(lgr,"dir '%s' notfound. Create it!", APP_DIR_NAME);
					bgSSMStateNext(pssm, cfgFsDirCreate);
					break;
				}

				DBG(lgr,"Unhandled error for dir=%s!", APP_DIR_NAME);
				//try to create dir anyway
				// TODO: nofify machine operator
				bgSSMStateNext(pssm, cfgFsDirCreate);
				break;
			}

			DBG(lgr,"dir='%s'  exists.", APP_DIR_NAME);
			bgSSMStateNext(pssm, cfgFsDirExistRec);

		break;	
		//================================
		case cfgFsDirCreate://create application data directory	

			if( bgSSMIsStateInit(pssm) ){
				dirCreateRequest(&paiFS, APP_DIR_NAME);	
				break;
			}

			//wait until request completes
			if( fileRequestIsBusy(&paiFS) )
				break;

			//error?
			if(paiFS.o.status){
				DBG(lgr,"dir='%s'  CreateError=%d", APP_DIR_NAME, paiFS.o.status);
				
				// TODO: nofify machine operator
				//bgSSMStateNext(pssm, cfgFsCfgRead);
				cfgFSPowerupSequence(pssm);
				break;
			}

			DBG(lgr,"dir='%s'  CreateOK", APP_DIR_NAME);
			bgSSMStateNext(pssm, cfgFsDirCreateRec);

		break;	
		//================================
		case cfgFsDirExistRec:	//check if rec direcotry exist

			if( bgSSMIsStateInit(pssm) ){
				//set rec dir path
				fileSetRecDirPath(recDirPath);
				
				//request directory read
				dirInfoRequest(&paiFS, recDirPath);	
				break;
			}

			//wait until request completes
			if( fileRequestIsBusy(&paiFS) )
				break;

			//error?
			if(paiFS.o.status){
				DBG(lgr,"dir='%s'  InfoError=%d", recDirPath, paiFS.o.status);

				if(paiFS.o.status == fiERR_DIR_NOT_EXIST){
					DBG(lgr,"Starting first time...");
					DBG(lgr,"dir '%s' notfound. Create it!", recDirPath);
					bgSSMStateNext(pssm, cfgFsDirCreateRec);
					break;
				}

				DBG(lgr,"Unhandled error for dir='%s'!", recDirPath);
				//try to create dir anyway
				// TODO: nofify machine operator
				bgSSMStateNext(pssm, cfgFsDirCreateRec);
				break;
			}

			DBG(lgr,"dir='%s'  exists.", recDirPath);
			cfgFSPowerupSequence(pssm);

		break;	
		//================================
		case cfgFsDirCreateRec:	//create rec directory

			if( bgSSMIsStateInit(pssm) ){
				//create recipe directory
				fileSetRecDirPath(recDirPath);
				dirCreateRequest(&paiFS, recDirPath);	
				break;
			}

			//wait until request completes
			if( fileRequestIsBusy(&paiFS) )
				break;

			//error?
			if(paiFS.o.status){
				DBG(lgr,"dir='%s'  CreateError=%d", recDirPath, paiFS.o.status);
				
				// TODO: nofify machine operator
				cfgFSPowerupSequence(pssm);
				break;
			}

			DBG(lgr,"dir='%s'  CreateOK", recDirPath);
			cfgFSPowerupSequence(pssm);

		break;	
		//================================
		//===END OF POWERUP PROCESSING====
		//================================
		
		//================================
		case cfgFsCfgRead:	//read machine cfg file
			cfgFsCfgReadState();
		break;
		//================================
		case cfgFsRecRead:	//read current recipe
			cfgFsRecReadState();
		break;
		//================================
		case cfgFsIdle: //idle state > wait for write requests from the application
			cfgFsIdleState();
		break;
		//================================
		case cfgFsReadCfgFactory:	//read machine factory cfg
			cfgFsReadCfgFactoryState();
		break;
		//================================
		case cfgFsBusyFileWrite: //wait for write complete

			if( fileRequestIsBusy(&paiFS) )
				break;
			
			DBG(lgr,"wr, stat=%d", paiFS.o.status);
			if(paiFS.o.status){
				writecountererr++;
				cfgFsSetResponseErr();	
			}
			else{
				cfgFsSetResponseOk();	
				writecounterok++;
			}
			bgSSMStateNext(pssm, cfgFsIdle);
		break;
		//================================
		case cfgFsBusyFileRead:	//wait for file read done

			if( fileRequestIsBusy(&paiFS) )
				break;
			
			if(paiFS.o.status){
				cfgFsSetResponseErr();	
				readcountererr++;
			}
			else{
				cfgFsSetResponseOk();	
				readcounterok++;
			}
			DBG(lgr,"rd, stat=%d", paiFS.o.status);
			bgSSMStateNext(pssm, cfgFsIdle);
			
		break;
		
		//================================
		case cfgFsBusyRecipeList:	//generate a list of available recipe files

			cfgFsBusyRecipeListState();
			
			break;
		//================================
		case cfgFsBusyRecipeList1:	//wait for recipe list read
			gRecipe.prv.counter1++;
			if(gRecipe.prv.counter1 <= APP_RECIPE_MAXID){
				bgSSMStateNext(pssm, cfgFsBusyRecipeList);		
				break;
			}
			
			//if recipe files found, read name field from each recipe file and copy it into a recipe names list
			if(gRecipe.prv.counter2){		
			
				//find first existing recipe file index
				gRecipe.prv.counter1 = 0;
				while( !gRecipe.prv.listRecUsed[gRecipe.prv.counter1] && (gRecipe.prv.counter1 <= APP_RECIPE_MAXID)){
					gRecipe.prv.counter1++;
				}
				bgSSMStateNext(pssm, cfgFsBusyRecipeListNames);
				break;
			}
			
			bgSSMStateNext(pssm, cfgFsIdle);
			break;
		//================================
		case cfgFsBusyRecipeListNames:	//generate a list of available recipe files

			cfgFsBusyRecipeListNamesState();
	
			break;
		//================================
		case cfgFsBusyRecipeListNames1:	//wait for recipe list read
			
			//find next existing recipe file index
			gRecipe.prv.counter1++;			
			while( !gRecipe.prv.listRecUsed[gRecipe.prv.counter1] && (gRecipe.prv.counter1 <= APP_RECIPE_MAXID)){
				gRecipe.prv.counter1++;
			}
			
			if(gRecipe.prv.counter1 <= APP_RECIPE_MAXID){
				bgSSMStateNext(pssm, cfgFsBusyRecipeListNames);		
				break;
			}

			if(gRecipe.prv.requestRestoreRecId ){
				gRecipe.prv.requestRestoreRecId = 0;
				restoreRecIdFromRecNames();				
			}
			
			bgSSMStateNext(pssm, cfgFsIdle);
			break;
		//================================
		case cfgFsBusyRecipeDel:	//wait for recipe delete
			
			if( bgSSMIsStateInit(pssm) ){
				//isue a request for recipe delete
				fileSetRecipeFilePathFromId(recFilename, gRecipe.prv.reqIdx);
				fileDelRequest(&paiFS, (UDINT)recFilename);
				break;
			}
				
			if( fileRequestIsBusy(&paiFS) )
				break;
			
			//delete error?
			if(paiFS.o.status){
				//delete error for a rec file
				DBG(lgr,"rec='%s' DEL error=%d", recFilename, paiFS.o.status);
				bgsprintf(gRecipe.prv.listRecNames[gRecipe.prv.reqIdx], "File delete error!(%d)", paiFS.o.status);
				cfgFsSetResponseErr();	
				bgSSMStateNext(pssm, cfgFsIdle);
				break;
			}
			//request recipe list refresh
			gRecipe.prv.reqList = 1;
			cfgFsSetResponseOk();	
			bgSSMStateNext(pssm, cfgFsIdle);
			break;
		//================================
		case cfgFsBusyRecipeSetCurrent:	//request recipe read > if success, set the recipe as new current recipe

			cfgFsBusyRecipeSetCurrentState();
			
		break;
		//================================
		case cfgFsBusyRecipeSetCurrent1:	//request recipe read > if success, set the recipe as new current recipe

			cfgFsBusyRecipeSetCurrent1State();
			
		break;
		//================================
		default:
			bgSSMStateNext(pssm, cfgFsIdle);
		break;
	}
}

//EOF
