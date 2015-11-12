#include "sim_hal.h"
#include <string.h>
#include "delay.h"
#include "flash.h"

#define SIM_HAL_BUFF_SIZE 128
unsigned char sim_hal_rx_buff[SIM_HAL_BUFF_SIZE];
unsigned char sim_hal_rx_len;

void sim_hal_init(int baudrate)
{
  // USART init
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* GPIOD Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  
  //GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);
   
  /* Configure PA2 and PA3 in output pushpull mode */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
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
  }
}

int sim_hal_Available(void)
{
  return sim_hal_rx_len;
}
int sim_hal_GetData(char * buffer, int len)
{
  memcpy(buffer, sim_hal_rx_buff, len);
  return len;
}
void sim_hal_Flush(void)
{
  sim_hal_rx_len = 0;
}

void sim_hal_sendChar(char c)
{
  USART_SendData(USART2, c);
  while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
  {}
}

void sim_hal_sendStr(char Str[])
{
  while(*Str)
  {
    USART_SendData(USART2, *Str++);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
    {}
  }
}

void sim_hal_sendNum(int num)
{
  int tmp = 10000000;
  if (num == 0)
  {
    sim_hal_sendChar('0');
    return;
  }
  if (num < 0)
  {
    sim_hal_sendChar('-');
    num = -num;
  }
  while (tmp > 0)
  {
    if (tmp <= num)
    {
      sim_hal_sendChar((num/tmp)%10 + '0');
    }
    tmp = tmp / 10;
  }
}

void sim_hal_sendFloat(float num)
{
  int __int = (int) num;
  sim_hal_sendNum(__int);
  sim_hal_sendChar('.');
  __int = (int)((num-__int)*100);
  if (__int < 0)
    __int = 0;
  sim_hal_sendNum(__int);
}

void sim_hal_sendByte(uint8_t b, BYTE_FORMAT f)
{
  uint8_t bitMask = 1;
  uint8_t i;
  switch (f)
  {
  case BIN:
    for (i = 8; i > 0; i--)
    {
      sim_hal_sendChar((b&(bitMask << i)) ? '1' : '0');
    }
    break;
  case OCT:
    break;
  case DEC:
    sim_hal_sendNum(b);
    break;
  case HEX:
    if(b/16 < 10){
      sim_hal_sendChar(0x30 + b/16);
    }else{
      sim_hal_sendChar(0x37 + b/16);
    }
    
    if(b%16 < 10){
      sim_hal_sendChar(0x30 + b%16);
    }else{
      sim_hal_sendChar(0x37 + b%16);
    }
    break;
  default:
    break;
  }
}
