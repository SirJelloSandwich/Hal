/**\file: bgBufMemAlloc.h
*Buffer memory (user supplied memory) simple allocation methods. 
*/
#ifndef _BG_BUFMEMALLOC_H
#define _BG_BUFMEMALLOC_H


/**
\def BGBUFMEMALLOC_MAX_BLOCKS 
Defines maximum number of items within the list of memory blocks. 
Used for data object: \link bgBufMemAlloc_typ \endlink
*/
#define BGBUFMEMALLOC_MAX_BLOCKS	16


/**
\struct bgBufMemAlloc_typ 
Data object that holds information about simple memory allocation method.\n
In a deeply embedded systems, memory allocation is rather uncommon practice. However,
there are situations (i.e. networking) where certain, simplified "mem allocation" scheme
is rather useful.\n
This data object and associated methods implement memory allocation scheme that uses 
user suppllied memory buffer as the "source memory". 
*/
typedef struct bgBufMemAlloc_typ{
	char* pBufMem;			/**<Input: User supplied memory buffer*/
	int lengthBufMemBytes; 	/**<Input: Lenght (bytes) of the user supplied memory buffer*/

    int countMaxBlocks;		/**<Input(const): Max number of mem blocks*/
    int countUsedBlocks;	/**<Output: Number of currently allocated blocks*/
    int lengthUsedMemBytes;	/**<Output: Total length (bytes) of currently allocated memory*/

    struct bgBufMemBlock_typ{
		int 	used;			/**<I/O: 0-block not used; 1-block is used*/
        char*   pMemBlock;		/**<Output: Ptr to the mem block*/
        int     lengthMemBlockBytes;	/**<Input: Size (bytes) of the mem block*/
    }memBlock[BGBUFMEMALLOC_MAX_BLOCKS];

}bgBufMemAlloc_typ;


/**
Init method for data object: \link bgBufMemAlloc_typ \endlink
\param p Ptr to data object: \link bgBufMemAlloc_typ \endlink
\param pBufMem Ptr to user supplied memory buffer
\param lengthBufMemBytes Size of the user supplied memory buffer
<b>IMPORTANT: </b>This method <b>must be always</b> called first for the associated data object!\n

*/
void bgBufMemAllocInit(bgBufMemAlloc_typ* p, char* pBufMem, int lengthBufMemBytes);



/**
Method for data object: \link bgBufMemAlloc_typ \endlink\n
Creates / allocates memory block of size (sizeMemBlock)
\param p Ptr to data object: \link bgBufMemAlloc_typ \endlink
\param blockSize Requested memory block size
\param pMemBlock Address to store the pointer to newly allocated mem block
\param sizeMemBlock Address to store the size of the newly allocated mem block
\return Id for the allocated mem block (0 to (BGBUFMEMALLOC_MAX_BLOCKS-1))
*/
int bgBufMemAllocCreateBlock(bgBufMemAlloc_typ* p, int blockSize, char** pMemBlock, int* sizeMemBlock);


/**
Method for data object: \link bgBufMemAlloc_typ \endlink\n
Give the memblock Id, returns the memory and size of the requesed memblock
\param p Ptr to data object: \link bgBufMemAlloc_typ \endlink
\param blockId Id of the mem block previosly allocated using \link bgBufMemAllocCreateBlock \endlink
\param blockSize Address to store the size of the newly allocated mem block
\return Address of the newly allocated mem block or NULL (in case the blockId is invalid)
*/
char* bgBufMemAllocGetBlockPtr(bgBufMemAlloc_typ* p, int blockId, int* blockSize);



/*
##################################################################################
String heap
##################################################################################
*/


/**
\struct bgStringHeap_typ 
Data object that holds information about simple memory allocation method for strings.\n
In a deeply embedded systems, dynamic string memory allocation is rather uncommon practice. 
However, there are situations (i.e. networking) where certain, simplified dynamic allocation
scheme for storing variable lenght strings is rather useful.\n
This data object and associated methods implement dynamic memory allocation scheme for 
strings that uses user suppllied memory buffer as a memory to store variable length strings.
*/
typedef struct bgStringHeap_typ{
	char* pMemStart;	/**<Start of the user supplied memory buffer*/
	int	memSize;		/**<Size (bytes) of the user supplied memory buffer*/
	char* pMemCurrent;	/**<Ptr to the current free memory location within the (pMemStart)*/
	int memFree;		/**<Size (bytes) of still available / free memory*/
}bgStringHeap_typ;


/**
Method for data object: \link bgStringHeap_typ \endlink\n
Creates / allocates memory block of size (sizeMemBlock)
\param pHeap Ptr to data object: \link bgStringHeap_typ \endlink
\param pHeapMem User supplied memory that will be used for storage of dynamic strings
\param memSize Size of the user supplied memory 
\return Ptr to the start of memory (same as pHeapMem) or NULL if error
<b>IMPORTANT: </b>This method <b>must be always</b> called first for the associated data object!\n
*/
char* bgStringHeapInit(bgStringHeap_typ* pHeap, char* pHeapMem, int memSize);



/**
Method for data object: \link bgStringHeap_typ \endlink\n
Stores given string (pStr) to the string heap memory. Returns ptr to the stored string.
\param pHeap Ptr to data object: \link bgStringHeap_typ \endlink
\param pStr String to store on the string heap
\return Ptr to string stored on the heap or NULL if error (not enough space)
*/
char* bgStringHeapAddString(bgStringHeap_typ* pHeap, char* pStr);

#endif
