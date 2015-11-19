#ifndef _delay_h_
#define _delay_h_

#ifdef __cplusplus
 extern "C" {
#endif
   
#include "stm32f10x.h"

extern void Delay_Init(void);
extern void Delay(unsigned int ms_time);
extern inline void DelayUs(unsigned int us_time);
extern unsigned int Millis(void);
extern unsigned int Micros(void);

// arduino compatiple
#define millis                  Millis
#define micros                  Micros
#define delay                   Delay
#define delayMicroseconds       DelayUs

#ifdef __cplusplus
}
#endif

#endif
