#include "lt3746.h"
#include "stm32f10x.h"
#include <string.h>

#define EVER (;;)

#include "delay.h"

int main()
{
  struct LT3746_Frame red, green, blue;
  int i, red_data = 4095, green_data = 4095, blue_data = 0, up = 0, hue = 0;
  
  Delay_Init();
  LT3746_Init(LT3746_MODE_3_WIRE);
  
  red.frame_type = FRAME_TYPE_GRAYSCALE;
  red.led_channels_enable = LED_CHANNELS_ENABLE;
  green.frame_type = FRAME_TYPE_GRAYSCALE;
  green.led_channels_enable = LED_CHANNELS_ENABLE;
  blue.frame_type = FRAME_TYPE_GRAYSCALE;
  blue.led_channels_enable = LED_CHANNELS_ENABLE;
  
  for (i = 0; i < 32; i++)
  {
    red.led_data[i] = red_data;
  }
  for (i = 0; i < 32; i++)
  {
    green.led_data[i] = green_data;
  }
  for (i = 0; i < 32; i++)
  {
    blue.led_data[i] = blue_data;
  }
  
  LT3746_Transfer(&blue, 0);
  LT3746_Transfer(&green, 0);
  LT3746_Transfer(&red, 1);
  
  for EVER
  {
        
    if (up)
    {
      if (hue == 0)
      {
        red_data += 1;
        if (red_data >= 4095)
        {
          up = 0;
          hue = 2;
        }
      }
      else if (hue == 1)
      {
        green_data += 1;
        if (green_data >= 4095)
        {
          up = 0;
          hue = 0;
        }
      }
      else if (hue == 2)
      {
        blue_data += 1;
        if (blue_data >= 4095)
        {
          up = 0;
          hue = 1;
        }
      }
    }
    else
    {
      if (hue == 0)
      {
        red_data -= 1;
        if (red_data <= 0)
        {
          up = 1;
          hue = 2;
        }
      }
      else if (hue == 1)
      {
        green_data -= 1;
        if (green_data <= 0)
        {
          up = 1;
          hue = 0;
        }
      }
      else if (hue == 2)
      {
        blue_data -= 1;
        if (blue_data <= 0)
        {
          up = 1;
          hue = 1;
        }
      }
    }
    
    for (i = 0; i < 32; i++)
    {
      red.led_data[i] = red_data;
    }
    for (i = 0; i < 32; i++)
    {
      green.led_data[i] = green_data;
    }
    for (i = 0; i < 32; i++)
    {
      blue.led_data[i] = blue_data;
    }
    
    LT3746_Transfer(&blue, 0);
    LT3746_Transfer(&green, 0);
    LT3746_Transfer(&red, 1);
    
//    Delay(1);
  }
}