//============================================================================
// CFAG12864T3 CFAG12864U3 128x64 LCD in SPI mode
// LCD_draw.cpp : Higher-level LCD drawing routines
// 2017 - 06 - 15 Brent A. Crosby
// 2019 - 05 - 02 Trevin Jorgenson
// Crystalfontz America, Inc. http://www.crystalfontz.com
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
//============================================================================
#include <Arduino.h>
#include <SPI.h>


#include "fnt_08x08_h.h"
#include "fnt_12x16_h.h"
#include "LCD_low.h"
#include "LCD_draw.h"
#include "images.h"

//============================================================================
uint8_t
  framebuffer[FRAME_HEIGHT_PIXELS][FRAME_WIDTH_BYTES];
//============================================================================
void Fill_Framebuffer(uint8_t data1, uint8_t data2)
  {
  uint16_t
    count;
  uint8_t
    *target;
  target=&framebuffer[0][0];
  for(count=0;count<512;count++)
    {
    *target=data1;
    target++;
    *target=data2;
    target++;
    }              
  }
//============================================================================
/*

#define FRAME_WIDTH_BYTES (8)
#define FRAME_WIDTH_PIXELS (FRAME_WIDTH_BYTES*8)
#define FRAME_HEIGHT_PIXELS (128)
extern uint8_t framebuffer[FRAME_HEIGHT_PIXELS][FRAME_WIDTH_BYTES];

*/

void Send_Framebuffer_To_Display(void)
  {
//  //Optimized pointer, plus unwrap SPI 1.94mS
  uint8_t
    page;
  uint8_t
    column;
//  uint8_t
//    *source;
//  source=&framebuffer[0][0];        
  for(page=0;page<8;page++)
    {
    //Set the LCD to the left of this line.
    Set_Address(0,page);
    // Select the LCD's data register
    SET_RS;
    // Select the LCD controller
    CLR_CS;
    for(column=0;column<128;column++)
      {
      //Send the data via SPI:
      SPI.transfer(framebuffer[column][7-page]);
//      SPI.transfer(*source);
//      source++;
      }
    // Deselect the LCD controller
    SET_CS;
    }
  }
//============================================================================
void logo_to_framebuffer(void)
  {
  // ~ 583uS. Bytes configured correctly using Image2Code -- refer to
  // Image2Code_Settings.png
  memcpy_P(framebuffer,Background_Bitmap,sizeof(framebuffer));
  }
//============================================================================
void Put_Pixel(uint8_t x, uint8_t y, uint8_t set)
  {
  //2.48uS ~ 4.70uS
  if(0 != set)
    {
    //Setting the bit
//    framebuffer[y>>3][x]|=(0x01 << (y&0x07));
//    framebuffer[y][x>>3]|=(0x01 << (x&0x07));
    framebuffer[y][x>>3]|=(0x80 >> (x&0x07));
    }
  else
    {
    //Clearing the bit
//    framebuffer[y>>3][x]&=~(0x01 << (y&0x07));
//    framebuffer[y][x>>3]&=~(0x01 << (x&0x07));
    framebuffer[y][x>>3]&=~(0x80 >> (x&0x07));

    }
  }
//============================================================================
// From: http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void LCD_Circle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t set)
  {
  uint8_t
    x;
  uint8_t
    y;
  int8_t
    radiusError;

  x = radius;
  y = 0;
  radiusError = 1 - (int8_t) x;

  while (x >= y)
    {
    //11 O'Clock
    Put_Pixel(x0 - y, y0 + x, set);
    //1 O'Clock
    Put_Pixel(x0 + y, y0 + x, set);
    //10 O'Clock
    Put_Pixel(x0 - x, y0 + y, set);
    //2 O'Clock
    Put_Pixel(x0 + x, y0 + y, set);
    //8 O'Clock
    Put_Pixel(x0 - x, y0 - y, set);
    //4 O'Clock
    Put_Pixel(x0 + x, y0 - y, set);
    //7 O'Clock
    Put_Pixel(x0 - y, y0 - x, set);
    //5 O'Clock
    Put_Pixel(x0 + y, y0 - x, set);

    y++;
    if (radiusError < 0)
      {
      radiusError += (int16_t)(2 * y + 1);
      }
    else
      {
      x--;
      radiusError += 2 * (((int16_t) y - (int16_t) x) + 1);
      }
    }
  }
//============================================================================
typedef union
  {
  uint8_t
    as_bytes[2];
  uint16_t
    as_word;
  }WORD_UNION;

//============================================================================
// put_letter() will display a single character from the 08x08 font at (x,y)
// It will pad a 1-pixel border to the left and top of the letter. The 08x08
// font already has a 1-pixel border at the right and bottom. This means that
// a 9x9 block will be modified.
//
// put_letter will use a "COPY_PUT"--replacing the area occupied by the
// character. This means that you can call put_letter() several times and
// then call UpdateLCD() without having to clear rectangles.
//---------------------------------------------------------------------------
void put_letter(uint8_t x, uint8_t y, uint8_t letter)
  {
  register uint8_t
    line;
  register uint8_t
    low_3_bits_of_x;
  register WORD_UNION
    *Pixel_Data;
  register const uint8_t
    *Font_Pointer;
  register uint16_t
    Clearing_Mask;

  //Bail for garbage in. We need at least 9 pixels below and to the right.
  if(((FRAME_WIDTH_PIXELS-9)<x)||
     ((FRAME_HEIGHT_PIXELS-9)<y))
    return;

  //Since we need to fuss with 9 bits in width, we will always have to
  //look at two bytes of display memory per line.

  //Calculate the address of the first byte in display in LCD_Memory,
  //and the second byte by the union.
  Pixel_Data=(WORD_UNION *)&framebuffer[y][x>>3];

  //Get a pointer into the font information for this character.
  Font_Pointer=&Font_08x08_Horiz[(letter-FONT_08X08_HORIZ_BASE)][0];

  //Avoid doing this mask a bunch of times.
  low_3_bits_of_x=x&0x07;

  //Avoid doing this shift a bunch of times
  Clearing_Mask=(0x7F00)>>low_3_bits_of_x;

  for(line=0;line<=8;line++)
    {
    //Clear the 9 bits in the display memory
    Pixel_Data->as_word&=Clearing_Mask;

    //0r in the data from the font
    if(line) //top line is blank
      {
      Pixel_Data->as_bytes[0]|=
        ((pgm_read_byte(Font_Pointer))>>((low_3_bits_of_x)+1));
      Pixel_Data->as_bytes[1]|=
        ((pgm_read_byte(Font_Pointer))<<(7-(low_3_bits_of_x)));
      //Move to the next line in the font
      Font_Pointer++;
      }

    //Move to the next line in the display. Pixel_Data is a WORD_UNION (2
    //bytes) so we only need to 1/2 of the framebuffer width.
    Pixel_Data+=(FRAME_WIDTH_BYTES/2);
    }
  }
//============================================================================
// void put_string(uint8_t x, uint8_t y, uint8_t Field_Width, sbyte flash *input);
//
// put_string() will display the null-terminated string pointed to by input
// at (x,y) in the 08x08 font. It will pad a 1-pixel border to the left and
// top of the text. The 08x08 font already has a 1-pixel border at the
// right and bottom.
//
// Put_String will use a "COPY_PUT", replacing the area occupied by the
// string. This means that you can call put_string() several times and then call
// UpdateLCD(), without having to clear rectangles.
//---------------------------------------------------------------------------
void put_string_P(uint8_t x, uint8_t y, uint8_t Field_Width,const __FlashStringHelper *input)
  {
  register uint8_t
    Terminator_Found;
  register uint8_t
    Characters_Placed;
  register uint8_t
    this_character;
  const char
    *local_input;
  local_input=(const char *)input;

  //Initialize Terminator_Found.
  Terminator_Found=0;
  //Move across the field. We will either put the character or a blank
  //in every position of Field_Width.
  for(Characters_Placed=0;Characters_Placed<Field_Width;Characters_Placed++)
    {
    //If we have not passed the terminator, then get the next
    //character in the string. If we find the terminator,
    //remember that we are out of characters.
    if(!Terminator_Found)
      {
      this_character=pgm_read_byte(local_input++);
      if(!this_character)
        {
        Terminator_Found=1;
        this_character=' ';
        }
      }
    else
      this_character=' ';

    //Print the character. put_letter does clipping.
    put_letter(x,y,this_character);

    //Move over one character.
    x+=8;
    }
  }
//---------------------------------------------------------------------------
void put_string(uint8_t x, uint8_t y, uint8_t Field_Width, char *input)
  {
  register uint8_t
    Terminator_Found;
  register uint8_t
    Characters_Placed;
  register uint8_t
    this_character;

  //Initialize Terminator_Found.
  Terminator_Found=0;
  //Move across the field. We will either put the character or a blank
  //in every position of Field_Width.
  for(Characters_Placed=0;Characters_Placed<Field_Width;Characters_Placed++)
    {
    //If we have not passed the terminator, then get the next
    //character in the string. If we find the terminator,
    //remember that we are out of characters.
    if(!Terminator_Found)
      {
      this_character=*input++;
      if(!this_character)
        {
        Terminator_Found=1;
        this_character=' ';
        }
      }
    else
      this_character=' ';

    //Print the character. put_letter does clipping.
    put_letter(x,y,this_character);

    //Move over one character.
    x+=8;
    }
  }
//============================================================================
// From: http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C
void LCD_Line(uint8_t x0, uint8_t y0,
              uint8_t x1, uint8_t y1,
              uint8_t set)
  {
  int8_t
    dx;
  int8_t
    sx;
  int8_t
    dy;
  int8_t
    sy;
  int8_t
    err;
  int8_t
    e2;      
  dx = abs((int16_t )x1 - (int16_t )x0);
  sx = x0 < x1 ? 1 : -1;
  dy = abs((int16_t )y1 - (int16_t )y0);
  sy = y0 < y1 ? 1 : -1;
  err = (dx > dy ? dx : -dy) / 2;

  for (;;)
    {
    Put_Pixel(x0, y0, set);
    if ((x0 == x1) && (y0 == y1))
      break;
    e2 = err;
    if (e2 > -dx)
      {
      err -= dy;
      x0 = (uint16_t)((int16_t) x0 + sx);
      }
    if (e2 < dy)
      {
      err += dx;
      y0 = (uint16_t)((int16_t) y0 + sy);
      }
    }
  }
//============================================================================
/*
void outline_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
  {
  LCD_Line(x1+1,y1,x2-1,y1,1);
  LCD_Line(x1+1,y2,x2-1,y2,1);
  LCD_Line(x1,y1,x1,y2,1);
  LCD_Line(x2,y1,x2,y2,1);
  }
*/
//============================================================================
/*
void clear_rectangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
  {
  register uint8_t
    *LCD_Memory;
  register uint8_t
    *LCD_Memory_1;
  register uint8_t
    *LCD_Memory_2;
  register uint8_t
    mask;
  register uint8_t
    column;
  //Draw the last pixel too.
  y2++;
  //Bail for bogus parametrers.
  if((x2<x1)||
     (y2<y1)||
     (127<x1)||
     (127<x2)||
     (63<y1)||
     (63<y2))
    return;
  //Calculate the address of the first ubyte in display in LCD_Memory
  LCD_Memory_1=&framebuffer[y1>>3][x1];
  //Calculate the address of the last ubyte in display in LCD_Memory
  LCD_Memory_2=&framebuffer[y2>>3][x1];
  //If they are the same, then this is a special case.
  if(LCD_Memory_1==LCD_Memory_2)
    {
    //The rectangle fits in one row.
    mask=~((0xFF<<(y1&0x07))&(~(0xFF<<(y2&0x07))));
    for(column=x1;column<=x2;column++)
      {
      *LCD_Memory_1&=mask;
      LCD_Memory_1++;
      }
    }
  else
    {
    //This is the case where we need to loop over multiple
    //(possibly 0) lines rows. Do the top ubyte.
    mask=~(0xFF<<(y1&0x07));
    LCD_Memory=LCD_Memory_1;
    for(column=x1;column<=x2;column++)
      {
      *LCD_Memory&=mask;
      LCD_Memory++;
      }
    //Do the bytes in the middle.
    for(LCD_Memory_1+=128;LCD_Memory_1!=LCD_Memory_2;LCD_Memory_1+=128)
      {
      LCD_Memory=LCD_Memory_1;
      for(column=x1;column<=x2;column++)
        {
        *LCD_Memory=0;
        LCD_Memory++;
        }
      }
    //Do the bottom ubyte.
    mask=0xFF<<(y2&0x07);
    for(column=x1;column<=x2;column++)
      {
      *LCD_Memory_2&=mask;
      LCD_Memory_2++;
      }
    }
  }
  */
//============================================================================
 #define mSwap(a,b,t)\
  {\
  t=a;\
  a=b;\
  b=t;\
  }
void Rounded_Rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t radius,uint8_t set)
  {
  uint8_t
    x;
  uint8_t
    y;
  int8_t
    radiusError;
  uint8_t
    Left_X_Center;
  uint8_t
    Lower_Y_Center;
  uint8_t
    Right_X_Center;
  uint8_t
    Upper_Y_Center;

  uint8_t
    temp;
  if (y1 < y0)
    mSwap(y0, y1, temp);
  if (x1 < x0)
    mSwap(x0, x1, temp);

  x = radius;
  y = 0;
  radiusError = 1 - (int8_t) x;

  Left_X_Center = x0 + radius;
  Lower_Y_Center = y0 + radius;

  Right_X_Center = x1 - radius;
  Upper_Y_Center = y1 - radius;

  if(0 != radius)
    {
    while(x >= y)
      {
      //Lower Left Corner, no offset
      Put_Pixel(Left_X_Center - x, Lower_Y_Center - y, set); //8 o'clock
      Put_Pixel(Left_X_Center - y, Lower_Y_Center - x, set); //7 o'clock
      //Lower Right Corner, X offset
      Put_Pixel(Right_X_Center + x, Lower_Y_Center - y, set); //4 o'clock
      Put_Pixel(Right_X_Center + y, Lower_Y_Center - x, set); //5 o'clock
      //Upper Left Corner, Y offset
      Put_Pixel(Left_X_Center - x, Upper_Y_Center + y, set); //10 o'clock
      Put_Pixel(Left_X_Center - y, Upper_Y_Center + x, set); //11 o'clock
      //Upper Right Corner, X & Y offset
      Put_Pixel(Right_X_Center + x, Upper_Y_Center + y, set); //2 o'clock
      Put_Pixel(Right_X_Center + y, Upper_Y_Center + x, set); //1 o'clock
      y++;
      if (radiusError < 0)
        {
        radiusError += (int16_t)(2 * y + 1);
        }
      else
        {
        x--;
        radiusError += 2 * (((int16_t) y - (int16_t) x) + 1);
        }
      }
    }

  //Put in the lines.
  //Bottom
  LCD_Line(Left_X_Center, y0, Right_X_Center, y0,1);
  //Top
  LCD_Line(Left_X_Center, y1, Right_X_Center, y1,1);
  //Left
  LCD_Line(x0, Lower_Y_Center, x0, Upper_Y_Center,1);
  //Right
  LCD_Line(x1, Lower_Y_Center, x1, Upper_Y_Center,1);
  }
//============================================================================
// Only '0'-'9', '.', '+', '-', '%' and 'e' will be displayed. The font is
// 12 x 16.
typedef union
  {
  uint8_t
    as_bytes[4];
  uint32_t
    as_dword;
  }DWORD_UNION;
/*
void put_big_number_v(uint8_t x, uint8_t y, uint8_t Field_Width, const char *input)
  {
  register uint8_t
    Terminator_Found;
  register uint8_t
    Characters_Placed;
  register uint8_t
    this_character;
  register uint8_t
    row;
  register uint8_t
    row_limit;
  register uint8_t
    *LCD_Memory;
  register const uint16_t
    *Font_Pointer;
  register uint8_t
    column;
  DWORD_UNION
    Clearing_Mask;
  DWORD_UNION
    Pixel_Data;

  //We typically need to manipulate 3 rows of data. We only need to do
  //two if we are in the bottom two rows or the top two rows of the display.
  row_limit=y>>3;

  //Calculate the address of the first ubyte in display in LCD_Memory
  LCD_Memory=&framebuffer[row_limit][x];

  if((row_limit<2)||(5<row_limit))
    row_limit=2;
  else
    row_limit=3;

  //Calculate Clearing_Mask, the vertical mask that we will and with
  //LCD_Memory to clear the space before we or in the data from the
  //font. It is 16 pixels.
  Clearing_Mask.as_dword=~(0x01FFFFL<<(y&0x07));

  //Clear the first col to the left of the string.
  for(row=0;row<=row_limit;row++)
    LCD_Memory[row*128]&=Clearing_Mask.as_bytes[row];
  LCD_Memory++;

  //Initialize Terminator_Found.
  Terminator_Found=0;
  //Move across the field. We will either put the character or a blank
  //in every position of Field_Width.
  for(Characters_Placed=0;Characters_Placed<Field_Width;Characters_Placed++)
    {
    //If we have not passed the terminator, then get the next
    //character in the string. If we find the terminator,
    //remember that we are out of characters.
    if(!Terminator_Found)
      {
      this_character=*input++;
      if(!this_character)
        {
        Terminator_Found=1;
        this_character=' ';
        }
      }
    else
      this_character=' ';
    //Get a pointer into the font information for this character.
    switch(this_character)
      {
      case '+': Font_Pointer=&v_big_plus[0]; break;
      case '-': Font_Pointer=&v_big_minus[0]; break;
      case '.': Font_Pointer=&v_big_dot[0]; break;
      case '0': Font_Pointer=&v_big_0[0]; break;
      case '1': Font_Pointer=&v_big_1[0]; break;
      case '2': Font_Pointer=&v_big_2[0]; break;
      case '3': Font_Pointer=&v_big_3[0]; break;
      case '4': Font_Pointer=&v_big_4[0]; break;
      case '5': Font_Pointer=&v_big_5[0]; break;
      case '6': Font_Pointer=&v_big_6[0]; break;
      case '7': Font_Pointer=&v_big_7[0]; break;
      case '8': Font_Pointer=&v_big_8[0]; break;
      case '9': Font_Pointer=&v_big_9[0]; break;
      default: Font_Pointer=&v_big_space[0];
      }

    //Write the twelve columns of this character.
    for(column=0;column<=11;column++)
      {
      //Clear the correct bits in this row and the next two rows down.
      for(row=0;row<=row_limit;row++)
        LCD_Memory[(uint16_t)row<<7]&=Clearing_Mask.as_bytes[row];
      //Get the font data, convert it to a word and shift it down. Leave
      //one blank row of pixels above as a spacer.
      Pixel_Data.as_dword=((uint32_t)(pgm_read_word_near(Font_Pointer++)))<<(y&0x07);
      //Set the correct bits in this row and the next row down.
      for(row=0;row<=row_limit;row++)
        LCD_Memory[(uint16_t)row<<7]|=Pixel_Data.as_bytes[row];
      LCD_Memory++;
      }
    }
  }
*/  
//============================================================================
/*
void put_big_number_h(uint8_t x, uint8_t y, uint8_t Field_Width, const char *input)
  {
  register uint8_t
    Terminator_Found;
  register uint8_t
    Characters_Placed;
  register uint8_t
    this_character;
  register uint8_t
    row;
  register uint8_t
    row_limit;
  register uint8_t
    *LCD_Memory;
  register const uint16_t
    *Font_Pointer;
  register uint8_t
    column;
  DWORD_UNION
    Clearing_Mask;
  DWORD_UNION
    Pixel_Data;


//  //Clear the first col to the left of the string.
//  for(row=0;row<=row_limit;row++)
//    LCD_Memory[row*128]&=Clearing_Mask.as_bytes[row];
//  LCD_Memory++;

  //Fix up y
  y-=12;


  //Initialize Terminator_Found.
  Terminator_Found=0;
  //Move across the field. We will either put the character or a blank
  //in every position of Field_Width.
  for(Characters_Placed=0;Characters_Placed<Field_Width;Characters_Placed++)
    {

    
    
  //The font is 12 pixels wide, 14 pixels high
  //We typically need to manipulate 3 rows of frame buffer data. We only
  //need to do two if we are in the bottom two rows or the top two rows
  //of the display.
  row_limit=(y>>3);

  //Calculate the address of the first byte in display in LCD_Memory
  LCD_Memory=&framebuffer[row_limit][x];

  if((row_limit<2)||(5<row_limit))
    row_limit=2;
  else
    row_limit=3;

  //Calculate Clearing_Mask, the vertical mask that we will "and" with
  //LCD_Memory to clear the space before we "or" in the data from the
  //font. It is 12 pixels.
  Clearing_Mask.as_dword=~(0x001FFFL<<(y&0x07));
    
    
    
    
    //If we have not passed the terminator, then get the next
    //character in the string. If we find the terminator,
    //remember that we are out of characters.
    if(!Terminator_Found)
      {
      this_character=*input++;
      if(!this_character)
        {
        Terminator_Found=1;
        this_character=' ';
        }
      }
    else
      this_character=' ';
    //Get a pointer into the font information for this character.
    switch(this_character)
      {
      case '+': Font_Pointer=&h_big_plus[0]; break;
      case '-': Font_Pointer=&h_big_minus[0]; break;
      case '.': Font_Pointer=&h_big_dot[0]; break;
      case '0': Font_Pointer=&h_big_0[0]; break;
      case '1': Font_Pointer=&h_big_1[0]; break;
      case '2': Font_Pointer=&h_big_2[0]; break;
      case '3': Font_Pointer=&h_big_3[0]; break;
      case '4': Font_Pointer=&h_big_4[0]; break;
      case '5': Font_Pointer=&h_big_5[0]; break;
      case '6': Font_Pointer=&h_big_6[0]; break;
      case '7': Font_Pointer=&h_big_7[0]; break;
      case '8': Font_Pointer=&h_big_8[0]; break;
      case '9': Font_Pointer=&h_big_9[0]; break;
      default: Font_Pointer=&h_big_space[0];
      }

    //Write the fourteen columns of this character.
    for(column=0;column<=13;column++)
      {
      //Clear the correct bits in this row and the next two rows down.
      for(row=0;row<=row_limit;row++)
        LCD_Memory[(uint16_t)row<<7]&=Clearing_Mask.as_bytes[row];
      //Get the font data, convert it to a word and shift it down. Leave
      //one blank row of pixels above as a spacer.
      Pixel_Data.as_dword=((uint32_t)(pgm_read_word_near(Font_Pointer++)))<<(y&0x07);
      //Set the correct bits in this row and the next row down.
      for(row=0;row<=row_limit;row++)
        LCD_Memory[(uint16_t)row<<7]|=Pixel_Data.as_bytes[row];
      LCD_Memory--;
      }

y-=12;
      
    }
  }
*/  
//============================================================================
//============================================================================
//============================================================================
//============================================================================
//============================================================================
//============================================================================
//============================================================================

  

//============================================================================
void horizontal_line(uint8_t x1, uint8_t y, uint8_t x2)
  {
  register uint8_t
    *LCD_Memory_1;
  register uint8_t
    *LCD_Memory_2;
  register uint8_t
    mask_left;
  register uint8_t
    mask_right;

  //Bail for bogus parametrers.
  if((x2<x1)||
     (FRAME_HEIGHT_PIXELS<=y)||
     (FRAME_WIDTH_PIXELS<=x1)||
     (FRAME_WIDTH_PIXELS<=x2))
    return;
  //Draw the last pixel too.
  x2++;

  //Calculate the ends.
  mask_left=  (0xFF>>(x1&0x07));
  mask_right=~(0xFF>>(x2&0x07));

  //Calculate the address of the first byte in display in LCD_Memory
  LCD_Memory_1=&framebuffer[y][x1>>3];
  //Calculate the address of the last byte in display in LCD_Memory
  LCD_Memory_2=&framebuffer[y][x2>>3];
  //If they are the same, then this is a special case.
  if(LCD_Memory_1==LCD_Memory_2)
    //The line fits in one byte.
    *LCD_Memory_1|=mask_left&mask_right;
  else
    {
    //This is the case where we need to loop over multiple
    //(possibly 0) colums. Do the left byte.
    *LCD_Memory_1|=mask_left;
    //Do the bytes in the middle.
    for(LCD_Memory_1++;LCD_Memory_1!=LCD_Memory_2;LCD_Memory_1++)
      *LCD_Memory_1=0xFF;
    //Do the right byte.
    *LCD_Memory_2|=mask_right;
    }
  }
//============================================================================
void vertical_line(uint8_t x, uint8_t y1, uint8_t y2)
  {
  register uint8_t
    *LCD_Memory;
  register uint8_t
    row;
  register uint8_t
    Set_Mask;

  //Bail for bogus parametrers.
  if((y2<y1)||
     (FRAME_WIDTH_PIXELS<=x)||
     (FRAME_HEIGHT_PIXELS<=y1)||
     (FRAME_HEIGHT_PIXELS<=y2))
    return;

  //Calculate the address of the first byte in display in LCD_Memory
  LCD_Memory=&framebuffer[y1][x>>3];

  //Calculate Set_Mask, the horizontal mask that we will or with
  //LCD_Memory to set the pixel.
  Set_Mask=0x80>>(x&0x07);

  //Move down memory, oring the mask in.
  for(row=y1;row<=y2;row++)
    {
    *LCD_Memory|=Set_Mask;
    LCD_Memory+=FRAME_WIDTH_BYTES;
    }
  }
//============================================================================
void outline_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
  {
  horizontal_line(x1+1,y1,x2-1);
  horizontal_line(x1+1,y2,x2-1);
  vertical_line(x1,y1,y2);
  vertical_line(x2,y1,y2);
  }
//============================================================================
void clear_rectangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
  {
  register uint8_t
    mask_left;
  register uint8_t
    mask_right;
  register uint8_t
    *LCD_Memory;
  register uint8_t
    *LCD_Memory_Left;
  register uint8_t
    *LCD_Memory_Right;
  register uint8_t
    row;

  //Bail for bogus parametrers. 
  if((x2<x1)||
     (y2<y1)||
     (FRAME_WIDTH_PIXELS<=x1)||
     (FRAME_WIDTH_PIXELS<=x2)||
     (FRAME_HEIGHT_PIXELS<=y1)||
     (FRAME_HEIGHT_PIXELS<=y2))
    return;

  //Draw the last pixel too.
  x2++;

  //Calculate the address of the left byte in display in LCD_Memory
  LCD_Memory_Left=&framebuffer[y1][x1>>3];
  //Calculate the address of the right byte in display in LCD_Memory
  LCD_Memory_Right=&framebuffer[y1][x2>>3];

  //Calculate the ends outside the loop.
  mask_left=~(0xFF>>(x1&0x07));
  mask_right=(0xFF>>(x2&0x07));

  //If they are the same, then this is a special case.
  if(LCD_Memory_Left==LCD_Memory_Right)
    {
    //The rectangle fits in one column of display memory. Calculate a
    //new clearing mask that is common between the right and left.
    mask_left=mask_right|mask_left;

    for(row=y1;row<=y2;row++)
      {
      *LCD_Memory_Left&=mask_left;
      LCD_Memory_Left+=FRAME_WIDTH_BYTES;
      }
    }
  else
    {
    //Loop over the rows.
    for(row=y1;row<=y2;row++)
      {
      LCD_Memory=LCD_Memory_Left;
      //This is the case where we need to loop over multiple
      //(possibly 0) columns. Do the left byte.
      *LCD_Memory&=mask_left;
      //Do the bytes in the middle.
      for(LCD_Memory++;LCD_Memory!=LCD_Memory_Right;LCD_Memory++)
        *LCD_Memory=0x00;
      //Do the right byte.
      *LCD_Memory_Right&=mask_right;
      //Move to the next row
      LCD_Memory_Left+=FRAME_WIDTH_BYTES;
      LCD_Memory_Right+=FRAME_WIDTH_BYTES;
      }
    }
  }
//============================================================================
void Draw_Bitmap(uint8_t x,
                 uint8_t y,
                 uint8_t x_size,
                 uint8_t y_size,
                 const uint8_t *Bitmap_Base_Pointer)
  {
  WORD_UNION
    Bitmap_Data;
  register uint8_t
    low_three_bits_of_x;
  register uint8_t
    *LCD_Memory;
  register uint8_t
    *LCD_Memory_Right;
  register uint8_t
    *LCD_Memory_Left;
  register uint8_t
    mask_left;
  register uint8_t
    mask_right;
  register uint8_t
    row;
  register const uint8_t
    *Bitmap_Pointer;
  register uint8_t
    Bitmap_Line_Increment;

  uint8_t
    x2;
  uint8_t
    low_three_bits_of_x2;

  //Bail for garbage in.
  if(((FRAME_WIDTH_PIXELS-x_size)<x)||
     ((FRAME_HEIGHT_PIXELS-y_size)<y))
    return;

  //Calculate the address of the upper left byte in LCD_Memory
  LCD_Memory_Left=&framebuffer[y][x>>3];
  //Calculate the address of the upper right byte in LCD_Memory
  x2=x+x_size;
  LCD_Memory_Right=&framebuffer[y][x2>>3];

  //Used a lot, Calculate once outside the loops.
  low_three_bits_of_x=x&0x07;
  low_three_bits_of_x2=x2&0x07;

  //Calculate how far to skip forward in the bitmap every line.
  Bitmap_Line_Increment=(x_size>>3);
  if(x_size&0x07)
    Bitmap_Line_Increment++;

  //Calculate the ends outside the loops.
  mask_left=~(0xFF>>(low_three_bits_of_x));
  mask_right=(0xFF>>(low_three_bits_of_x2));

  //If they are the same, then this is a special case.
  if(LCD_Memory_Left==LCD_Memory_Right)
    {
    //Set the bitmap pointer to the start of this line
    Bitmap_Pointer=Bitmap_Base_Pointer;
    //The bit-blt fits in one column of display memory. Calculate a
    //new clearing mask that is common between the right and left.
    mask_left=mask_right|mask_left;

    for(row=0;row<y_size;row++)
      {
      //Clear the pixels.
      *LCD_Memory_Left&=mask_left;
      //Or in the Bitmap data, and move to the next line.
      //The bitmap data can only be one byte wide, since it does
      //not cross a byte boundary in display memory.
      *LCD_Memory_Left|=
        (pgm_read_byte(Bitmap_Pointer++)>>(low_three_bits_of_x))&(~mask_left);
      //Move to the next line in display memory.
      LCD_Memory_Left+=30;
      }
    }
  else
    {
    //This is the case where the bitmap spans at least two (or more) columns
    //of display memory.
    //Loop over the rows.
    for(row=0;row<y_size;row++)
      {
      //Set the bitmap pointer to the start of this line
      Bitmap_Pointer=Bitmap_Base_Pointer;
      //Set the display pointer to the start of this line.
      LCD_Memory=LCD_Memory_Left;
      //Do the left-most column, clear first then set.
      *LCD_Memory&=mask_left;
      *LCD_Memory++|=pgm_read_byte(Bitmap_Pointer)>>(low_three_bits_of_x);

      //Now we need to loop over multiple (possibly 0) 8-bit wide columns.
      while(LCD_Memory!=LCD_Memory_Right)
        {
        //Need to reverse "endian" to get word shift to work correctly.
        Bitmap_Data.as_bytes[1]=pgm_read_byte(Bitmap_Pointer++);
        Bitmap_Data.as_bytes[0]=pgm_read_byte(Bitmap_Pointer);
        //Slide it over as a word
        Bitmap_Data.as_word>>=(low_three_bits_of_x);
        //Push it into the display memory.
        *LCD_Memory++=Bitmap_Data.as_bytes[0];
        }

      //Do the right byte. Clear the pixels first.
      *LCD_Memory_Right&=mask_right;
      //Now calculate which bits should be set.

      //Need to reverse "endian" to get word shift to work correctly.
      Bitmap_Data.as_bytes[1]=pgm_read_byte(Bitmap_Pointer++);
      Bitmap_Data.as_bytes[0]=pgm_read_byte(Bitmap_Pointer);
      //Slide it over as a word
      Bitmap_Data.as_word>>=(low_three_bits_of_x);
      //Put that into display memory, observing the mask.
      *LCD_Memory_Right|=Bitmap_Data.as_bytes[0]&(~mask_right);

      //Move to the next row of display memory
      LCD_Memory_Left+=FRAME_WIDTH_BYTES;
      LCD_Memory_Right+=FRAME_WIDTH_BYTES;
      //Move to the next line in the bitmap
      Bitmap_Base_Pointer+=Bitmap_Line_Increment;
      }
    }
  }
//============================================================================
// Only '0'-'9', '.', '+', '-', and ' ' will be displayed. The font is
// 12 x 16.
void put_big_number(uint8_t x, uint8_t y, uint8_t Field_Width, char *input)
  {
  register uint8_t
    Terminator_Found;
  register uint8_t
    Characters_Placed;
  register uint8_t
    this_character;

  //Initialize Terminator_Found.
  Terminator_Found=0;
  //Move across the field. We will either put the character or a blank
  //in every position of Field_Width.
  for(Characters_Placed=0;Characters_Placed<Field_Width;Characters_Placed++)
    {
    //If we have not passed the terminator, then get the next
    //character in the string. If we find the terminator,
    //remember that we are out of characters.
    if(!Terminator_Found)
      {
      this_character=*input++;
      if(!this_character)
        {
        Terminator_Found=1;
        this_character=' ';
        }
      }
    else
      this_character=' ';

    //Print the character. Draw_Bitmap does clipping.
    switch(this_character)
      {
      case '+': Draw_Bitmap(x,y,12,16,big_plus); break;
      case '-': Draw_Bitmap(x,y,12,16,big_minus); break;
      case '.': Draw_Bitmap(x,y,12,16,big_dot); break;
      case '0': Draw_Bitmap(x,y,12,16,big_0); break;
      case '1': Draw_Bitmap(x,y,12,16,big_1); break;
      case '2': Draw_Bitmap(x,y,12,16,big_2); break;
      case '3': Draw_Bitmap(x,y,12,16,big_3); break;
      case '4': Draw_Bitmap(x,y,12,16,big_4); break;
      case '5': Draw_Bitmap(x,y,12,16,big_5); break;
      case '6': Draw_Bitmap(x,y,12,16,big_6); break;
      case '7': Draw_Bitmap(x,y,12,16,big_7); break;
      case '8': Draw_Bitmap(x,y,12,16,big_8); break;
      case '9': Draw_Bitmap(x,y,12,16,big_9); break;
      default: Draw_Bitmap(x,y,12,16,big_space);
      }

    //Move over one character.
    x+=12;
    }
  }
//============================================================================
  
//============================================================================
