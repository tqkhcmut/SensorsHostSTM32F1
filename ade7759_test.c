#include "ade7759.h"
#include "stm32f10x.h"
#include <string.h>

#define EVER (;;)

#include "delay.h"

int main()
{  
  Delay_Init();
  
  for EVER
  {
  }
}

