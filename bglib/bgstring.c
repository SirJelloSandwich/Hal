#ifndef _BG_STRING_C
#define _BG_STRING_C

#ifdef WIN32
#include "string.h"
#else
#include <bur/plctypes.h>
#endif
#include "bgstring.h"

/*
File: 
bgString.c

About:
------
- implements  additional string manipulation functions

B&R System Dependencies:
------------------------
- B&R system library: libasstring.a
- B&R system header files: asstring.h 


History:
--------
09/11/2005
- rectified issue with bgStringListLines() to properly handle zero length
input string

05/08/2005
-first version

Test and example code:
----------------------
- to run this file standalone:
--- enable following code section (remove or comment out #if 0 / #endif) 
--- create cyclic object (task) from C Language and add this file
*/

#if 0
#include <bur/plc.h>
#include <bur/plctypes.h>

#define TXT_SIZE 64
_LOCAL char textBuffer1[TXT_SIZE];
_LOCAL char textBuffer2[TXT_SIZE];
_LOCAL int idxResult;
_LOCAL int idxStart;
_LOCAL int idxEnd;


_INIT void Init(void) {

	/*use defaults*/
	idxStart 	= -1;
	idxEnd 		= -1;
	/*start with empty strings*/
	textBuffer1[0] = 0;
	textBuffer2[0] = 0;	
}

_CYCLIC void Cyclic(void) {

	idxResult = bgStrFind(textBuffer1, textBuffer2, start, end);

}
#endif

/*End: Example/Test code*/
int bgStrFind(char *s, char *sub, int idxStart, int idxEnd)
{
	int i, j;
	int subLen;
	char *ptrStr;
	char *ptrSub;
	

	/*exit if null ptrs*/
	if(!s || !sub)
		return -1;

	/*exit if zero strings*/
	if( !(*s) || !(*sub) )
		return -1;

	subLen = strlen(sub);

	/*check starting index: default = -1 -> start @ s[0]*/
	if(idxStart < 0)
		idxStart = 0;

	/*check end index: default = -1 -> end is sLength - subLength*/
	if(idxEnd < 0){
		idxEnd = strlen(s) - subLen;
	}

	/*check if start is beyond the end*/
	if(idxStart > idxEnd)
		return -1;

	/*search for string <sub> within the string <s>*/
	for(i=idxStart; i<=idxEnd; i++)
	{
		ptrStr = s + i;
		ptrSub = sub;
		
		for(j=0; j<subLen; j++, ptrStr++, ptrSub++)
		{
			if( *ptrStr!= *ptrSub)
				break; 		
		}
		
		if( j == subLen)
			return i;
	}
	
	return -1;
}

/*
bgStrFindEx - has an extra option
option = 0; same as bgStrFind
option = 1; before comparing, both strings are converted to lower case
*/
int bgStrFindEx(char *s, char *sub, int idxStart, int idxEnd, int option)
{
	int i, j;
	int subLen;
	char *ptrStr;
	char *ptrSub;
	

	/*exit if null ptrs*/
	if(!s || !sub)
		return -1;

	/*exit if zero strings*/
	if( !(*s) || !(*sub) )
		return -1;

	subLen = strlen(sub);

	/*check starting index: default = -1 -> start @ s[0]*/
	if(idxStart < 0)
		idxStart = 0;

	/*check end index: default = -1 -> end is sLength - subLength*/
	if(idxEnd < 0){
		idxEnd = strlen(s) - subLen;
	}

	/*check if start is beyond the end*/
	if(idxStart > idxEnd)
		return -1;

	/*search for string <sub> within the string <s>*/
	for(i=idxStart; i<=idxEnd; i++)
	{
		ptrStr = s + i;
		ptrSub = sub;
		
		for(j=0; j<subLen; j++, ptrStr++, ptrSub++)
		{
			if(!option){
				if( *ptrStr!= *ptrSub)
					break; 
			}else{
				if( bgCharToLowerCase(*ptrStr) != bgCharToLowerCase(*ptrSub) )
					break; 
			}				
		}
		
		if( j == subLen)
			return i;
	}
	
	return -1;
}


int bgStrCpyN(char *dest, char *src, int maxNoOfCharsToCopy)
{
int i;

	/*maxNoOfCharsToCopy-1 - takes into consideration that supplied max 
	length is usually the buffer size, 1 char is needed for null termination
	*/
	for(i=0; i<(maxNoOfCharsToCopy-1);i++){
		*(dest+i)=*(src+i);
		if(! (*(src+i) ) ) 
			break;
	}

	/*if max reached, terminate dest string*/
	if(i >= (maxNoOfCharsToCopy-1))
		*(dest+i)=0;

	/*string termination is not included in the string length*/
	return i;
}


int bgStrTrimCRLF(char *s)
{
int i;
char c;

	for(i=0;i<255;i++){

		c = s[i];

		if( !c )
			break;

		if( (c == 0xa) || (c == 0xd) ){
			s[i] = 0;
			break;
		} 
	}

	return i;
}
	


int bgStrCpyNTrimLeadingSpaces(char *dest, char *src, int maxNoOfCharsToCopy)
{
int i;
int found;
char *srcStart;


	/*look for leading spaces*/
	found = 0;
	for(i=0; i<(maxNoOfCharsToCopy-1);i++){
		if( *(src+i) != ' ' ){
			srcStart = src + i;
			found = 1;
			break;
		} 
	}

	if(!found){
	/*src string is all spaces->set destination to empty string*/
		*dest = 0;
		return 0;
	}
	
	return bgStrCpyN(dest, srcStart, maxNoOfCharsToCopy);
}


/*
copy string, don't go over the size of dest buffer, trim leading / trailing spaces / convert
to lower case
*/
int bgStrCpyNTrimSpcsToLwr(char *dest, char *src, int maxNoOfCharsToCopy)
{
int i;
char* srcStart = src;
char* srcEnd;
int len = strlen(src);


	if( !len )
		return len;
	
	/*find src start, avoid leading spaces*/
	while(1){
		if( *srcStart != ' ')
			break;

		if( *srcStart == 0)
			return 0;

		srcStart++;
	}


	/*find src end, trim trailing spaces*/
	srcEnd = (char*)(src + len - 1);
	while(1){
		if( *srcEnd != ' ')
			break;

		if( *srcEnd == 0)
			return 0;

		srcEnd--;
	}

	srcEnd++;

	if( srcStart >= srcEnd )
		return 0;
	
	for(i=0; i<(maxNoOfCharsToCopy-1); i++){
		dest[i] = bgCharToLowerCase(*srcStart);
		srcStart++;
		if(srcStart >= srcEnd){
			i++;
			break;
		}
	}	
	
	dest[i] = 0;
	return i; 
}


int bgIsDigit(char c){
	return ((c>='0') && (c<='9')) ? 1 : 0;
}



#if 0
/*
Test for: bgStringListLinesEx()

The xxxxEx functions use externaly supplied buffer space to store result pointers to 
lines / words.
Based on the supplied buffer size, functions will calculate max. number of items (char ptr-s)
that may be stored within the given / allocated storage area.
See: pList->itemCountMax

To test it out, create a new cyclic task then use following code!
*/
#include <bur/plc.h>
#include <bur/plctypes.h>

#include "bglib/bgString.h"
#include "bglib/bgString.c"

#include "bglib/bgdebug.h"
#include "bglib/bgringbuffer.c"

_LOCAL bgStringListEx_typ listLines;
_LOCAL bgStringListEx_typ listWords;
_LOCAL char multiLineTextBuffer[256];
_LOCAL int lineCount;
_LOCAL int wordCount;
_LOCAL int test;
_LOCAL char listPtrBuf[128];    /*this is a "free" storage for list ptr-s! use it with caution!*/

_INIT void init()
{
	strcpy(multiLineTextBuffer, "hello world\r\nThis is a test for second line!\r\nThird line is here and there...\r\nLine #4 : right.left dot !\r\n");
	test = 0;
}

_CYCLIC void cyclic()
{
int i, j;

/*
This is just an example on how to use external / user supplied buffer in order to
store line / word lists. Based on the supplied buffer size, functions will calculate
max. number of items that may be stored within the given / allocated storage area.
See: pList->itemCountMax
*/
char* lineListStorage = &listPtrBuf[0];
char* wordListStorage = &listPtrBuf[64];


	if( test ){
		test = 0;
		
		lineCount = bgStringListLinesEx(&listLines, multiLineTextBuffer, lineListStorage, 64);
		DBG("multiLineTextBuffer has lineCount=%d lines. (same as listLines.itemCount= %d). Lines are:\r\n", lineCount, listLines.itemCount);		
		for(i=0; i<listLines.itemCount; i++){
			DBG("Line[%d]->%s\r\n", i, listLines.items[i]);
			wordCount = bgStringSplitLineEx(&listWords, ' ', listLines.items[i], wordListStorage, 64);
				DBG("     Line[%d] has wordCount=%d words. (same as listWords.itemCount= %d). Words are:\r\n", i, wordCount, listWords.itemCount)
			for(j = 0; j<listWords.itemCount; j++)
				DBG("     Word[%d]->%s\r\n", j, listWords.items[j]);
		}
	}	
}
#endif
int bgStringListLines(bgStringList_typ * p){
unsigned char *ptrItemStart, *ptrItemEnd;
int	calcItemCount;

/*
	length = strlen( p->src );
*/	
	p->itemCount = 0;
	
	ptrItemStart = p->src;
	ptrItemEnd = p->src;

	if( *ptrItemStart == 0){
		return 0;
	}

/*process the rest; build list of item pointers*/
	calcItemCount 	= 0;
	while( 1 ){
		if( (*ptrItemEnd == 0x0D) || (*ptrItemEnd == 0x0A)){
			*ptrItemEnd = 0;
			p->items[calcItemCount] = ptrItemStart;
			ptrItemEnd++;

			/*check for an additional \r or \n*/
			if( (*ptrItemEnd == 0x0D) || (*ptrItemEnd == 0x0A)){
				ptrItemEnd++;
			}			
			
			ptrItemStart = ptrItemEnd;
			calcItemCount++;

			if(calcItemCount >= BG_STRINGLIST_MAXITEMCOUNT){
				p->itemCount = calcItemCount;
				return -1;
			}
			
			/*check for an end of the src string*/
			if( (*ptrItemEnd == 0) ){
				p->itemCount = calcItemCount;		
				return calcItemCount;
			}	
		}else
		if( *ptrItemEnd == 0){
			p->items[calcItemCount] = ptrItemStart;
			calcItemCount++;		
			break;		
		}else
			ptrItemEnd++;
	}/*while*/
	
	p->itemCount = calcItemCount;		
	return calcItemCount;
}


void bgStringListExMemSet(bgStringListEx_typ *p, char* pAddrListBuffer, int sizeListBuffer)
{
	/*NOTE: we are going to use supplied char buffer as the storage for the array of char ptr-s!*/
    p->items = (unsigned char**)pAddrListBuffer;

	p->itemCountMax = sizeListBuffer / sizeof(char*);
}

int bgStringListLinesEx(bgStringListEx_typ * p, char* pContentBuffer, char* pAddrListBuffer, int sizeListBuffer){
unsigned char *ptrItemStart, *ptrItemEnd;
int	calcItemCount;

    p->src = (unsigned char*)pContentBuffer;

    /*if mem data is defined, use it; otherwise, mem shall have been set using a separate call to xxxExMemSet()*/
    if(pAddrListBuffer && sizeListBuffer)
        bgStringListExMemSet(p, pAddrListBuffer, sizeListBuffer);

	p->itemCount = 0;
	
	ptrItemStart = p->src;
	ptrItemEnd = p->src;

	if( *ptrItemStart == 0){
		return 0;
	}

/*process the rest; build list of item pointers*/
	calcItemCount 	= 0;
	while( 1 ){
		if( (*ptrItemEnd == 0x0D) || (*ptrItemEnd == 0x0A)){
			*ptrItemEnd = 0;
			p->items[calcItemCount] = ptrItemStart;
			ptrItemEnd++;

			/*check for an additional \r or \n*/
			if( (*ptrItemEnd == 0x0D) || (*ptrItemEnd == 0x0A)){
				ptrItemEnd++;
			}			
			
			ptrItemStart = ptrItemEnd;
			calcItemCount++;

			if(calcItemCount >= p->itemCountMax){
				p->itemCount = calcItemCount;
				return -1;
			}
			
			/*check for an end of the src string*/
			if( (*ptrItemEnd == 0) ){
				p->itemCount = calcItemCount;		
				return calcItemCount;
			}	
		}else
		if( *ptrItemEnd == 0){
			p->items[calcItemCount] = ptrItemStart;
			calcItemCount++;		
			break;		
		}else
			ptrItemEnd++;
	}/*while*/
	
	p->itemCount = calcItemCount;		
	return calcItemCount;
}





#if 0
Example:
(AS: Make a "C" task; add <filename>.c to the task; copy & paste this code)
#######
#include <bur/plc.h>
#include <bur/plctypes.h>

#include "bgString.h"
#include "bgString.c"

#include "bgdebug.h"

_LOCAL bgStringList_typ myList;
_LOCAL bgStringList_typ mySplitList;
_LOCAL char buffer[256];
_LOCAL int lineCount;
_LOCAL int test;

_INIT void init()
{
	strcpy(buffer, "hello world\r\nwhat is new?\r\nthird line\r\nline #4\r\n");
	test = 0;
}

_CYCLIC void cyclic()
{
int i, j;

	if( test ){
		test = 0;
		myList.src = buffer;
		lineCount = bgStringListLines(&myList);

		for(i=0; i<myList.itemCount; i++){
			DBG("Line[%d]->%s\r\n", i, myList.items[i]);

			mySplitList.src = myList.items[i];
			lineCount = bgStringSplitLine(&mySplitList, ' ');
			for(j = 0; j<mySplitList.itemCount; j++)
				DBG("     Word[%d]->%s\r\n", j, mySplitList.items[j]);
		}
	}	
}
######
The above code generates following output (debug telnet client):
>Line[0]->hello world
     Word[0]->hello
     Word[1]->world
Line[1]->what is new?
     Word[0]->what
     Word[1]->is
     Word[2]->new?
Line[2]->third line
     Word[0]->third
     Word[1]->line
Line[3]->line #4
     Word[0]->line
     Word[1]->#4


#endif
int bgStringSplitLine(bgStringList_typ * p, char separator){
unsigned char *ptrItemStart, *ptrItemEnd;
int	calcItemCount;

	p->itemCount = 0;
	
	ptrItemStart = p->src;
	ptrItemEnd = p->src;


/*process the rest; build list of item pointers*/
	calcItemCount 	= 0;
	while( 1 ){
		if( (*ptrItemEnd == separator) ){
			*ptrItemEnd = 0;
			p->items[calcItemCount] = ptrItemStart;
			ptrItemEnd++;

			calcItemCount++;

			/*check for an additional separtor */
			while( (*ptrItemEnd == separator) ){
				ptrItemEnd++;
				if( (*ptrItemEnd == 0) ){
					p->itemCount = calcItemCount;		
					return calcItemCount;
				}	
			}			
			
			ptrItemStart = ptrItemEnd;

			if(calcItemCount >= BG_STRINGLIST_MAXITEMCOUNT){
				p->itemCount = calcItemCount;
				return -1;
			}
			
			/*check for an end of the src string*/
			if( (*ptrItemEnd == 0) ){
				p->itemCount = calcItemCount;		
				return calcItemCount;
			}	
		}else
		if( *ptrItemEnd == 0){
			p->items[calcItemCount] = ptrItemStart;
			calcItemCount++;		
			break;		
		}else
			ptrItemEnd++;
	}/*while*/
	
	p->itemCount = calcItemCount;		
	return calcItemCount;
}



int bgStringSplitLineEx(bgStringListEx_typ * p, char separator, char* pLineBuffer, char* pAddrListBuffer, int sizeListBuffer){
unsigned char *ptrItemStart, *ptrItemEnd;
int	calcItemCount;

    
    p->src = (unsigned char*)pLineBuffer;

    /*if mem data is defined, use it; otherwise, mem shall have been set using a separate call to xxxExMemSet()*/
    if(pAddrListBuffer && sizeListBuffer)
        bgStringListExMemSet(p, pAddrListBuffer, sizeListBuffer);


	p->itemCount = 0;
	
	ptrItemStart = p->src;
	ptrItemEnd = p->src;


/*process the rest; build list of item pointers*/
	calcItemCount 	= 0;
	while( 1 ){
		if( (*ptrItemEnd == separator) ){
			*ptrItemEnd = 0;
			p->items[calcItemCount] = ptrItemStart;
			ptrItemEnd++;

			calcItemCount++;

			/*check for an additional separtor */
			while( (*ptrItemEnd == separator) ){
				ptrItemEnd++;
				if( (*ptrItemEnd == 0) ){
					p->itemCount = calcItemCount;		
					return calcItemCount;
				}	
			}			
			
			ptrItemStart = ptrItemEnd;

			if(calcItemCount >= p->itemCountMax){
				p->itemCount = calcItemCount;
				return -1;
			}
			
			/*check for an end of the src string*/
			if( (*ptrItemEnd == 0) ){
				p->itemCount = calcItemCount;		
				return calcItemCount;
			}	
		}else
		if( *ptrItemEnd == 0){
			p->items[calcItemCount] = ptrItemStart;
			calcItemCount++;		
			break;		
		}else
			ptrItemEnd++;
	}/*while*/
	
	p->itemCount = calcItemCount;		
	return calcItemCount;
}


char *bgStringLower(char *s)
{
int i;
char c;

	for(i=0;i<255;i++){

		c = s[i];

		if( !c )
			break;

		if( (c >= 'A') && (c <= 'Z') ){
			s[i] = c - 'A' + 'a';
		} 
	}

	return s;
}


char bgCharToLowerCase(char c)
{
char charLowerCase = c;

	if( (c >= 'A') && (c <= 'Z') ){
		charLowerCase = c - 'A' + 'a';
	} 

	return charLowerCase;
}

char *bgStringUpper(char *s)
{
int i;
char c;

	for(i=0;i<255;i++){

		c = s[i];

		if( !c )
			break;

		if( (c >= 'a') && (c <= 'z') ){
			s[i] = c - 'a' + 'A';
		} 
	}

	return s;
}



int bgStringIsMatch(char *s1, char *s2, int minChrCount)
{
	if( !strcmp(s1, s2) ){
		return 1;
	}else
		return 0;

}


int bgIsNumber_uint(char *s)
{
int i;
char c;

	/*limit execution to 255: don't want cycle time violation due to wrong ptr*/
	for(i=0;i<255;i++){

		c = s[i];

		/*end of valid input string?*/
		if( !c && i)
			return 1;

		/*zero input string?*/
		if( !c && !i)
			return 0;

		/*check if digit or space*/
		if( !bgIsDigit(c) && (c != ' '))
			return 0;
	}

	return 0;
}

int bgIsNumber_int(char *s)
{
int i;
char c;

	/*limit execution to 255: don't want cycle time violation due to wrong ptr*/
	for(i=0;i<255;i++){

		c = s[i];

		/*end of valid input string?*/
		if( !c && i)
			return 1;

		/*zero input string?*/
		if( !c && !i)
			return 0;

		/*check if digit or space*/
		if( !bgIsDigit(c) && (c != ' ') && (c != '-')  && (c != '+'))
			return 0;
	}

	return 0;
}


int bgAsciiToUint(char *src, unsigned int *dst)
{

unsigned int value;
	
	if( !bgIsNumber_uint(src) )
		return 0;

#ifdef WIN32
	/*TODO!!!*/
	value = 0;
#else
	value = atoi((UDINT)src);
#endif

	*dst = value;

	return 1;
}

int bgAsciiToInt(char *src, unsigned int *dst)
{

unsigned int value;
	
	if( !bgIsNumber_int(src) )
		return 0;

#ifdef WIN32
	/*TODO!!!*/
	value = 0;
#else
	value = atoi((UDINT)src);
#endif
	*dst = value;

	return 1;
}

int bgStrCopyFromTo(char* src, int srcStartPosition, int srcEndPosition, char* dest)
{
/*
-copy part of the memory from the section of src (from srcStartPosition to srcEndPosition) to dest
-srcEndPosition is not included in the copy (last copied byte position is srcEndPosition-1)
-length of the copied string is (srcEndPosition - srcStartPosition)
Example:
src="01234567890"
srcStartPosition=3
srcEndPosition=7

dest="3456"

return value: number of bytes copied (i.e. length of the dest string)
*/
int size = 0;
int i;

	if( srcEndPosition >= srcStartPosition )
		size = srcEndPosition - srcStartPosition;

	/*do memcopy! end position char is not included in the copy!*/
	for(i=0; i<size; i++)
		dest[i] = src[srcStartPosition + i];

	/*terminate string*/
	dest[size] = 0;

	return size;
}

int bgStrFindEOL(char* src, int startPosition)
{
/*
- search supplied c string "src" (zero terminated!!!) for the eof
- start search at startPosition and look for the first '\n'
- return:
-1 		- EOF (\n) not found
>=0		- EOF position
*/
int position = startPosition;
int i = 0;

	while(1){
		/*end of line?*/
		if( src[position] == '\n' ){
			/*if \r preceeds \n, dec EOL position by 1*/
			if( position && (src[position-1] == '\r') )
				position--;	
		 
		 	return position;
		}
 		
 		/*end of string?*/
		if( src[position] == 0 ) return -1;

		/*prevent endless loop*/
		if( ++i >= 4096 ) return -1;

		position++;
	}

	return -1;
}

#endif
/*EOF*/
