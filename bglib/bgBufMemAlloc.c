#ifndef _BG_BUFMEMALLOC_C
#define _BG_BUFMEMALLOC_C

#include "bgBufMemAlloc.h"
//#include "string.h"

/*
##################################################################################
Buffer memory allocation
##################################################################################
*/
void bgBufMemAllocInit(bgBufMemAlloc_typ* p, char* pBufMem, int lengthBufMemBytes)
{
int i;

	p->pBufMem = pBufMem;
	p->lengthBufMemBytes = lengthBufMemBytes;
	p->lengthUsedMemBytes = 0;
	p->countUsedBlocks = 0;
	p->countMaxBlocks = BGBUFMEMALLOC_MAX_BLOCKS;
	for(i=0; i < p->countMaxBlocks; i++){
		p->memBlock[i].used = 0;
		p->memBlock[i].pMemBlock = 0;
		p->memBlock[i].lengthMemBlockBytes = 0;
	}
}

int bgBufMemAllocCreateBlock(bgBufMemAlloc_typ* p, int blockSize, char** pMemBlock, int* sizeMemBlock)
{
int i;
int availableBufMem = p->lengthBufMemBytes - p->lengthUsedMemBytes;

	if(blockSize >= availableBufMem)
		goto errorExit;

	if(p->countUsedBlocks >= p->countMaxBlocks)
		goto errorExit;

	for(i=0; i < p->countMaxBlocks; i++){
		if(!p->memBlock[i].used){
			p->memBlock[i].used = 1;			
			p->memBlock[i].pMemBlock = (p->pBufMem + p->lengthUsedMemBytes);
			/*0 block size - use all available memory*/
			if(!blockSize)
				blockSize = availableBufMem;				
			p->memBlock[i].lengthMemBlockBytes = blockSize;
			p->lengthUsedMemBytes += blockSize;
			p->countUsedBlocks++;
			if(pMemBlock)
				*pMemBlock = p->memBlock[i].pMemBlock;
			if(sizeMemBlock)
				*sizeMemBlock = p->memBlock[i].lengthMemBlockBytes;
			return 0;
		}
	}

errorExit:
	if(pMemBlock)
		*pMemBlock = 0;
	if(sizeMemBlock)
		*sizeMemBlock = 0;
	return 1;
}

char* bgBufMemAllocGetBlockPtr(bgBufMemAlloc_typ* p, int blockId, int* blockSize)
{
	if(blockSize)
		*blockSize = 0;

	if((blockId < p->countMaxBlocks) && (!p->memBlock[blockId].used)){
		if(blockSize)
			*blockSize = p->memBlock[blockId].lengthMemBlockBytes;
		return	p->memBlock[blockId].pMemBlock;
	}

	return 0;
}


/*
##################################################################################
String heap
##################################################################################
*/
char* bgStringHeapInit(bgStringHeap_typ* pHeap, char* pHeapMem, int memSize)
{
	pHeap->pMemStart = pHeapMem;
	pHeap->pMemCurrent = pHeap->pMemStart;
	*pHeap->pMemCurrent = 0;
	pHeap->memSize = memSize;
	pHeap->memFree = pHeap->memSize;
	return pHeap->pMemStart;
}

char* bgStringHeapAddString(bgStringHeap_typ* pHeap, char* pStr)
{
int len = strlen(pStr);
char* pCurString = pHeap->pMemCurrent;;
	
	/*adjust lentgh for 0 termination*/
	len += 1;

	if( len >= pHeap->memFree )
		return 0;

	strcpy(pCurString, pStr);

	/*move current ptr*/
	pHeap->pMemCurrent += len;

	/*reduce free mem*/
	pHeap->memFree -= len;

	return pCurString;
}
#endif
