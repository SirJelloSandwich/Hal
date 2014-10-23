/**\file: bgString.h
*Useful string manipulation functions.
*/
#ifndef _BG_STRING_H
#define _BG_STRING_H

#ifndef WIN32
/*B&R string functions*/
//#include <string.h>
//#include <asstring.h>
#endif

/**
Find if string (*sub) is present within the string (*s). 
Start search at (start) index, don't search beyond (end) index of input string (*s)
\param s prt to source string
\param sub prt to sub string
\param start location (s[start]) to start the search (-1:use default(start at 0) | 0..strlen(s)-1 )
\param end end search at location s[end] (-1=use default(EndOfString) OR 0..strlen(s)-1 )
\return -1 substring not found
\return >=0 index of (s) where the (sub) starts
*/
int bgStrFind(char *s, char *sub, int start, int end);

/**
Similar to bgStrFind, except for and extra option.
\param option (0-same as bgStrFind; 1-use case insensitive string compare)
\return -1 substring not found
\return >=0 index of (s) where the (sub) starts
*/
int bgStrFindEx(char *s, char *sub, int start, int end, int option);


/**
Copy up to (maxNoOfCharsToCopy) chars from (*src) string to (*dest) string.
If (*src) string is shorter then (maxNoOfCharsToCopy-1) return value will be 
equal to the length of the src string.
*\return Number of bytes actually copied 
*/
int bgStrCpyN(char *dest, char *src, int maxNoOfCharsToCopy);

/**
Copy up to (maxNoOfCharsToCopy) chars from (*src) string to (*dest) string and trim leading spaces when coping
If (*src) string is shorter then (maxNoOfCharsToCopy-1) return value will be 
equal to the length of the src string.
*\return Number of bytes actually copied 
*/
int bgStrCpyNTrimLeadingSpaces(char *dest, char *src, int maxNoOfCharsToCopy);


/**
Terminate (by writing 0) supplied string at CR or LF which ever is found first.
If oldStrLen (before calling this func) equals to the return value of this 
function it means that string termination (removal of LF or CR) didn't take 
place.
\return New string length 
*/
int bgStrTrimCRLF(char *dest);

	
/**
Check if input char (c) is a digit ('0','1', to '9')
\return 0 - no
\return 1 - yes
*/
int bgIsDigit(char c);

/**
Check if input string (s) is a unsigned integer number.
Or, in other words, if all chars in the input string are digits.
\return 0 - no
\return 1 - yes
*/
int bgIsNumber_uint(char *s);


/**
Check if string is a integer (+/-) number (i.e all chars in strings are digits)
\return 0 - no
\return 1 - yes
*/
int bgIsNumber_int(char *s);



/**
Convert input string (s) to unsigned int (dst)
\return 0 - failed (input not a valid string, *dst not changed)
\return 1 - ok (*dst set to new value)
*/
int bgAsciiToUint(char *src, unsigned int *dst);


/**
Convert input string to int value
\return 0 - failed (input not a valid string, *dst not changed)
\return 1 - ok (*dst set to new value)
*/
int bgAsciiToInt(char *src, unsigned int *dst);



/**
\def BG_STRINGLIST_MAXITEMCOUNT 
Defines maximum number of items (pointers) within the list of strings
*/
#define BG_STRINGLIST_MAXITEMCOUNT	512

/**
\struct bgStringList_typ 
Data object that holds list of strings.\n
Typical usage is following: input/source stream (*src), that contains lines 
separated by CRLF or LF, is scanned and all line ends are terminated with 0. 
Pointers to the start of each line are entered into the (items) array of ptrs.\n
<b>NOTE:</b> All ptrs in (items[]) point into the space within the (*src) buffer!\n
<b>IMPORTANT:</b> Generating list of line ptrs is a destructive process as the 
(*src) buffer gets modified (0 are writen at the place of each LF or CR).
*/
typedef struct bgStringList_typ
{
/*in*/
	unsigned char	*src;			/**<Input: ptr to the source stream/input buffer */
/*out*/
	int				itemCount;		/**<Output: number of items in the list*/
	unsigned char 	*items[BG_STRINGLIST_MAXITEMCOUNT]; /**<Output: list of pointers to strings*/
}bgStringList_typ;



/**
\struct bgStringListEx_typ 
Data object that holds list of strings. In this case, list of ptrs (items) is supplied externally (allocate memory)\n
Typical usage: same as \link bgStringList_typ \endlink\n
<b>IMPORTANT:</b> Before using this data object, the list of items (items) must be initialized 
(i.e. allocated or external memory must be assigned to (items) ptr!)\n
Use method \link bgStringListExMemSet \endlink to set the ptrs array memory.
*/
typedef struct bgStringListEx_typ
{
/*in*/
	unsigned char	*src;		/**<Input: ptr to the source stream/input buffer */		
	int				itemCountMax;	/**<Input: Maximum number of items (depends on the size of externaly supplied memory*/
/*out*/
	int				itemCount;	/**<Output: number of items in the list*/
	unsigned char 	**items;	/**<Output: list of pointers to strings (memory is user supplied*/
}bgStringListEx_typ;



/**
Method for data object \link bgStringList_typ \endlink\n
\param p Ptr to data object: (\link bgStringList_typ \endlink)
Example: Given user supplied input stream (*src), i.e.:
\code
"hello world\r\nwhat is new?\r\nthird line\r\nline #4\r\n"\n
\endcode
Method generates following: (p-> points to \link bgStringList_typ \endlink data object):
\code
p->itemCount = 4; <*src> string has 4 lines
p->items[0] = ptr to 1st line ("hello world"); 
p->items[1] = ptr to 2nd line ("what is new?"); 
p->items[2] = ptr to 3rd line ("third line"); 
p->items[3] = ptr to 4th line ("line #4"); 
\endcode
\n
Method is used when processing ascii based client/server protocols (i.e. FTP, Telnet...)\n
<b>IMPORTANT:</b>Method write 0-s (string termination) into the source stream (*src) at the position of
each line end!\n 
(i.e. It overwrites all \r or \n or \r\n with 0)!!!)
*/
int bgStringListLines(bgStringList_typ *p); 


/**
Method for data object \link bgStringListEx_typ \endlink\n
\param Ptr to data object: (\link bgStringList_typ \endlink)
\param pAddrListBuffer Ptr to memory that will hold list of items (string ptr-s)
\param sizeListBuffer Size of memory (bytes) for list the list of items
*/
void bgStringListExMemSet(bgStringListEx_typ *p, char* pAddrListBuffer, int sizeListBuffer); 


/**
Method for data object \link bgStringListEx_typ \endlink\n
\param p Ptr to data object: (\link bgStringListEx_typ \endlink)
\param pContentBuffer Ptr to the input/source stream 
\param pAddrListBuffer Ptr to memory that will hold list of items (string ptr-s) (NOTE: if 0, \n
method \link bgStringListExMemSet \endlink must be used prior to calling this method!)
\param sizeListBuffer Size of memory (bytes) for the list of items (NOTE: if 0, \n
method \link bgStringListExMemSet \endlink must be used prior to calling this method!)

<b>IMPORTANT: </b>Method write 0-s (string termination) into the source stream (*pContentBuffer) at the 
position of each line end!

See also: \link bgStringListLines \endlink and \link bgStringListExMemSet \endlink
*/
int bgStringListLinesEx(bgStringListEx_typ *p, char* pContentBuffer, char* pAddrListBuffer, int sizeListBuffer); 




/**
Method for data object \link bgStringList_typ \endlink\n
\param Ptr to data object: (\link bgStringList_typ \endlink)
Example: Given the input string (*src), i.e.:
\code
"hello world what is new?"
\endcode
Method generates following: (p-> points to \link bgStringList_typ \endlink data object):
\code
p->itemCount = 5; <*src> string has 5 words
p->items[0] = ptr to 1st word ("hello"); 
p->items[1] = ptr to 2nd word ("world"); 
p->items[2] = ptr to 3rd word ("what"); 
p->items[3] = ptr to 4th word ("is"); 
p->items[4] = ptr to 5th word ("new?"); 
\endcode
\n
Method is used when processing ascii based client/server protocols (i.e. FTP, Telnet...)\n
<b>IMPORTANT:</b>Method write 0-s (string termination) into the source string (*src) at the position of
each separator!\n 
(i.e. It overwrites all seprator characters with 0)!)
*/
int bgStringSplitLine(bgStringList_typ *p, char separator); 



/**
Method for data object \link bgStringListEx_typ \endlink\n
\param p Ptr to data object: (\link bgStringListEx_typ \endlink)
\param separator Seperator character (i.e. word separation)
\param pLineBuffer Ptr to the input/source string (stream )
\param pAddrListBuffer Ptr to memory that will hold list of items (string ptr-s) (NOTE: if 0, \n
method \link bgStringListExMemSet \endlink must be used prior to calling this method!)
\param sizeListBuffer Size of memory (bytes) for the list of items (NOTE: if 0, \n
method \link bgStringListExMemSet \endlink must be used prior to calling this method!)

<b>IMPORTANT: </b>Method write 0-s (string termination) into the source string (*pLineBuffer) at the 
position of each (separator) character!

See also: \link bgStringSplitLine \endlink and \link bgStringListExMemSet \endlink
*/
int bgStringSplitLineEx(bgStringListEx_typ *p, char separator, char* pLineBuffer, char* pAddrListBuffer, int sizeListBuffer); 




/**
Convert string to lower case.
Example: 'Hello'->'hello', 'How Are YOU?'->'how are you?'
\param s Input string
\return Pointer to the same, input string
<b>IMPORTANT: </b>Method modifies the input string (*s) by changing all upper case letters to lower case
letters (if any)!\n
*/
char *bgStringLower(char *s);



/**
Convert string to upper case.
Example: 'Hello'->'HELLO', 'How Are YOU?'->'HOW ARE YOU?'
\param s Input string
\return Pointer to the same, input string
<b>IMPORTANT: </b>Method modifies the input string (*s) by changing all lower case letters to upper case
letters (if any)!\n
*/
char *bgStringUpper(char *s);


/**
Convert character to lower case.
Example: 'H' -> 'h'
\param c Input character 
\return Lower case of the input character if possible. If not, returns the same char.
*/
char bgCharToLowerCase(char c);



/**
Compare strings using strcmp.
\param s1 String 1
\param s2 String 2
\param minChrCount not used
\return 0 - if s1 != s2
\return 1 - if s1 == s2
*/
int bgStringIsMatch(char *s1, char *s2, int minChrCount);



/**
Copy part of the source string (*src) to destination (*dest).
\param src Source string ptr
\param StartPosition Defines position in (*src) where the copy begins (src[startPosition])
\param srcEndPosition Defines position in (*src) where the copy ends (src[endPosition - 1])
\param dest Destination buffer
\return Number of bytes copied (i.e. length of the dest string)
NOTE: srcEndPosition is not included in the copy (last copied byte position is srcEndPosition-1)\n
Length of the copied string is (srcEndPosition - srcStartPosition)\n
Example:
\code
src="01234567890"
srcStartPosition=3
srcEndPosition=7
dest="3456"
\endcode
*/
int bgStrCopyFromTo(char* src, int srcStartPosition, int srcEndPosition, char* dest);



/**
Copy source string (*src) to destination (*dest), remove leading/trailing spaces, convert (*dest) to lower
\param src Source string ptr
\param dest Destination buffer 
\param maxNoOfCharsToCopy Destination buffer size
\return Number of bytes copied (i.e. length of the dest string)
Example:
\code
src="  0123AB  cdeFG  "
dest="0123ab  cdefg"
\endcode
*/
int bgStrCpyNTrimSpcsToLwr(char *dest, char *src, int maxNoOfCharsToCopy);


/**
Search supplied c string (*src) (zero terminated!!!) for the eol (LF or CRLF).
\param src Source string
\param startPosition Start of the search position
\return Position (>=0) of the EOL character or -1 if EOL not found

*/
int bgStrFindEOL(char* src, int startPosition);


#endif
