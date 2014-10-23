/**\file: bgXmlRpcParser.h
*Xml Rpc parser 
*/
#ifndef _BG_XMLRPCPARSER_H
#define _BG_XMLRPCPARSER_H

#include "bgBufMemAlloc.h"
#include "bgDataStructures.h"

/**\cond 0*/
#define XMLRPC_STR_ARRAY				"array"
#define XMLRPC_STR_BASE64				"base64"
#define XMLRPC_STR_BOOL					"boolean"
#define XMLRPC_STR_DATA		    		"data"
#define XMLRPC_STR_TIME			    	"datetime.iso8601"
#define XMLRPC_STR_DOUBLE				"double"
#define XMLRPC_STR_FAULT				"fault"
#define XMLRPC_STR_I4					"i4"
#define XMLRPC_STR_INT					"int"
#define XMLRPC_STR_METHOD_CALL       	"methodCall"
#define XMLRPC_STR_METHOD_NAME       	"methodName"
#define XMLRPC_STR_METHOD_RSP       	"methodResponse"
#define XMLRPC_STR_MEMBER				"member"
#define XMLRPC_STR_NAME					"name"
#define XMLRPC_STR_PARAM				"param"
#define XMLRPC_STR_PARAMS				"params"
#define XMLRPC_STR_STRUCT				"struct"
#define XMLRPC_STR_STRING				"string"
#define XMLRPC_STR_VALUE				"value"
#define XMLRPC_STR_XMLHEADLINE			"?xml version=\"1.0\"?"
#define XMLRPC_STR_MAX			        "EndOfListMarker!"

#define XMLRPC_ID_ARRAY				    0
#define XMLRPC_ID_BASE64				1
#define XMLRPC_ID_BOOL					2
#define XMLRPC_ID_DATA				    3
#define XMLRPC_ID_TIME				    4
#define XMLRPC_ID_DOUBLE				5
#define XMLRPC_ID_FAULT				    6
#define XMLRPC_ID_I4					7
#define XMLRPC_ID_INT					8
#define XMLRPC_ID_METHOD_CALL        	9
#define XMLRPC_ID_METHOD_NAME        	10
#define XMLRPC_ID_METHOD_RSP        	11
#define XMLRPC_ID_MEMBER				12
#define XMLRPC_ID_NAME					13
#define XMLRPC_ID_PARAM				    14
#define XMLRPC_ID_PARAMS				15
#define XMLRPC_ID_STRUCT				16
#define XMLRPC_ID_STRING				17
#define XMLRPC_ID_VALUE				    18
#define XMLRPC_ID_XMLHEADLINE			19
#define XMLRPC_ID_MAX			        20


#define XMLRPC_ERROR_INVALID_XML_BLOCK1	1
#define XMLRPC_ERROR_INVALID_XML_BLOCK2	2
#define XMLRPC_ERROR_INVALID_XML_BLOCK3	3
#define XMLRPC_ERROR_INVALID_XML_BLOCK4	4
#define XMLRPC_ERROR_INVALID_XML_BLOCK5	5
#define XMLRPC_ERROR_INVALID_XML_BLOCK6	6
#define XMLRPC_ERROR_INVALID_XML_STRUCT	10
#define XMLRPC_ERROR_NODE_CREATE		11


/**\endcond*/





/**
\struct bgXmlRpcParser_typ 
Data object that holds information for processing / parsing Xml Rpc stream.
*/
typedef struct bgXmlRpcParser_typ{
    int initOk;

    
    char* pInputStream;

    bgTree4C_typ xmlTree;
    char* pXmlTreeMem;
    int sizeXmlTreeMem;
    
    bgStack_typ  stack;

    int state;
	int keywLineNo;
	int keywColNo;
	int error;
	int keywId;	

	bgStringHeap_typ strHeap;
    char* pStrHeapMem;
    int sizeStrHeapMem;
}bgXmlRpcParser_typ;



void bgXmlRpcParserInit(bgXmlRpcParser_typ* p, char *pInputStream, char* pTreeMem, int sizeTreeMem, char* pStringMem, int sizeStringMem);
void bgXmlRpcParserParse(bgXmlRpcParser_typ* p);
char* bgXmlRpcGetKeywordStr(int keywId);



#endif
