#ifndef _flash_h_
#define _flash_h_

#include "stm32f10x.h"

struct Scheduler
{
  unsigned char start_hour;
  unsigned char start_minute;
  unsigned char stop_hour;
  unsigned char stop_minute;
  unsigned char light_level;
};

#define BT_NAME_SIZE    21
#define BT_PIN_SIZE     5

#ifdef LONG_ID
#define ID_LENGTH       4
extern const unsigned char BROADCAST_ID[ID_LENGTH];
extern const unsigned char CONNECT_BOX_ID[ID_LENGTH];
extern const unsigned char DEFAULT_ID[ID_LENGTH];
#else
#define ID_LENGTH       1
extern const unsigned char BROADCAST_ID[ID_LENGTH];
extern const unsigned char CONNECT_BOX_ID[ID_LENGTH];
extern const unsigned char DEFAULT_ID[ID_LENGTH];
#endif

extern const char * BT_PREFIX_NAME;
extern const char * BT_DEFAULT_PIN;

struct FlashData
{
  float power_offset;
  unsigned char id[ID_LENGTH];
  unsigned char timezone;
  char bt_name[BT_NAME_SIZE];
  char bt_pin[BT_PIN_SIZE];
  struct Scheduler sch[7];
};

extern int flash_write_buffer(char * buff, int size);
extern int flash_read_buffer(char * buff, int size);
extern int flash_test(void);

#endif
