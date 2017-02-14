/*
 * CP210xSupportFunctions.cpp
 *
 *  Created on: Oct 30, 2012
 *      Author: strowlan
 */


#include "CP210xSupportFunctions.h"

//------------------------------------------------------------------------
// ValidParam(LPVOID)
//
// Checks validity of an LPVOID pointer value.
//------------------------------------------------------------------------

BOOL ValidParam(LPVOID lpVoidPointer) {
    BYTE temp = 0;

    try {
        temp = *((BYTE*) lpVoidPointer);
    } catch (...) {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------
// ValidParam(LPBYTE)
//
// Checks validity of an LPBYTE pointer value.
//------------------------------------------------------------------------

BOOL ValidParam(LPBYTE lpbPointer) {
    BYTE temp = 0;

    try {
        temp = *((BYTE*) lpbPointer);
    } catch (...) {
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------------
// ValidParam(LPWORD)
//
// Checks validity of an LPWORD pointer value.
//------------------------------------------------------------------------

BOOL ValidParam(LPWORD lpwPointer) {
    WORD temp = 0;

    try {
        temp = *lpwPointer;
    } catch (...) {
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------------
// ValidParam(LPVOID, LPBYTE)
//
// Checks validity of LPVOID, LPBYTE pair of pointer values.
//------------------------------------------------------------------------

BOOL ValidParam(LPVOID lpVoidPointer, LPBYTE lpbPointer) {
    if (ValidParam(lpVoidPointer))
        if (ValidParam(lpbPointer))
            return TRUE;
    
    return FALSE;
}

//------------------------------------------------------------------------
// CopyToString()
//
// Copies string into the remaining part of a setup buffer.  The first
// byte is the length and the second byte is 0x03.  This leaves 246 bytes
// for the string.  If it is not already unicode, the string must be
// converted to unicode when copied.
//------------------------------------------------------------------------

void CopyToString(BYTE* setup, LPVOID string, BYTE* bLength, BOOL bConvertToUnicode) {
    BYTE length = *bLength;

    // If not already unicode it will require twice as many bytes as bLength parameter.
    if (bConvertToUnicode) {
        ConvertToUnicode(&setup[2], (BYTE*) string, length);
        length = length * 2;
    } else {
        length = length * 2;
        memcpy(&setup[2], string, length);
    }

    setup[0] = length + 2;
    setup[1] = 0x03; // Indicates a string

    // return the possibly modified length value
    *bLength = length;
}


//------------------------------------------------------------------------
// ConvertToUnicode()
//
// Copy ASCII character to the low byte and '0' to the high byte to
// convert string to unicode.
// bLength is the length in bytes of the unicode buffer.
//------------------------------------------------------------------------

void ConvertToUnicode(BYTE* dest, BYTE* source, BYTE bLength) {
    for (int i = 0; i < bLength; i++) {
        dest[i * 2] = source[i];
        dest[i * 2 + 1] = 0;
    }
}
