#ifndef __LCD_DRAW_H__
#define __LCD_DRAW_H__
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
#define FRAME_WIDTH_BYTES (8)
#define FRAME_WIDTH_PIXELS (FRAME_WIDTH_BYTES*8)
#define FRAME_HEIGHT_PIXELS (128)
extern uint8_t framebuffer[FRAME_HEIGHT_PIXELS][FRAME_WIDTH_BYTES];
//----------------------------------------------------------------------------
void Fill_Framebuffer(uint8_t data1, uint8_t data2);
void Send_Framebuffer_To_Display(void);
void logo_to_framebuffer(void);
void Put_Pixel(uint8_t x, uint8_t y, uint8_t set);
void LCD_Line(uint8_t x0, uint8_t y0,
               uint8_t x1, uint8_t y1,
               uint8_t set);
void LCD_Circle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t set);
//void put_string_P(uint8_t x, uint8_t y, uint8_t Field_Width,const char *input);
void put_string_P(uint8_t x, uint8_t y, uint8_t Field_Width,const __FlashStringHelper *input);

void put_string(uint8_t x, uint8_t y, uint8_t Field_Width, char *input);
void invert_string(uint8_t x, uint8_t y, uint8_t Field_Width);
void horizontal_line(uint8_t x1, uint8_t y, uint8_t x2);
void vertical_line(uint8_t x, uint8_t y1, uint8_t y2);
void outline_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void clear_rectangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);
void Rounded_Rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t radius,uint8_t set);
//void put_big_number_v(uint8_t x, uint8_t y, uint8_t Field_Width, const char *input);
//void put_big_number(uint8_t x, uint8_t y, uint8_t Field_Width, const char *input);
void put_big_number(uint8_t x, uint8_t y, uint8_t Field_Width, char *input);
//============================================================================
#endif //  __LCD_DRAW_H__
