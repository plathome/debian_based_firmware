/*
 * CP210xSupportFunctions.h
 *
 *  Created on: Oct 30, 2012
 *      Author: strowlan
 */

#ifndef CP210XSUPPORTFUNCTIONS_H_
#define CP210XSUPPORTFUNCTIONS_H_

#include <string.h>
#include "Types.h"

#define		CP210x_MAX_SETUP_LENGTH				512

BOOL ValidParam(LPVOID lpVoidPointer);
BOOL ValidParam(LPWORD lpwPointer);
BOOL ValidParam(LPBYTE lpbPointer);
BOOL ValidParam(LPVOID lpVoidPointer);
BOOL ValidParam(LPVOID lpVoidPointer, LPBYTE lpbPointer);
void CopyToString(BYTE* setup, LPVOID string, BYTE* bLength, BOOL bConvertToUnicode);
void ConvertToUnicode(BYTE* dest, BYTE* source, BYTE bLength);

#endif /* CP210XSUPPORTFUNCTIONS_H_ */
