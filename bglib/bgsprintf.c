#ifndef _BG_SPRINTF_C
#define _BG_SPRINTF_C

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif
//#include <string.h>
/*
File: 
bgSprintf.c

About:
------
- short and limited implementation of the sprintf() function
- following format chars are supported:
%d %x %X %s %u %c

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
#if 0
#include<bur/plc.h>
#include <bgsprintf.h>

_LOCAL char textBuffer[64];
_LOCAL int counter;

_INIT init()
{
	counter = 0;
}

_CYCLIC void Cyclic(void) {

	counter++;

	bgsprintf(textBuffer, "Counter=%d\n", counter);
}
#endif
/*End: Example/Test code*/

#include <stdarg.h>

#define PAD_RIGHT 1
#define PAD_ZERO 2

static void bgprintchar(char **str, int c)
{
	extern int putchar(int c);
	
	if (str) {
		**str = c;
		++(*str);
	}
}

static int bgprints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			bgprintchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		bgprintchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		bgprintchar (out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int bgprinti(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return bgprints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			bgprintchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + bgprints (out, s, width, pad);
}

static int bgprint(char **out, const char *format, va_list args )
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = (char *)va_arg( args, int );
				pc += bgprints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += bgprinti (out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += bgprinti (out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += bgprinti (out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += bgprinti (out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += bgprints (out, scr, width, pad);
				continue;
			}
		}
		else {
		out:
			bgprintchar (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}

int bgprintf(const char *format, ...)
{
        va_list args;
        
        va_start( args, format );
        return bgprint( 0, format, args );
}

int bgsnprintf(char *out, int size, const char *format, ...)
{
	static char tmp[512];
	char* pTmp = &tmp[0];
	int charCount;
	int ii;

	if(size > 512){
		strcpy(out, "fatal error! size > 512");
		return 0;
	}

	va_list args;
        
	va_start( args, format );

	charCount =  bgprint( &pTmp, format, args );

	if(charCount >= (size-1) ){
		charCount = (size-1);
	}

	for(ii=0; ii<charCount; ii++){
		out[ii] = tmp[ii];
	}

	out[charCount] = 0;
	
	return 0;
}
int bgsprintf(char *out, const char *format, ...)
{
        va_list args;
        
        va_start( args, format );
        return bgprint( &out, format, args );
}

int bgvsprintf(char *out, const char *format, va_list args)
{
      va_list ap;

     __va_copy (ap, args);
		
        return bgprint( &out, format, ap);
}
#endif
/*EOF*/
