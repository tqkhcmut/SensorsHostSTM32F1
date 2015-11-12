#ifndef _enrf24_h_
#define _enrf24_h_


#include <stm32f10x.h>
#include "spi.h"
#include <stdint.h>
#include "nRF24L01.h"

#ifndef size_t
typedef unsigned int size_t;
#endif

#ifndef BITF
#define BIT0                (0x0001)
#define BIT1                (0x0002)
#define BIT2                (0x0004)
#define BIT3                (0x0008)
#define BIT4                (0x0010)
#define BIT5                (0x0020)
#define BIT6                (0x0040)
#define BIT7                (0x0080)
#define BIT8                (0x0100)
#define BIT9                (0x0200)
#define BITA                (0x0400)
#define BITB                (0x0800)
#define BITC                (0x1000)
#define BITD                (0x2000)
#define BITE                (0x4000)
#define BITF                (0x8000)
#endif

/* Constants for speed, radio state */
#define ENRF24_STATE_NOTPRESENT 0
#define ENRF24_STATE_DEEPSLEEP 1
#define ENRF24_STATE_IDLE 2
#define ENRF24_STATE_PTX 3
#define ENRF24_STATE_PRX 4

/* Internal IRQ handling */
#define ENRF24_IRQ_TX       0x20
#define ENRF24_IRQ_RX       0x40
#define ENRF24_IRQ_TXFAILED 0x10
#define ENRF24_IRQ_MASK     0x70

#define ENRF24_CFGMASK_IRQ 0

#define CE_PIN          1 
#define CSN_PIN         2
#define IRQ_PIN         3


extern int lastTXfailed;

void Enrf24_init(uint8_t cePin, uint8_t csnPin, uint8_t irqPin);
void Enrf24_begin(uint32_t datarate, uint8_t channel);  // Specify bitrate & channel
//void Enrf24_begin(uint32_t datarate=1000000, uint8_t channel=0);  // Specify bitrate & channel
void Enrf24_end();      // Shut it off, clear the library's state
//    void setSPI(SPIClass *spi_instance_ptr) { spibus = spi_instance_ptr; }  // Change the SPI instance used

// I/O
//int Enrf24_available(int checkIrq=0);  // Check if incoming data is ready to be read
int Enrf24_available(int checkIrq);  // Check if incoming data is ready to be read
size_t Enrf24_read(void *inbuf, uint8_t maxlen); 
//size_t Enrf24_read(void *inbuf, uint8_t maxlen=32);  
/* Read contents of RX buffer up to
* 'maxlen' bytes, return final length.
* 'inbuf' should be maxlen+1 since a
* null '\0' is tacked onto the end.
*/
size_t Enrf24_write(uint8_t);  // Single-byte write, implements TX ring-buffer & auto-send
size_t Enrf24_write_buff(const void *buf, size_t len);// { return write((const uint8_t *)buf, len); };
//    using Print::write;  // Includes the multi-byte write for repeatedly hitting write(uint8_t)
void Enrf24_flush();    // Force transmission of TX ring buffer contents
void Enrf24_purge();    // Ignore TX ring buffer contents, return ring pointer to 0.

// Power-state related stuff-
uint8_t Enrf24_radioState();  // Evaluate current state of the transceiver (see ENRF24_STATE_* defines)
void Enrf24_deepsleep();  // Enter POWERDOWN mode, ~0.9uA power consumption
void Enrf24_enableRX();   // Enter PRX mode (~14mA)
void Enrf24_disableRX();  /* Disable PRX mode (PRIM_RX bit in CONFIG register)
* Note this won't necessarily push the transceiver into deep sleep, but rather
* an idle standby mode where its internal oscillators are ready & running but
* the RF transceiver PLL is disabled.  ~26uA power consumption.
*/

// Custom tweaks to RF parameters, packet parameters
void Enrf24_autoAck(int onoff);  // Enable/disable auto-acknowledgements (enabled by default)
//void Enrf24_autoAck(int onoff=1);  // Enable/disable auto-acknowledgements (enabled by default)
void Enrf24_setChannel(uint8_t channel);
void Enrf24_setTXpower(int8_t dBm);  // Only a few values supported by this (0, -6, -12, -18 dBm)
//void Enrf24_setTXpower(int8_t dBm=0);  // Only a few values supported by this (0, -6, -12, -18 dBm)
void Enrf24_setSpeed(uint32_t rfspeed);  // Set 250000, 1000000, 2000000 speeds.
void Enrf24_setCRC(int onoff, int crc16bit); 
//void Enrf24_setCRC(int onoff, int crc16bit=0); 
/* Enable/disable CRC usage inside nRF24's
* hardware packet engine, specify 8 or
* 16-bit CRC.
*/
// Set AutoACK retry count, timeout params (0-15, 250-4000 respectively)
void Enrf24_setAutoAckParams(uint8_t autoretry_count, uint16_t autoretry_timeout);
//void Enrf24_setAutoAckParams(uint8_t autoretry_count=15, uint16_t autoretry_timeout=2000);

// Protocol addressing -- receive, transmit addresses
void Enrf24_setAddressLength(size_t len);  // Valid parameters = 3, 4 or 5.  Defaults to 5.
void Enrf24_setRXaddress(const void *rxaddr);    // 3-5 byte RX address loaded into pipe#1
void Enrf24_setTXaddress(const void *txaddr);    // 3-5 byte TX address loaded into TXaddr register

// Miscellaneous feature
int Enrf24_rfSignalDetected();  /* Read RPD register to determine if transceiver has presently detected an RF signal
* of -64dBm or greater.  Only works in PRX (enableRX()) mode.
*/

// Query current parameters
//int Enrf24_getChannel(void) { return _readReg(RF24_RF_CH); };
int Enrf24_getChannel(void);
uint32_t Enrf24_getSpeed(void);
int8_t Enrf24_getTXpower(void);
//size_t Enrf24_getAddressLength(void) { return rf_addr_width; };
size_t Enrf24_getAddressLength(void);
void Enrf24_getRXaddress(void *);
void Enrf24_getTXaddress(void *);
int Enrf24_getAutoAck(void);
unsigned int Enrf24_getCRC(void);


#endif
