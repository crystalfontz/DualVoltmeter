#ifndef __UTILITIES_H__
#define __UTILITIES_H__
//============================================================================
//
// Debug and string utitilities.
//
// 2022-04-26 Brent A. Crosby / Crystalfontz America, Inc.
//
//============================================================================
//  This is free and unencumbered software released into the public domain.
//
//  Anyone is free to copy, modify, publish, use, compile, sell, or
//  distribute this software, either in source code form or as a compiled
//  binary, for any purpose, commercial or non-commercial, and by any
//  means.
//
//  In jurisdictions that recognize copyright laws, the author or authors
//  of this software dedicate any and all copyright interest in the
//  software to the public domain. We make this dedication for the benefit
//  of the public at large and to the detriment of our heirs and
//  successors. We intend this dedication to be an overt act of
//  relinquishment in perpetuity of all present and future rights to this
//  software under copyright law.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
//  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//  OTHER DEALINGS IN THE SOFTWARE.
//
//  For more information, please refer to <http://unlicense.org/>
//===========================================================================
// Debug LED, or used for scope trigger or precise timing
#define DEBUG_PORT   (3)
#define CLR_DEBUG_PORT         (PORTD &= ~(0x08))
#define SET_DEBUG_PORT         (PORTD |=  (0x08))
#define DEBUG_LED   (4)
#define CLR_DEBUG_LED         (PORTD &= ~(0x10))
#define SET_DEBUG_LED         (PORTD |=  (0x10))
//===========================================================================
// Don't change these
#define DEBUG_NONE   (0)
#define DEBUG_STATUS (1)
//---------------------------------------------------------------------------
// You can set DEBUG_LEVEL to:
//   DEBUG_NONE    (0K flash, serial not used)
//   DEBUG_STATUS  (1.4K ~ 2.5K flash)
// to send messages to the console (Ctrl+Shift+M) in the arduino IDE.
#define DEBUG_LEVEL (DEBUG_STATUS)
//---------------------------------------------------------------------------
//Don't call SerPrintFF() directly, use DBG_STAT() or DBG_GEEK() macros.
#if (DEBUG_LEVEL == DEBUG_STATUS)    
void SerPrintFF(const __FlashStringHelper *fmt, ... );
#endif // (DEBUG_LEVEL == DEBUG_STATUS)
//---------------------------------------------------------------------------
//Conditional debugging "printf()" style debug message examples, string
//stored in flash.
//
// DBG_STAT("Printed if DEBUG_LEVEL is DEBUG_STATUS Some int= %3d\n",9600);
//
// Use DBG_STAT() for normal messages that a normal user might be interested in.
//
// DEBUG_NONE will print nothing and not use the serial console.
//
//These macros conditionally expand to a SerPrintFF() call.
//ref: https://stackoverflow.com/questions/5765175/macro-to-turn-off-printf-statements
#if (DEBUG_LEVEL == DEBUG_NONE)
    #define    DBG_STAT(f,...)
#endif

#if (DEBUG_LEVEL == DEBUG_STATUS)
    #define    DBG_STAT(f,...)    SerPrintFF(F(f),##__VA_ARGS__)
#endif
//============================================================================
void Voltage_To_Strings(int16_t mv,
                        char *units_string,
                        char *fraction_string);
//============================================================================
void Scale_To_String(int8_t volts,
                     char *volts_string);
//============================================================================
#endif // __UTILITIES_H__
