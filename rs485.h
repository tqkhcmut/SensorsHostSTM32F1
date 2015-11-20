#ifndef _rs485_h_
#define _rs485_h_

#ifdef __cplusplus
 extern "C" {
#endif
	 
//#define USE_USART

#include "stm32f10x.h"

int RS485_CE_IsActive(void);
int RS485_CE_ISInActive(void);
void RS485_DIR_Output(void);
void RS485_DIR_Input(void);
   
void RS485_Init(void);
void RS485_SendChar(char c);
void RS485_SendStr(char Str[]);
void RS485_SendNum(int num);
void RS485_SendFloat(float num);
void RS485_SendByte(uint8_t b, BYTE_FORMAT f);
int RS485_Available(void);
int RS485_GetData(char * buffer, int len);
void RS485_Flush(void);

#ifdef __cplusplus
}
#endif

#endif

