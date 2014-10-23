/*
File: 
bgsscanf.h

About:
------
- short and limited implementation of the sscanf() function

*/
#ifndef _BG_SSCANF_H
#define _BG_SSCANF_H

#include <stdarg.h>

/*
Public data types:
-----------------
*/

/*
Public functions:
-----------------
*/
int bgsscanf(const char *out, const char *format, ...);

int bgvsscanf(const char *out, const char *format, va_list args);

#endif

