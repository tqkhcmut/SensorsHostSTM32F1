#ifndef _ade7759_spi_h_
#define _ade7759_spi_h_

#include <stdint.h>

#define SPI_SPEED_SLOW        (u32)(300 * 1000)
#define SPI_SPEED_BURST       (u32)(1000 * 1000)
#define SPI_SPEED_FAST        (u32)(2000 * 1000)

void spi_setup(u32 speed, u8 mode);
int spi_speed_set(u32 speed);
int spi_speed_get(u32 * speed_buff);
int spi_write(u8 * buffer, u32 buffer_len);
int spi_w8r8(u8 dummy); // write 8 bit, read 8 bit
int spi_w8r16(u8 dummy); // write 8 bit, read 8 bit

#endif // _ade7759_spi_h_
