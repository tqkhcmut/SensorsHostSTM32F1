#include "sim_hal.h"
#include <string.h>
#include "delay.h"
#include "flash.h"

#define SIM_HAL_BUFF_SIZE 128
unsigned char sim_hal_rx_buff[SIM_HAL_BUFF_SIZE];
unsigned char sim_hal_rx_len;

unsigned int sim_hal_break_time = 1000; // default: 1000 us

#define SIM_HAL_POW_PORT					GPIOB
#define SIM_HAL_POW_PIN						GPIO_Pin_5
#define SIM_HAL_POW_PERIPH_CLK		RCC_APB2Periph_GPIOB
#define SIM_HAL_RESET_PORT				GPIOB
#define SIM_HAL_RESET_PIN					GPIO_Pin_4
#define SIM_HAL_RESET_PERIPH_CLK	RCC_APB2Periph_GPIOB

void sim_hal_power_low(void)
{
	GPIO_WriteBit(SIM_HAL_POW_PORT, SIM_HAL_POW_PIN, Bit_RESET);
}
void sim_hal_power_high(void)
{
	GPIO_WriteBit(SIM_HAL_POW_PORT, SIM_HAL_POW_PIN, Bit_SET);
}
void sim_hal_reset_low(void)
{
	GPIO_WriteBit(SIM_HAL_RESET_PORT, SIM_HAL_RESET_PIN, Bit_RESET);
}
void sim_hal_reset_high(void)
{
	GPIO_WriteBit(SIM_HAL_RESET_PORT, SIM_HAL_RESET_PIN, Bit_SET);
}

void sim_hal_init(int baudrate)
{
  // USART init
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
  /* GPIOD Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_APB2PeriphClockCmd(SIM_HAL_POW_PERIPH_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(SIM_HAL_RESET_PERIPH_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
   
  /* Configure PA2 and PA3 in output pushpull mode */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = SIM_HAL_POW_PIN;
  GPIO_Init(SIM_HAL_POW_PORT, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = SIM_HAL_RESET_PIN;
  GPIO_Init(SIM_HAL_RESET_PORT, &GPIO_InitStructure);
	
	sim_hal_reset_low();
  
  USART_InitStructure.USART_BaudRate            = baudrate;
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;
  USART_InitStructure.USART_Parity              = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_Init(USART2, &USART_InitStructure);
  
  /* Enable USART2 Receive and Transmit interrupts */
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  
  USART_Cmd(USART2, ENABLE);
  
  // calculate break_time
  sim_hal_break_time = 64000000/baudrate;
  
  // init timer base

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 64;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  /* TIM3 enable counter */
  TIM_Cmd(TIM4, ENABLE);
}


void USART2_IRQHandler(void)
{
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
  {
    /* Read one byte from the receive data register */
    sim_hal_rx_buff[sim_hal_rx_len++] = USART_ReceiveData(USART2);
    if (sim_hal_rx_len >= SIM_HAL_BUFF_SIZE)
    {
      sim_hal_rx_len = 0;
    }
    TIM_SetCounter(TIM4, 0);
  }
}

int sim_hal_available(void)
{
  if (TIM_GetCounter(TIM4) > sim_hal_break_time)
    return sim_hal_rx_len;
  else
    return 0;
}
int sim_hal_get_data(char * buffer, int len)
{
  memcpy(buffer, sim_hal_rx_buff, len);
  return len;
}
void sim_hal_flush(void)
{
  sim_hal_rx_len = 0;
}

void sim_hal_send_char(char c)
{
  USART_SendData(USART2, c);
  while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
  {}
}

void sim_hal_send_string(char Str[])
{
  while(*Str)
  {
    USART_SendData(USART2, *Str++);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
    {}
  }
}

void sim_hal_send_numberic(int num)
{
  int tmp = 10000000;
  if (num == 0)
  {
    sim_hal_send_char('0');
    return;
  }
  if (num < 0)
  {
    sim_hal_send_char('-');
    num = -num;
  }
  while (tmp > 0)
  {
    if (tmp <= num)
    {
      sim_hal_send_char((num/tmp)%10 + '0');
    }
    tmp = tmp / 10;
  }
}

void sim_hal_send_float(float num)
{
  int __int = (int) num;
  sim_hal_send_numberic(__int);
  sim_hal_send_char('.');
  __int = (int)((num-__int)*100);
  if (__int < 0)
    __int = 0;
  sim_hal_send_numberic(__int);
}

void sim_hal_send_byte(uint8_t b, BYTE_FORMAT f)
{
  uint8_t bitMask = 1;
  uint8_t i;
  switch (f)
  {
  case BIN:
    for (i = 8; i > 0; i--)
    {
      sim_hal_send_char((b&(bitMask << i)) ? '1' : '0');
    }
    break;
  case OCT:
    break;
  case DEC:
    sim_hal_send_numberic(b);
    break;
  case HEX:
    if(b/16 < 10){
      sim_hal_send_char(0x30 + b/16);
    }else{
      sim_hal_send_char(0x37 + b/16);
    }
    
    if(b%16 < 10){
      sim_hal_send_char(0x30 + b%16);
    }else{
      sim_hal_send_char(0x37 + b%16);
    }
    break;
  default:
    break;
  }
}
