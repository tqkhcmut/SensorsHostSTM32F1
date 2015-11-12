#ifndef _delay_h_
#define _delay_h_

#ifdef __cplusplus
 extern "C" {
#endif
   
#include "stm32f10x.h"

void Delay_Init(void);
void Delay(unsigned int ms_time);
void DelayUs(unsigned int us_time);
unsigned int Millis(void);
unsigned int Micros(void);

// arduino compatiple
#define millis                  Millis
#define micros                  Micros
#define delay                   Delay
#define delayMicroseconds       DelayUs

#ifdef __cplusplus
}
#endif

#endif
