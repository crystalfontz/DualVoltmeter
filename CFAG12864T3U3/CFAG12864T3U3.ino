//===========================================================================
//
//  !! IMPORTANT !!
//
//  The CFAG12864T3 and CFAG12864U3 are a 3.3v devices. You need a 3.3v
//  Arduino to operate this code properly. We used a seeedunio v4.2 set
//  to 3.3v:
//
//  http://www.seeedstudio.com/item_detail.html?p_id=2517
//
//===========================================================================
//
// Code written for Seeeduino v4.2 running at 3.3v
//
// CFAG12864T3 CFAG12864U3 128x64 LCD in SPI mode
//
//  ref: https://www.crystalfontz.com/product/CFAG12864T3
//
//    Small, backlit:      https://www.crystalfontz.com/product/cfag12864t3tfh
//    Small, no backlight: https://www.crystalfontz.com/product/cfag12864t3nfh
//    Large, backlit:      https://www.crystalfontz.com/product/cfag12864u3tfh
//    Large, no backlight: https://www.crystalfontz.com/product/cfag12864u3nfh
//
//
// 2017 - 06 - 15 Brent A. Crosby
// 2019 - 05 - 01 Trevin Jorgenson
// Crystalfontz America, Inc. http://www.crystalfontz.com
//===========================================================================
// This code can also be found on GitHub:
//
//===========================================================================
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
//
//  For Pin Connection information, see "LCD_low.h"
//
//===========================================================================
// In order to have enough RAM for data buffering I reduced
// the I2C buffers to 8 bytes from the default of 32. Since
// there are five (!) buffers this reduces RAM usage by
// 120 bytes.

#include <Arduino.h>
#include <SPI.h>
#include "Wire.h"
#include "Adafruit_ADS1X15.h"

#include "utilities.h"
#include "LCD_low.h"
#include "LCD_draw.h"

//============================================================================
//Declare the ADC object
Adafruit_ADS1115 ads;
//============================================================================
// ref: https://www.robotshop.com/community/forum/t/arduino-101-timers-and-interrupts/13072
volatile uint8_t
  _Time_To_Update_Display=0;

#define READ_2_3_START_0_1 (0)
#define READ_0_1_START_2_3 (1)
volatile uint8_t
  _ISR_State_=READ_2_3_START_0_1;

volatile uint16_t
  _Channel_0_1_Latest_half_mV_Sample=0;
volatile uint16_t
  _Channel_2_3_Latest_half_mV_Sample=0;

// timer compare interrupt service routine
// Entire ISR is ~ 1.9ms
// Doing slow I/O like I2C in the ISR is not ecactly
// The best practice. In this simple application we
// can get away with it.
ISR(TIMER1_COMPA_vect)
  {
  //SET_DEBUG_PORT;
  if(READ_2_3_START_0_1 == _ISR_State_)
    {
    //READ_2_3_START_0_1
    //The ADC / I2C code needs interrupts on
    interrupts();
    //Save the 2:3 result
    // ~ 575uS
    _Channel_2_3_Latest_half_mV_Sample = ads.getLastConversionResults();
    //Start channel 1:0 differential conversion. The actual conversion
    //portion takes ~17.56mS
    //1.3mS
    ads.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, false);
    noInterrupts();
    _ISR_State_=READ_0_1_START_2_3;
    }
  else
    {
    //READ_0_1_START_2_3
    //The ADC / I2C code needs interrupts on
    interrupts();
    //Save the 0:1 result
    // ~ 575uS
    _Channel_0_1_Latest_half_mV_Sample = ads.getLastConversionResults();
    //Start channel 2:3 differential conversion. The actual conversion
    //portion takes ~17.56mS
    //1.3mS
    ads.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_2_3, false);
    noInterrupts();
    _ISR_State_=READ_2_3_START_0_1;

    //Let the foreground know that a set of samples is ready
    _Time_To_Update_Display=1;  
    }
  //CLR_DEBUG_PORT;
  }
//============================================================================
// ~3.95mS flat line, ~4.1mS 20 sec sine ~5.1mS all black
void Strip_Chart(uint8_t y1,
                 uint8_t y2,
                 int16_t data_min,
                 int16_t data_max,
                 uint8_t samples,
                 int16_t *min_history,
                 int16_t *max_history)
  {
  uint8_t
    i;
   
  for(i=0;i<samples;i++)
    {
      
    //Calculate the min/bottom of the bar in pixels
    //Calcs take 875nS
    int16_t
      min_half_delta;
    min_half_delta = min_history[i] - data_min;
    
    //Convert from (data_max-data_min) in 16th mV land to (y2-y1) land in pixels
    min_half_delta=((int32_t)min_half_delta*(int32_t)(y2-y1))/((int32_t)(data_max-data_min));
    
    int16_t
      max_half_delta;
    max_half_delta = max_history[i] - data_min;
    //Convert from (data_max-data_min) land (16th mV) to (y2-y1) land (pixels)
    max_half_delta=((int32_t)max_half_delta*(int32_t)(y2-y1))/((int32_t)(data_max-data_min));

    // ~80uS to 110uS
    vertical_line(i,y1+min_half_delta, y1+max_half_delta);
    }
  }
//============================================================================
int16_t Hysteresis(int16_t &current,
                   int16_t &previous,
                   int16_t dead_zone_ticks)
  {
  if(current < previous)
    {
    //Going down. Is it big enough to make us change?
    if(dead_zone_ticks <= (previous-current))
      {
      //Larger change, move but at the top of the dead band
      current+=dead_zone_ticks;
      //remember our new base line
      previous=current;
      }
    else
      {
      //Small change, do not move.
      current=previous;
      }
    }
  else
    {
    //Going up. Is it big enough to make us change?
    if(dead_zone_ticks <= (current-previous))
      {
      //Larger change, move but at the bottom of the dead band
      current-=dead_zone_ticks;
      //remember our new base line
      previous=current;
      }
    else
      {
      //Small change, do not move.
      current=previous;
      }
    }
  }
//============================================================================
// 50mS per sample = 20Hz sample rate
//   120 samples per bin makes 6 seconds per bin
//   47 bins * 6 seconds = 282 seconds = 4:42 m:s
// * 40 samples per bin makes 2 second per bin
// * 47 bins * 2 seconds =  94 seconds = 1:34 m:s
//   20 samples per bin makes 1 second per bin
//   47 bins * 1 seconds =  47 seconds = 0:47 m:s
#define SAMPLES_PER_BIN (40)
#define HISTORY_DEPTH   (47)
uint8_t
  history_is_initialized;
uint8_t
  sample_count;
uint8_t
  i;

#define HYSTERESIS_TICKS_HALF_MV   (6)  // so is +- 3mV

int16_t
  Channel_0_1_half_mV;
int16_t
  Channel_0_1_half_mV_current;
int16_t
  Channel_0_1_half_mV_previous;
int16_t
  Channel_0_1_mV_display;
//int16_t
//  Channel_0_1_half_mV_min;
//int16_t
//  Channel_0_1_half_mV_max;
int16_t
  Channel_0_1_half_mV_min_history[HISTORY_DEPTH];
int16_t
  Channel_0_1_half_mV_max_history[HISTORY_DEPTH];

int16_t
  Channel_2_3_half_mV;
int16_t
  Channel_2_3_half_mV_current;
int16_t
  Channel_2_3_half_mV_previous;
int16_t
  Channel_2_3_mV_display;
//int16_t
//  Channel_2_3_half_mV_min;
//int16_t
//  Channel_2_3_half_mV_max;
int16_t
  Channel_2_3_half_mV_min_history[HISTORY_DEPTH];
int16_t
  Channel_2_3_half_mV_max_history[HISTORY_DEPTH];
//============================================================================

void setup(void)
  {
  // Optional pins used for LED or oscilloscope debugging.
  pinMode(DEBUG_PORT, OUTPUT);
  pinMode(DEBUG_LED, OUTPUT);

  DDRB |= 0x2F;

  //Drive the ports to a reasonable starting state.
  CLR_RESET;
  CLR_RS;
  SET_CS;
  CLR_MOSI;
  CLR_SCK;

  // Set up Timer 1 to hit every 25mS = 40Hz
  // ref: https://www.robotshop.com/community/forum/t/arduino-101-timers-and-interrupts/13072
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 1562;             // compare match register 16MHz/256/40Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts

#if (DEBUG_LEVEL != DEBUG_NONE)    
  // Initialize UART for debugging messages
  Serial.begin(115200);
#endif // (DEBUG_LEVEL != DEBUG_NONE)
  DBG_STAT("void setup(void)\n");

  // Initialize SPI. By default the clock is 4MHz.
  SPI.begin();
  //Bump the clock to 8MHz. Appears to be the maximum.
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  //Fire up the SPI LCD
  DBG_STAT("Initialize_CFAG12864T3U3()\n");
  Initialize_CFAG12864T2U2();

  //
  // We have 1 4:1 input divider (15.000K / 5.000K)
  // Using 1x gain (4.096 max) that should give us
  // 4.096 * 4 = 16.384 volts range
  // "The ADS111x analog inputs can withstand continuous
  //  currents as large as 10 mA."
  // To drive 10mA through the 15K would take 150v, so
  // we should be safe for likely transients.
  //
  // Our bit size should be 0.125mV times 4 for the
  // resistor divider = 0.5mV per bit
  //
  // At maximum input of 16.384 volts, the count should
  // be +- 32768 ticks so it should fit in an int16_t
  //

  // Adafruit
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV (1/16 mV)
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  ads.setGain(GAIN_ONE);

  // RATE_ADS1115_8SPS (0x0000)   ///< 8 samples per second 127.1mS for conversion
  // RATE_ADS1115_16SPS (0x0020)  ///< 16 samples per second 64.52mS
  // RATE_ADS1115_32SPS (0x0040)  ///< 32 samples per second 33.21mS
  // RATE_ADS1115_64SPS (0x0060)  ///< 64 samples per second 17.56mS
  // RATE_ADS1115_128SPS (0x0080) ///< 128 samples per second 9.44mS (default)
  // RATE_ADS1115_250SPS (0x00A0) ///< 250 samples per second 5.38mS
  // RATE_ADS1115_475SPS (0x00C0) ///< 475 samples per second 3.65mS
  // RATE_ADS1115_860SPS (0x00E0) ///< 860 samples per second 3.06mS
  ads.setDataRate(RATE_ADS1115_64SPS);

  Fill_Framebuffer(0,0);

  if (!ads.begin())
    {
    Serial.println("Failed to initialize ADS.");
    put_string_P(0,  //X
                 100, //Y
                 8,  //string length
                 F("ADS Bad"));
    Send_Framebuffer_To_Display();
    while(1);
    }

  put_string_P(0,  //X
               100, //Y
               8,  //string length
               F("ADS OK"));

  Send_Framebuffer_To_Display();

  // Get things initialized
  // (in theory C takes care of this)
  sample_count=0;
  Channel_0_1_half_mV=0;
  Channel_0_1_half_mV_current=0;
  Channel_0_1_half_mV_previous=0;
  Channel_0_1_mV_display=0;

  Channel_2_3_half_mV=0;
  Channel_2_3_half_mV_current=0;
  Channel_2_3_half_mV_previous=0;
  Channel_2_3_mV_display=0;

  history_is_initialized=0;

  DBG_STAT("Setup complete, about to loop.\n");

  //Spin here with interrupts on to load the initial ADC
  //readings.
  delay(100);
  }
//============================================================================
void loop(void)
  {
  //Spin here until the interrupt has read two new samples
  while(0==_Time_To_Update_Display)
    {
    };
  //The ISR just set _Time_To_Update_Display, so 50mS has passed.
  //This flag will get set again in another 50mS
  //SET_DEBUG_LED;
  _Time_To_Update_Display=0;

  //Get the most recent ADC readings from the ISR
  noInterrupts();
  Channel_0_1_half_mV=_Channel_0_1_Latest_half_mV_Sample;
  Channel_2_3_half_mV=_Channel_2_3_Latest_half_mV_Sample;
  interrupts();

  //Get the screen background loaded
  //~ 583uS
  logo_to_framebuffer();

  //If the history has not been initialized, use this first
  //reading to initialize the history.
  if(0==history_is_initialized)
    {
    //The history
    for(i=0;i<HISTORY_DEPTH;i++)
      {
      Channel_0_1_half_mV_min_history[i]=Channel_0_1_half_mV;
      Channel_0_1_half_mV_max_history[i]=Channel_0_1_half_mV;
      Channel_2_3_half_mV_min_history[i]=Channel_2_3_half_mV;
      Channel_2_3_half_mV_max_history[i]=Channel_2_3_half_mV;
      }
    //And the hysteresis varaibles
    Channel_0_1_half_mV_previous=Channel_0_1_half_mV;
    Channel_2_3_half_mV_previous=Channel_2_3_half_mV;
    //Do not to this initiaization again.
    history_is_initialized=1;
    }

  //Update the max and min for the leading edge "live" bin
  // ~ 3uS
  if(Channel_0_1_half_mV < Channel_0_1_half_mV_min_history[HISTORY_DEPTH-1])
    {
    Channel_0_1_half_mV_min_history[HISTORY_DEPTH-1]=Channel_0_1_half_mV;
    }
  if(Channel_0_1_half_mV_max_history[HISTORY_DEPTH-1] < Channel_0_1_half_mV)
    {
    Channel_0_1_half_mV_max_history[HISTORY_DEPTH-1]=Channel_0_1_half_mV;
    }
  if(Channel_2_3_half_mV < Channel_2_3_half_mV_min_history[HISTORY_DEPTH-1])
    {
    Channel_2_3_half_mV_min_history[HISTORY_DEPTH-1]=Channel_2_3_half_mV;
    }
  if(Channel_2_3_half_mV_max_history[HISTORY_DEPTH-1] < Channel_2_3_half_mV)
    {
    Channel_2_3_half_mV_max_history[HISTORY_DEPTH-1]=Channel_2_3_half_mV;
    }

  //Channel 0_1 hysteresis
  // ~ 1.9uS
  Channel_0_1_half_mV_current=Channel_0_1_half_mV;
  Hysteresis(Channel_0_1_half_mV_current,
             Channel_0_1_half_mV_previous,
             HYSTERESIS_TICKS_HALF_MV);
  //Channel 2_3 hysteresis
  // ~ 1.9uS
  Channel_2_3_half_mV_current=Channel_2_3_half_mV;
  Hysteresis(Channel_2_3_half_mV_current,
             Channel_2_3_half_mV_previous,
             HYSTERESIS_TICKS_HALF_MV);

  // We are using an ADS1115 with GAIN_ONE, so each bit represents
  // 0.125mV. We have a divide by 4 resistive divider, so each bit
  // is 0.5mV It turns out that a simple right shift by 1 (/2) gets
  // us there. Adding 1 implements rounding instead of truncation.
  Channel_0_1_mV_display = (Channel_0_1_half_mV_current+1)>>1;

//Non hysterisis for testing. Fix after testing
  Channel_2_3_mV_display = (Channel_2_3_half_mV_current+1)>>1;
//  Channel_2_3_mV_display = (Channel_2_3_half_mV+1)>>1;

  //Scan through the history, find the graph min and max
  // ~ 138.9uS using stack vars (166uS using static (!))
  int16_t
    Channel_0_1_half_mV_display_min;
  int16_t
    Channel_0_1_half_mV_display_max;
  int16_t
    Channel_2_3_half_mV_display_min;
  int16_t
    Channel_2_3_half_mV_display_max;

  Channel_0_1_half_mV_display_min=+32767;
  Channel_0_1_half_mV_display_max=-32768;
  Channel_2_3_half_mV_display_min=+32767;
  Channel_2_3_half_mV_display_max=-32768;

  for(i=0;i<HISTORY_DEPTH;i++)
    {
    if(Channel_0_1_half_mV_min_history[i] < Channel_0_1_half_mV_display_min)
      {
      Channel_0_1_half_mV_display_min = Channel_0_1_half_mV_min_history[i];
      }
    if(Channel_0_1_half_mV_display_max < Channel_0_1_half_mV_max_history[i])
      {
      Channel_0_1_half_mV_display_max = Channel_0_1_half_mV_max_history[i];
      }
    if(Channel_2_3_half_mV_min_history[i] < Channel_2_3_half_mV_display_min)
      {
      Channel_2_3_half_mV_display_min = Channel_2_3_half_mV_min_history[i];
      }
    if(Channel_2_3_half_mV_display_max < Channel_2_3_half_mV_max_history[i])
      {
      Channel_2_3_half_mV_display_max = Channel_2_3_half_mV_max_history[i];
      }
    }

  //Screen layout:
  //  Vehicle Voltage:
  //    Units:        0,104 : 2
  //    Thousandths: 28,104 : 3
  //    Graph Max: 47,95 : 2
  //    Graph Mid: 47,80 : 2
  //    Graph Min: 47,65,: 2
  //    Strip chart: 65,103
  //
  //  Camper Voltage:
  //    Units:        0,39 : 2
  //    Thousandths: 28,39 : 3
  //    Graph Max: 47,30 : 2
  //    Graph Mid: 47,15 : 2
  //    Graph Min: 47,0,: 2
  //    Strip chart: 0,38

  // Channel 1_0: Calculate upper, mid, and lower graph
  // limits - nearest integer volts
  // ~80uS
  char
    v_string[3];
  char
    mv_string[4];
  int16_t
    graph_max;
  int16_t
    graph_min;

  //Get the max in volts, rounded "up"
  if(0 < Channel_0_1_half_mV_display_max)
    {
    //Positive - straight forward
    graph_max= ((int32_t)Channel_0_1_half_mV_display_max+(int32_t)1999)/((int32_t)2000);
    }
  else
    {
    //Negative - need to fiddle the sign to round "up" to the next smaller negative number
    graph_max=-((int32_t)(-Channel_0_1_half_mV_display_max))/((int32_t)2000);
    }

  //Get the min in volts, rounded "down"
  if(0 < Channel_0_1_half_mV_display_min)
    {
    //Positive - straight forward
    graph_min= ((int32_t)Channel_0_1_half_mV_display_min)/((int32_t)2000);
    }
  else
    {
    //Negative - need to fiddle the sign to round "down" to the next larger negative number
    graph_min=-((int32_t)(-Channel_0_1_half_mV_display_min)+(int32_t)1999)/((int32_t)2000);
    }

  //The spread needs to be at least 1v. If it is even, there can be a mid marker.
  int16_t
    scale_span;

  scale_span=graph_max-graph_min;
  if(scale_span < 1)
    {
    graph_max++;
    }
  //If the scale spread is even, then we can have a mid scale
  if(0 == (scale_span & 0x01))
    {
    Scale_To_String((graph_max+graph_min)>>1,
                    v_string);
    put_string(47, //X
               80, //Y
               2,  //string length
               v_string);
    }
    
  // Print upper and lower graph limits
  // ~ 280uS
  Scale_To_String(graph_max,
                  v_string);
  put_string(47, //X
             95, //Y
             2,  //string length
             v_string);

  Scale_To_String(graph_min,
                  v_string);
  put_string(47, //X
             65, //Y
             2,  //string length
             v_string);

  // Chart 1_0 data in history based on upper and lower limits
  // ~3.95mS flat line, ~4.1mS 20 sec sine ~5.1mS all black
  Strip_Chart(65,  //Y1
              103, //Y2
              graph_min*2000,
              graph_max*2000,
              HISTORY_DEPTH,
              Channel_0_1_half_mV_min_history,
              Channel_0_1_half_mV_max_history);

  //Channel 1_0 / Vehicle Voltage:
  // ~770uSmS for string conversion and display
  Voltage_To_Strings(Channel_0_1_mV_display,
                     v_string,
                     mv_string);
  put_big_number( 0,104,2,v_string);
  put_big_number(28,104,3,mv_string);    


  // Channel 2_3: Calculate upper, mid, and lower graph limits - nearest volts
  //Get the max in volts, rounded "up"
  if(0 < Channel_2_3_half_mV_display_max)
    {
    //Positive - straight forward
    graph_max= ((int32_t)Channel_2_3_half_mV_display_max+(int32_t)1999)/((int32_t)2000);
    }
  else
    {
    //Negative - need to fiddle the sign to round "up" to the next smaller negative number
    graph_max=-((int32_t)(-Channel_2_3_half_mV_display_max))/((int32_t)2000);
    }

  //Get the min in volts, rounded "down"
  if(0 < Channel_2_3_half_mV_display_min)
    {
    //Positive - straight forward
    graph_min= ((int32_t)Channel_2_3_half_mV_display_min)/((int32_t)2000);
    }
  else
    {
    //Negative - need to fiddle the sign to round "down" to the next larger negative number
    graph_min=-((int32_t)(-Channel_2_3_half_mV_display_min)+(int32_t)1999)/((int32_t)2000);
    }

  //The spread needs to be at least 1v. If it is even, there can be a mid marker.
  scale_span=graph_max-graph_min;
  if(scale_span < 1)
    {
    graph_max++;
    }
  //If the scale spread is even, then we can have a mid scale
  if(0 == (scale_span & 0x01))
    {
    Scale_To_String((graph_max+graph_min)>>1,
                    v_string);
    put_string(47, //X
               15, //Y
               2,  //string length
               v_string);
    }
    
  // Print upper and lower graph limits
  Scale_To_String(graph_max,
                  v_string);
  put_string(47, //X
             30, //Y
             2,  //string length
             v_string);

  Scale_To_String(graph_min,
                  v_string);
  put_string(47, //X
             0, //Y
             2,  //string length
             v_string);
  // Chart 2_3 data in history based on upper and lower limits
  // ~3.95mS flat line, ~4.1mS 20 sec sine ~5.1mS all black
  Strip_Chart(0,  //Y1
              38, //Y2
              graph_min*2000,
              graph_max*2000,
              HISTORY_DEPTH,
              Channel_2_3_half_mV_min_history,
              Channel_2_3_half_mV_max_history);

  //Channel 2_3 / Camper Voltage:
  // ~770uSmS for string conversion and display
  Voltage_To_Strings(Channel_2_3_mV_display,
                     v_string,
                     mv_string);
  put_big_number( 0,39,2,v_string);
  put_big_number(28,39,3,mv_string);

  //Everything is rendered to the frame buffer, send it to the
  //physical LCD
  //~2.7mS
  Send_Framebuffer_To_Display();

  //That is all the work for one sample. Move to the next sample.
  // ~625nS skipped, 221uS taken
  sample_count++;
  //Check if it is time to scroll the graph by one bin.
  if(SAMPLES_PER_BIN <= sample_count)
    {
    // ~ 202uS (47 bins)
    sample_count=0;

    //Shift the history down
    for(i=1;i<HISTORY_DEPTH;i++)
      {
      Channel_0_1_half_mV_min_history[i-1]=Channel_0_1_half_mV_min_history[i];
      Channel_0_1_half_mV_max_history[i-1]=Channel_0_1_half_mV_max_history[i];
      Channel_2_3_half_mV_min_history[i-1]=Channel_2_3_half_mV_min_history[i];
      Channel_2_3_half_mV_max_history[i-1]=Channel_2_3_half_mV_max_history[i];
      }
    
    //Reset the leading live bin
    Channel_0_1_half_mV_min_history[HISTORY_DEPTH-1]=+32767;
    Channel_0_1_half_mV_max_history[HISTORY_DEPTH-1]=-32768;
    Channel_2_3_half_mV_min_history[HISTORY_DEPTH-1]=+32767;
    Channel_2_3_half_mV_max_history[HISTORY_DEPTH-1]=-32768;
    }
  //CLR_DEBUG_LED;
  }
//============================================================================
