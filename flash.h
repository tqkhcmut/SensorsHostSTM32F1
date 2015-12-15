#ifndef _flash_h_
#define _flash_h_

#include "stm32f10x.h"
#include "packet.h"

typedef union
{
  uint8_t int_bytes[4];
  uint32_t uint_num;
} int_struct_t;

struct FlashData
{
  uint8_t id;
  uint8_t unique_number[4];
  struct Thesis _thesis;
  uint16_t random_number; 
};

extern const char InvalidUniqueNumber[];
extern const char Default_UniqueNumber[];
extern const char Default_id[];


#define Default_TempC    50
#define Default_Gas      2048
#define Default_Lighting 100

#define Default_Buzzer   1
#define Default_Speaker  1
#define Default_Relay    1

#define Default_tar_num_len  12
#define Default_tar_num  "016778805331"
#define Default_sms      1
#define Default_sms_msg  "Fire detected."

#define Default_time_poll 1000  // 1000ms

#define Default_hour      0x11
#define Default_minute    0x11
#define Default_second    0x11
#define Default_day_of_week 0x0
#define Default_day       0x11
#define Default_month     0x01
#define Default_year      0x15
#define Default_padding   0


extern struct FlashData __flash_data;

extern int flash_write_buffer(char * buff, int size);
extern int flash_read_buffer(char * buff, int size);
extern int flash_test(void);

#endif
