#include "ds1307.h"

#define DS1307_ADDRESS_READ     0xD1
#define DS1307_ADDRESS_WRITE	0xD0

/* Private defines */
#define I2C_TRANSMITTER_MODE   0
#define I2C_RECEIVER_MODE      1
#define I2C_ACK_ENABLE         1
#define I2C_ACK_DISABLE        0

int16_t I2C_Start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction, uint8_t ack);
void I2C_WriteData(I2C_TypeDef* I2Cx, uint8_t data);
uint8_t I2C_ReadAck(I2C_TypeDef* I2Cx);
uint8_t I2C_ReadNack(I2C_TypeDef* I2Cx);
uint8_t I2C_Stop(I2C_TypeDef* I2Cx);
uint8_t I2C_IsDeviceConnected(I2C_TypeDef* I2Cx, uint8_t address);

uint8_t ds1307_getReg(uint8_t addr)
{
  uint8_t received_data;
  I2C_Start(I2C1, DS1307_ADDRESS_WRITE, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
  I2C_WriteData(I2C1, addr);
  I2C_Stop(I2C1);
  I2C_Start(I2C1, DS1307_ADDRESS_READ, I2C_RECEIVER_MODE, I2C_ACK_DISABLE);
  received_data = I2C_ReadNack(I2C1);
  return received_data;
}

uint8_t ds1307_setReg(uint8_t addr, uint8_t value)
{
  I2C_Start(I2C1, DS1307_ADDRESS_WRITE, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
  I2C_WriteData(I2C1, addr);
  I2C_WriteData(I2C1, value);
  I2C_Stop(I2C1);
  return 0;
}

int DS1307_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  I2C_InitTypeDef I2C_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  I2C_InitStructure.I2C_ClockSpeed = 100000;
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(I2C1, &I2C_InitStructure);
  I2C_Cmd(I2C1, ENABLE);
  
  return 0;
}

uint8_t Bcd2Bin(uint8_t bcd) 
{
  uint8_t dec = 10 * (bcd >> 4);
  dec += bcd & 0x0F;
  return dec;
}

uint8_t Bin2Bcd(uint8_t bin) 
{
  uint8_t low = 0;
  uint8_t high = 0;
  high = bin / 10;
  low = bin - (high * 10);
  return high << 4 | low;
}


uint8_t CheckMinMax(uint8_t val, uint8_t min, uint8_t max) 
{
  if (val < min) 
  {
    return min;
  } 
  else if (val > max) 
  {
    return max;
  }
  return val;
}

int DS1307_GetTime(Time_t * t)
{
  uint8_t byte1, byte2, byte3, byte4;
  byte1 = ds1307_getReg(0);
  byte2 = ds1307_getReg(1);
  byte3 = ds1307_getReg(2);
  byte4 = ds1307_getReg(3);
    
  t->second = Bcd2Bin(byte1);
  t->minute = Bcd2Bin(byte2);
  t->hour = Bcd2Bin(byte3);
  t->day_of_week = Bcd2Bin(byte4);
  
  return 0;
}
int DS1307_SetTime(Time_t t)
{
  uint8_t byte1, byte2, byte3, byte4;
  
  if (t.day_of_week < 1 || t.day_of_week > 7)
    t.day_of_week = 1;
    
  byte1 = Bin2Bcd(CheckMinMax(t.second, 0, 59));
  byte2 = Bin2Bcd(CheckMinMax(t.minute, 0, 59));
  byte3 = Bin2Bcd(CheckMinMax(t.hour, 0, 23));
  byte4 = Bin2Bcd(CheckMinMax(t.day_of_week, 1, 7));
  
  ds1307_setReg(0, byte1);
  ds1307_setReg(1, byte2);
  ds1307_setReg(2, byte3);
  ds1307_setReg(3, byte4);
  
  return 0;
}
int DS1307_GetDate(Date_t * d)
{
  uint8_t byte1, byte2, byte3, byte4;
  byte1 = ds1307_getReg(4);
  byte2 = ds1307_getReg(5);
  byte3 = ds1307_getReg(6);
  byte4 = ds1307_getReg(7);
  
  d->day = Bcd2Bin(byte1);
  d->month = Bcd2Bin(byte2);
  d->year = Bcd2Bin(byte3);
  d->control = Bcd2Bin(byte4);
  
  return 0;
}
int DS1307_SetDate(Date_t d)
{
  uint8_t byte1, byte2, byte3, byte4;
  
  CheckMinMax(d.day, 0, 31);
    
  byte1 = Bin2Bcd(CheckMinMax(d.day, 1, 31));
  byte2 = Bin2Bcd(CheckMinMax(d.month, 1, 12));
  byte3 = Bin2Bcd(CheckMinMax(d.year, 0, 99));
  byte4 = Bin2Bcd(d.control);
  
  ds1307_setReg(4, byte1);
  ds1307_setReg(5, byte2);
  ds1307_setReg(6, byte3);
  ds1307_setReg(7, byte4);
  return 0;
}

int16_t I2C_Start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction, uint8_t ack) 
{
  int timeout = 0x2000;
  I2Cx->CR1 |= I2C_CR1_START;
  while (!(I2Cx->SR1 & I2C_SR1_SB)) 
  {
    if (--timeout == 0x00) 
    {
      return 1;
    }
  }
  if (ack) 
  {
    I2Cx->CR1 |= I2C_CR1_ACK;
  }
  if (direction == I2C_TRANSMITTER_MODE) 
  {
    int timeout1 = 0x2000;
    I2Cx->DR = address & ~I2C_OAR1_ADD0;
    while (!(I2Cx->SR1 & I2C_SR1_ADDR)) 
    {
      if (--timeout1 == 0x00) 
      {
        return 1;
      }
    }
  }
  if (direction == I2C_RECEIVER_MODE) 
  {
    int timeout2 = 0x2000;
    I2Cx->DR = address | I2C_OAR1_ADD0;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) 
    {
      if (--timeout2 == 0x00) 
      {
        return 1;
      }
    }
  }
  I2Cx->SR2;
  return 0;
}

void I2C_WriteData(I2C_TypeDef* I2Cx, uint8_t data) 
{
  int timeout = 0x2000;
  while (!(I2Cx->SR1 & I2C_SR1_TXE) && timeout) 
  {
    timeout--;
  }
  I2Cx->DR = data;
}

uint8_t I2C_ReadAck(I2C_TypeDef* I2Cx) 
{
  uint8_t data;
  int timeout = 0x2000;
  I2Cx->CR1 |= I2C_CR1_ACK;
  while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) 
  {
    if (--timeout == 0x00) 
    {
      return 1;
    }
  }
  data = I2Cx->DR;
  return data;
}

uint8_t I2C_ReadNack(I2C_TypeDef* I2Cx) 
{
  uint8_t data;
  int timeout = 0x2000;
  I2Cx->CR1 &= ~I2C_CR1_ACK;
  I2Cx->CR1 |= I2C_CR1_STOP;
  
  while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) 
  {
    if (--timeout == 0x00) 
    {
      return 1;
    }
  }
  data = I2Cx->DR;
  return data;
}

uint8_t I2C_Stop(I2C_TypeDef* I2Cx) 
{
  int timeout= 0x2000;
  while (((!(I2Cx->SR1 & I2C_SR1_TXE)) || (!(I2Cx->SR1 & I2C_SR1_BTF)))) 
  {
    if (--timeout == 0x00) 
    {
      return 1;
    }
  }
  I2Cx->CR1 |= I2C_CR1_STOP;
  return 0;
}

uint8_t I2C_IsDeviceConnected(I2C_TypeDef* I2Cx, uint8_t address) 
{
  uint8_t connected = 0;
  if (!I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_ENABLE)) 
  {
    connected = 1;
  }
  I2C_Stop(I2Cx);
  return connected;
}
