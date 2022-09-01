#ifndef __LCD_LOW_H__
#define __LCD_LOW_H__
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
// LCD SPI & control lines
//   ARD      | Port | LCD
// -----------+------+-------------------------
//  #8/D8     |  PB0 | LCD_RS
//  #9/D9     |  PB1 | LCD_RESET
// #10/D10    |  PB2 | LCD_CS_NOT   (or SPI SS)
// #11/D11    |  PB3 | LCD_MOSI/DB7 (hardware SPI)
// #12/D12    |  PB4 | not used     (would be MISO)
// #13/D13    |  PB5 | LCD_SCK/DB6  (hardware SPI)
//
// The NT7534 datasheet recommends tying DB0-DB5 to VCC (3.3v) or ground.
// DB0-DB5 appear to have ~12K pull-up to VCC, so to mimimize current
// consumption and still follow the advice of the controller datasheet,
// you would want to tie the unused data pins to VCC.
//

#define CLR_RS    (PORTB &= ~(0x01))
#define SET_RS    (PORTB |=  (0x01))
#define CLR_RESET (PORTB &= ~(0x02))
#define SET_RESET (PORTB |=  (0x02))
#define CLR_CS    (PORTB &= ~(0x04))
#define SET_CS    (PORTB |=  (0x04))
#define CLR_MOSI  (PORTB &= ~(0x08))
#define SET_MOSI  (PORTB |=  (0x08))
#define CLR_SCK   (PORTB &= ~(0x20))
#define SET_SCK   (PORTB |=  (0x20))

//PD.7
#define PIN_TIME   (7)
#define CLR_TIME   (PORTD &= ~(0x80))
#define SET_TIME   (PORTD |=  (0x80))

#define CFAG12864T3 (0)
#define CFAG12864U3 (1)

// Define the display type.
#define display_type CFAG12864U3

#define HRES 128
#define VRES 64
//============================================================================
//Constants for CFAG12864U3 Static Indicator (SI_) bit field
#define SI_NONE         (0x0000)
#define SI_ALL          (0xFFFF)
#define SI_LEAF         (0x8000)
#define SI_DOC          (0x4000)
#define SI_BATTERY      (0x2000)
#define SI_BATT_BAR_0   (0x1000)
#define SI_BATT_BAR_1   (0x0800)
#define SI_BATT_BAR_2   (0x0400)
#define SI_COMPUTER     (0x0200)
#define SI_ANTENNA      (0x0100)
#define SI_SPEAKER      (0x0080)
#define SI_SPEAKER_MUTE (0x0040)
#define SI_MAIL         (0x0020)
#define SI_PHONE        (0x0010)
#define SI_BELL         (0x0008)
#define SI_CLOCK        (0x0004)
#define SI_WARNING      (0x0002)
#define SI_SYNC         (0x0001)
//============================================================================
void SPI_sendCommand(uint8_t command);
void SPI_sendData(uint8_t data);
void Set_Address(uint8_t column, uint8_t page);
void Set_Static_Indicators(uint16_t state_mask);
void Set_Contrast(uint8_t contrast);
void Initialize_CFAG12864T2U2(void);
//============================================================================
#endif //  __LCD_LOW_H__
