#include "lt3746.h"
#include "stm32f10x.h"


// GPIO define
#define LT3746_LDO_PORT
#define LT3746_LDO_PIN
#define LT3746_LDO_GPIO_CLK
#define LT3746_SCKO_PORT
#define LT3746_SCKO_PIN
#define LT3746_SCKO_GPIO_CLK
#define LT3746_SDO_PORT
#define LT3746_SDO_PIN
#define LT3746_SDO_GPIO_CLK
#define LT3746_LDO_LOW    {LT3746_LDO_PORT->BSRR = LT3746_LDO_PIN;}
#define LT3746_LDO_HIGH   {LT3746_LDO_PORT->BRR = LT3746_LDO_PIN;}
#define LT3746_SCKO_LOW   {LT3746_LDO_PORT->BSRR = LT3746_LDO_PIN;}
#define LT3746_SCKO_HIGH  {LT3746_LDO_PORT->BRR = LT3746_LDO_PIN;}
#define LT3746_SDO_LOW    {LT3746_LDO_PORT->BSRR = LT3746_LDO_PIN;}
#define LT3746_SDO_HIGH   {LT3746_LDO_PORT->BRR = LT3746_LDO_PIN;}

#define LT3746_LDI_PORT
#define LT3746_LDI_PIN
#define LT3746_LDI_GPIO_CLK
#define LT3746_SCKI_PORT
#define LT3746_SCKI_PIN
#define LT3746_SCKI_GPIO_CLK
#define LT3746_SDI_PORT
#define LT3746_SDI_PIN
#define LT3746_SDI_GPIO_CLK
#define LT3746_LDI_IS_LOW   ((GPIOx->ODR & GPIO_Pin) == (uint32_t)Bit_RESET)
#define LT3746_LDI_IS_HIGH  ((GPIOx->ODR & GPIO_Pin) != (uint32_t)Bit_RESET)
#define LT3746_SCKI_IS_LOW  ((GPIOx->ODR & GPIO_Pin) == (uint32_t)Bit_RESET)
#define LT3746_SCKI_IS_HIGH ((GPIOx->ODR & GPIO_Pin) != (uint32_t)Bit_RESET)
#define LT3746_SDI_IS_LOW   ((GPIOx->ODR & GPIO_Pin) == (uint32_t)Bit_RESET)
#define LT3746_SDI_IS_HIGH  ((GPIOx->ODR & GPIO_Pin) != (uint32_t)Bit_RESET)

LT3746_Mode_t __mode;

int LT3746_Init(LT3746_Mode_t mode)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  __mode = mode;
  switch (__mode)
  {
  case LT3746_MODE_6_WIRE:
    // 
    RCC_APB2PeriphClockCmd(LT3746_LDO_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SCKO_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SDO_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_LDI_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SCKI_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SDI_GPIO_CLK, ENABLE);
    
    // 
    GPIO_InitStructure.GPIO_Pin = LT3746_LDO_PIN;
    GPIO_Init(LT3746_LDO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LT3746_SCKO_PIN;
    GPIO_Init(LT3746_SCKO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LT3746_SDO_PIN;
    GPIO_Init(LT3746_SDO_PORT, &GPIO_InitStructure);
    // 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    // 
    GPIO_InitStructure.GPIO_Pin = LT3746_LDI_PIN;
    GPIO_Init(LT3746_LDI_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LT3746_SCKI_PIN;
    GPIO_Init(LT3746_SCKI_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LT3746_SDI_PIN;
    GPIO_Init(LT3746_SDI_PORT, &GPIO_InitStructure);
    break;
  case LT3746_MODE_4_WIRE:
    // 
    RCC_APB2PeriphClockCmd(LT3746_LDO_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SCKO_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SDO_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_LDI_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SCKI_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SDI_GPIO_CLK, ENABLE);
    
    // 
    GPIO_InitStructure.GPIO_Pin = LT3746_LDO_PIN;
    GPIO_Init(LT3746_LDO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LT3746_SCKO_PIN;
    GPIO_Init(LT3746_SCKO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LT3746_SDO_PIN;
    GPIO_Init(LT3746_SDO_PORT, &GPIO_InitStructure);
    // 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    // 
    GPIO_InitStructure.GPIO_Pin = LT3746_SDI_PIN;
    GPIO_Init(LT3746_SDI_PORT, &GPIO_InitStructure);
    break;
  case LT3746_MODE_3_WIRE:
    // 
    RCC_APB2PeriphClockCmd(LT3746_LDO_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SCKO_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SDO_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_LDI_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SCKI_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LT3746_SDI_GPIO_CLK, ENABLE);
    
    // 
    GPIO_InitStructure.GPIO_Pin = LT3746_LDO_PIN;
    GPIO_Init(LT3746_LDO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LT3746_SCKO_PIN;
    GPIO_Init(LT3746_SCKO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LT3746_SDO_PIN;
    GPIO_Init(LT3746_SDO_PORT, &GPIO_InitStructure);
    break;
  default:
    //
    return LT3746_Init(LT3746_MODE_6_WIRE);
    break;
  }
  return 0;
}
int LT3746_Transfer(struct LT3746_Frame * frame)
{
  int latch = 0, bit_counter = 0, i, j;
  switch (__mode)
  {
  case LT3746_MODE_6_WIRE:
    if (frame_type == FRAME_TYPE_GRAYSCALE)
    {
      for (i = 0; i < frame->led_data; i++)
      {
        for (j = 11; j >= 0; j--)
        {
          LT3746_SCKO_LOW;
          if(frame->led_data[i] & (1 << j))
          {
            LT3746_SDO_HIGH;
          }
          else
          {
            LT3746_SDO_LOW
          }
          LT3746_SCKO_HIGH;
          bit_counter++;
        }
      }
      if (frame->led_channels_enable == LED_CHANNELS_ENABLE)
      {
        LT3746_SCKO_LOW;
        LT3746_SDO_HIGH;
        LT3746_SCKO_HIGH;
      }
      else
      {
        LT3746_SCKO_LOW;
        LT3746_SDO_LOW;
        LT3746_SCKO_HIGH;
      }
      // frame select
      LT3746_SCKO_LOW;
      LT3746_SDO_LOW;
      LT3746_SCKO_HIGH;
    }
    else if (frame_type == FRAME_TYPE_DOT_CORRECTION)
    {
      for (i = 0; i < frame->dot_correction; i++)
      {
        for (j = 5; j >= 0; j--)
        {
          LT3746_SCKO_LOW;
          if(frame->dot_correction[i] & (1 << j))
          {
            LT3746_SDO_HIGH;
          }
          else
          {
            LT3746_SDO_LOW
          }
          LT3746_SCKO_HIGH;
          bit_counter++;
        }
        for (j = 5; j >= 0; j--)
        {
          // don't care bit
          LT3746_SCKO_LOW;
          LT3746_SDO_LOW
          LT3746_SCKO_HIGH;
          bit_counter++;
        }
      }
      if (frame->led_channels_enable == LED_CHANNELS_ENABLE)
      {
        LT3746_SCKO_LOW;
        LT3746_SDO_HIGH;
        LT3746_SCKO_HIGH;
      }
      else
      {
        LT3746_SCKO_LOW;
        LT3746_SDO_LOW;
        LT3746_SCKO_HIGH;
      }
      // Frame select 
      LT3746_SCKO_LOW;
      LT3746_SDO_HIGH;
      LT3746_SCKO_HIGH;
    }
    else
    {
    }
    break;
  case LT3746_MODE_4_WIRE:
    break;
  case LT3746_MODE_3_WIRE:
    break;
  default:
    break;
  }
  return 0;
}

int LT3746_GetLedStatus(struct LT3746_Frame frame, uint8_t led_number)
{
}
