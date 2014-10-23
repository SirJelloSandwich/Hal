/********************************************************************
 * COPYRIGHT -- HP
 ********************************************************************
 * Library: bglibas
 * File: bgfifo.c
 * Author: goran
 * Created: February 01, 2013
 *******************************************************************/

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "bglibas.h"

#ifdef __cplusplus
	};
#endif
#include <string.h>

static void bgFIFOPushEntry(bgFIFO_typ* p)
{
	char *pSrc = (char*)p->iPtrPushData;//src data
	char* pDst = (char*)((char*)p->prvFIFOMemory + (p->cfgEntrySize * p->oPushIdx));//empty FIFO location
	memcpy(pDst, pSrc, p->cfgEntrySize);
	
	p->oPushIdx++;//inc location index to next empty FIFO spot
	if(p->oPushIdx >= p->cfgMaxEntries){
		p->oPushIdx = 0;//handle roll over (ring buffer)
	}
	
	p->oFIFOCount++;//inc number of FIFO entries
	if(p->oFIFOCount >= p->cfgMaxEntries){
		p->oFIFOFull = 1;//set FIFO full indication
	} 	
}

static void bgFIFOPullEntry(bgFIFO_typ* p)
{
	char* pSrc = (char*)((char*)p->prvFIFOMemory + (p->cfgEntrySize * p->oPullIdx));//oldest FIFO entry 
	char *pDst = (char*)p->oPtrPullData;//destination
	memcpy(pDst, pSrc, p->cfgEntrySize);
	
	p->oPullIdx++;//inc oldest stored FIFO entry position
	if(p->oPullIdx >= p->cfgMaxEntries){
		p->oPullIdx = 0;//handle roll over (ring buffer)
	}
	
	if(p->oFIFOCount)//dec number of entris in the FIFO
		p->oFIFOCount--;
	
	p->oFIFOFull = 0;//reset FIFO full indication
}

/**
bgFIFOInit()
-simple FIFO. Stores upto (entryTotalCount) number of elements. Elements MUST be of the same size.
-supports push/pull action
-push:
- user supplied data is copied into the available FIFO memory space, number of items in the FIFO gets incremented, 
index variable that keeps track of next empty FIFO location is adjusted; if FIFO full - no action, error returned
-pull:
- oldest entry from FIFO copied to user supplied data space; number of items in the FIFO gets decremented, 
index variable that keeps track of oldeset FIFO entry is adjusted; if FIFO empty - no action, error is returned
*/
unsigned long bgFIFOInit(struct bgFIFO_typ* p, unsigned long  entrySize, unsigned long  entryTotalCount, unsigned long  pFIFOMem)
{
	if(!p || !pFIFOMem || !entrySize || (entryTotalCount <=5))
		return 1;
	
	p->iPtrPushData = 0;
	p->oPtrPullData = 0;
	
	p->oFIFOCount = 0;
	p->oFIFOFull = 0;
	
	p->cfgEntrySize = entrySize;
	p->cfgMaxEntries = entryTotalCount;
	p->prvFIFOMemory = pFIFOMem;
	p->oPullIdx = 0;
	p->oPushIdx = 0;
	return 0;
}

/* Push data to FIFO. Content from pSrc is copied into the available FIFO memory. In case of error return value is != 0*/
unsigned long bgFIFOPush(struct bgFIFO_typ* p, unsigned long pSrc, unsigned long srcSize)
{
	if(!p)
		return 1;
	
	if(!p->prvFIFOMemory)
		return 2;

	if(!pSrc)
		return 3;

	if(!srcSize || (srcSize != p->cfgEntrySize) )
		return 4;
	
	if(p->oFIFOFull)
		return 5;
	
	p->iPtrPushData = pSrc;
	bgFIFOPushEntry(p);
	return 0;
}

/* Pull (retrive) data from FIFO. Oldest entry from FIFO memory is copied into the provided pDst space. In case of error return value is != 0*/
unsigned long bgFIFOPull(struct bgFIFO_typ* p, unsigned long pDst, unsigned long dstSize)
{
	if(!p)
		return 1;
	
	if(!p->prvFIFOMemory)
		return 2;

	if(!pDst)
		return 3;

	if(!dstSize || (dstSize != p->cfgEntrySize) )
		return 4;
	
	if(!p->oFIFOCount)
		return 5;
	
	p->oPtrPullData = pDst;
	bgFIFOPullEntry(p);
	return 0;
}

/* Get the number of entries stored in the FIFO.*/
unsigned long bgFIFOGetCount(struct bgFIFO_typ* p)
{
	if(!p)
		return 0;
	
	return p->oFIFOCount;
}

/* Check if FIFO is full.*/
unsigned long bgFIFOIsFull(struct bgFIFO_typ* p)
{
	if(!p)
		return 0;
	
	return p->oFIFOFull;
}
//EOF
