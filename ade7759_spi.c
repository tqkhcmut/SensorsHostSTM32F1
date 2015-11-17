#include "ade7759_spi.h"

u8 _mode = 0;
u32 _speed = 1000000;

#define SPI_MASTER                   SPI2
#define SPI_MASTER_CLK               RCC_APB1Periph_SPI2
#define SPI_MASTER_GPIO              GPIOB
#define SPI_MASTER_GPIO_CLK          RCC_APB2Periph_GPIOB  
#define SPI_MASTER_PIN_NSS           GPIO_Pin_12
#define SPI_MASTER_PIN_SCK           GPIO_Pin_13
#define SPI_MASTER_PIN_MISO          GPIO_Pin_14
#define SPI_MASTER_PIN_MOSI          GPIO_Pin_15

void spi_setup(u32 speed, u8 mode)
{
  u16 baudrate_prescale;
  _speed = speed; 
  _mode = mode;
  /* PCLK2 = HCLK/4 */
  RCC_PCLK2Config(RCC_HCLK_Div4); 

  /* Enable SPI_MASTER Periph clock */
  RCC_APB1PeriphClockCmd(SPI_MASTER_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(SPI_MASTER_GPIO_CLK, ENABLE);
  
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure SPI_MASTER pins: NSS, SCK and MOSI */
  GPIO_InitStructure.GPIO_Pin = SPI_MASTER_PIN_SCK | SPI_MASTER_PIN_MOSI;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(SPI_MASTER_GPIO, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = SPI_MASTER_PIN_MISO;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(SPI_MASTER_GPIO, &GPIO_InitStructure);
  
  // 64MHz / 4 
  switch (speed)
  {
  case SPI_SPEED_SLOW:
    baudrate_prescale = SPI_BaudRatePrescaler_64;
    break;
  case SPI_SPEED_BURST:
    baudrate_prescale = SPI_BaudRatePrescaler_16;
    break;
  case SPI_SPEED_FAST:
    baudrate_prescale = SPI_BaudRatePrescaler_8;
    break;
  default:
    baudrate_prescale = SPI_BaudRatePrescaler_64;
    break;
  }

  /* SPI_MASTER configuration ------------------------------------------------------*/
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = baudrate_prescale;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI_MASTER, &SPI_InitStructure);

  /* Enable SPI_MASTER NSS output for master mode */
  SPI_SSOutputCmd(SPI_MASTER, ENABLE);

  /* Enable SPI_MASTER */
  SPI_Cmd(SPI_MASTER, ENABLE);
}
int spi_speed_set(u32 speed)
{
  _speed = speed;
  spi_setup(_speed, _mode);
  return 0;
}

int spi_speed_get(u32 * speed_buff)
{
  return _speed;
}

int spi_write(u8 * buffer, u32 buffer_len)
{
}

int spi_w8r8(u8 dummy)
{
  int ret = 0;
  /* Wait for SPI_MASTER Tx buffer empty */
  while (SPI_I2S_GetFlagStatus(SPI_MASTER, SPI_I2S_FLAG_TXE) == RESET);
  /* Send SPI_MASTER data */
  SPI_I2S_SendData(SPI_MASTER, dummy);
  
  while (SPI_I2S_GetFlagStatus(SPI_MASTER, SPI_I2S_FLAG_RXNE) == RESET);
  ret = SPI_I2S_ReceiveData(SPI_MASTER);
  return ret;
}
int spi_w8r16(u8 dummy)
{
  int ret = 0;
  /* Wait for SPI_MASTER Tx buffer empty */
  while (SPI_I2S_GetFlagStatus(SPI_MASTER, SPI_I2S_FLAG_TXE) == RESET);
  /* Send SPI_MASTER data */
  SPI_I2S_SendData(SPI_MASTER, dummy);
  
  while (SPI_I2S_GetFlagStatus(SPI_MASTER, SPI_I2S_FLAG_RXNE) == RESET);
  ret = SPI_I2S_ReceiveData(SPI_MASTER);
  return ret;
}
