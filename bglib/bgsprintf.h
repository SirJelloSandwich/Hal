/*
File: 
bgSprintf.h

About:
------
- short and limited implementation of the sprintf() function

*/
#ifndef _BG_SPRINTF_H
#define _BG_SPRINTF_H

//#include <stdarg.h>

/*
Public data types:
-----------------
*/

/*
Public functions:
-----------------
*/
int bgsprintf(char *out, const char *format, ...);

int bgsnprintf(char *out, int size, const char *format, ...);

//int bgvsprintf(char *out, const char *format, va_list args);

#endif
