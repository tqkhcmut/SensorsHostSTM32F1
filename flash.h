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
};

extern struct FlashData __flash_data;

extern int flash_write_buffer(char * buff, int size);
extern int flash_read_buffer(char * buff, int size);
extern int flash_test(void);

#endif
