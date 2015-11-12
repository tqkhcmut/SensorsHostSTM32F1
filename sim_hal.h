#ifndef _sim_hal_h_
#define _sim_hal_h_

#ifdef __cplusplus
extern "C" {
#endif
  
  
#include "stm32f10x.h"
  
  
  void sim_hal_init(int baudrate);
  void sim_hal_sendChar(char c);
  void sim_hal_sendStr(char Str[]);
  void sim_hal_sendNum(int num);
  void sim_hal_sendFloat(float num);
  void sim_hal_sendByte(uint8_t b, BYTE_FORMAT f);
  int sim_hal_Available(void);
  int sim_hal_GetData(char * buffer, int len);
  void sim_hal_Flush(void);
  
  
#ifdef __cplusplus
}
#endif

#endif

