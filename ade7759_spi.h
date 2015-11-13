#ifndef _ade7759_spi_h_
#define _ade7759_spi_h_

#include <stdint.h>

void spi_setup(u32 speed, u8 mode);
int spi_speed_set(u32 speed);
int spi_speed_get(u32 * speed_buff);
int spi_write(u8 * buffer, u32 buffer_len);
int spi_w8r8(u8 dummy); // write 8 bit, read 8 bit
int spi_w8r16(u8 dummy); // write 8 bit, read 8 bit

#endif // _ade7759_spi_h_
