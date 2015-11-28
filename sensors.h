#ifndef _sensors_h_
#define _sensors_h_

#include <stdint.h>

#ifndef SENSORS_DEBUG
#define SENSORS_DEBUG 0
#endif // SENSORS_DEBUG

struct SensorData
{
  float TempC;
  float Gas;
  float Lighting;
};

void Sensors_Init(void);
void Sensors_Poll(void);


extern struct SensorData sensors;

#endif
