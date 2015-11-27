#ifndef _lt3746_h_
#define _lt3746_h_

#include <stdint.h>

/* led enable */
#define LED_CHANNELS_ENABLE   1
#define LED_CHANNELS_DISABLE  0

/* frame type */
#define FRAME_TYPE_GRAYSCALE        0
#define FRAME_TYPE_DOT_CORRECTION   1

/* led status */
#define LED_FAULT 1
#define LED_OK    0

typedef enum 
{
  LT3746_MODE_6_WIRE = 1, // full loopback, with more feelback informations
  LT3746_MODE_4_WIRE,     // only SDO loopback, less feelback informations
  LT3746_MODE_3_WIRE      // don't care feelback informations
} LT3746_Mode_t;

struct LT3746_Frame
{
  uint16_t led_data[32];
  uint16_t led_status[32];
  uint16_t dot_correction[32];
  uint8_t over_temperature;
  uint8_t frame_type;
  uint8_t led_channels_enable;
};

int LT3746_Init(LT3746_Mode_t mode);
int LT3746_Transfer(struct LT3746_Frame * frame);
int LT3746_GetLedStatus(struct LT3746_Frame frame, uint8_t led_number);

#endif // _lt3746_h_

