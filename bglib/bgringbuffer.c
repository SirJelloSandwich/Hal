#ifndef _BG_RINGBUFFER_C
#define _BG_RINGBUFFER_C
/*
File: 
bgRingBuffer.c

About:
------
- ring buffer implementation

B&R System Dependencies:
------------------------
- B&R system library: none
- B&R system header files: stdarg.h 


History:
--------
03/11/2005
-first version

Test and example code:
----------------------
- to run this file standalone:
--- enable following code section (remove or comment out #if 0 / #endif) 
--- create cyclic object (task) from C Language and add this file
*/
#include <stdarg.h>

#include "bgringbuffer.h"
#include "bgdebug.h"

#include "bgsprintf.h"
#include "bgsprintf.c"

#if 0
#include<bur/plc.h>
#include <bur/plctypes.h>

#include <bgsprintf.h>
#include <bgringbuffer.h>

_LOCAL int counter;

_LOCAL bgRingBuffer_typ myRB;

#define TXT_SIZE 64
_LOCAL char textBuffer1[TXT_SIZE];
_LOCAL char textBuffer2[TXT_SIZE];

#define RB_SIZE 64
_LOCAL char ringBuffer[RB_SIZE];

_INIT void Init(void) {

	rbInit(&myRB, ringBuffer, RB_SIZE);	
}

_CYCLIC void Cyclic(void) {

	counter++;

	bgsprintf(textBuffer1, "Counter=%d\n", counter);
	bgRingBuffPutString(&myRB, textBuffer1);
    bgRingBuffGet(&myRB, textBuffer2, TXT_SIZE);
}
#endif
/*End: Example/Test code*/

static char *bgRingBuffGetPtrToFileName(char *pFileNameWithPath)
{
/*
Input Example:
*pFileNameWithPath = "d:/data/temp/cobble/2005-9-1/new_ng.pgp/pgm/new_ng/cpu/bgstatemachine.c"
                                                                             | 
Output value is ptr to: bgstatemachine.c ------------------------------------+
*/
int i;
int len = strlen(pFileNameWithPath);
char* pStart = (char*)(pFileNameWithPath + len);
	for(i=0;i<len; i++, pStart--){
		if( (*pStart == '/') || (*pStart == '\\') ){
			pStart++;
			return pStart;
		}
	}
	return pFileNameWithPath;
}

static void bgRingBuffIncDataInPtr(bgRingBuffer_typ *rb){
	rb->ptrDataIn++;
    if(rb->ptrDataIn >= rb->ptrCircularBufferEnd){
    	rb->ptrDataIn = rb->ptrCircularBufferBegin;
#if 0 
/*mark wrap arround for testing*/   	
    	*rb->ptrDataIn = '@';
    	rb->ptrDataIn++;
#endif    	
    }	
}

static void bgRingBuffIncDataOutPtr(bgRingBuffer_typ *rb){
	rb->ptrDataOut++;
	if(rb->ptrDataOut >= rb->ptrCircularBufferEnd)
		rb->ptrDataOut = rb->ptrCircularBufferBegin;
}

void bgRingBuffInit(bgRingBuffer_typ *rb, char *buffer, int bufSize)
{
	if(!buffer || !bufSize)
		return;

	rb->init = 1;
	rb->overrun = 0;
	rb->bufSize = bufSize;
    rb->ptrCircularBufferBegin = buffer;
    rb->ptrCircularBufferEnd = buffer + rb->bufSize;
    rb->ptrDataIn = rb->ptrCircularBufferBegin;
    rb->ptrDataOut = rb->ptrCircularBufferBegin;
}
/*
----------------------------------------------------------
*/
void bgRingBuffPutString(bgRingBuffer_typ *rb, char *sourceString)
{
    char *src = sourceString;
    
	if(!rb->init)
		return;
		
    /*input is string! look for zero termination*/
    while( *src ){

        /*store char*/
        *rb->ptrDataIn = *src; 

        /*inc ptrs*/
		src++;		
        
        bgRingBuffIncDataInPtr(rb);
                
        /*check if ring-buffer full*/
        if(rb->ptrDataIn == rb->ptrDataOut){
        	/*move out ptr one step ahead of in ptr*/
	        bgRingBuffIncDataOutPtr(rb);
			
			/*mark ring buffer overrun*/	        	
			if(!rb->overrun){
		        char text[3]={'<','?','>'};
		        int i;
				rb->overrun = 1;	        
	        	for(i=0;i<3;i++){
			        *rb->ptrDataIn = text[i];
			        bgRingBuffIncDataInPtr(rb);
			        bgRingBuffIncDataOutPtr(rb);			        
		        } 
		    }                       
        }
    }
}
/*
----------------------------------------------------------
*/
int bgRingBuffGet(bgRingBuffer_typ *rb, char *destination, int destSize)
{
char *dst = destination;
char *dstMax = dst + destSize;
int length = 0;

	if(!rb->init)
		return 0;
		
    if(rb->ptrDataIn == rb->ptrDataOut)
        return 0;

    while(1){
        *dst = *rb->ptrDataOut;
		length++;
		
        dst++;
        /*check if max size riched*/
        if(dst >= dstMax)
            break;
  
  		bgRingBuffIncDataOutPtr(rb);
        
        /*check if ring-buffer empty*/
        if(rb->ptrDataIn == rb->ptrDataOut){
        	
        	/*mark end of ring buffer overrun*/	        	
			if(rb->overrun){
		        char text[3]={'<','!','>'};
		        int i;
		        
				rb->overrun = 0;	        
	        	for(i=0;i<3;i++){
			        *dst = text[i];
			        dst++;
			        length++;
			        if(dst >= dstMax)
			            break;
		        } 
		    }                      
		             
            break;
        }    
    }
    return length;
}


void bgRingBuffPutDbgInfo(bgRingBuffer_typ *rb, char *file, char *function, unsigned int level, unsigned int unused, char *format, ...)
{
char txt[256];

va_list args;

	/*if dbg entry level > then currently set dbg level -> exit*/
	if( level > rb->dbgLevel )
		return;		

	/*timestap needed? default=no*/
	if( rb->dbgFormat & BG_DEBUG_FORMAT_LEVEL){		
		switch(level){
			case 0: bgRingBuffPutString(rb, "CRT|"); break;
			case 1: bgRingBuffPutString(rb, "ERR|"); break;
			case 2: bgRingBuffPutString(rb, "WAR|"); break;
			case 3: bgRingBuffPutString(rb, "INF|"); break;
			case 4: bgRingBuffPutString(rb, "DBG|"); break;
			default: bgRingBuffPutString(rb, "???|"); break;
		}
	}

	/*timestap needed? default=no*/
	if( rb->dbgFormat & BG_DEBUG_FORMAT_TIMESTAMP){		
		bgsprintf(txt, "%s|", "2005-10-01 12:32:44.123" );
		bgRingBuffPutString(rb, txt);
	}

	/*filename needed? default=no*/
	if( rb->dbgFormat & BG_DEBUG_FORMAT_FILENAME){		
		bgsprintf(txt, "%s|", bgRingBuffGetPtrToFileName(file) );
		bgRingBuffPutString(rb, txt);
	}

	/*function name needed? default=no*/
	if( rb->dbgFormat & BG_DEBUG_FORMAT_FUNCTION){		
		bgsprintf(txt, "%s()|", function);
		bgRingBuffPutString(rb, txt);
	}

    va_start( args, format );

	bgvsprintf(txt, format, args);

	bgRingBuffPutString(rb, txt);
}


void bgRingBuffPrintf(bgRingBuffer_typ *rb, char *format, ...)
{
char txt[256];
va_list args;

    va_start( args, format );

	bgvsprintf(txt, format, args);

	bgRingBuffPutString(rb, txt);
}


#endif
/*EOF*/
