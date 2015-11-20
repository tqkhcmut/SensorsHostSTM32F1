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
  
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, DISABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  
  GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;
  GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = SPEAKER_PIN;
  GPIO_Init(SPEAKER_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = RELAY_PIN;
  GPIO_Init(RELAY_PORT, &GPIO_InitStructure);
}

void TurnBuzzerOn(void)
{
  GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN, Bit_RESET); // OFF
}
                                                                                                                                                                                                                                                                                      
void TurnBuzzerOff(void)
{
  GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN, Bit_SET); // OFF
}

void TurnSpeakerOn(void)
{
  GPIO_WriteBit(SPEAKER_PORT, SPEAKER_PIN, Bit_RESET); // OFF
}
void TurnSpeakerOff(void)
{
  GPIO_WriteBit(SPEAKER_PORT, SPEAKER_PIN, Bit_SET); // OFF
}

void TurnRelayOn(void)
{
  GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET); // OFF
}
void TurnRelayOff(void)
{
  GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET); // OFF
}
