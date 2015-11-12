#ifndef _spi_cpp_h_
#define _spi_cpp_h_

#include <stm32f10x.h>

struct SPI 
{
  unsigned char (* transfer)(unsigned char);
};

extern struct SPI spibus;
void NRF_SPI_Init(void);

#endif
