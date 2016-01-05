#include "stm32f10x.h"
#include <string.h>

#define EVER (;;)

#include "delay.h"
#include "ds1307.h"

#include "flash.h"
#include "sim_hal.h"
#if DEBUG
#include "usart.h"
#endif
#include "one_wire.h"
#include "sensors.h"

#include "enrf24.h"

#include "thesis.h"
#include "rs485.h"
#include "output.h"

#include "sim900.h"

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

#define GAS_LIMIT 1500
#define LIGHT_LIMIT 100
#define TEMPC_LIMIT 40

int main()
{
  RCC_ClocksTypeDef RCC_Clocks;
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
	int rf_len = 0;
  int rs485_len = 0;
#if DEBUG
//  int usart_len = 0;
//	char buff_usart[BUFFER_SIZE];
#endif
	char buff_rf[BUFFER_SIZE];
	char buff_rs485[BUFFER_SIZE];
  
  unsigned int sensors_time_poll = 0;
//  int temp_time_poll = 0;
//  int sms_test_time = 0;
  
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
	
#if DEBUG
	USART1_Init(115200);
#endif
	DS1307_Init();
  Sensors_Init();
  RS485_Init(115200);
	sim_hal_init(115200);
  OutputInit();
  
  if (ThesisInit() == THESIS_FLASH_ERROR)
	{
#if DEBUG
		USART1_SendStr("\nFlash write error.\n");
#endif
		TurnBuzzerOn();
		Delay(1000);
	}
  
#if DEBUG
	RCC_GetClocksFreq(&RCC_Clocks);
  USART1_SendStr("System Clock: ");
  USART1_SendNum(RCC_Clocks.SYSCLK_Frequency);
  USART1_SendStr("\r\n");
  
  
  USART1_SendStr("Device ID: ");
  USART1_SendByte(__flash_data.id, HEX);
  USART1_SendStr("\r\n");
  USART1_SendStr("Device Unique Number: ");
  USART1_SendByte(__flash_data.unique_number[0], HEX);
  USART1_SendByte(__flash_data.unique_number[1], HEX);
  USART1_SendByte(__flash_data.unique_number[2], HEX);
  USART1_SendByte(__flash_data.unique_number[3], HEX);
  USART1_SendStr("\r\n");
#endif
  
  
	OneWire_Init();
	for EVER
	{
    if (millis() - sensors_time_poll > 100)
    {
      led_toggle();
      Sensors_Poll();
      sensors_time_poll = millis();
      //      buzzer_toggle();
      //      output_toggle();
      if (millis() > 10000)
      {
        if (sensors.Gas >= GAS_LIMIT)
        {
#if DEBUG
          USART1_SendStr("Gas detected.\r\n");
          USART1_SendStr("Current Gas: ");
          USART1_SendFloat(sensors.Gas);
          USART1_SendStr("Limited Gas: ");
          USART1_SendFloat(GAS_LIMIT);
          USART1_SendStr("\r\n");
#endif
          TurnBuzzerOn();
          TurnSpeakerOn();
          TurnRelayOn();
        }
        else if (sensors.Lighting >= LIGHT_LIMIT)
        {
#if DEBUG
          USART1_SendStr("Light detected.\r\n");
          USART1_SendStr("Current Light: ");
          USART1_SendFloat(sensors.Lighting);
          USART1_SendStr("Limited Light: ");
          USART1_SendFloat(LIGHT_LIMIT);
          USART1_SendStr("\r\n");
#endif
          TurnBuzzerOn();
          TurnSpeakerOn();
          TurnRelayOn();
        }
        else if (sensors.TempC >= TEMPC_LIMIT)
        {
#if DEBUG
          USART1_SendStr("Tempc detected.\r\n");
          USART1_SendStr("Current Tempc: ");
          USART1_SendFloat(sensors.TempC);
          USART1_SendStr("Limited Tempc: ");
          USART1_SendFloat(TEMPC_LIMIT);
          USART1_SendStr("\r\n");
#endif
          TurnBuzzerOn();
          TurnSpeakerOn();
          TurnRelayOn();
        }
        else
        {
          TurnBuzzerOff();
          TurnSpeakerOff();
          TurnRelayOff();
        }
      }
      
      //      Sim900_Process();
    }
    
    //    if (millis() - sms_test_time > 10000)
    //    {
    //      Sim900_SendSMS("Hi kieu", "01677880531");
    //      sms_test_time = millis();
    //    }
    
    //		usart_len = USART1_Available();
    //		
    //		// usart: for test
    //		if (usart_len > 4)
    //		{
    //			int i;
    //			USART1_SendStr("\nUSART1 received packet: \n");
    //			USART1_GetData(buff_usart, usart_len);
    //			for (i = 0; i < usart_len; i++)
    //				USART1_SendByte(buff_usart[i], HEX);
    //			USART1_SendChar('\n');
    //			if (ThesisProcess(buff_usart, usart_len) == THESIS_OK)
    //			{
    //				memset(buff_usart, 0, usart_len);
    //				USART1_Flush();
    //				if (thesis_need_to_send)
    //				{
    //					int i;
    //					USART1_SendStr("\nNeed to send packet: ");
    //					for (i = 0; i < thesis_msg_len; i++)
    //					{
    //						USART1_SendByte(thesis_sent_msg[i], HEX);
    //					}
    //					USART1_SendStr("\nNeed to send packet length: ");
    //					USART1_SendNum(thesis_msg_len);
    //					USART1_SendStr("\n");
    //					thesis_msg_len = 0;
    //					thesis_need_to_send = 0;
    //				}
    //				USART1_SendStr("\nPacket processed.\n"); 
    //			}
    //			else if (thesis_errn == THESIS_FLASH_ERROR)
    //			{
    //				USART1_SendStr("\n");
    //				USART1_SendStr(thesis_err_msg);
    //				USART1_SendStr("\n");
    //				led_toggle();
    //				for(;;);
    //			}
    //			else if (thesis_errn != THESIS_PACKET_NOT_ENOUGH_LENGTH)
    //			{
    //				memset(buff_usart, 0, usart_len);
    //				USART1_Flush();
    //				USART1_SendStr("Packet processing fail.\n");
    //			}
    //			
    //			USART1_SendStr("\n");
    //			USART1_SendStr(thesis_err_msg);
    //			USART1_SendStr("\n");
    //		}
		
    // rf
		if (Enrf24_available(1))
		{
			int i;
			rf_len = Enrf24_read(buff_rf + rf_len, BUFFER_SIZE - 1 - rf_len);
#if DEBUG
			USART1_SendStr("\nRF received packet.\n");
			for (i = 0; i < rf_len; i++)
				USART1_SendByte(buff_rf[i], HEX);
			USART1_SendChar('\n');
#endif
			
			if (ThesisProcess(buff_rf, rf_len) == THESIS_OK)
			{
				memset(buff_rf, 0, rf_len);
        rf_len = 0;
				if (thesis_need_to_send)
				{
					int i;
#if DEBUG
					USART1_SendStr("\nNeed to send packet: ");
					for (i = 0; i < thesis_msg_len; i++)
					{
            //						Enrf24_write(thesis_sent_msg[i]);
						USART1_SendByte(thesis_sent_msg[i], HEX);
					}
#endif
          Enrf24_write_buff(thesis_sent_msg, thesis_msg_len);
          Enrf24_flush();
#if DEBUG
					USART1_SendStr("\nNeed to send packet length: ");
					USART1_SendNum(thesis_msg_len);
					USART1_SendStr("\n");
#endif
					thesis_msg_len = 0;
					thesis_need_to_send = 0;
				}
#if DEBUG
				USART1_SendStr("\nPacket processed.\n");
#endif
			}
			else if (thesis_errn == THESIS_FLASH_ERROR)
			{
#if DEBUG
				USART1_SendStr("\n");
				USART1_SendStr(thesis_err_msg);
				USART1_SendStr("\n");
#endif
				led_toggle();
				for(;;);
			}
			else if (thesis_errn != THESIS_PACKET_NOT_ENOUGH_LENGTH)
			{
				memset(buff_rf, 0, rf_len);
        //				RF_Flush();
        rf_len = 0;
#if DEBUG
				USART1_SendStr("Packet processing fail.\n");
#endif
			}
			
#if DEBUG
			USART1_SendStr("\n");
			USART1_SendStr(thesis_err_msg);
			USART1_SendStr("\n");
#endif
		}
    
    // rs485
		rs485_len = RS485_Available();
		if (rs485_len > 4)
		{
			int i;
			RS485_GetData(buff_rs485, rs485_len);
#if DEBUG
			USART1_SendStr("\nUSART1 received packet: \n");
			for (i = 0; i < rs485_len; i++)
				USART1_SendByte(buff_rs485[i], HEX);
			USART1_SendChar('\n');
#endif
			if (ThesisProcess(buff_rs485, rs485_len) == THESIS_OK)
			{
				memset(buff_rs485, 0, rs485_len);
				RS485_Flush();
				if (thesis_need_to_send)
				{
					int i;
#if DEBUG
					USART1_SendStr("\nNeed to send packet: ");
#endif
          RS485_DIR_Output();
					for (i = 0; i < thesis_msg_len; i++)
					{
						RS485_SendChar(thesis_sent_msg[i]);
#if DEBUG
						USART1_SendByte(thesis_sent_msg[i], HEX);
#endif
					}
          RS485_DIR_Input();
#if DEBUG
					USART1_SendStr("\nNeed to send packet length: ");
					USART1_SendNum(thesis_msg_len);
					USART1_SendStr("\n");
#endif
					thesis_msg_len = 0;
					thesis_need_to_send = 0;
				}
#if DEBUG
				USART1_SendStr("\nPacket processed.\n"); 
#endif
			}
			else if (thesis_errn == THESIS_FLASH_ERROR)
			{
#if DEBUG
				USART1_SendStr("\n");
				USART1_SendStr(thesis_err_msg);
				USART1_SendStr("\n");
#endif
				led_toggle();
				for(;;);
			}
			else if (thesis_errn != THESIS_PACKET_NOT_ENOUGH_LENGTH)
			{
				memset(buff_rs485, 0, rs485_len);
				RS485_Flush();
#if DEBUG
				USART1_SendStr("Packet processing fail.\n");
#endif
			}
			
#if DEBUG
			USART1_SendStr("\n");
			USART1_SendStr(thesis_err_msg);
			USART1_SendStr("\n");
#endif
		}
    
	}
	
}

