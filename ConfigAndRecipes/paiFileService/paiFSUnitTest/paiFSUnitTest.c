/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: paiFileTestClient
 * File: paiFileTestClient.c
 * Author: goran_2
 * Created: August 05, 2011
 ********************************************************************
 * Implementation of program paiFileTestClient
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
 #include <AsDefault.h>
#endif

/*
###############################################################################
PV-s
###############################################################################
*/


/*
###############################################################################
PV-s
###############################################################################
*/
#define BUF_LEN 1000
_LOCAL char txt[BUF_LEN];

_LOCAL char filename[32];

_LOCAL int readcounterok ;
_LOCAL int readcountererr ;

_LOCAL int writecounterok ;
_LOCAL int writecountererr ;

_LOCAL int myInitDone;

/*
###############################################################################
Init, Cyclic, Exit
###############################################################################
*/
void _INIT paiFileTestClientINIT( void )
{
	/* TODO: Add code here */	
	myInitDone = 0;	
}

void _CYCLIC paiFileTestClientCYCLIC( void )
{
	/* TODO: Add code here */
	
	/*wait till file service ready then init application access to file service*/
	if(!myInitDone && paiFS.io.initOk){	
		myInitDone = 1;
		/*copy filename into a buffer so it may be changed from the watch window*/
		strcpy(filename, "appcfg.txt");
		paiFS.io.fileName = (UDINT)filename;

		/*in this example same buffer used for read and write*/
		paiFS.io.readDstAdr = (UDINT)txt;
		paiFS.io.readDstLen = BUF_LEN;
		paiFS.io.writeSrcAdr= (UDINT)txt;	

		
		/*aux PV-s to monitor the activity*/		
		readcounterok = 0;
		readcountererr = 0;
		
		writecounterok = 0;
		writecountererr = 0;
	}


	/*in this example continously update length for write request. That way, when writeRequest get's issued from watch window, correct value gets set!*/	
	paiFS.io.writeSrcLen = strlen(&txt[0]);


	/*example on how to monitor if file service has completed a request*/
	if(paiFS.io.readDone){
		/*reset signal here*/
		paiFS.io.readDone = 0;
		
		if(paiFS.io.status)
			readcountererr++;
		else
			readcounterok++;
	}

	if(paiFS.io.writeDone){
		/*reset signal here*/
		paiFS.io.writeDone = 0;
		
		if(paiFS.io.status)
			writecountererr++;
		else
			writecounterok++;
	}
}

void _EXIT paiFileTestClientEXIT( void )
{
	/* TODO: Add code here */
}
