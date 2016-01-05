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
  
  /* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  BKP_DeInit();

  /* Enable LSE */
  RCC_LSEConfig(RCC_LSE_OFF);
  
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
unsigned char buzzer_status = 1;
unsigned char speaker_status = 1;
unsigned char relay_status = 1;

void TurnBuzzerOn(void)
{
	if (buzzer_status == 0)
	{
#if OLD_PCB
  	GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN, Bit_SET); // OFF
#else
    GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN, Bit_RESET); // OFF
#endif
    buzzer_status = 1;
  }
}
                                                                                                                                                                                                                                                                                      
void TurnBuzzerOff(void)
{
	if (buzzer_status == 1)
	{
#if OLD_PCB
    GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN, Bit_RESET); // OFF
#else
  	GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN, Bit_SET); // OFF
#endif
		buzzer_status = 0;
	}
}

void TurnSpeakerOn(void)
{
	if (speaker_status == 0)
	{
#if OLD_PCB
		GPIO_WriteBit(SPEAKER_PORT, SPEAKER_PIN, Bit_SET); // OFF
#else
    GPIO_WriteBit(SPEAKER_PORT, SPEAKER_PIN, Bit_RESET); // OFF
#endif
    speaker_status = 1;
  }
}
void TurnSpeakerOff(void)
{
	if (speaker_status == 1)
	{
#if OLD_PCB
    GPIO_WriteBit(SPEAKER_PORT, SPEAKER_PIN, Bit_RESET); // OFF
#else
		GPIO_WriteBit(SPEAKER_PORT, SPEAKER_PIN, Bit_SET); // OFF
#endif
		speaker_status = 0;
	}
}

void TurnRelayOn(void)
{
	if (relay_status == 0)
	{
#if OLD_PCB
		GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET); // OFF
#else
  	GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET); // OFF
#endif
    relay_status = 1;
  }
}
void TurnRelayOff(void)
{
	if (relay_status == 1)
	{
#if OLD_PCB
  	GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET); // OFF
#else
		GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET); // OFF
#endif
		relay_status = 0;
	}
}
