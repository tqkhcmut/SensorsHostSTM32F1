#ifndef _ds1307_h_
#define _ds1307_h_

#include "stm32f10x.h"

typedef struct Time 
{
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t day_of_week;
} Time_t;

typedef struct Date 
{
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t control;
} Date_t;

int DS1307_Init(void);
int DS1307_GetTime(Time_t * t);
int DS1307_SetTime(Time_t t);
int DS1307_GetDate(Date_t * d);
int DS1307_SetDate(Date_t d);


#endif
