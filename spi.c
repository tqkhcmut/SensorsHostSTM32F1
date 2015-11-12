#include "spi.h"

#define SPI_MASTER                   SPI2
#define SPI_MASTER_CLK               RCC_APB1Periph_SPI2
#define SPI_MASTER_GPIO              GPIOB
#define SPI_MASTER_GPIO_CLK          RCC_APB2Periph_GPIOB  
#define SPI_MASTER_PIN_NSS           GPIO_Pin_12
#define SPI_MASTER_PIN_SCK           GPIO_Pin_13
#define SPI_MASTER_PIN_MISO          GPIO_Pin_14
#define SPI_MASTER_PIN_MOSI          GPIO_Pin_15

struct SPI spibus;
SPI_InitTypeDef   SPI_InitStructure;

static uint8_t spi_transceiver(uint8_t data)
{
  /* Wait for SPI_MASTER Tx buffer empty */
  while (SPI_I2S_GetFlagStatus(SPI_MASTER, SPI_I2S_FLAG_TXE) == RESET);
  /* Send SPI_MASTER data */
  SPI_I2S_SendData(SPI_MASTER, data);
  
  while (SPI_I2S_GetFlagStatus(SPI_MASTER, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(SPI_MASTER);
}

void NRF_SPI_Init(void)
{
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

  /* SPI_MASTER configuration ------------------------------------------------------*/
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI_MASTER, &SPI_InitStructure);

  /* Enable SPI_MASTER NSS output for master mode */
  SPI_SSOutputCmd(SPI_MASTER, ENABLE);

  /* Enable SPI_MASTER */
  SPI_Cmd(SPI_MASTER, ENABLE);
  
  spibus.transfer = spi_transceiver;
}
