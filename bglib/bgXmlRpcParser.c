#ifndef _BG_XMLRPCPARSER_C
#define _BG_XMLRPCPARSER_C

#include "bgXmlRpcParser.h"
#include "bgString.h"

#ifdef WIN32
#include <stdio.h>
#include <string.h>
#define DBGMSG printf 
#else
#include "bgSscanf.h"
#endif

/*
##################################################################################
Xml Rpc Parser
##################################################################################
*/
static int	bgXmlRpcFindNextKeyword(char* pSrc, int* isKeywStartOrEnd, char** pBegKeyw, char** pEndKeyw, char* pKeywCopyBuf, int keywCopyBufSize, int* keywLineNo, int* keywColNo);
static int	bgXmlRpcCopyMemBlock(char* pDst, char* pStart, char* pEnd, int destSize, int* copyTrunc);
static int	bgXmlRpcFindKeywordId(char* pKeywStr);

static char* xmlRpcKeywDict[XMLRPC_ID_MAX]={
XMLRPC_STR_ARRAY,		
XMLRPC_STR_BASE64,		
XMLRPC_STR_BOOL,			
XMLRPC_STR_DATA,		   
XMLRPC_STR_TIME,			
XMLRPC_STR_DOUBLE,		
XMLRPC_STR_FAULT,		
XMLRPC_STR_I4,			
XMLRPC_STR_INT,			
XMLRPC_STR_METHOD_CALL,      
XMLRPC_STR_METHOD_NAME,      
XMLRPC_STR_METHOD_RSP,      
XMLRPC_STR_MEMBER,		
XMLRPC_STR_NAME,		
XMLRPC_STR_PARAM,		
XMLRPC_STR_PARAMS,		
XMLRPC_STR_STRUCT,		
XMLRPC_STR_STRING,		
XMLRPC_STR_VALUE,		
XMLRPC_STR_XMLHEADLINE,
};

char*	bgXmlRpcGetKeywordStr(int keywId)
{
	if(keywId >= XMLRPC_ID_MAX)
		return "not found";

	return xmlRpcKeywDict[keywId];
}


static int bgXmlRpcConvertScalarType(char* valueStr, int keywId)
{
int value;
float fvalue;

	if( (keywId == XMLRPC_ID_I4) ||
		(keywId == XMLRPC_ID_INT)){
		bgsscanf(valueStr, "%d", &value);
		return value;
	}

	if( (keywId == XMLRPC_ID_DOUBLE)){
		bgsscanf(valueStr, "%f", &fvalue);
/*		printf("Float value=%f\n", fvalue);
*/
		return (int)fvalue;
	}


	if( (keywId == XMLRPC_ID_BOOL)){
		char txtBuf[64];
		bgStrCpyNTrimSpcsToLwr(txtBuf, valueStr, 64);
		if( !strcmp(txtBuf, "false") || !strcmp(txtBuf, "0") ){
			value = 0;
		}else{
			value = 1;
		}
		return value;
	}

	return 0;
}

static int	bgXmlRpcFindKeywordId(char* pKeywStr)
{
/*
Return:
>=0 - match id
-1  - match not found
-2  - more then one match
*/
int i;
int j;

int matchList[XMLRPC_ID_MAX];

char* pKeywFromList;
char* pThis = pKeywStr;

int matchId;
    
    for(i=0;i<XMLRPC_ID_MAX;i++){
        matchList[i] = i;        
    }

    j = 0;
    while(1){
        for(i=0;i<XMLRPC_ID_MAX;i++){
            if( matchList[i] >= 0 ){            
                pKeywFromList = xmlRpcKeywDict[i];

				if( (*(pThis + j)) == (*(pKeywFromList + j)) )
					matchList[i] = i;
				else
					matchList[i] = -1;                
			}
        } 

		if( ! (*(pThis + j)) )
			break;
		
		j++;
    }

    matchId = -1;    
    for(i=0;i<XMLRPC_ID_MAX;i++){
        if( matchList[i] >= 0){
            if( matchId < 0)
                matchId = i;
            else
                return -2;
        }            
    }

    return matchId;
}


static int	bgXmlRpcCopyMemBlock(char* pDst, char* pStart, char* pEnd, int destSize, int* copyTruncated)
{
int size = pEnd - pStart;

	*copyTruncated = 0;
    
	if( !pStart || !pEnd || !pDst)
        return 0;

    if( size <= 0 )
        return  0;

	if( size >= destSize ){
        size = destSize;
		*copyTruncated = 1;
	}

    memcpy(pDst, pStart, size);
    
	pDst[size] = 0;

    return size;
}


static int	bgXmlRpcFindNextKeyword(char* pSrc, int* isKeywStartOrEnd, char** pBegKeyw, char** pEndKeyw, char* pKeywCopyBuf, int keywCopyBufSize,  int* keywLineNo, int* keywColNo)
{
/*search starting position*/
char* p = pSrc;
char* dst = 0;
int len;
static int line = 0;
static int col = 0;

    /*is keyw copy buffer defined?*/
    if(pKeywCopyBuf){
        dst = pKeywCopyBuf; 
        *dst = 0;
        len = 0;
    }

	/*initial call*/
	if(!(*pBegKeyw) || !(*pEndKeyw)){
		line = 1;
		col = 1;
	}

    *isKeywStartOrEnd = 0;
    
    /*find start marker*/
    while(1){
        if( *p == '<' )
            break;

        if( *p == 0 )
            return 1;
        
		/*increment line & col count*/
		if( *p == '\n'){
			line++;
			col = 1;
		}else{
			col++;
		}

        p++;
    }  

    /*found start marker ('<')! it marks the begin of the xml keyword!*/
	*keywLineNo = line;
	*keywColNo = col;

    /*output parametar: start marker ptr*/
    if(pBegKeyw)
        *pBegKeyw = p;

    /*next: advance to the next char*/
    p++;
    
    /*look for end marker, copy src to dst, look for '/' and don't copy spaces*/
    while(1){
        if( *p == '>' )
            break;

        if( *p == 0 )
            return 1;

        
        if( *p == '/' ){            /*if end marker, mark a flag*/
            *isKeywStartOrEnd = 1;
        }else 
        if( *p == ' ' ){            /*don't copy spaces*/

        }
        else{                       /*copy keyw into supplied buffer*/
            if( dst && (len < (keywCopyBufSize - 1))){              
                *dst = *p;                  
    			dst++;
                len++;
            }
        }

		/*increment line & col count*/
		if( *p == '\n'){
			line++;
			col = 1;
		}else{
			col++;
		}

        p++;
    }  

    /*output parametar: end marker ptr*/
    if(pEndKeyw)
        *pEndKeyw = p;

    /*terminate dst buffer that contains a keyword!*/
    if( dst ){
        *dst = 0;
        
        /*convert copy of the keyw into lowercase*/
        bgStringLower(dst);
    }

#if 0
    /*find what keyword is this*/
    DBGMSG("keyword: %s, %d, pBeg=%p, pEnd=%p\n", pKeywCopyBuf?pKeywCopyBuf:"n/a", *isKeywStartOrEnd, *pBegKeyw, *pEndKeyw);
#endif    
    
	return 0;
}


void bgXmlRpcParserInit(bgXmlRpcParser_typ* p, char *pInputStream, char* pTreeMem, int sizeTreeMem, char* pStringMem, int sizeStringMem)
{
    p->initOk = 0;
    
    if( !p || !pInputStream || !pTreeMem || !pStringMem)
        return;

    if( (sizeTreeMem < 255) || (sizeStringMem < 255) )
        return;

    p->pInputStream = pInputStream;
    p->pXmlTreeMem = pTreeMem;
    p->sizeXmlTreeMem = sizeTreeMem;
    p->pStrHeapMem = pStringMem;
    p->sizeStrHeapMem = sizeStringMem;

    p->initOk = 1;

}

int bgXmlRpcParserSerialize(bgXmlRpcParser_typ* p)
{

	return 0;
} 

static int isItemOnTopOfStack(bgStack_typ* s, int keywId)
{
	if( !bgStackGetItemCount(s) || ( (int)bgStackPeek(s) != keywId) )		
		return 0;
	else
		return 1;
} 

static void setErrorInfo(bgXmlRpcParser_typ* p, int error, int keywId)
{
	p->error = error;
	p->keywId = keywId;
} 

void bgXmlRpcParserParse(bgXmlRpcParser_typ* p)
{
int keywIdStack = 0;
int isBlockEnd = 0;
int isBlockBeg = 0;
char* pSearchStart = p->pInputStream;
char* pKeywStart = 0;
char* pKeywEnd = 0;
char* pBegKeywContent = 0;
bgTree4C_typ* pXmlTree = &p->xmlTree;
bgStack_typ* s = &p->stack;
bgStringHeap_typ* pStrHeap = &p->strHeap;
bgTree4CNode_typ* pXmlNodeCurrent = 0;
bgTree4CNode_typ* pXmlNode = 0;
char keywText[32];


    if(!p->initOk)
        return;

	setErrorInfo(p, 0,0); /*no errors*/
	p->keywLineNo = 0;
	p->keywColNo = 0;    
	keywText[0] = 0;

    /*init xml tree*/
	bgTree4CInit(pXmlTree, p->pXmlTreeMem, p->sizeXmlTreeMem);
	
	/*create root node*/
	if( !(pXmlNodeCurrent = bgTree4CNodeCreate(pXmlTree)) ){
		setErrorInfo(p, XMLRPC_ERROR_NODE_CREATE, 0);		
		return;
	}

    /*init stack*/
	bgStackInit(s);

    /*init string heap*/
	bgStringHeapInit(pStrHeap, p->pStrHeapMem, p->sizeStrHeapMem);

    while( !bgXmlRpcFindNextKeyword(pSearchStart, &isBlockEnd, &pKeywStart, &pKeywEnd, keywText, 32, &p->keywLineNo, &p->keywColNo) ){
		char txtBuf[128];
		int cpTrunc;
		int cpLen;
        int keywId;
		char* pStr;

		/*extract/copy keyword text into a private buffer*/
		cpLen = bgXmlRpcCopyMemBlock(txtBuf, (char*)(pKeywStart + 1), pKeywEnd, 128, &cpTrunc);

		/*find keyword id*/
        keywId = bgXmlRpcFindKeywordId(keywText);

		/*begining keyword or ending keyword?*/
		isBlockBeg = isBlockEnd ? 0 : 1;

		/*skip following keywords, not used, not checked*/
        if( (keywId == XMLRPC_ID_PARAMS) ||
			(keywId == XMLRPC_ID_PARAM) ||
			(keywId == XMLRPC_ID_XMLHEADLINE) ){
/*			
            DBGMSG("found keyword [%s], isBlockBeg=%d\n", xmlRpcKeywDict[keywId], isBlockBeg);
*/            
			pSearchStart = pKeywEnd;
			continue;
        }
		
/*
		DBGMSG("Current NodeId=%d\n", pXmlNodeCurrent ? pXmlNodeCurrent->id : -1);
*/       

		if( bgTree4CGetCountFreeNodes(pXmlTree) <= 2 ){
			setErrorInfo(p, XMLRPC_ERROR_NODE_CREATE, keywId);
			return;
		}

		if(isBlockBeg){

			pBegKeywContent = pKeywEnd + 1;	

            switch( keywId ){
                case XMLRPC_ID_METHOD_CALL:
                case XMLRPC_ID_METHOD_NAME:
                case XMLRPC_ID_METHOD_RSP:
                case XMLRPC_ID_VALUE:
					/*create new node*/
					pXmlNode = bgTree4CNodeCreate(pXmlTree);
					/*set node data*/
					bgTree4CNodeSetDataType( pXmlNode, keywId);
					/*connect node to "down" at the bottom end from the current node*/
					bgTree4CNodeAppendToDwEnd(pXmlNodeCurrent, pXmlNode);
					/*set newly created node as the new current node*/
					pXmlNodeCurrent = pXmlNode;
                break;
				case XMLRPC_ID_ARRAY:
                case XMLRPC_ID_STRUCT:
					/*struct, array must be inside the value block*/
					if( !isItemOnTopOfStack(s, XMLRPC_ID_VALUE) ){
						setErrorInfo(p, XMLRPC_ERROR_INVALID_XML_BLOCK1, keywId);
                        return;					
					}
					/*create new node*/
					pXmlNode = bgTree4CNodeCreate(pXmlTree);
					/*set node data type*/
					bgTree4CNodeSetDataType(pXmlNode, keywId);
					/*check if the current node has higher level*/
					if(!pXmlNodeCurrent->rt ){
						/*higher level=no; insert new node to the rt, create new level*/
						bgTree4CNodeInsertRt(pXmlNodeCurrent, pXmlNode);
					}else{
						/*higher level=yes; insert new node down, on the same level*/
						bgTree4CNodeAppendToDwEnd(pXmlNodeCurrent, pXmlNode);					
					}
					/*set new node as the new current node*/
					pXmlNodeCurrent = pXmlNode;                
				break;
                case XMLRPC_ID_MEMBER:
					/*member must be inside struct*/
					if( !isItemOnTopOfStack(s, XMLRPC_ID_STRUCT) ){
						setErrorInfo(p, XMLRPC_ERROR_INVALID_XML_BLOCK2, keywId);
                        return;					
					}

					/*create new node*/
					pXmlNode = bgTree4CNodeCreate(pXmlTree);
					/*set node data type*/
					bgTree4CNodeSetDataType(pXmlNode, keywId);
					if(pXmlNodeCurrent->dataType == XMLRPC_ID_STRUCT){
						/*insert new node to the rt, create new level*/
						bgTree4CNodeInsertRt(pXmlNodeCurrent, pXmlNode);
					}else{
						/*insert new node to dw, same level*/
						bgTree4CNodeAppendToDwEnd(pXmlNodeCurrent, pXmlNode);					
					}
					/*set new node as the new current node*/
					pXmlNodeCurrent = pXmlNode;                
                break;
                case XMLRPC_ID_NAME:
					/*name must be inside member*/
					if( !isItemOnTopOfStack(s, XMLRPC_ID_MEMBER) ){
						setErrorInfo(p, XMLRPC_ERROR_INVALID_XML_BLOCK3, keywId);
                        return;					
					}

					/*create new node*/
					pXmlNode = bgTree4CNodeCreate(pXmlTree);
					/*set node data*/
					bgTree4CNodeSetDataType(pXmlNode, keywId);					
					/*insert new node to the rt, create new level*/
					bgTree4CNodeInsertRt(pXmlNodeCurrent, pXmlNode);
					/*set new node as the new current node*/
					pXmlNodeCurrent = pXmlNode;                
                break;
				case XMLRPC_ID_I4:
				case XMLRPC_ID_DOUBLE:
                case XMLRPC_ID_INT:
                case XMLRPC_ID_STRING:
                case XMLRPC_ID_BOOL:
					/*scalar types must be inside  value*/
					if( !isItemOnTopOfStack(s, XMLRPC_ID_VALUE) ){
						setErrorInfo(p, XMLRPC_ERROR_INVALID_XML_BLOCK4, keywId);
                        return;					
					}
					/*create new node*/
					pXmlNode = bgTree4CNodeCreate(pXmlTree);
					/*set node data*/
					bgTree4CNodeSetDataType(pXmlNode, keywId);
					/*highest level nodes > always to the end of the current node*/
					bgTree4CNodeAppendToRtEnd(pXmlNodeCurrent, pXmlNode);
				break;
                default:
                break;                                
			}

            bgStackPush(s, (char*)(keywId)); 
/*
            DBGMSG("push= %s\n", xmlRpcKeywDict[keywId]);                                            
*/            
        
		}else{

            keywIdStack = (int)bgStackPop(s);
/*
            DBGMSG("pop= %s\n", xmlRpcKeywDict[keywId]);
*/		
/*			
			DBGMSG("---------------------------------------------------------------\n");
			bgTree4CPrint(pXmlTree);
			DBGMSG("---------------------------------------------------------------\n");
*/

            if((keywId != XMLRPC_ID_VALUE) && (keywIdStack != keywId)){
				setErrorInfo(p, XMLRPC_ERROR_INVALID_XML_BLOCK6, keywId);
                return;					
            }

			switch( keywId ){
                case XMLRPC_ID_METHOD_RSP:
				break;
                case XMLRPC_ID_VALUE:
					/*back to the previous level*/
					pXmlNode = bgTree4CNodePreviousLevel(pXmlNodeCurrent);
					if(pXmlNode)
						pXmlNodeCurrent = pXmlNode;
				break;
				case XMLRPC_ID_ARRAY:
                case XMLRPC_ID_STRUCT:
					/*find next lower level with nodeType = _VALUE*/
					while(1){
						pXmlNode = bgTree4CNodePreviousLevel(pXmlNodeCurrent);

						if(pXmlNode){
							pXmlNodeCurrent = pXmlNode;
						}else{
							setErrorInfo(p, XMLRPC_ERROR_INVALID_XML_BLOCK6, keywId);
	                        return;					
						}
						
						if( pXmlNodeCurrent->dataType == XMLRPC_ID_VALUE )
							break;

					}
				break;
                case XMLRPC_ID_METHOD_NAME:
					/*extract/copy keyword text into a private buffer*/
					cpLen = bgXmlRpcCopyMemBlock(txtBuf, pBegKeywContent, pKeywStart, 128, &cpTrunc);
					pStr = bgStringHeapAddString(pStrHeap, txtBuf);
					bgTree4CNodeSetDataPtr(pXmlNodeCurrent, pStr);
				break;
                case XMLRPC_ID_NAME:
                case XMLRPC_ID_STRING:
                case XMLRPC_ID_TIME:
					/*extract/copy keyword text into a private buffer*/
					cpLen = bgXmlRpcCopyMemBlock(txtBuf, pBegKeywContent, pKeywStart, 128, &cpTrunc);
					/*above types are never "current nodes", they are on the right from the current node (i.e. value)*/
					pXmlNode = pXmlNodeCurrent->rt ? pXmlNodeCurrent->rt : pXmlNodeCurrent; 
					pStr = bgStringHeapAddString(pStrHeap, txtBuf);
					bgTree4CNodeSetDataPtr(pXmlNode, pStr);
				break;
                case XMLRPC_ID_I4:
                case XMLRPC_ID_INT:
                case XMLRPC_ID_BOOL:
                case XMLRPC_ID_DOUBLE:
					/*extract/copy keyword text into a private buffer*/
					cpLen = bgXmlRpcCopyMemBlock(txtBuf, pBegKeywContent, pKeywStart, 128, &cpTrunc);
					/*above types are never "current nodes", they are on the right from the current node (i.e. value)*/
					pXmlNode = pXmlNodeCurrent->rt ? pXmlNodeCurrent->rt : pXmlNodeCurrent; 
					pStr = bgStringHeapAddString(pStrHeap, txtBuf);
					bgTree4CNodeSetDataPtr(pXmlNode, pStr);

					/*for scalar types, also convert value and store it on the node*/
					bgTree4CNodeSetDataValue(pXmlNode, bgXmlRpcConvertScalarType(txtBuf, keywId));
				break;
                default:
                break;                    
            }


        }

/*
		DBGMSG("---------------------------------------------------------------\n");
		bgTree4CPrint(pXmlTree);
		DBGMSG("---------------------------------------------------------------\n");
*/
		pSearchStart = pKeywEnd;
    }
	return;
} 


#endif
