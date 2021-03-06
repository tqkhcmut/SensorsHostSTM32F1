#ifndef _ADE7759_H
#define _ADE7759_H

#include <stdint.h>
#include "ade7759_spi.h"

#define ADE7759_WAVEFORM  0x01
#define ADE7759_AENERGY   0x02
#define ADE7759_RSTENERGY 0x03
#define ADE7759_STATUS    0x04
#define ADE7759_RSTSTATUS 0x05
#define ADE7759_MODE      0x06
#define ADE7759_CFDEN     0x07
#define ADE7759_CH1OS     0x08
#define ADE7759_CH2OS     0x09
#define ADE7759_GAIN      0x0A
#define ADE7759_APGAIN    0x0B
#define ADE7759_PHCAL     0x0C
#define ADE7759_APOS      0x0D
#define ADE7759_ZXTOUT    0x0E
#define ADE7759_SAGCYC    0x0F
#define ADE7759_IRQEN     0x10
#define ADE7759_SAGLVL    0x11
#define ADE7759_TEMP      0x12
#define ADE7759_LINECYC   0x13
#define ADE7759_LENERGY   0x14
#define ADE7759_CFNUM     0x15
#define ADE7759_CHKSUM    0x1E
#define ADE7759_DIEREV    0x1F

#define ADE7759_READ_REG(a)    a
#define ADE7759_WRITE_REG(a) ((a) | 0x80)

#define ADE7759_MAX_TX    6
#define ADE7759_MAX_RX    6
#define ADE7759_STARTUP_DELAY 1

#define ADE7759_SPI_SLOW        SPI_SPEED_SLOW
#define ADE7759_SPI_BURST       SPI_SPEED_BURST
#define ADE7759_SPI_FAST        SPI_SPEED_FAST

#define DRIVER_NAME             "ade7759"


#endif
