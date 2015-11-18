#include "flash.h"
#include <string.h>

#define FLASH_WRITE_ADDR ((uint32_t)0x0800FC00)

struct FlashData __flash_data;

int flash_write_buffer(char * buff, int size)
{
  int tmp_buff[0x100], i;
  if (size > 0x400)
    return -1;
  memcpy(&tmp_buff, buff, size);
  
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  if (FLASH_ErasePage(FLASH_WRITE_ADDR) != FLASH_COMPLETE)
    return -1;
  for (i = 0; i < size / 4 + 1; i++)
  {
    if (FLASH_ProgramWord(FLASH_WRITE_ADDR + 4 * i, tmp_buff[i]) != FLASH_COMPLETE)
      return -1;
  }
  FLASH_Lock();
  
  memcpy(tmp_buff, (void const *)FLASH_WRITE_ADDR, size);
  return memcmp(tmp_buff, buff, size);
}

int flash_read_buffer(char * buff, int size)
{
  memcpy(buff, (void const *)FLASH_WRITE_ADDR, size);
  return 0;
}

int flash_test(void)
{
  char buffer[60];
  memcpy(buffer, "The Quick Brown Fox Jump Over The Lazy Dog 1234567890.", 54);
  flash_write_buffer(buffer, strlen(buffer));
  memset(buffer, 0, 60);
  flash_read_buffer(buffer, 60);
  return memcmp(buffer, "The Quick Brown Fox Jump Over The Lazy Dog 1234567890.", 54);
}