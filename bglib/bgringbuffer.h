/*
File: 
bgRingBuffer.c

About:
------
- ring buffer implementation

*/
#ifndef _BG_RINGBUFFER_H
#define _BG_RINGBUFFER_H

#define BG_DEBUG_FORMAT_FILENAME	0x1
#define BG_DEBUG_FORMAT_FUNCTION	0x2
#define BG_DEBUG_FORMAT_TIMESTAMP	0x4
#define BG_DEBUG_FORMAT_LEVEL		0x8


/*
Public data types:
-----------------
*/
typedef struct bgRingBuffer_typ{

    int     bufSize;
    int     init;
    int     overrun;
    int     dbgLevel;
    int     dbgFormat;
	
    char    *ptrCircularBufferBegin;
    char    *ptrCircularBufferEnd;

    char    *ptrDataIn;    
    char    *ptrDataOut;    

}bgRingBuffer_typ;

/*
Public functions:
-----------------
*/
/*
- copy max. destSize bytes of data from the ring buffer to *destination
*/
int bgRingBuffGet(bgRingBuffer_typ *rb, char *destination, int destSize);

/*
- copy *sourceString (null terminated c-style string) into the ring buffer
*/
void bgRingBuffPutString(bgRingBuffer_typ *rb, char *sourceString);

/*
- before RingBuffer can be used data buffer of size destinationBufferSize must be defined
(use allocated or _LOCAL/_GLOBAL space)
*/
void bgRingBuffInit(bgRingBuffer_typ *rb, char *destination, int destinationBufferSize);


/*
- debug specific function for entering data into the ring buffer
*/
void bgRingBuffPutDbgInfo(bgRingBuffer_typ *rb, char *file, char *function, unsigned int level, unsigned int context, char *format, ...);


/*
- put formated message (i.e. printf) into the ring buffer
*/
void bgRingBuffPrintf(bgRingBuffer_typ *rb, char *format, ...);

#endif

