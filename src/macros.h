/*
File:   macros.h
Author: Taylor Robbins
Date:   09\12\2026
*/

#ifndef _MACROS_H
#define _MACROS_H

// When decoding models, we need to take a relative pointer from memory and fix it
// up so it's an absolute pointer to memory based on the model header absolute pointer.
// A value of '0' is left as '0'. Don't use this function if 0 is a valid relative pointer!
#define RELATIVE_PNTR_TO_ABSOLUTE(pntrType, basePntr, relativePntr) (pntrType*)(((relativePntr) == 0) ? 0 : ((u8*)(basePntr) + (u32)(relativePntr)))

#endif //  _MACROS_H
