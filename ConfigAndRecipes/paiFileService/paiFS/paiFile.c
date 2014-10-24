/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: paiFile
 * File: paiFile.c
 * Author: goran_2
 * Created: August 05, 2011
 ********************************************************************
 * Implementation of program paiFile
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
#include <../../../bglib/bgPVLogger.c>
#define LOG_ENTRY_COUNT 20	//max number of log entries
#define LOG_ENTRY_LENGTH  32 //max length of individual log message. Longer msg are truncated
_LOCAL STRING log[LOG_ENTRY_COUNT][LOG_ENTRY_LENGTH]; //log output visible from Automation Studio!
static bgPVLogger_typ lgr;	//PV logger object
#else
#define DBG(format, args...) ;
#define DBGINIT()  ;
#endif

/*
###############################################################################
Defines, Consts
###############################################################################
*/
/*file device name*/
#define FILE_DEVICE_NAME	"CFDISK"
//#define FILE_DEVICE_CONF	 "/DEVICE=F:\\"
//#define FILE_DEVICE_CONF	 "/DEVICE=C:\\"
/*#define FILE_DEVICE_CONF	 "/DEVICE=D:\\Temp"*/


//IMPORTANT:
//in order for the following conditional compilation to work properly, PLC object in the Configuration view
//must contain following defintion:
//Folder: Build
//Field: Additional Build Options
//Value: -D $(AS_CONFIGURATION)
//Using the above, name of the current AS configuration (i.e. S70_90ARwin or simgb or S70_90) becomes 
//a defined preprocessor name. As a result, it's possible to destinguish between different configurations! 
#ifdef Winchmans_Simulator_ARWin
//In case of ARwin target, disk access uses networking and the disk storage is on the windows side
//CRITICAL NOTE: following IP address must match the Windows IP address within the ARWin networking configuration
//#define FILE_DEVICE_CONF "/SIP=192.168.1.15 /PROTOCOL=cifs /SHARE=abc /USER=abc /PASSWORD=abc"
#define FILE_DEVICE_CONF "/SIP=192.168.2.105 /PROTOCOL=cifs /SHARE=Data /USER=ism /PASSWORD=ism"
#warning "paiFS - ARwin target!"
#endif

#ifndef FILE_DEVICE_CONF
//For all other target (i.e ARsim and ARemb), storage resides on a physical disk present in the system
#define FILE_DEVICE_CONF	 "/DEVICE=C:\\"
#warning "paiFS - NONE ARwin target!"
#endif



#define APP_DATA_FILEMAXSIZE 1024
#define APP_DATA_ITEMSIZE 32

/*component states*/
enum{
	paiFileServiceStateIdle = 0,
	paiFileServiceStateLink,
	paiFileServiceStateLinkError,
	paiFileServiceStateOpenForRead,
	paiFileServiceStateRead,
	paiFileServiceStateOpenForWrite,
	paiFileServiceStateWrite,
	paiFileServiceStateClose,
	paiFileServiceStateCreate,
	paiFileServiceStateDelete,
	paiFileServiceStateInfo,
	paiFileServiceStateDirInfo,
	paiFileServiceStateDirCreate,
	paiFileServiceLast
}APPCFGSTATEMACHINE;

const char* getPaiFileServiceStateName(int state)
{
static const char* stateNames[] = {
"Idle",
"Link",	
"LinkError",	
"OpenForRead",
"Read",
"OpenForWrite",
"Write",
"Close",
"Create",
"Delete",
"Info",
"DirInfo",
"DirCreate",
};

	if( state >= paiFileServiceLast)
		return "unknow";
	
	return stateNames[state];
};


/*
###############################################################################
Typedefs
###############################################################################
*/
//pDevice is CFDISK and pParams changes based on ARWin or ARSim/ARemb
unsigned short paiFileServiceInit(paiFileService_typ* p, char* pDevice, char* pDeviceParam)
{
	if(!p)
		return 0;
	
	memset(p, 0, sizeof(paiFileService_typ));

	p->prv.device.link.pDevice = (UDINT)pDevice;
	p->prv.device.link.pParam= (UDINT)pDeviceParam;
	
	p->prv.file.open.pDevice = (UDINT)pDevice;

	p->prv.requestType = paiFileServiceStateLink;

	bgSSMInit(&p->prv.bgssm, (UDINT)getPaiFileServiceStateName, 0);
	bgSSMStateNext(&p->prv.bgssm, paiFileServiceStateLink);


	p->o.busy = 0;
	p->o.status = 0;
	p->o.initOk = 0;
				
	p->prv.device.link.enable = 0;
	
	p->prv.file.create.enable = 0;
	p->prv.file.open.enable = 0;
	p->prv.file.close.enable = 0;
	p->prv.file.read.enable = 0;
	p->prv.file.readex.enable = 0;
	p->prv.file.write.enable = 0;

	p->prv.dir.info.enable = 0;
	p->prv.dir.create.enable = 0;
	return 0;
}

void paiFileServiceCyclicNextState(paiFileService_typ* p, unsigned short currentStatus, int nextstate, int errorstate)
{
	if(currentStatus == ERR_FUB_BUSY)
		return;

	if(currentStatus != ERR_OK){
		p->prv.status = currentStatus;
		DBG(lgr,"err=%d", currentStatus);
		bgSSMStateNext(&p->prv.bgssm, errorstate);
	}else{
		p->prv.status = ERR_OK;
		bgSSMStateNext(&p->prv.bgssm, nextstate);
	}
}

static BOOL paiFileServiceIsBusy(unsigned short currentStatus)
{
	if(currentStatus == ERR_FUB_BUSY)
		return 1;
	
	return 0;
}

static BOOL paiFileServiceIsError(unsigned short currentStatus)
{
	if(currentStatus != ERR_OK)
		return 1;
	
	return 0;
}


void	paiFileServiceIO(paiFileService_typ* p)
{

	if(!p) 
		return;

	if(!p->o.initOk)
		return;

	/*process file read request*/
	if(!p->o.busy && (p->i.request == PAI_FS_RDFILE)){
		p->o.busy = 1;
		p->o.status = 0;
		p->prv.file.open.pDevice = p->prv.device.link.pDevice;
		p->prv.file.open.pFile = p->i.adrName;
		p->prv.file.open.mode = fiREAD_ONLY;
		p->prv.file.readex.pDest = p->i.adrReadDst;
		p->prv.file.readex.len = p->i.readDstLen;
		p->prv.requestType = paiFileServiceStateRead;	
		DBG(lgr,"RDFile, fName=%s", (char*)p->i.adrName);
		bgSSMStateNext(&p->prv.bgssm, paiFileServiceStateOpenForRead);
		return;
	}


	/*process file write request*/
	if(!p->o.busy && (p->i.request == PAI_FS_WRFILE)){
		p->o.busy = 1;
		p->o.status = 0;
		
		p->prv.file.open.pDevice = p->prv.device.link.pDevice;
		p->prv.file.open.pFile = p->i.adrName;
		p->prv.file.open.mode = fiREAD_WRITE;
		p->prv.file.open.mode = fiWRITE_ONLY;
		p->prv.file.write.pSrc = p->i.adrWriteSrc;
		p->prv.file.write.len = p->i.writeSrcLen;
		p->prv.requestType = paiFileServiceStateWrite;
		DBG(lgr,"WRFile, fName=%s", (char*)p->i.adrName);
		bgSSMStateNext(&p->prv.bgssm, paiFileServiceStateOpenForWrite);
		return;
	}

	/*process file info request*/
	if(!p->o.busy && (p->i.request == PAI_FS_INFOFILE)){
		p->o.busy = 1;
		p->o.status = 0;
		
		p->prv.file.info.pDevice = p->prv.device.link.pDevice;
		p->prv.file.info.pName = p->i.adrName;
		p->prv.file.info.pInfo = (UDINT)&p->prv.file.infoData;
		
		DBG(lgr,"INFFile, fName=%s", (char*)p->i.adrName);
		bgSSMStateNext(&p->prv.bgssm, paiFileServiceStateInfo);
		return;
	}

	/*process file delete request*/
	if(!p->o.busy && (p->i.request == PAI_FS_DELFILE)){
		p->o.busy = 1;
		p->o.status = 0;
		
		p->prv.file.delete.pDevice = p->prv.device.link.pDevice;
		p->prv.file.delete.pName = p->i.adrName;
		
		DBG(lgr,"DELFile, fName=%s", (char*)p->i.adrName);
		bgSSMStateNext(&p->prv.bgssm, paiFileServiceStateDelete);
		return;
	}

	
	/*process dir info request*/
	if(!p->o.busy && (p->i.request == PAI_FS_INFODIR)){
		p->o.busy = 1;
		p->o.status = 0;
		p->prv.dir.info.pDevice = p->prv.device.link.pDevice;
		p->prv.dir.info.pPath = p->i.adrName;
		
		DBG(lgr,"DirInfo, path=%s", (char*)p->i.adrName);
		bgSSMStateNext(&p->prv.bgssm, paiFileServiceStateDirInfo);
		return;
	}

	/*process dir create request*/
	if(!p->o.busy && (p->i.request == PAI_FS_CRDIR)){
		p->o.busy = 1;
		p->o.status = 0;
		p->prv.dir.create.pDevice = p->prv.device.link.pDevice;
		p->prv.dir.create.pName = p->i.adrName;
		
		DBG(lgr,"DirCreate, name=%s", (char*)p->i.adrName);
		bgSSMStateNext(&p->prv.bgssm, paiFileServiceStateDirCreate);
		return;
	}


	
}

void paiFileServiceCyclic(paiFileService_typ* p)
{

bgSSM_typ *pssm = &p->prv.bgssm;
	
	if(!p)
		return;

	paiFileServiceIO(p);


	/*execute control state machine cyclic handler*/		
	bgSSMCyclic(pssm);
	//show change of state in a log
	if( bgSSMIsStateInit(pssm) ){
		DBG(lgr,"sm=%s(%d)", bgSSMGetStateText(pssm), bgSSMGetState(pssm));
	}
	switch( bgSSMGetState(pssm) ){
		/*--------------------------------------------------*/
		case paiFileServiceStateIdle:
			if( bgSSMIsStateInit(pssm) ){
				p->o.busy = 0;

				p->prv.device.link.enable = 0;
				
				p->prv.file.create.enable = 0;
				p->prv.file.open.enable = 0;
				p->prv.file.close.enable = 0;
				p->prv.file.read.enable = 0;
				p->prv.file.readex.enable = 0;
				p->prv.file.write.enable = 0;

				p->prv.dir.info.enable = 0;
				p->prv.dir.create.enable = 0;
			}

		break;
		/*--------------------------------------------------*/
		case paiFileServiceStateLink:
		{
			if( bgSSMIsStateInit(pssm) ){
				p->prv.device.link.enable = 1;
				break;
			}
			

			if( paiFileServiceIsBusy(p->prv.device.link.status) )
				break;

			p->prv.device.link.enable = 0;
			if( paiFileServiceIsError(p->prv.device.link.status) ){
				/*override default handling for certain errors*/
				DBG(lgr,"link.status=%d", p->prv.device.link.status);
				if(p->prv.device.link.status == fiERR_DEVICE_ALREADY_EXIST){
					p->prv.device.link.status = 0;
				}else{
					bgSSMStateNext(pssm, paiFileServiceStateLinkError);
					break;
				}
			}
			
			p->o.initOk = 1;
			bgSSMStateNext(pssm, paiFileServiceStateIdle);
		}
		break;
		/*--------------------------------------------------*/
		case paiFileServiceStateLinkError:
		{
			if( bgSSMIsStateInit(pssm) ){
				p->o.initOk = 0;
			}
			// TODO: device link error handling
		}
		break;
		/*--------------------------------------------------*/
		case paiFileServiceStateOpenForRead:
		{

			if( bgSSMIsStateInit(pssm) ){
				p->prv.file.open.enable = 1;
				break;
			}

			if( paiFileServiceIsBusy(p->prv.file.open.status) )
				break;

			p->prv.file.open.enable = 0;
			if( paiFileServiceIsError(p->prv.file.open.status) ){
				DBG(lgr,"open.status=%d", p->prv.file.open.status);
				p->o.status = p->prv.file.open.status;
				bgSSMStateNext(pssm, paiFileServiceStateIdle);
				break;
			}

			p->prv.file.readex.ident = p->prv.file.open.ident;
			p->prv.file.close.ident = p->prv.file.open.ident;
			bgSSMStateNext(pssm, paiFileServiceStateRead);
		}
		break;
		/*--------------------------------------------------*/
		case paiFileServiceStateRead:
		{
			if( bgSSMIsStateInit(pssm) ){
				p->prv.file.readex.enable = 1;
				break;
			}
			
			if( paiFileServiceIsBusy(p->prv.file.readex.status) )
				break;

			p->prv.file.readex.enable = 0;
			if( paiFileServiceIsError(p->prv.file.readex.status) ){
				DBG(lgr,"readex.status=%d", p->prv.file.readex.status);
				p->o.status = p->prv.file.readex.status;
			}

			//write 0 into the read buffer at the position where content that was just read ends
			//this is important when reading from a text file and than post processing using 
			//string functions
			if(p->prv.file.readex.bytesread < p->prv.file.readex.len){
				*((char*)(p->prv.file.readex.pDest + p->prv.file.readex.bytesread)) = 0;
			}

			bgSSMStateNext(pssm, paiFileServiceStateClose);
		}
		break;
		/*--------------------------------------------------*/
		case paiFileServiceStateOpenForWrite:
		{
			if( bgSSMIsStateInit(pssm) ){
				p->prv.file.open.enable = 1;
				break;
			}
			
			if( paiFileServiceIsBusy(p->prv.file.open.status) )
				break;

			p->prv.file.open.enable = 0;
			if( paiFileServiceIsError(p->prv.file.open.status) ){
				DBG(lgr,"open.status=%d", p->prv.file.open.status);
				/*override default handling for certain errors*/
				if(p->prv.file.open.status == fiERR_FILE_NOT_FOUND ){
					/*write request & file not found > create file*/
					bgSSMStateNext(pssm, paiFileServiceStateCreate);
					break;
				}			
				
				p->o.status = p->prv.file.open.status;
				bgSSMStateNext(pssm, paiFileServiceStateIdle);
				break;				
			}

			p->prv.file.write.ident = p->prv.file.open.ident;
			p->prv.file.close.ident = p->prv.file.open.ident;
			bgSSMStateNext(pssm, paiFileServiceStateWrite);			
		}
		break;
		/*--------------------------------------------------*/
		case paiFileServiceStateWrite:
		{
			if( bgSSMIsStateInit(pssm) ){
				p->prv.file.write.enable = 1;
				break;
			}
			
			if( paiFileServiceIsBusy(p->prv.file.write.status) )
				break;

			p->prv.file.write.enable = 0;
			if( paiFileServiceIsError(p->prv.file.write.status) ){
				DBG(lgr,"write.status=%d", p->prv.file.write.status);
				p->o.status = p->prv.file.write.status;
			}

			bgSSMStateNext(pssm, paiFileServiceStateClose);			
		}
		break;
		/*--------------------------------------------------*/
		case paiFileServiceStateClose:
		{
			if( bgSSMIsStateInit(pssm) ){
				p->prv.file.close.enable = 1;
				break;
			}
			
			if( paiFileServiceIsBusy(p->prv.file.close.status) )
				break;

			p->prv.file.close.enable = 0;
			if( paiFileServiceIsError(p->prv.file.close.status) && !p->o.status){
				DBG(lgr,"close.status=%d", p->prv.file.close.status);
				p->o.status = p->prv.file.close.status;
			}

			bgSSMStateNext(pssm, paiFileServiceStateIdle);			
		}
		break;
		/*--------------------------------------------------*/
		case paiFileServiceStateCreate:
		{	
			if( bgSSMIsStateInit(pssm) ){
				p->prv.file.create.enable = 1;
				p->prv.file.create.pDevice = p->prv.device.link.pDevice;
				p->prv.file.create.pFile = p->i.adrName;
				break;
			}
			
			if( paiFileServiceIsBusy(p->prv.file.create.status) )
				break;

			p->prv.file.create.enable = 0;
			if( paiFileServiceIsError(p->prv.file.create.status) ){
				DBG(lgr,"create.status=%d", p->prv.file.create.status);
				p->o.status = p->prv.file.create.status;
				bgSSMStateNext(pssm, paiFileServiceStateIdle);
				break;
			}

			p->prv.file.write.ident = p->prv.file.create.ident;
			p->prv.file.close.ident = p->prv.file.create.ident;
			bgSSMStateNext(pssm, paiFileServiceStateWrite);			
		}
		break;
		/*--------------------------------------------------*/
		case paiFileServiceStateInfo:
			{	
				if( bgSSMIsStateInit(pssm) ){
					p->prv.file.info.enable = 1;
					break;
				}
			
				if( paiFileServiceIsBusy(p->prv.file.info.status) )
					break;

				p->prv.file.info.enable = 0;
				if( paiFileServiceIsError(p->prv.file.info.status) ){
					DBG(lgr,"info.status=%d", p->prv.file.info.status);
					p->o.status = p->prv.file.info.status;
					bgSSMStateNext(pssm, paiFileServiceStateIdle);
					break;
				}

				p->o.status = p->prv.file.info.status;
				bgSSMStateNext(pssm, paiFileServiceStateIdle);
			}
			break;
		/*--------------------------------------------------*/
		case paiFileServiceStateDelete:
			{	
				if( bgSSMIsStateInit(pssm) ){
					p->prv.file.delete.enable = 1;
					break;
				}
			
				if( paiFileServiceIsBusy(p->prv.file.delete.status) )
					break;

				p->prv.file.delete.enable = 0;
				if( paiFileServiceIsError(p->prv.file.delete.status) ){
					DBG(lgr,"delete.status=%d", p->prv.file.delete.status);
					p->o.status = p->prv.file.delete.status;
					bgSSMStateNext(pssm, paiFileServiceStateIdle);
					break;
				}

				p->o.status = p->prv.file.delete.status;
				bgSSMStateNext(pssm, paiFileServiceStateIdle);
			}
			break;
		/*--------------------------------------------------*/		
		case paiFileServiceStateDirInfo:
		{	
			if( bgSSMIsStateInit(pssm) ){
				p->prv.dir.info.enable = 1;
				break;
			}
			
			if( paiFileServiceIsBusy(p->prv.dir.info.status) )
				break;

			p->prv.dir.info.enable = 0;
			if( paiFileServiceIsError(p->prv.dir.info.status) ){
				DBG(lgr,"dir.info.status=%d", p->prv.dir.info.status);
				p->o.status = p->prv.dir.info.status;
				bgSSMStateNext(pssm, paiFileServiceStateIdle);
				break;
			}

			bgSSMStateNext(pssm, paiFileServiceStateIdle);			
		}
		break;
		/*--------------------------------------------------*/
		case paiFileServiceStateDirCreate:
		{	
			if( bgSSMIsStateInit(pssm) ){
				p->prv.dir.create.enable = 1;
				break;
			}
			
			if( paiFileServiceIsBusy(p->prv.dir.create.status) )
				break;

			p->prv.dir.create.enable = 0;
			if( paiFileServiceIsError(p->prv.dir.create.status) ){
				DBG(lgr,"dir.create.status=%d", p->prv.dir.create.status);
				p->o.status = p->prv.dir.create.status;
				bgSSMStateNext(pssm, paiFileServiceStateIdle);
				break;
			}

			bgSSMStateNext(pssm, paiFileServiceStateIdle);			
		}
		break;
		/*--------------------------------------------------*/
		default:
		{
			bgSSMStateNext(&p->prv.bgssm, paiFileServiceStateIdle);
		}
		break;			
		/*--------------------------------------------------*/
	}


	DevLink(&p->prv.device.link);
	
	FileCreate(&p->prv.file.create);
	FileOpen(&p->prv.file.open);
	FileReadEx(&p->prv.file.readex);
	FileWrite(&p->prv.file.write);
	FileClose(&p->prv.file.close);
	FileInfo(&p->prv.file.info);
	FileDelete(&p->prv.file.delete);
			
	DirInfo(&p->prv.dir.info);
	DirCreate(&p->prv.dir.create);


	return;
}

/*
###############################################################################
PV-s
###############################################################################
*/

/*
###############################################################################
Init, Cyclic, Exit
###############################################################################
*/
void _INIT paiFileSrvINIT( void )
{
	/* TODO: Add code here */
	DBGINIT(lgr, log, LOG_ENTRY_COUNT, LOG_ENTRY_LENGTH);
	
	paiFileServiceInit(&paiFS, FILE_DEVICE_NAME, FILE_DEVICE_CONF);	
	
	
	
}

void _CYCLIC paiFileSrvCYCLIC( void )
{
	/* TODO: Add code here */	
	paiFileServiceCyclic(&paiFS);
}

void _EXIT paiFileSrvEXIT( void )
{
	/* TODO: Add code here */
}
