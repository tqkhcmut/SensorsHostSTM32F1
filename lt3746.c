#include "lt3746.h"
#include "stm32f10x.h"

// GPIO define
#define LT3746_LDO_PORT       GPIOB
#define LT3746_LDO_PIN        GPIO_Pin_12
#define LT3746_LDO_GPIO_CLK   RCC_APB2Periph_GPIOB
#define LT3746_SCKO_PORT      GPIOB
#define LT3746_SCKO_PIN       GPIO_Pin_13
#define LT3746_SCKO_GPIO_CLK  RCC_APB2Periph_GPIOB
#define LT3746_SDO_PORT       GPIOB
#define LT3746_SDO_PIN        GPIO_Pin_14
#define LT3746_SDO_GPIO_CLK   RCC_APB2Periph_GPIOB
#define LT3746_LDO_LOW    {LT3746_LDO_PORT->BRR = LT3746_LDO_PIN;}
#define LT3746_LDO_HIGH   {LT3746_LDO_PORT->BSRR = LT3746_LDO_PIN;}
#define LT3746_SCKO_LOW   {LT3746_SCKO_PORT->BRR = LT3746_SCKO_PIN;}
#define LT3746_SCKO_HIGH  {LT3746_SCKO_PORT->BSRR = LT3746_SCKO_PIN;}
#define LT3746_SDO_LOW    {LT3746_SDO_PORT->BRR = LT3746_SDO_PIN;}
#define LT3746_SDO_HIGH   {LT3746_SDO_PORT->BSRR = LT3746_SDO_PIN;}

#define LT3746_LDI_PORT       GPIOB
#define LT3746_LDI_PIN        GPIO_Pin_15
#define LT3746_LDI_GPIO_CLK   RCC_APB2Periph_GPIOB
#define LT3746_SCKI_PORT      GPIOB
#define LT3746_SCKI_PIN       GPIO_Pin_15
#define LT3746_SCKI_GPIO_CLK  RCC_APB2Periph_GPIOB
#define LT3746_SDI_PORT       GPIOB
#define LT3746_SDI_PIN        GPIO_Pin_15
#define LT3746_SDI_GPIO_CLK   RCC_APB2Periph_GPIOB
#define LT3746_LDI_IS_LOW   ((LT3746_LDI_PORT->ODR & LT3746_LDI_PIN) == (uint32_t)Bit_RESET)
#define LT3746_LDI_IS_HIGH  ((LT3746_LDI_PORT->ODR & LT3746_LDI_PIN) != (uint32_t)Bit_RESET)
#define LT3746_SCKI_IS_LOW  ((LT3746_SCKI_PORT->ODR & LT3746_SCKI_PIN) == (uint32_t)Bit_RESET)
#define LT3746_SCKI_IS_HIGH ((LT3746_SCKI_PORT->ODR & LT3746_SCKI_PIN) != (uint32_t)Bit_RESET)
#define LT3746_SDI_IS_LOW   ((LT3746_SDI_PORT->ODR & LT3746_SDI_PIN) == (uint32_t)Bit_RESET)
#define LT3746_SDI_IS_HIGH  ((LT3746_SDI_PORT->ODR & LT3746_SDI_PIN) != (uint32_t)Bit_RESET)

LT3746_Mode_t __mode;

#if LT3746_DELAY
static inline void LT3746_delay(void)
{
  unsigned int time_us = 1;
  while (time_us--);
}
#endif

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
  LT3746_LDO_LOW;
  LT3746_SCKO_LOW;
  LT3746_SDO_LOW;
  
  return 0;
}
int LT3746_Transfer(struct LT3746_Frame * frame, char latch)
{
  int i, j;
  switch (__mode)
  {
  case LT3746_MODE_6_WIRE:
    break;
  case LT3746_MODE_4_WIRE:
    break;
  case LT3746_MODE_3_WIRE:
    if (frame->frame_type == FRAME_TYPE_GRAYSCALE)
    {
      for (i = 0; i < 32; i++)
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
#if LT3746_DELAY
          LT3746_delay();
#endif
          LT3746_SCKO_HIGH;
#if LT3746_DELAY
          LT3746_delay();
#endif
        }
      }
      if (frame->led_channels_enable == LED_CHANNELS_ENABLE)
      {
        LT3746_SCKO_LOW;
        LT3746_SDO_HIGH;
#if LT3746_DELAY
        LT3746_delay();
#endif
        LT3746_SCKO_HIGH;
#if LT3746_DELAY
        LT3746_delay();
#endif
      }
      else
      {
        LT3746_SCKO_LOW;
        LT3746_SDO_LOW;
#if LT3746_DELAY
        LT3746_delay();
#endif
        LT3746_SCKO_HIGH;
#if LT3746_DELAY
        LT3746_delay();
#endif
      }
      // frame select
      LT3746_SCKO_LOW;
      LT3746_SDO_LOW;
#if LT3746_DELAY
      LT3746_delay();
#endif
      LT3746_SCKO_HIGH;
#if LT3746_DELAY
      LT3746_delay();
#endif
      
      if (latch)
      {
        // latch
        LT3746_SCKO_LOW;
#if LT3746_DELAY
        LT3746_delay();
#endif
        LT3746_LDO_HIGH;
#if LT3746_DELAY
        LT3746_delay();
#endif
        LT3746_LDO_LOW;
#if LT3746_DELAY
        LT3746_delay();
#endif
      }
    }
    else if (frame->frame_type == FRAME_TYPE_DOT_CORRECTION)
    {
      for (i = 0; i < 32; i++)
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
#if LT3746_DELAY
          LT3746_delay();
#endif
          LT3746_SCKO_HIGH;
#if LT3746_DELAY
          LT3746_delay();
#endif
        }
        for (j = 5; j >= 0; j--)
        {
          // don't care bit
          LT3746_SCKO_LOW;
          LT3746_SDO_LOW;
#if LT3746_DELAY
          LT3746_delay();
#endif
          LT3746_SCKO_HIGH;
#if LT3746_DELAY
          LT3746_delay();
#endif
        }
      }
      if (frame->led_channels_enable == LED_CHANNELS_ENABLE)
      {
        LT3746_SCKO_LOW;
        LT3746_SDO_HIGH;
#if LT3746_DELAY
        LT3746_delay();
#endif
        LT3746_SCKO_HIGH;
#if LT3746_DELAY
        LT3746_delay();
#endif
      }
      else
      {
        LT3746_SCKO_LOW;
        LT3746_SDO_LOW;
#if LT3746_DELAY
        LT3746_delay();
#endif
        LT3746_SCKO_HIGH;
#if LT3746_DELAY
        LT3746_delay();
#endif
      }
      // frame select
      LT3746_SCKO_LOW;
      LT3746_SDO_LOW;
#if LT3746_DELAY
      LT3746_delay();
#endif
      LT3746_SCKO_HIGH;
#if LT3746_DELAY
      LT3746_delay();
#endif
      
      if (latch)
      {
        // latch
        LT3746_SCKO_LOW;
#if LT3746_DELAY
        LT3746_delay();
#endif
        LT3746_LDO_HIGH;
#if LT3746_DELAY
        LT3746_delay();
#endif
        LT3746_LDO_LOW;
#if LT3746_DELAY
        LT3746_delay();
#endif
      }
    }
    else
    {
    }
    break;
  default:
    break;
  }
  return 0;
}

//int LT3746_Latch(void)
//{
//  // latch
//  LT3746_SCKO_LOW;
//  LT3746_delay();
//  LT3746_LDO_HIGH;
//  LT3746_delay();
//  LT3746_LDO_LOW;
//  LT3746_delay();
//  return 0;
//}

int LT3746_GetLedStatus(struct LT3746_Frame frame, uint8_t led_number)
{
  return 0;
}
