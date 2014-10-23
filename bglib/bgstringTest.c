#include <bur/plc.h>
#include <bur/plctypes.h>

#include "bgString.h"
#include "bgString.c"

#include "bgdebug.h"

_LOCAL bgStringList_typ myList;
_LOCAL bgStringList_typ mySplitList;
_LOCAL char buffer[256];
_LOCAL int lineCount;
_LOCAL int testListLines;
_LOCAL int testUpper;
_LOCAL int testLower;

_INIT void init()
{
	strcpy(buffer, "hello world\r\nwhat is new?\r\nthird line\r\nline #4\r\n");
	testListLines = 0;
	testUpper = 0;
	testLower = 0;
}

_CYCLIC void cyclic()
{
int i, j;


	if( testListLines ){
		testListLines = 0;
		myList.src = buffer;
		lineCount = bgStringListLines(&myList);

		for(i=0; i<myList.itemCount; i++){
			DBG("Line[%d]->%s\r\n", i, myList.items[i]);

			mySplitList.src = myList.items[i];
			lineCount = bgStringSplitLine(&mySplitList, ' ');
			for(j = 0; j<mySplitList.itemCount; j++)
				DBG("     Word[%d]->'%s'\r\n", j, mySplitList.items[j]);
		}
	}	
	
	
	if( testLower ){
		testLower = 0;
		DBG("Input String        : '%s'\r\n", buffer);
		bgStringLower( buffer );
		DBG("Input String (Lower): '%s'\r\n", buffer);
	}	
	
	
	if( testUpper ){
		testUpper = 0;
		DBG("Input String        : '%s'\r\n", buffer);
		bgStringUpper( buffer );
		DBG("Input String (Upper): '%s'\r\n", buffer);
	}	
	
}
