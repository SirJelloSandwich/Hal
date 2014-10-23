/*********************************************************
*	NAME	: sprintf2.h
*	FUNCTION: Headerfile for the sprintf2.a implementation
*			  of the standard "C" function: sprintf().
**********************************************************/
#define _SPRINTF_

#ifndef __SPRINTF_H_
#define __SPRINTF_H_

int sprintf2(char *s, const char *fmt, ...);
int snprintf2(char *s, int buf_size, const char *fmt, ...);                    // WBR: added snprintf2 function

#endif
