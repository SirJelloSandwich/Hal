#include <bur/plc.h>
#include <bur/plctypes.h>

#include "bglib/bgdebug.h"
#include "bglib/bgstring.h"
#include "bglib/bgSecMD5Signature.h"
#include "bgfsig.h"

/*define a limit for number of lines within a file*/
#define MAX_NUMBER_OF_LINES	3000

static int bgGetLineFromTextBuffer(char* pText, char** pLineNext, char* pDest, int maxLen, int* charCount);

#define  secretWord "NONE!"

/*=============================================================*/
int bgFileSignatureVerify(char *inFileBuffer, char *resultStringMinLen64)
{
int i, index;
char lineBuffer[255];
char lineBufferRaw[255];
int lineCount;
int charCount;
bgStringList_typ splitLine;
md5_state_t state;
md5_byte_t digest[16];
char hexDigestCalculated[40];
char hexDigestFromFile[40];

char *pLineCurrent;
char *pLineNext;


	/*init digest*/
	md5_init(&state);

	/*init hexDigestFromFile as empty*/
	hexDigestFromFile[0] = 0;


	/*init line ptr-s*/
	pLineCurrent = inFileBuffer;
	pLineNext = 0L;
	
	lineCount = 0;
	
	/*process all lines from the file*/	
	while( 1 ){
	int isNextLinePresent;

		isNextLinePresent = bgGetLineFromTextBuffer(pLineCurrent, &pLineNext, lineBufferRaw, 255, &charCount);

		if( lineCount++ > MAX_NUMBER_OF_LINES){
			DBGCRT("Supplied file buffer has more then %d lines!\r\n", lineCount);
			if( resultStringMinLen64 )
				strcpy(resultStringMinLen64, "Err: file is too long!");
			return -2;			
		}

		
		/*copy a trimmed line into the local buffer*/		
		charCount = bgStrCpyNTrimLeadingSpaces(lineBuffer, lineBufferRaw, 255);

		/*check for empty lines & comments*/
		if( !charCount || (lineBuffer[0] == '*')){
			/*empty lines or comments -> skip and go to the next line*/
		}else{
			/*check if this is a signature line*/
			index = bgStrFind(lineBuffer, "SIGNATURE", -1, 0);
			if( index >= 0){
				/*line w. signature -> extract signature stored in the file*/
				splitLine.src = lineBuffer;
				bgStringSplitLine(&splitLine, '=');
				if( splitLine.itemCount != 2){
					DBGCRT("Can't extract signature from the supplied file buffer\r\n");
					if( resultStringMinLen64 )
						strcpy(resultStringMinLen64, "Err: can't extract signature");
					return -2;			
				}
				/*copy extracted signature from the file into the local buffer*/
				bgStrCpyN(hexDigestFromFile, splitLine.items[1], 40);
				bgStringUpper(hexDigestFromFile);
				
			}else{
				/*line wo. signature -> use (append) for digest calculation*/
				md5_append(&state, (const md5_byte_t *)lineBuffer, strlen(lineBuffer));
			}
		}

		/*process next line*/
		if( isNextLinePresent )
			pLineCurrent = pLineNext;
		else
			break;
	}

	/*append secret content*/
	md5_append(&state, (const md5_byte_t *)secretWord, strlen(secretWord));

	/*generate digest*/
	md5_finish(&state, digest);

	/*convert calculated digest into a hex string*/
	for (i = 0; i < 16; ++i)
	    bgsprintf(hexDigestCalculated + i * 2, "%02X", digest[i]);


	/*check if supplied file buffer had stored digital signature*/
	if( !hexDigestFromFile[0] ){
		/*digital signature was NOT present in this file->return result and current (calculated) file digest*/
		if( resultStringMinLen64 )
			bgsprintf(resultStringMinLen64, "StoredSig=NONE CalcSig=%s\n\r", hexDigestCalculated);
		return 0;			

	}else{
		/*digital signature IS present in this file->compare calculated and stored signaure*/

		for (i = 0; i < 32; i++){
			if(hexDigestCalculated[i] != hexDigestFromFile[i] ){
				/*digital signatures are not equal -> exit*/
				if( resultStringMinLen64 )
					bgsprintf(resultStringMinLen64, "StoredSig=%s CalcSig=%s\n\r", hexDigestFromFile, hexDigestCalculated);

				return 2;			
			}
		}

		/*digital signature verified! stored and calculated signatures are equal!*/
		if( resultStringMinLen64 )
			bgsprintf(resultStringMinLen64, "StoredSigEQUALCalcSig=%s\n\r", hexDigestCalculated);

		return 1;			

	}
}
/*=============================================================*/

/*
Input(s):
char *pText - ptr to the text buffer from which to extract a line
char *pDest - destination buffer where to copy the content of a line
int maxLen - max length of the destination buffer

Output(s):
char **pLineNext - ptr to the next line
int *charCount - total number of chars copied into the dest buffer

Return:
0 - there is no more lines in the supplied text buffer
1 - more lines are present in the supplied text buffer
*/
static int bgGetLineFromTextBuffer(char* pText, char** pLineNext, char* pDest, int maxLen, int* charCount)
{

unsigned char *ptrItemStart, *ptrItemEnd;
unsigned char *ptrCpyDest;

	/*init ptrs to the start of the text buffer*/
	ptrItemStart = pText;
	ptrItemEnd = pText;
	ptrCpyDest = pDest;

	/*basic check*/
	if( *ptrItemStart == 0){
		return 0;
	}

	*charCount = 0;
	while( 1 ){
		if( (*ptrItemEnd == 0x0D) || (*ptrItemEnd == 0x0A)){
			/*EOF line char found! -> exit*/			
			ptrItemEnd++;

			/*check for an additional \r or \n*/
			if( (*ptrItemEnd == 0x0D) || (*ptrItemEnd == 0x0A)){
				ptrItemEnd++;
			}			
			
			ptrItemStart = ptrItemEnd;

			/*terminate dest buffer*/
			*ptrCpyDest = 0;
			
			/*check for an end of the text*/
			if( (*ptrItemEnd == 0) ){
				*pLineNext = 0L;
				/*0 -> there are NO more lines in the text buffer*/
				return 0;
			}else{
				*pLineNext = ptrItemStart;
				/*1 -> there are more lines in the text buffer*/
				return 1;
			}	
		}else
		if( *ptrItemEnd == 0){
			/*end of text buffer found! -> exit*/			

			/*terminate dest buffer*/
			*ptrCpyDest = 0;

			*pLineNext = 0L;
			/*0 -> there are NO more lines in the text buffer*/
			return 0;
		}else{
			/*copy line content into the supplied dest buffer
			check for buffer overflow*/

			if( (*charCount) < maxLen ){
				*ptrCpyDest = *ptrItemEnd;
				ptrCpyDest++;
				(*charCount)++;
			}else{
			
				DBGCRT("Line is longer then the max allowed size!\r\n");

				/*terminate dest buffer*/
				*ptrCpyDest = 0;

				*pLineNext = 0L;
				/*0 -> there are NO more lines in the text buffer! Error*/
				return 0;
			}
			
			ptrItemEnd++;
		}	
	}/*while*/	
}
