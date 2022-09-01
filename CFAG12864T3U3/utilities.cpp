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
#include <Arduino.h>
#include "utilities.h"
#if (DEBUG_LEVEL == DEBUG_STATUS)
#include <stdarg.h>
#endif // (DEBUG_LEVEL == DEBUG_STATUS)
//============================================================================
// Don't call SerPrintFF() directly, use DBG_STAT().
//
// ref http://playground.arduino.cc/Main/Printf
//
// Example to dump a uint32_t in hex and decimal
//   SerPrintFF(F("RAM_G_Unused_Start: 0x%08lX = %lu\n"),RAM_G_Unused_Start,RAM_G_Unused_Start);
// Example to dump a uint16_t in hex and decimal
//   SerPrintFF(F("Initial Offset Read: 0x%04X = %u\n"),FWo ,FWo);
#if (DEBUG_LEVEL == DEBUG_STATUS)
void SerPrintFF(const __FlashStringHelper *fmt, ... )
  {
  char
    tmp[128]; // resulting string limited to 128 chars
  va_list
    args;
  va_start(args, fmt );
  vsnprintf_P(tmp, 128, (const char *)fmt, args);
  va_end (args);
  Serial.print(tmp);
  }
#endif // (DEBUG_LEVEL == DEBUG_STATUS)
//============================================================================
// Example to dump a uint32_t in hex and decimal
//   StringPrintFF(mystring,32,F("RAM_G_Unused_Start: 0x%08lX = %lu\n"),RAM_G_Unused_Start,RAM_G_Unused_Start);
// Example to dump a uint16_t in hex and decimal
//   StringPrintFF(mystring,32,F("Initial Offset Read: 0x%04X = %u\n"),FWo ,FWo);
#if (0)    
void StringPrintFF(char *dest_string,
                   uint8_t max_length,
                   const __FlashStringHelper *fmt, ... )
  {
  va_list
    args;
  va_start(args, fmt );
  vsnprintf_P(dest_string, max_length, (const char *)fmt, args);
  va_end (args);
  }
#endif
//============================================================================  
void Voltage_To_Strings(int16_t mv,
                        char *units_string,
                        char *fraction_string)
  {
  // If it is positive, we can display up to 32.767 volts
  // If it is negative, we can display down to -9.999 volts
  // Below -9.99 will get --.----
  
  uint8_t
    digit;

  if(mv<-9999)
    {
    //Out of range negative
    units_string[0]=
    units_string[1]=
    fraction_string[0]=
    fraction_string[1]=
    fraction_string[2]='-';
    }
  else
    {
    // In range, might be negative or positive.
    if(mv<0)
      {
      //Negative case, replace 10's digit with sign
      // ref: https://www.youtube.com/watch?v=BDvFtZtVf4s
      units_string[0]='-';
      // Change the mv to positive and proceed
      mv=-mv;
      }
    else
      {
      //Positive case, 10's digit
      //Get ten's digit
      digit=mv/10000U;
      if(0U != digit)
        {
        units_string[0]=digit+'0';
        }
      else
        {
        //Blank leading zero
        units_string[0]=' ';
        }
      }
    }
    
  // One's digit
  mv%=10000U;    
  digit=mv/1000U;
  units_string[1]=digit+'0';
  // Tenth's digit
  mv%=1000U;
  digit=mv/100U;
  fraction_string[0]=digit+'0';
  // Hundreth's digit
  mv%=100U;
  digit=mv/10U;
  fraction_string[1]=digit+'0';

  // Thousandth's digit
  fraction_string[2]=(mv%10U)+'0';

  //Terminate units_string
  units_string[2]=0;
  //Terminate the fraction_string
  fraction_string[3]=0;
  }
//============================================================================  
void Scale_To_String(int8_t volts,
                     char *volts_string)
  {
  // If it is positive, we can display up to 99 volts
  // If it is negative, we can display down to -9 volts
  // Above 99 will get ++
  // Below -9 will get --
  
  uint8_t
    digit;

  if(volts<-9)
    {
    //Out of range negative
    volts_string[0]=
    volts_string[1]='-';
    }
  else
    if(99<volts)
      {
      //Out of range positive
      volts_string[0]=
      volts_string[1]='+';
      }
    else
      {
      // In range, might be negative or positive.
      if(volts<0)
        {
        //Negative case, replace 10's digit with sign.
        volts_string[0]='-';
        // Change the mv to positive and proceed
        volts=-volts;
        }
      else
        {
        //Positive case, 10's digit
        //Get ten's digit
        digit=volts/10U;
        if(0U != digit)
          {
          volts_string[0]=digit+'0';
          }
        else
          {
          //Blank leading zero
          volts_string[0]=' ';
          }
        }
      }
    
  // One's digit
  volts_string[1]=(volts%10U) + (uint8_t)'0';
  //Terminate the volts_string
  volts_string[2]=0;
  }
//============================================================================  
