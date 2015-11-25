#include "rs485.h"
#include "delay.h"
#include <string.h>

#define USART_BUFF_SIZE 50
unsigned char rs485_rx_buff[USART_BUFF_SIZE];
unsigned char rs485_rx_len;

unsigned int rs485_break_time = 1000; // default: 1000 us

#define RS485     USART3

#define RS485_CE_PIN    GPIO_Pin_13
#define RS485_CE_PORT   GPIOA
#define RS485_DIR_PIN   GPIO_Pin_12
#define RS485_DIR_PORT  GPIOA

int RS485_CE_IsActive(void)
{
  if (GPIO_ReadInputDataBit(RS485_CE_PORT, RS485_CE_PIN) == 0)
    return 1;
  return 0;
}
int RS485_CE_ISInActive(void)
{
  if (GPIO_ReadInputDataBit(RS485_CE_PORT, RS485_CE_PIN) == 0)
    return 0;
  return 1;
}
void RS485_DIR_Output(void)
{
  GPIO_WriteBit(RS485_DIR_PORT, RS485_DIR_PIN, Bit_SET); // OFF
  DelayUs(1);
}
void RS485_DIR_Input(void)
{
  DelayUs(10);
  GPIO_WriteBit(RS485_DIR_PORT, RS485_DIR_PIN, Bit_RESET); // OFF
}

void RS485_Init(uint32_t baudrate)
{
  // USART init
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* GPIOD Periph clock enable */
  
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
   
   
  /* Configure PB10 and PB11 in output pushpull mode */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  // CE pin
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = RS485_CE_PIN;
  GPIO_Init(RS485_CE_PORT, &GPIO_InitStructure);
  
  // DIR pin
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = RS485_DIR_PIN;
  GPIO_Init(RS485_DIR_PORT, &GPIO_InitStructure);
  
  USART_InitStructure.USART_BaudRate            = baudrate;
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;
  USART_InitStructure.USART_Parity              = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_Init(RS485, &USART_InitStructure);
  
  /* Enable RS485 Receive and Transmit interrupts */
  USART_ITConfig(RS485, USART_IT_RXNE, ENABLE);
  
  USART_Cmd(RS485, ENABLE);
  
  // 
  RS485_DIR_Input();
  
  // calculate break_time
  rs485_break_time = 64000000/baudrate;
  
  // init timer base

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 64;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}


void USART3_IRQHandler(void)
{
  if(USART_GetITStatus(RS485, USART_IT_RXNE) != RESET)
  {
    /* Read one byte from the receive data register */
    rs485_rx_buff[rs485_rx_len++] = USART_ReceiveData(RS485);
    if (rs485_rx_len >= USART_BUFF_SIZE)
    {
      rs485_rx_len = 0;
    }

//    if(RxCounter1 == NbrOfDataToRead1)
//    {
//      /* Disable the USARTy Receive interrupt */
//      USART_ITConfig(USARTy, USART_IT_RXNE, DISABLE);
//    }
    TIM_SetCounter(TIM3, 0);
  }
}

int RS485_Available(void)
{
  if (TIM_GetCounter(TIM3) > rs485_break_time)
    return rs485_rx_len;
  else
    return 0;
}
int RS485_GetData(char * buffer, int len)
{
  memcpy(buffer, rs485_rx_buff, len);
  return len;
}
void RS485_Flush(void)
{
  rs485_rx_len = 0;
}

void RS485_SendChar(char c)
{
  USART_SendData(RS485, c);
  while (USART_GetFlagStatus(RS485, USART_FLAG_TC) == RESET)
  {}
}

void RS485_SendStr(char Str[])
{
  while(*Str)
  {
    USART_SendData(RS485, *Str++);
    while (USART_GetFlagStatus(RS485, USART_FLAG_TC) == RESET)
    {}
  }
}

void RS485_SendNum(int num)
{
  int tmp = 10000000;
  if (num == 0)
  {
    RS485_SendChar('0');
    return;
  }
  if (num < 0)
  {
    RS485_SendChar('-');
    num = -num;
  }
  while (tmp > 0)
  {
    if (tmp <= num)
    {
      RS485_SendChar((num/tmp)%10 + '0');
    }
    tmp = tmp / 10;
  }
}

void RS485_SendFloat(float num)
{
  int __int = (int) num;
  RS485_SendNum(__int);
  RS485_SendChar('.');
  __int = (int)((num-__int)*100);
  if (__int < 0)
    __int = 0;
  RS485_SendNum(__int);
}

void RS485_SendByte(uint8_t b, BYTE_FORMAT f)
{
  uint8_t bitMask = 1;
  uint8_t i;
  switch (f)
  {
  case BIN:
    for (i = 8; i > 0; i--)
    {
      RS485_SendChar((b&(bitMask << i)) ? '1' : '0');
    }
    break;
  case OCT:
    break;
  case DEC:
    RS485_SendNum(b);
    break;
  case HEX:
    if(b/16 < 10){
      RS485_SendChar(0x30 + b/16);
    }else{
      RS485_SendChar(0x37 + b/16);
    }
    
    if(b%16 < 10){
      RS485_SendChar(0x30 + b%16);
    }else{
      RS485_SendChar(0x37 + b%16);
    }
    break;
  default:
    break;
  }
}


