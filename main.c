#include "stm32f10x.h"
#include <string.h>

#define EVER (;;)

#include "delay.h"
#include "ds1307.h"

#include "flash.h"
#include "sim_hal.h"
#include "usart.h"
#include "one_wire.h"

#include "enrf24.h"

unsigned char connect_box_active = 0;
unsigned char bt_first_run_config = 30, get_power_offset_flag = 1;

#define BUFFER_SIZE 128

typedef unsigned char byte;

byte i;
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];
float celsius, fahrenheit;
uint8_t led_state = 0;
void led_toggle(void)
{
  if (led_state)
  {
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_SET); // OFF
    led_state = 0;
  }
  else
  {
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_RESET);
    led_state = 1;
  }
}

int main()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_SET); // off
		
	Delay_Init();
	
	USART1_Init();
	DS1307_Init();
  
  Enrf24_init(CE_PIN, CSN_PIN, IRQ_PIN);
	Enrf24_begin(1000000, 0);  // Defaults 1Mbps, channel 0, max TX power
  
  Enrf24_setTXaddress((void*)addr);
  Enrf24_setRXaddress((void*)addr);
  
  Enrf24_enableRX();  // Start listening
  
	OneWire_Init();
	for EVER
	{
    led_toggle();
    Enrf24_write_buff("Hello world.", 12);
    Enrf24_flush();
//		if ( !OneWire_search(addr)) {
			USART1_SendStr("\nNo more addresses.\n");
//			USART1_SendStr("\n\n");
//			OneWire_reset_search();
//			delay(250);
//		}
//		else
//		{		
//			USART1_SendStr("ROM =");
//			for( i = 0; i < 8; i++) {
//				USART1_SendChar(' ');
//				USART1_SendByte(addr[i], HEX);
//			}
//			
//			if (OneWire_crc8(addr, 7) != addr[7]) {
//				USART1_SendStr("\nCRC is not valid!\n");
//				goto skip;
//			}
//			USART1_SendStr("\n\n");
//			
//			// the first ROM byte indicates which chip
//			switch (addr[0]) {
//			case 0x10:
//				USART1_SendStr("\n  Chip = DS18S20\n");  // or old DS1820
//				type_s = 1;
//				break;
//			case 0x28:
//				USART1_SendStr("\n  Chip = DS18B20\n");
//				type_s = 0;
//				break;
//			case 0x22:
//				USART1_SendStr("\n  Chip = DS1822\n");
//				type_s = 0;
//				break;
//			default:
//				USART1_SendStr("\nDevice is not a DS18x20 family device.\n");
//				goto skip;
//			} 
//			
//			OneWire_reset();
//			OneWire_select(addr);
//			OneWire_write(0x44, 1);        // start conversion, with parasite power on at the end
//			
//			delay(1000);     // maybe 750ms is enough, maybe not
//			// we might do a OneWire_depower() here, but the reset will take care of it.
//			
//			present = OneWire_reset();
//			OneWire_select(addr);    
//			OneWire_write(0xBE, 0);         // Read Scratchpad
//			
//			USART1_SendStr("  Data = ");
//			USART1_SendByte(present, HEX);
//			USART1_SendStr(" ");
//			for ( i = 0; i < 9; i++) {           // we need 9 bytes
//				data[i] = OneWire_read();
//				USART1_SendByte(data[i], HEX);
//				USART1_SendStr(" ");
//			}
//			USART1_SendStr(" CRC=");
//			USART1_SendByte(OneWire_crc8(data, 8), HEX);
//			USART1_SendStr("\n\n");
//			
//			// Convert the data to actual temperature
//			// because the result is a 16 bit signed integer, it should
//			// be stored to an "int16_t" type, which is always 16 bits
//			// even when compiled on a 32 bit processor.
//			int16_t raw = (data[1] << 8) | data[0];
//			if (type_s) {
//				raw = raw << 3; // 9 bit resolution default
//				if (data[7] == 0x10) {
//					// "count remain" gives full 12 bit resolution
//					raw = (raw & 0xFFF0) + 12 - data[6];
//				}
//			} else {
//				byte cfg = (data[4] & 0x60);
//				// at lower res, the low bits are undefined, so let's zero them
//				if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
//				else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
//				else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
//				//// default is 12 bit resolution, 750 ms conversion time
//			}
//			celsius = (float)raw / 16.0;
//			fahrenheit = celsius * 1.8 + 32.0;
//			USART1_SendStr("  Temperature = ");
//			USART1_SendFloat(celsius);
//			USART1_SendStr(" Celsius, ");
//			USART1_SendFloat(fahrenheit);
//			USART1_SendStr(" Fahrenheit\n");
//		}
//	skip:
		
		Delay(250);
	}
	
}

