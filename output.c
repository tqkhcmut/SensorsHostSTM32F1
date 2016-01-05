#include "output.h"

#include <stm32f10x.h>

#define BUZZER_PIN    GPIO_Pin_13
#define BUZZER_PORT   GPIOC
#define SPEAKER_PIN   GPIO_Pin_14
#define SPEAKER_PORT  GPIOC
#define RELAY_PIN     GPIO_Pin_15
#define RELAY_PORT    GPIOC

void OutputInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
	/* PWR and BKP Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
 
  /* GPIOC Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
 
  PWR_BackupAccessCmd(ENABLE);
 
  RCC_LSEConfig(RCC_LSE_OFF); // Disable LSE - PC14 PC15 as GPIO 
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  
  GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;
  GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = SPEAKER_PIN;
  GPIO_Init(SPEAKER_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = RELAY_PIN;
  GPIO_Init(RELAY_PORT, &GPIO_InitStructure);
		
	TurnBuzzerOff();
	TurnSpeakerOff();
	TurnRelayOff();
}
unsigned char buzzer_status = 0;
unsigned char speaker_status = 0;
unsigned char relay_status = 0;

void TurnBuzzerOn(void)
{
	GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN, Bit_RESET); // OFF
	buzzer_status = 1;
}
                                                                                                                                                                                                                                                                                      
void TurnBuzzerOff(void)
{
	if (buzzer_status == 1)
	{
  	GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN, Bit_SET); // OFF
		speaker_status = 0;
	}
}

void TurnSpeakerOn(void)
{
	GPIO_WriteBit(SPEAKER_PORT, SPEAKER_PIN, Bit_RESET); // OFF
	speaker_status = 1;
}
void TurnSpeakerOff(void)
{
	if (speaker_status == 1)
	{
		GPIO_WriteBit(SPEAKER_PORT, SPEAKER_PIN, Bit_SET); // OFF
		speaker_status = 0;
	}
}

void TurnRelayOn(void)
{
  	GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET); // OFF
	speaker_status = 1;
}
void TurnRelayOff(void)
{
	if (speaker_status == 1)
	{
		GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET); // OFF
		speaker_status = 0;
	}
}
