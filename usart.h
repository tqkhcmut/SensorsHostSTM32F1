#ifndef _usart_h_
#define _usart_h_

#ifdef __cplusplus
 extern "C" {
#endif
	 
//#define USE_USART

#include "stm32f10x.h"

//#ifndef BYTE_FORMAT
//typedef enum 
//{
//	BIN = 0,
//	OCT,
//	DEC,
//	HEX
//} BYTE_FORMAT;
//#endif

//#define USART_BUFFER_SIZE 	32
//#define USART_BUFFERED

void USART1_Init(uint32_t baudrate);
void USART1_SendChar(char c);
void USART1_SendStr(char Str[]);
void USART1_SendNum(int num);
void USART1_SendFloat(float num);
void USART1_SendByte(uint8_t b, BYTE_FORMAT f);
int USART1_Available(void);
int USART1_GetData(char * buffer, int len);
void USART1_Flush(void);

#ifdef __cplusplus
}
#endif

#endif

