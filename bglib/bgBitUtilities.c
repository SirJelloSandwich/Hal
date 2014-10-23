#include <bur/plctypes.h>
#ifndef _BG_BITUTILS_C
#define _BG_BITUTILS_C

void bgBitSetUDINT(UDINT* dest,  UINT bitIndex, BOOL bitValue)
{
	UDINT mask = 0x1;
	UDINT current = *dest;

	mask <<= bitIndex;


	if(!(bitValue && 0x1) ){
		mask = ~(mask);
		current &= mask;
	}else{
		current |= mask;
	}

	*dest = current;
}

void bgBitSetUINT(UINT* dest,  UINT bitIndex, BOOL bitValue)
{
	UINT mask = 0x1;
	UINT current = *dest;

	mask <<= bitIndex;


	if(!(bitValue && 0x1) ){
		mask = ~(mask);
		current &= mask;
	}else{
		current |= mask;
	}

	*dest = current;
}

void bgBitSetUSINT(USINT* dest,  UINT bitIndex, BOOL bitValue)
{
	USINT mask = 0x1;
	USINT current = *dest;

	mask <<= bitIndex;


	if(!(bitValue && 0x1) ){
		mask = ~(mask);
		current &= mask;
	}else{
		current |= mask;
	}

	*dest = current;
}
#endif
//eof
