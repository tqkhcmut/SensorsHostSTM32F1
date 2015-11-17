#include "stm32f10x.h"
#include <string.h>

#define EVER (;;)

#include "delay.h"
#include "ds1307.h"

#include "flash.h"
#include "sim_hal.h"
#include "usart.h"
#include "one_wire.h"
#include "sensors.h"

#include "enrf24.h"

uint8_t enrf24_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x0A };

#define BUFFER_SIZE 128


uint8_t led_state = 0;
void led_toggle(void)
{
  if (led_state)
  {
    GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET); // OFF
    led_state = 0;
  }
  else
  {
    GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET);
    led_state = 1;
  }
}

int main()
{
  RCC_ClocksTypeDef RCC_Clocks;
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  
  unsigned int sensors_time_poll = 0, temp_time_poll = 0;
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET); // off
  
	Delay_Init();
  
  Enrf24_init(CE_PIN, CSN_PIN, IRQ_PIN);
	Enrf24_begin(1000000, 0);  // Defaults 1Mbps, channel 0, max TX power
  
  Enrf24_setTXaddress((void*)enrf24_addr);
  Enrf24_setRXaddress((void*)enrf24_addr);
  
  Enrf24_enableRX();  // Start listening
	
	USART1_Init();
	DS1307_Init();
  Sensors_Init();
  
	RCC_GetClocksFreq(&RCC_Clocks);
  USART1_SendStr("System Clock: ");
  USART1_SendNum(RCC_Clocks.SYSCLK_Frequency);
  USART1_SendStr("\r\n");
  
  
	OneWire_Init();
	for EVER
	{
    if (millis() - sensors_time_poll > 100)
    {
      led_toggle();
      Sensors_Poll();
      sensors_time_poll = millis();
    }
//    Enrf24_write_buff("Hello world.", 12);
//    Enrf24_flush();

	}
	
}

