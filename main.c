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

#define BUFFER_SIZE 128

int main()
{
  RCC_ClocksTypeDef RCC_Clocks;
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
	int rf_len, usart_len;
	char buff_rf[BUFFER_SIZE];
	char buff_usart[BUFFER_SIZE];
  
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
    
    
		usart_len = USART1_Available();
		
		// usart: for test
		if (usart_len > 4)
		{
			int i;
			USART1_SendStr("\nUSART1 received packet: \n");
			USART1_GetData(buff_usart, usart_len);
			for (i = 0; i < usart_len; i++)
				USART1_SendByte(buff_usart[i], HEX);
			USART1_SendChar('\n');
			if (ThesisProcess(buff_usart, usart_len) == PACKET_OK)
			{
				memset(buff_usart, 0, usart_len);
				USART1_Flush();
				if (need_to_send)
				{
					int i;
					USART1_sendStr("\nNeed to send packet: ");
					for (i = 0; i < trans_buff_len; i++)
					{
						USART1_sendByte(trans_buff[i], HEX);
					}
					USART1_sendStr("\nNeed to send packet length: ");
					USART1_sendNum(trans_buff_len);
					USART1_sendStr("\n");
					trans_buff_len = 0;
					need_to_send = 0;
				}
				USART1_sendStr("\nPacket processed.\n"); 
			}
			else if (errn == FLASH_ERROR)
			{
				USART1_sendStr("\n");
				USART1_sendStr(err_msg);
				USART1_sendStr("\n");
				LEDs_TurnOn(LED_RUN);
				for(;;);
			}
			else if (errn != PACKET_NOT_ENOUGH_LENGTH)
			{
				memset(buff_usart, 0, usart_len);
				USART1_Flush();
				USART1_sendStr("Packet processing fail.\n");
			}
			
			USART1_sendStr("\n");
			USART1_sendStr(err_msg);
			USART1_sendStr("\n");
		}
		
		if (Enrf24_available(1))
		{
			int i;
			USART1_sendStr("\nRF received packet.\n");
			RF_GetData(buff_rf, rf_len);
			for (i = 0; i < rf_len; i++)
				USART1_sendByte(buff_rf[i], HEX);
			USART1_sendChar('\n');
			
			if (SmartDimmer_ProcessPacket(buff_rf, rf_len) == PACKET_OK)
			{
				memset(buff_rf, 0, rf_len);
				RF_Flush();
				if (need_to_send)
				{
					int i;
					USART1_sendStr("\nNeed to send packet: ");
					for (i = 0; i < trans_buff_len; i++)
					{
						RF_sendChar(trans_buff[i]);
						USART1_sendByte(trans_buff[i], HEX);
					}
					USART1_sendStr("\nNeed to send packet length: ");
					USART1_sendNum(trans_buff_len);
					USART1_sendStr("\n");
					trans_buff_len = 0;
					need_to_send = 0;
				}
				USART1_sendStr("\nPacket processed.\n");
			}
			else if (errn == FLASH_ERROR)
			{
				USART1_sendStr("\n");
				USART1_sendStr(err_msg);
				USART1_sendStr("\n");
				LEDs_TurnOn(LED_RUN);
				for(;;);
			}
			else if (errn != PACKET_NOT_ENOUGH_LENGTH)
			{
				memset(buff_rf, 0, rf_len);
				RF_Flush();
				USART1_sendStr("Packet processing fail.\n");
			}
			
			USART1_sendStr("\n");
			USART1_sendStr(err_msg);
			USART1_sendStr("\n");
		}
	}
	
}

