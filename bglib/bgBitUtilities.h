/**\file: bgBitUtilities.h
*Bit utilities. 
*/
#include <bur/plctypes.h>
#ifndef _BG_BITUTILS_H
#define _BG_BITUTILS_H



/**
Set UDINT bits.
\param dest,  Ptr to UDINT.
\param bitIndex within the above UDINT
\param bitValue is the value to set the bit to
*/
void bgBitSetUDINT(UDINT* dest,  UINT bitIndex, BOOL bitValue);



/**
Set UINT bits.
\param dest,  Ptr to UINT.
\param bitIndex within the above UINT
\param bitValue is the value to set the bit to
*/
void bgBitSetUINT(UINT* dest,  UINT bitIndex, BOOL bitValue)



/**
Set USINT bits.
\param dest,  Ptr to USINT.
\param bitIndex within the above USINT
\param bitValue is the value to set the bit to
*/
void bgBitSetUSINT(USINT* dest,  UINT bitIndex, BOOL bitValue);


#endif
