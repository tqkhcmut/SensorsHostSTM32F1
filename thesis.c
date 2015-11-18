#include "thesis.h"
#include "packet.h"
#include "flash.h"
#include "sensors.h"

#include <string.h>
#include <stdio.h>

int thesis_errn;
char thesis_err_msg[THESIS_MSG_SIZE];

unsigned char thesis_need_to_send;
char thesis_sent_msg[THESIS_MSG_SIZE];

//// my thesis error codes
//#define THESIS_OK                     1
//#define THESIS_PACKET_FAIL            2
//#define THESIS_PACKET_CHECKSUM_FAIL   3
//#define THESIS_PACKET_NULL            3

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
