#include "enrf24.h"

#include <stdint.h>
#include "Enrf24.h"
#include "delay.h"
#include "spi.h"

#define OUTPUT  1
#define INPUT   2
#define HIGH    1
#define LOW     2

#define RF_GPIO_PORT            GPIOA
#define RF_GPIO_CLK             RCC_APB2Periph_GPIOA   
#define RF_CE_GPIO_PIN          GPIO_Pin_8
#define RF_CSN_GPIO_PIN         GPIO_Pin_11
#define RF_IRQ_GPIO_PIN         GPIO_Pin_12

GPIO_InitTypeDef Enrf24_GPIO_InitStructure;

int lastTXfailed;

uint8_t rf_status;
uint8_t rf_addr_width;
uint8_t txbuf_len;
uint8_t txbuf[32];
uint8_t lastirq, readpending;
uint8_t _cePin, _csnPin, _irqPin;


uint8_t Enrf24__readReg(uint8_t addr);
void Enrf24__readRegMultiLSB(uint8_t addr, uint8_t *buf, size_t len);
void Enrf24__writeReg(uint8_t addr, uint8_t val);
void Enrf24__writeRegMultiLSB(uint8_t addr, uint8_t *buf, size_t len);
void Enrf24__issueCmd(uint8_t cmd);
void Enrf24__readCmdPayload(uint8_t addr, uint8_t *buf, size_t len, size_t maxlen);
void Enrf24__issueCmdPayload(uint8_t cmd, uint8_t *buf, size_t len);
uint8_t Enrf24__irq_getreason();
uint8_t Enrf24__irq_derivereason();  // Get IRQ status from rf_status w/o querying module over SPI.
void Enrf24__irq_clear(uint8_t irq);
int Enrf24__isAlive();
void Enrf24__readTXaddr(uint8_t *buf);
void Enrf24__writeRXaddrP0(uint8_t *buf);
void Enrf24__maintenanceHook();  // Handles IRQs and purges RX queue when erroneous contents exist.
    
GPIO_InitTypeDef Enrf24_GPIO_InitStructure;

void pinMode(int pin, int mode)
{
  switch (mode)
  {
  case OUTPUT:
    Enrf24_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    break;
  case INPUT:
    Enrf24_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    break;
  default:
    break;
  }
  
  switch (pin)
  {
  case CE_PIN:
    Enrf24_GPIO_InitStructure.GPIO_Pin = RF_CE_GPIO_PIN;
    break;
  case CSN_PIN:
    Enrf24_GPIO_InitStructure.GPIO_Pin = RF_CSN_GPIO_PIN;
    break;
  case IRQ_PIN:
    Enrf24_GPIO_InitStructure.GPIO_Pin = RF_IRQ_GPIO_PIN;
    GPIO_Init(GPIOB, &Enrf24_GPIO_InitStructure);
    return;
    break;
  default:
    break;
  }
  GPIO_Init(RF_GPIO_PORT, &Enrf24_GPIO_InitStructure);
}
void digitalWrite(int pin, int state)
{
  switch (state)
  {
  case HIGH:
    switch (pin)
    {
    case CE_PIN:
      GPIO_WriteBit(RF_GPIO_PORT, RF_CE_GPIO_PIN, Bit_SET);
      break;
    case CSN_PIN:
      GPIO_WriteBit(RF_GPIO_PORT, RF_CSN_GPIO_PIN, Bit_SET);
      break;
    case IRQ_PIN:
      GPIO_WriteBit(GPIOB, RF_IRQ_GPIO_PIN, Bit_SET);
      break;
    default:
      break;
    }
    break;
  case LOW:
    switch (pin)
    {
    case CE_PIN:
      GPIO_WriteBit(RF_GPIO_PORT, RF_CE_GPIO_PIN, Bit_RESET);
      break;
    case CSN_PIN:
      GPIO_WriteBit(RF_GPIO_PORT, RF_CSN_GPIO_PIN, Bit_RESET);
      break;
    case IRQ_PIN:
      GPIO_WriteBit(GPIOB, RF_IRQ_GPIO_PIN, Bit_RESET);
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}
int digitalRead(int pin)
{
  int res = HIGH;
  switch (pin)
  {
  case CE_PIN:
    if (GPIO_ReadInputDataBit(RF_GPIO_PORT, RF_CE_GPIO_PIN) == Bit_SET)
    {
      res = HIGH;
    }
    else
    {
      res = LOW;
    }
    break;
  case CSN_PIN:
    if (GPIO_ReadInputDataBit(RF_GPIO_PORT, RF_CSN_GPIO_PIN) == Bit_SET)
    {
      res = HIGH;
    }
    else
    {
      res = LOW;
    }
    break;
  case IRQ_PIN:
    if (GPIO_ReadInputDataBit(GPIOB, RF_IRQ_GPIO_PIN) == Bit_SET)
    {
      res = HIGH;
    }
    else
    {
      res = LOW;
    }
    break;
  default:
    break;
  }
  return res;
}

/* Constructor */
void Enrf24_init(uint8_t cePin, uint8_t csnPin, uint8_t irqPin)
{
  _cePin = cePin;
  _csnPin = csnPin;
  _irqPin = irqPin;
  
  NRF_SPI_Init();
  
  rf_status = 0;
  rf_addr_width = 5;
  txbuf_len = 0;
  readpending = 0;
  
  Enrf24_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  RCC_APB2PeriphClockCmd(RF_GPIO_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
}

/* Initialization */
void Enrf24_begin(uint32_t datarate, uint8_t channel)
{
  pinMode(_cePin, OUTPUT);
  digitalWrite(_cePin, LOW);
  pinMode(_csnPin, OUTPUT);
  digitalWrite(_csnPin, HIGH);
  pinMode(_irqPin, INPUT);
  digitalWrite(_irqPin, LOW);  // No pullups; the transceiver provides this!
  
  spibus.transfer(0);  // Strawman transfer, fixes USCI issue on G2553
  
  // Is the transceiver present/alive?
  if (!Enrf24__isAlive())
    return;  // Nothing more to do here...
  
  // Wait 100ms for module to initialize
  while ( millis() < 100 )
    ;
  
  //Write to FEATURE Register and see if changes stick 
  Enrf24__writeReg(RF24_FEATURE, RF24_EN_DPL);
  
  if (Enrf24__readReg(RF24_FEATURE) == 0x00) {
    //If changes do not stick, issue an activate command
    uint8_t _activate_data = 0x73;
    Enrf24__issueCmdPayload(RF24_ACTIVATE, &_activate_data, 1);
  }
  
  // Init certain registers
  Enrf24__writeReg(RF24_CONFIG, 0x00);  // Deep power-down, everything disabled
  Enrf24__writeReg(RF24_EN_AA, 0x03);
  Enrf24__writeReg(RF24_EN_RXADDR, 0x03);
  Enrf24__writeReg(RF24_RF_SETUP, 0x00);
  Enrf24__writeReg(RF24_STATUS, ENRF24_IRQ_MASK);  // Clear all IRQs
  Enrf24__writeReg(RF24_DYNPD, 0x03);
  Enrf24__writeReg(RF24_FEATURE, RF24_EN_DPL);  // Dynamic payloads enabled by default
  
  // Set all parameters
  if (channel > 125)
    channel = 125;
  Enrf24_deepsleep();
  Enrf24__issueCmd(RF24_FLUSH_TX);
  Enrf24__issueCmd(RF24_FLUSH_RX);
  readpending = 0;
  Enrf24__irq_clear(ENRF24_IRQ_MASK);
  Enrf24_setChannel(channel);
  Enrf24_setSpeed(datarate);
  Enrf24_setTXpower(0);
  Enrf24_setAutoAckParams(15, 2000);
  Enrf24_setAddressLength(rf_addr_width);
  Enrf24_setCRC(1, 0);  // Default = CRC on, 8-bit
}

/* Formal shut-down/clearing of library state */
void Enrf24_end()
{
  txbuf_len = 0;
  rf_status = 0;
  rf_addr_width = 5;
  
  if (!Enrf24__isAlive())
    return;
  Enrf24_deepsleep();
  Enrf24__issueCmd(RF24_FLUSH_TX);
  Enrf24__issueCmd(RF24_FLUSH_RX);
  readpending = 0;
  Enrf24__irq_clear(ENRF24_IRQ_MASK);
  digitalWrite(_cePin, LOW);
  digitalWrite(_csnPin, HIGH);
}

/* Basic SPI I/O */
uint8_t Enrf24__readReg(uint8_t addr)
{
  uint8_t result;
  
  digitalWrite(_csnPin, LOW);
  rf_status = spibus.transfer(RF24_R_REGISTER | addr);
  result = spibus.transfer(RF24_NOP);
  digitalWrite(_csnPin, HIGH);
  return result;
}

void Enrf24__readRegMultiLSB(uint8_t addr, uint8_t *buf, size_t len)
{
  uint8_t i;
  digitalWrite(_csnPin, LOW);
  rf_status = spibus.transfer(RF24_R_REGISTER | addr);
  for (i=0; i<len; i++) {
    buf[len-i-1] = spibus.transfer(RF24_NOP);
  }
  digitalWrite(_csnPin, HIGH);
}

void Enrf24__writeReg(uint8_t addr, uint8_t val)
{
  digitalWrite(_csnPin, LOW);
  rf_status = spibus.transfer(RF24_W_REGISTER | addr);
  spibus.transfer(val);
  digitalWrite(_csnPin, HIGH);
}

void Enrf24__writeRegMultiLSB(uint8_t addr, uint8_t *buf, size_t len)
{
  size_t i;
  
  digitalWrite(_csnPin, LOW);
  rf_status = spibus.transfer(RF24_W_REGISTER | addr);
  for (i=0; i<len; i++) {
    spibus.transfer(buf[len-i-1]);
  }
  digitalWrite(_csnPin, HIGH);
}

void Enrf24__issueCmd(uint8_t cmd)
{
  digitalWrite(_csnPin, LOW);
  rf_status = spibus.transfer(cmd);
  digitalWrite(_csnPin, HIGH);
}

void Enrf24__issueCmdPayload(uint8_t cmd, uint8_t *buf, size_t len)
{
  size_t i;
  
  digitalWrite(_csnPin, LOW);
  rf_status = spibus.transfer(cmd);
  for (i=0; i<len; i++) {
    spibus.transfer(buf[i]);
  }
  digitalWrite(_csnPin, HIGH);
}

void Enrf24__readCmdPayload(uint8_t cmd, uint8_t *buf, size_t len, size_t maxlen)
{
  size_t i;
  
  digitalWrite(_csnPin, LOW);
  rf_status = spibus.transfer(cmd);
  for (i=0; i<len; i++) {
    if (i < maxlen) {
      buf[i] = spibus.transfer(RF24_NOP);
    } else {
      spibus.transfer(RF24_NOP);  // Beyond maxlen bytes, just discard the remaining data.
    }
  }
  digitalWrite(_csnPin, HIGH);
}

int Enrf24__isAlive()
{
  uint8_t aw;
  
  aw = Enrf24__readReg(RF24_SETUP_AW);
  return ((aw & 0xFC) == 0x00 && (aw & 0x03) != 0x00);
}

uint8_t Enrf24__irq_getreason()
{
  lastirq = Enrf24__readReg(RF24_STATUS) & ENRF24_IRQ_MASK;
  return lastirq;
}

// Get IRQ from last known rf_status update without querying module over SPI.
uint8_t Enrf24__irq_derivereason()
{
  lastirq = rf_status & ENRF24_IRQ_MASK;
  return lastirq;
}

void Enrf24__irq_clear(uint8_t irq)
{
  Enrf24__writeReg(RF24_STATUS, irq & ENRF24_IRQ_MASK);
}

#define ENRF24_CFGMASK_CRC(a) (a & (RF24_EN_CRC | RF24_CRCO))

void Enrf24__readTXaddr(uint8_t *buf)
{
  Enrf24__readRegMultiLSB(RF24_TX_ADDR, buf, rf_addr_width);
}

void Enrf24__writeRXaddrP0(uint8_t *buf)
{
  Enrf24__writeRegMultiLSB(RF24_RX_ADDR_P0, buf, rf_addr_width);
}


/* nRF24 I/O maintenance--called as a "hook" inside other I/O functions to give
* the library a chance to take care of its buffers et al
*/
void Enrf24__maintenanceHook()
{
  uint8_t i;
  
  Enrf24__irq_getreason();
  
  if (lastirq & ENRF24_IRQ_TXFAILED) {
    lastTXfailed = 1;
    Enrf24__issueCmd(RF24_FLUSH_TX);
    Enrf24__irq_clear(ENRF24_IRQ_TXFAILED);
  }
  
  if (lastirq & ENRF24_IRQ_TX) {
    lastTXfailed = 0;
    Enrf24__irq_clear(ENRF24_IRQ_TX);
  }
  
  if (lastirq & ENRF24_IRQ_RX) {
    if ( !(Enrf24__readReg(RF24_FIFO_STATUS) & RF24_RX_FULL) ) {  /* Don't feel it's necessary
      * to be notified of new
      * incoming packets if the RX
      * queue is full.
      */
      Enrf24__irq_clear(ENRF24_IRQ_RX);
    }
    
    /* Check if RX payload is 0-byte or >32byte (erroneous conditions)
    * Also check if data was received on pipe#0, which we are ignoring.
    * The reason for this is pipe#0 is needed for receiving AutoACK acknowledgements,
    * its address gets reset to the module's default and we do not care about data
    * coming in to that address...
    */
    Enrf24__readCmdPayload(RF24_R_RX_PL_WID, &i, 1, 1);
    if (i == 0 || i > 32 || ((rf_status & 0x0E) >> 1) == 0) {
      /* Zero-width RX payload is an error that happens a lot
      * with non-AutoAck, and must be cleared with FLUSH_RX.
      * Erroneous >32byte packets are a similar phenomenon.
      */
      Enrf24__issueCmd(RF24_FLUSH_RX);
      Enrf24__irq_clear(ENRF24_IRQ_RX);
      readpending = 0;
    } else {
      readpending = 1;
    }
    // Actual scavenging of RX queues is performed by user-directed use of read().
  }
}



/* Public functions */
int Enrf24_available(int checkIrq)
{
  if (checkIrq && digitalRead(_irqPin) == HIGH && readpending == 0)
    return 0;
  Enrf24__maintenanceHook();
  if ( !(Enrf24__readReg(RF24_FIFO_STATUS) & RF24_RX_EMPTY) ) {
    return 1;
  }
  if (readpending) {
    return 1;
  }
  return 0;
}

size_t Enrf24_read(void *inbuf, uint8_t maxlen)
{
  uint8_t *buf = (uint8_t *)inbuf;
  uint8_t plwidth;
  
  Enrf24__maintenanceHook();
  readpending = 0;
  if ((Enrf24__readReg(RF24_FIFO_STATUS) & RF24_RX_EMPTY) || maxlen < 1) {
    return 0;
  }
  Enrf24__readCmdPayload(RF24_R_RX_PL_WID, &plwidth, 1, 1);
  Enrf24__readCmdPayload(RF24_R_RX_PAYLOAD, buf, plwidth, maxlen);
  buf[plwidth] = '\0';  // Zero-terminate in case this is a string.
  if (Enrf24__irq_derivereason() & ENRF24_IRQ_RX) {
    Enrf24__irq_clear(ENRF24_IRQ_RX);
  }
  
  return (size_t) plwidth;
}

size_t Enrf24_write_buff(const void *buf, size_t len)
{
  uint8_t * tmp_buff = (uint8_t *)buf;
    size_t n = 0;
    while (len--) 
    {
        n += Enrf24_write(*tmp_buff++);
    }
    return (n);
}

int Enrf24_getChannel(void) 
{ 
  return Enrf24__readReg(RF24_RF_CH); 
}

size_t Enrf24_getAddressLength(void) 
{
  return rf_addr_width; 
}

// Perform TX of current ring-buffer contents
void Enrf24_flush()
{
  uint8_t reg, addrbuf[5];
  int enaa=0, origrx=0;
  
  if (!txbuf_len)
    return;  // Zero-length buffer?  Nothing to send!
  
  reg = Enrf24__readReg(RF24_FIFO_STATUS);
  if (reg & BIT5) {  // RF24_TX_FULL #define is BIT0, which is not the correct bit for FIFO_STATUS.
    // Seen this before with a user whose CE pin was messed up.
    Enrf24__issueCmd(RF24_FLUSH_TX);
    txbuf_len = 0;
    return;  // Should never happen, but nonetheless a precaution to take.
  }
  
  Enrf24__maintenanceHook();
  
  if (reg & RF24_TX_REUSE) {
    // If somehow TX_REUSE is enabled, we need to flush the TX queue before loading our new payload.
    Enrf24__issueCmd(RF24_FLUSH_TX);
  }
  
  if (Enrf24__readReg(RF24_EN_AA) & 0x01 && (Enrf24__readReg(RF24_RF_SETUP) & 0x28) != 0x20) {
    /* AutoACK enabled, must write TX addr to RX pipe#0
    * Note that 250Kbps doesn't support auto-ack, so we check RF24_RF_SETUP to verify that.
    */
    enaa = 1;
    Enrf24__readTXaddr(addrbuf);
    Enrf24__writeRXaddrP0(addrbuf);
  }
  
  reg = Enrf24__readReg(RF24_CONFIG);
  if ( !(reg & RF24_PWR_UP) ) {
    //digitalWrite(_cePin, HIGH);  // Workaround for SI24R1 knockoff chips
    Enrf24__writeReg(RF24_CONFIG, ENRF24_CFGMASK_IRQ | ENRF24_CFGMASK_CRC(reg) | RF24_PWR_UP);
    delay(5);  // 5ms delay required for nRF24 oscillator start-up
    //digitalWrite(_cePin, LOW);
  }
  if (reg & RF24_PRIM_RX) {
    origrx=1;
    digitalWrite(_cePin, LOW);
    Enrf24__writeReg(RF24_CONFIG, ENRF24_CFGMASK_IRQ | ENRF24_CFGMASK_CRC(reg) | RF24_PWR_UP);
  }
  
  Enrf24__issueCmdPayload(RF24_W_TX_PAYLOAD, txbuf, txbuf_len);
  digitalWrite(_cePin, HIGH);
  delayMicroseconds(100);
  digitalWrite(_cePin, LOW);
  
  txbuf_len = 0;  // Reset TX ring buffer
  
  while (digitalRead(_irqPin) == HIGH)  // Wait until IRQ fires
    ;
  // IRQ fired
  Enrf24__maintenanceHook();  // Handle/clear IRQ
  
  // Purge Pipe#0 address (set to module's power-up default)
  if (enaa) {
    addrbuf[0] = 0xE7; addrbuf[1] = 0xE7; addrbuf[2] = 0xE7; addrbuf[3] = 0xE7; addrbuf[4] = 0xE7;
    Enrf24__writeRXaddrP0(addrbuf);
  }
  
  // If we were in RX mode before writing, return back to RX mode.
  if (origrx) {
    Enrf24_enableRX();
  }
}

void Enrf24_purge()
{
  txbuf_len = 0;
}

size_t Enrf24_write(uint8_t c)
{
  if (txbuf_len == 32) {  // If we're trying to stuff an already-full buffer...
    Enrf24_flush();  // Blocking OTA TX
  }
  
  txbuf[txbuf_len] = c;
  txbuf_len++;
  
  return 1;
}

uint8_t Enrf24_radioState()
{
  uint8_t reg;
  
  if (!Enrf24__isAlive())
    return ENRF24_STATE_NOTPRESENT;
  
  reg = Enrf24__readReg(RF24_CONFIG);
  if ( !(reg & RF24_PWR_UP) )
    return ENRF24_STATE_DEEPSLEEP;
  
  // At this point it's either Standby-I, II or PRX.
  if (reg & RF24_PRIM_RX) {
    if (digitalRead(_cePin))
      return ENRF24_STATE_PRX;
    // PRIM_RX=1 but CE=0 is a form of idle state.
    return ENRF24_STATE_IDLE;
  }
  // Check if TX queue is empty, if so it's idle, if not it's PTX.
  if (Enrf24__readReg(RF24_FIFO_STATUS) & RF24_TX_EMPTY)
    return ENRF24_STATE_IDLE;
  return ENRF24_STATE_PTX;
}

void Enrf24_deepsleep()
{
  uint8_t reg;
  
  reg = Enrf24__readReg(RF24_CONFIG);
  if (reg & (RF24_PWR_UP | RF24_PRIM_RX)) {
    Enrf24__writeReg(RF24_CONFIG, ENRF24_CFGMASK_IRQ | ENRF24_CFGMASK_CRC(reg));
  }
  digitalWrite(_cePin, LOW);
}

void Enrf24_enableRX()
{
  uint8_t reg;
  
  reg = Enrf24__readReg(RF24_CONFIG);
  Enrf24__writeReg(RF24_CONFIG, ENRF24_CFGMASK_IRQ | ENRF24_CFGMASK_CRC(reg) | RF24_PWR_UP | RF24_PRIM_RX);
  digitalWrite(_cePin, HIGH);
  
  if ( !(reg & RF24_PWR_UP) ) {  // Powering up from deep-sleep requires 5ms oscillator start delay
    delay(5);
  }
}

void Enrf24_disableRX()
{
  uint8_t reg;
  
  digitalWrite(_cePin, LOW);
  
  reg = Enrf24__readReg(RF24_CONFIG);
  if (reg & RF24_PWR_UP) {  /* Keep us in standby-I if we're coming from RX mode, otherwise stay
    * in deep-sleep if we call this while already in PWR_UP=0 mode.
    */
    Enrf24__writeReg(RF24_CONFIG, ENRF24_CFGMASK_IRQ | ENRF24_CFGMASK_CRC(reg) | RF24_PWR_UP);
  } else {
    Enrf24__writeReg(RF24_CONFIG, ENRF24_CFGMASK_IRQ | ENRF24_CFGMASK_CRC(reg));
  }
}

void Enrf24_autoAck(int onoff)
{
  uint8_t reg;
  
  reg = Enrf24__readReg(RF24_EN_AA);
  if (onoff) {
    if ( !(reg & 0x01) || !(reg & 0x02) ) {
      Enrf24__writeReg(RF24_EN_AA, 0x03);
    }
  } else {
    if (reg & 0x03) {
      Enrf24__writeReg(RF24_EN_AA, 0x00);
    }
  }
}

void Enrf24_setChannel(uint8_t channel)
{
  if (channel > 125)
    channel = 125;
  Enrf24__writeReg(RF24_RF_CH, channel);
}

void Enrf24_setTXpower(int8_t dBm)
{
  uint8_t reg, pwr;
  
  reg = Enrf24__readReg(RF24_RF_SETUP) & 0xF8;  // preserve RF speed settings
  pwr = 0x06;
  if (dBm >= 7)
    pwr = 0x07;
  if (dBm < 0)
    pwr = 0x04;
  if (dBm < -6)
    pwr = 0x02;
  if (dBm < -12)
    pwr = 0x00;
  Enrf24__writeReg(RF24_RF_SETUP, reg | pwr);
}

void Enrf24_setSpeed(uint32_t rfspeed)
{
  uint8_t reg, spd;
  
  reg = Enrf24__readReg(RF24_RF_SETUP) & 0xD7;  // preserve RF power settings
  spd = 0x01;
  if (rfspeed < 2000000)
    spd = 0x00;
  if (rfspeed < 1000000)
    spd = 0x04;
  Enrf24__writeReg(RF24_RF_SETUP, reg | (spd << 3));
}

void Enrf24_setCRC(int onoff, int crc16bit)
{
  uint8_t reg, crcbits=0;
  
  reg = Enrf24__readReg(RF24_CONFIG) & 0xF3;  // preserve IRQ mask, PWR_UP/PRIM_RX settings
  if (onoff)
    crcbits |= RF24_EN_CRC;
  if (crc16bit)
    crcbits |= RF24_CRCO;
  Enrf24__writeReg(RF24_CONFIG, reg | crcbits);
}

void Enrf24_setAutoAckParams(uint8_t autoretry_count, uint16_t autoretry_timeout)
{
  uint8_t setup_retr=0;
  
  setup_retr = autoretry_count & 0x0F;
  autoretry_timeout -= 250;
  setup_retr |= ((autoretry_timeout / 250) & 0x0F) << 4;
  Enrf24__writeReg(RF24_SETUP_RETR, setup_retr);
}

void Enrf24_setAddressLength(size_t len)
{
  if (len < 3)
    len = 3;
  if (len > 5)
    len = 5;
  
  Enrf24__writeReg(RF24_SETUP_AW, len-2);
  rf_addr_width = len;
}

void Enrf24_setRXaddress(const void *rxaddr)
{
  Enrf24__writeRegMultiLSB(RF24_RX_ADDR_P1, (uint8_t*)rxaddr, rf_addr_width);
}

void Enrf24_setTXaddress(const void *rxaddr)
{
  Enrf24__writeRegMultiLSB(RF24_TX_ADDR, (uint8_t*)rxaddr, rf_addr_width);
}

int Enrf24_rfSignalDetected()
{
  uint8_t rpd;
  
  rpd = Enrf24__readReg(RF24_RPD);
  return (int)rpd;
}

uint32_t Enrf24_getSpeed()
{
  uint8_t reg = Enrf24__readReg(RF24_RF_SETUP) & 0x28;
  
  switch (reg) {
  case 0x00:
    return 1000000UL;
  case 0x08:
    return 2000000UL;
  case 0x20:
    return 250000UL;
  }
  return 0UL;
}

int8_t Enrf24_getTXpower()
{
  uint8_t reg = Enrf24__readReg(RF24_RF_SETUP) & 0x07;
  
  if (reg & 0x01)
    return 7;  // SI24R1-only +7dBm mode
  switch (reg) {
  case 0x02:
    return -12;
  case 0x04:
    return -6;
  case 0x06:
    return 0;
  }
  return -18;
}

int Enrf24_getAutoAck()
{
  uint8_t reg = Enrf24__readReg(RF24_EN_AA);
  
  if (reg)
    return 1;
  return 0;
}

void Enrf24_getRXaddress(void *buf)
{
  Enrf24__readRegMultiLSB(RF24_RX_ADDR_P1, (uint8_t*)buf, rf_addr_width);
}

void Enrf24_getTXaddress(void *buf)
{
  Enrf24__readRegMultiLSB(RF24_TX_ADDR, (uint8_t*)buf, rf_addr_width);
}

unsigned int Enrf24_getCRC()
{
  uint8_t reg = Enrf24__readReg(RF24_CONFIG) & 0x0C;
  
  switch (reg) {
  case 0x08:
    return 8;
  case 0x0C:
    return 16;
  }
  
  return 0;
}

