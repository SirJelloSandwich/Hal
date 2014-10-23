/********************************************************************
 * COPYRIGHT -- HP
 ********************************************************************
 * Program: counter
 * File: counter.c
 * Author: goran
 * Created: March 07, 2013
 ********************************************************************
 * Implementation of program counter
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

_LOCAL int counter;

void _INIT counterINIT(void)
{
	/* TODO: Add code here */
}

void _CYCLIC counterCYCLIC(void)
{
	/* TODO: Add code here */
	counter++;
}
