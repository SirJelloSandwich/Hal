/********************************************************************
 * COPYRIGHT -- Microsoft
 ********************************************************************
 * Program: Files
 * File: Files.c
 * Author: palmerk
 * Created: October 07, 2014
 ********************************************************************
 * Implementation of program Files
 ********************************************************************/

#include <bur/plctypes.h>
#include <fileio.h>
#include <string.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#define RD_DATA_BUF 2000
#define NUM_OF_QUES 10
#define QUES_MAX_LENGTH 1000
#define ANSWER_LENGTH 10
//#define CFDISK "CFDISK"
//#define PARAM "/SIP=192.168.2.105 /PROTOCOL=cifs /SHARE=C_Data /USER=ism /PASSWORD=ism"

#define KP "KP"

_GLOBAL char *pch;
char bOK;
_GLOBAL UDINT *ppp;
_GLOBAL UDINT fileOffset,dwIdent _VAR_INIT (0);
_GLOBAL USINT byStep;
_GLOBAL USINT byErrorLevel;
_GLOBAL STRING byReadData[RD_DATA_BUF];//, WriteData[1000];
_GLOBAL STRING ques[NUM_OF_QUES][QUES_MAX_LENGTH];
_GLOBAL STRING answer[NUM_OF_QUES][ANSWER_LENGTH];
_GLOBAL SINT ii _VAR_INIT (0);
_GLOBAL SINT jj _VAR_INIT (0);
_GLOBAL SINT kk _VAR_INIT (0);
_GLOBAL UINT Errorcode;
_GLOBAL FileOpen_typ FOpen     _VAR_INIT (0);
_GLOBAL FileRead_typ FRead     _VAR_INIT (0);
_GLOBAL DevLink_typ DLink      _VAR_INIT(0);
_GLOBAL FileCreate_typ FCreate _VAR_INIT(0);
_GLOBAL UINT Status, wError;
  
_LOCAL FileClose_typ FClose;
_LOCAL FileCreate_typ FCreate;
_LOCAL FileWrite_typ FWrite;
_LOCAL FileDelete_typ FDelete;


_GLOBAL STRING devicePtr[6] _VAR_INIT("CFDISK");
_GLOBAL STRING paramPtr[100] _VAR_INIT("/SIP=192.168.2.105 /PROTOCOL=cifs /SHARE=C_Data /USER=ism /PASSWORD=ism");
//_GLOBAL STRING *CFdevice;
_GLOBAL UDINT *device;
_GLOBAL UDINT *param;

_GLOBAL char *PK;
_GLOBAL UDINT one;

enum
{
	LINK = 0,
	OPEN,
	CREATE,
	READ,
	STRCPY,
	LIMBO
		
};

void _INIT FilesInit(void)
{
	int i;
	//ppp = CFDISK;
	
	/* Initialize variables */
	bOK          = 0;
	byStep       = LINK;
	byErrorLevel = 0;
	
	/* Initialize read and write data */
	for (i = 0; i < RD_DATA_BUF; i ++)
	{
		//WriteData[i]  = i + 1;
		byReadData[i] = 0;
	}
	//CFdevice = CFDISK;
	device = &devicePtr;
    param = &paramPtr;
	//device = CFDISK;
	//param = PARAM;
	DLink.pDevice   =  device;
	DLink.pParam    =  param;
	//FOpen.pDevice   = devicePtr;
	
	PK = KP;
	one = (UDINT) PK;
	
}

void _CYCLIC FilesCyclic(void)
{	
	switch (byStep)
	{
		case LINK: 
			
			/**** Error step ****/
			bOK = 0;
			//Link step
			
			DLink.enable = 1;
			DevLink(&DLink);
		
			if( DLink.status == ERR_FUB_BUSY)
			{						
				//DLink.enable = 0;	
				break;
			}
			else if (DLink.status == fiERR_SYSTEM )
			{
				//DLink.enable = 0;
				Errorcode = FileIoGetSysError();
				break;
			//	byStep = LIMBO;
			}
			else if (DLink.status == fiERR_FILE_DEVICE )
			{
			
			}
			
			
			break;
		
		case OPEN: 
			
			// Try to open existing file 
			// Initialize file open structrue 
			FOpen.enable      = 1;
			//FOpen.pDevice   = (UDINT) "CFDISK";
			FOpen.pFile     = (UDINT) "Data/ISM/testfile.txt";
			FOpen.mode      = fiREAD_ONLY;                        // Read and write access

			// Call FUB 
			FileOpen(&FOpen);

			 //Get FBK output information 
			dwIdent = FOpen.ident;
			Status  = FOpen.status;
			
			 //Verify status (20708 -> File doesn't exist) 
			if (Status == 20708)
			{
				byStep = CREATE;
			}
				//OK NO Error
			else if (Status == 0)
			{
				byStep = 3;
			}
				//65535 == FB is busy and still working
				//20798 == error in device manager
			else if (Status != 65535)
			{
				byErrorLevel = 1;
				byStep = 0;
			
				//system error
				if (Status == 20799)
				{
					wError = FileIoGetSysError();
				}
			}
		
			break;
		
		case CREATE:
			
			
			FCreate.pDevice = (UDINT) "CFDISK";
			FCreate.pFile   = (UDINT) "Data\ISM\testfile.txt";
			FCreate.enable    = 1;
			// Call FUB 
			FileCreate(&FCreate);
		
			break;
		
		case READ:
			
			FRead.enable    = 1;
			FRead.ident     = dwIdent;//file to be read
			FRead.offset    = fileOffset;//where to start reading
		//	FRead.pDest     = (UDINT) &byReadData[0];
			FRead.pDest     = (UDINT) &byReadData;//target address where to copy data
			FRead.len       = sizeof (byReadData);
			
			/* Call FBK */
			FileRead(&FRead);
			
			/* Get status */
			Status = FRead.status;
			
			/* Verify status */
			if (Status == 0)
			{
				byStep = 5;
				pch    = strtok(byReadData, "\r\n");//carriage return then new line feed
			}
			else if (Status != 65535)
			{
				byErrorLevel = 4;
				byStep = 0;
			
				if (Status == 20799)
				{
					wError = FileIoGetSysError();
				}
			}
           
			break;
		
		case STRCPY:
		
			
			if (pch != NULL)
			{
							
				if( ii % 2 == 0)
				{
					strcpy(ques[kk], pch);
					kk++;
					pch = strtok(NULL, "\r\n");//carriage return then new line feed
				}
				else 
				{
					strcpy(answer[jj], pch);
					jj++;
					pch = strtok(NULL, "\r\n");//carriage return then new line feed
				}
				
			
			}
			else
			{
				byStep = 6;
			}
			
			ii++;
			//memcpy(byReadData, 0 , strlen(pch));
			break;
		
		case LIMBO:
		
			break;
		
		
	}

}
