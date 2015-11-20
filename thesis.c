#include "thesis.h"
#include "packet.h"
#include "flash.h"
#include "sensors.h"

#include <string.h>
#include <stdio.h>

int thesis_errn = THESIS_OK;
char thesis_err_msg[THESIS_MSG_SIZE];

unsigned char thesis_need_to_send = 0;
char thesis_sent_msg[THESIS_MSG_SIZE];
int thesis_msg_len = 0;

uint16_t RDN(uint16_t start_state)
{
	uint16_t lfsr = start_state;
	uint16_t bit;                    /* Must be 16bit to allow bit<<15 later in the code */
  /* taps: 10 7; feedback polynomial: x^10 + x^7 + 1 */
  bit = ((lfsr >> 0) ^ (lfsr >> 3)) & 1;
  lfsr = (lfsr >> 1) | (bit << 9);
	return lfsr;
}

int IsValidString(char * str, int len)
{
  int i = 0;
  for (i = 0; i < len; i++)
  {
    if (str[i] > 'z' || str[i] < '!')
      return 0;
  }
  return i;
}

int ThesisInit(void)
{
  struct Thesis InvalidThesis;
  memset(&InvalidThesis, 0xff, sizeof (struct Thesis));
  // restore data saved in flash
  flash_read_buffer((char *)&__flash_data, sizeof (struct FlashData));
  if (IS_BROADCAST_ID(__flash_data.id))
  {
    // use default id
    __flash_data.id = Default_id[0];
  }
  if (memcmp(__flash_data.unique_number, InvalidUniqueNumber, 4) == 0)
  {
    memcpy(__flash_data.unique_number, Default_UniqueNumber, 4);
  }
  if (memcmp(&__flash_data._thesis, &InvalidThesis, sizeof (struct Thesis)) == 0)
  {
    __flash_data._thesis._data.Gas = Default_Gas;
    __flash_data._thesis._data.Lighting = Default_Lighting;
    __flash_data._thesis._data.TempC = Default_TempC;
    __flash_data._thesis._output.Buzzer = Default_Buzzer;
    __flash_data._thesis._output.Relay = Default_Relay;
    __flash_data._thesis._output.Speaker = Default_Speaker;
    __flash_data._thesis._time_poll.time_poll = Default_time_poll;
    __flash_data._thesis._sim.sms = Default_sms;
    memset(__flash_data._thesis._sim.sms_msg, 0, 50);
    memcpy(__flash_data._thesis._sim.sms_msg, Default_sms_msg, strlen(Default_sms_msg));
    memset(__flash_data._thesis._sim.tar_num, 0, 13);
    memcpy(__flash_data._thesis._sim.tar_num, Default_tar_num, strlen(Default_tar_num));
    __flash_data._thesis._sim.tar_num_len = Default_tar_num_len;
  }
  
  if (flash_write_buffer((char*)&__flash_data, sizeof(__flash_data)) != 0)
  {
    return THESIS_FLASH_ERROR;
  }
  return THESIS_OK;
}

int ThesisProcess(char * packet, int len)
{
  thesis_errn = THESIS_OK;
  memset(thesis_err_msg, 0, THESIS_MSG_SIZE);
  
  if (packet == NULL)
  {
    thesis_errn = THESIS_PACKET_NULL;
    strcpy(thesis_err_msg, "Thesis packet null.");
    return thesis_errn;
  }
    
  if (len < getPacketLength(packet))
  {
    thesis_errn = THESIS_PACKET_NOT_ENOUGH_LENGTH;
    strcpy(thesis_err_msg, "Packet not enough length.");
    return thesis_errn;
  }
  
  if (checksum(packet) != *(packet + getPacketLength(packet)))
  {
    thesis_errn = THESIS_PACKET_CHECKSUM_FAIL;
    strcpy(thesis_err_msg, "Packet checksum fail.");
    return thesis_errn;
  }
    
  // 
  struct Packet * _packet = (struct Packet *)packet;
  struct Packet * _sent_packet = (struct Packet *)thesis_sent_msg;
  
  if (_packet->id != __flash_data.id)
  {
    thesis_errn = THESIS_NOT_OWN_ID;
    strcpy(thesis_err_msg, "Not own id.");
    return thesis_errn;
  }
  
  if (memcmp(_packet->unique_number, __flash_data.unique_number, 4) != 0)
  {
    thesis_errn = THESIS_INVALID_UNIQUE_NUMBER;
    strcpy(thesis_err_msg, "Invalid unique number.");
    return thesis_errn;
  }
  
  switch(CMD_MASK(_packet->cmd))
  {
  case CMD_CLOCK:
    switch(CMD_TYPE_MASK(_packet->cmd))
    {
    case CMD_TYPE_QUERY:
      break;
    case CMD_TYPE_CONTROL:
      break;
    default:
      break;
    }
    break;
  case CMD_TIME_POLLING:
    switch(CMD_TYPE_MASK(_packet->cmd))
    {
    case CMD_TYPE_QUERY:
      break;
    case CMD_TYPE_CONTROL:
      break;
    default:
      break;
    }
    break;
  case CMD_SMS:
    switch(CMD_TYPE_MASK(_packet->cmd))
    {
    case CMD_TYPE_QUERY:
      break;
    case CMD_TYPE_CONTROL:
      break;
    default:
      break;
    }
    break;
  case CMD_SENSORS_VALUE:
    switch(CMD_TYPE_MASK(_packet->cmd))
    {
    case CMD_TYPE_QUERY:
      {
        struct ThesisData * _data = (struct ThesisData *)_sent_packet->data;
        memcpy(thesis_sent_msg, packet, len);
        _data->Gas = sensors.Gas;
        _data->Lighting = sensors.Lighting;
        _data->TempC = sensors.TempC;
        // clear command type
        _sent_packet->cmd = CMD_MASK(_sent_packet->cmd);
        // add return type
        _sent_packet->cmd |= CMD_TYPE_RESPONSE;
        // turn on flag to send back
				thesis_msg_len = getPacketLength(packet);
        thesis_need_to_send = 1;
        break;
      }
    case CMD_TYPE_CONTROL:
      thesis_errn = THESIS_CMD_NOT_SUPPORT;
      strcpy(thesis_err_msg, "Command not support.");
      break;
    default:
      break;
    }
    break;
  case CMD_OUTPUT:
    switch(CMD_TYPE_MASK(_packet->cmd))
    {
    case CMD_TYPE_QUERY:
      break;
    case CMD_TYPE_CONTROL:
      break;
    default:
      break;
    }
    break;
  case CMD_ID:
    switch(CMD_TYPE_MASK(_packet->cmd))
    {
    case CMD_TYPE_QUERY:
      break;
    case CMD_TYPE_CONTROL:
      break;
    default:
      break;
    }
    break;
  case CMD_UNIQUE_NUMBER:
    switch(CMD_TYPE_MASK(_packet->cmd))
    {
    case CMD_TYPE_QUERY:
      break;
    case CMD_TYPE_CONTROL:
      break;
    default:
      break;
    }
    break;
  default:
      thesis_errn = THESIS_CMD_NOT_SUPPORT;
      strcpy(thesis_err_msg, "Command not support.");
    break;
  }
  
  return thesis_errn;
}
