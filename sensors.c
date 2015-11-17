#include "sensors.h"
#include "one_wire.h"

#include "stm32f10x.h"

#include "usart.h"

struct SensorData sensors;

#define ADC1_DR_Address    ((uint32_t)0x4001244C)

__IO uint16_t ADCConvertedValue[6];

enum 
{
  OneWire_Idle = 1,
  OneWire_Delay,
  OneWire_StartConv,
  OneWire_GetValue
} OW_CurrState, OW_NextState;

unsigned int OW_Delay = 0, SS_Debug_delay = 0;

typedef unsigned char byte;
byte i;
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];
float celsius, fahrenheit;

void Sensors_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  ADC_InitTypeDef ADC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;
  
  // following codes are call in previous functions
  //#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
  //  /* ADCCLK = PCLK2/2 */
  //  RCC_ADCCLKConfig(RCC_PCLK2_Div2); 
  //#else
  //  /* ADCCLK = PCLK2/4 */
  //  RCC_ADCCLKConfig(RCC_PCLK2_Div4); 
  //#endif
  /* Enable peripheral clocks ------------------------------------------------*/
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  /* Enable ADC1 and GPIOC clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE);
  
  
  /* Configure PB0, PB1 (ADC Channel8, Channel9) as analog input -------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 6;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 2;
  ADC_Init(ADC1, &ADC_InitStructure);
  
  /* ADC1 regular channelx configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_239Cycles5);
  
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  
  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));
  
  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
  
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  while(ADC_GetSoftwareStartConvStatus(ADC1));
  
  OW_CurrState = OneWire_Delay;
  OW_NextState = OneWire_Idle;
  OW_Delay = 3;
}

void Sensors_Poll(void)
{
  /* This function must be call 100 ms periods */
  
  // recalculate sensor values
  sensors.Gas = (ADCConvertedValue[0] + ADCConvertedValue[2] + ADCConvertedValue[4])/3;
  sensors.Lighting = (ADCConvertedValue[1] + ADCConvertedValue[3] + ADCConvertedValue[5])/3;
  
#if SENSORS_DEBUG
  if (SS_Debug_delay == 0)
  {
    USART1_SendStr("\nGas Value: ");
    USART1_SendFloat(sensors.Gas);
    USART1_SendStr(" kppm.\n");
    USART1_SendStr("\nLighting Value: ");
    USART1_SendFloat(sensors.Lighting);
    USART1_SendStr(" Lux.\n");
    SS_Debug_delay = 10;
  }
  else
  {
    SS_Debug_delay--;
  }
#endif
  
  switch (OW_CurrState)
  {
  case OneWire_Idle:
    if ( !OneWire_search(addr)) {
#if SENSORS_DEBUG
			USART1_SendStr("\nNo more addresses.\n");
			USART1_SendStr("\n\n");
#endif
			OneWire_reset_search();
      // delay 300 ms before try again
      OW_CurrState = OneWire_Delay;
      OW_NextState = OneWire_Idle;
      OW_Delay = 3;
		}
    else
    {
      OW_CurrState = OneWire_Delay;
      OW_NextState = OneWire_StartConv;
      OW_Delay = 3;
    }
    break;
  case OneWire_Delay:
    if (OW_Delay == 0)
    {
      // switch state
      OW_CurrState = OW_NextState;
    }
    else
    {
      OW_Delay--;
    }
    break;
  case OneWire_StartConv:
#if SENSORS_DEBUG
    USART1_SendStr("ROM =");
    for( i = 0; i < 8; i++) {
      USART1_SendChar(' ');
      USART1_SendByte(addr[i], HEX);
    }
#endif
    
    if (OneWire_crc8(addr, 7) != addr[7]) {
#if SENSORS_DEBUG
      USART1_SendStr("\nCRC is not valid!\n");
#endif
      // goto delay 300ms before switch to idle to find device again
      OW_CurrState = OneWire_Delay;
      OW_NextState = OneWire_Idle;
      OW_Delay = 3;
    }
    
#if SENSORS_DEBUG
    USART1_SendStr("\n\n");
#endif
    
    // the first ROM byte indicates which chip
    switch (addr[0]) {
    case 0x10:
#if SENSORS_DEBUG
      USART1_SendStr("\n  Chip = DS18S20\n");  // or old DS1820
#endif
      type_s = 1;
      break;
    case 0x28:
#if SENSORS_DEBUG
      USART1_SendStr("\n  Chip = DS18B20\n");
#endif
      type_s = 0;
      break;
    case 0x22:
#if SENSORS_DEBUG
      USART1_SendStr("\n  Chip = DS1822\n");
#endif
      type_s = 0;
      break;
    default:
#if SENSORS_DEBUG
      USART1_SendStr("\nDevice is not a DS18x20 family device.\n");
#endif
      
      // goto delay 300ms before switch to idle to find device again
      OW_CurrState = OneWire_Delay;
      OW_NextState = OneWire_Idle;
      OW_Delay = 3;
      break;
    } 
    
    OneWire_reset();
    OneWire_select(addr);
    OneWire_write(0x44, 1);        // start conversion, with parasite power on at the end
    
    // goto delay 1000ms before read data
    OW_CurrState = OneWire_Delay;
    OW_NextState = OneWire_GetValue;
    OW_Delay = 10;
    break;
  case OneWire_GetValue:
    present = OneWire_reset();
    OneWire_select(addr);    
    OneWire_write(0xBE, 0);         // Read Scratchpad
    
#if SENSORS_DEBUG
    USART1_SendStr("  Data = ");
    USART1_SendByte(present, HEX);
    USART1_SendStr(" ");
#endif
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = OneWire_read();
#if SENSORS_DEBUG
      USART1_SendByte(data[i], HEX);
      USART1_SendStr(" ");
#endif
    }
#if SENSORS_DEBUG
    USART1_SendStr(" CRC=");
    USART1_SendByte(OneWire_crc8(data, 8), HEX);
    USART1_SendStr("\n\n");
#endif
    
    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
      raw = raw << 3; // 9 bit resolution default
      if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
    } else {
      byte cfg = (data[4] & 0x60);
      // at lower res, the low bits are undefined, so let's zero them
      if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
      //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    fahrenheit = celsius * 1.8 + 32.0;
#if SENSORS_DEBUG
    USART1_SendStr("  Temperature = ");
    USART1_SendFloat(celsius);
    USART1_SendStr(" Celsius, ");
    USART1_SendFloat(fahrenheit);
    USART1_SendStr(" Fahrenheit\n");
#endif
    sensors.TempC = celsius;
    
    // goto delay 300ms before next convert
    OW_CurrState = OneWire_Delay;
    OW_NextState = OneWire_StartConv;
    OW_Delay = 3;
    break;                   
  } // switch
  
}
