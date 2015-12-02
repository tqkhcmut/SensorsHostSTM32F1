#include "sim900.h"
#include <string.h>
#include <stdio.h>

#ifndef USE_SIM_HAL
#define USE_SIM_HAL 1
#endif

#if USE_SIM_HAL
#include "sim_hal.h"
#else
#include "usart.h"
#endif
#include "rs485.h"

typedef enum SimState
{
  sim_idle = 0,
  sim_delay,
  sim_wait,
  sim_skip,
  sim_sms_receved,
  sim_sms_sending,        // setup charset, sending buffer, phone number..
  sim_sms_sending_stage1, // setup charset ok
  sim_sms_sending_stage2, // sms setup ok
  sim_sms_sending_stage3, // sms phone number ok
  sim_pow_unknown,			// unknown power state
  sim_pow_turn_on,			// turn on
  sim_pow_reset,			// reset 
	sim_plug_test				// test sim insert
} SimState_t;

int sms_available = 0;
#define SMS_BUFF_SIZE   128
char sms_send_buff[SMS_BUFF_SIZE];
char sms_recv_buff[SMS_BUFF_SIZE];
char sim_temp_buff[SMS_BUFF_SIZE];

char target_phone[15];

int sim_delay_count = 0;
int sim_wait_count = 0;
int sim_skip_count = 0;

SimState_t sim_curr_state = sim_idle, sim_next_state = sim_idle, sim_skip_state = sim_idle;

// sim900 module status
char sim900_power_on = 0;
char sim900_sim_plug = 0;

int Sim900_Process(void)
{
#if USE_SIM_HAL
  if (sim_hal_available())
#else
  if (USART1_Available())
#endif
  {
    memset(sim_temp_buff, 0, SMS_BUFF_SIZE);
#if USE_SIM_HAL
    sim_hal_get_data(sim_temp_buff, sim_hal_available());
    sim_hal_flush();
#else
    USART1_GetData(sim_temp_buff, USART1_Available());
    USART1_Flush();
#endif
    // process response message here
    // with any response message, it mean power is on 
    sim900_power_on = 1;
    
    //printf("SIM900 received message: %s.\n", Sim900RxBuffer);
    
    // common response
    if (memcmp("\r\n", sim_temp_buff, strlen("\r\n")) == 0)
    {
      if (sim_next_state == sim_sms_sending_stage3)
      {
        sim_curr_state = sim_next_state;
      }
//      RS485_SendStr("Got new line\r\n");
    }
    else if (memcmp("OK", sim_temp_buff, strlen("OK")) == 0)
    {
      // switch to next state
//      sim_curr_state = sim_next_state;
//      RS485_SendStr("Got OK\r\n");
    }
    
    // power and sim
    else if (memcmp("NORMAL POWER DOWN", sim_temp_buff, strlen("NORMAL POWER DOWN")) == 0)
    {
      sim900_power_on = 0;
//      RS485_SendStr("Got Power down\r\n");
    }
    else if (memcmp("RDY", sim_temp_buff, strlen("RDY")) == 0)
    {
      sim900_power_on = 1;
//      RS485_SendStr("Got RDY\r\n");
    }
    else if (memcmp(sim_temp_buff, "+CPIN: READY", strlen("+CPIN: READY")) == 0)
    {
      sim900_sim_plug = 1;
//      RS485_SendStr("Got sim ready\r\n");
    }
    else if (memcmp(sim_temp_buff, "+CPIN: NOT INSERTED", strlen("+CPIN: NOT INSERTED")) == 0)
    {
      sim900_sim_plug = 0;
//      RS485_SendStr("Got sim not insert\r\n");
    }
    else if (memcmp(sim_temp_buff, "Call Ready", strlen("Call Ready")) == 0)
    {
      //        CallReady = SIM900_CALL_OK;
//      RS485_SendStr("Got call ready\r\n");
    }
    
    // SMS
    // setting charset
    else if (memcmp(sim_temp_buff, "AT+CSCS=\"GSM\"", strlen("AT+CSCS=\"GSM\"")) == 0)
    {
      sim_curr_state = sim_next_state;
//      RS485_SendStr("Got AT+CSCS=\"GSM\"\r\n");
    }
    // sms not setup
    else if (memcmp(sim_temp_buff, "+CMS ERROR: operation not allowed", strlen("+CMS ERROR: operation not allowed")) == 0)
    {
      sim_curr_state = sim_idle;
//      RS485_SendStr("Got +CMS ERROR: operation not allowed\r\n");
    }
    // sms setup ok
    else if (memcmp(sim_temp_buff, "AT+CMGF=1", strlen("AT+CMGF=1")) == 0)
    {
      sim_curr_state = sim_next_state;
//      RS485_SendStr("Got AT+CMGF=1\r\n");
    }
    // send number ok
    else if (memcmp(sim_temp_buff, "AT+CMGS=\"", strlen("AT+CMGS=\"")) == 0)
    {
      sim_curr_state = sim_next_state;
//      RS485_SendStr("Got AT+CMGS=\"\r\n");
    }
    // send sms done
    else if (memcmp(sim_temp_buff, "+CMGS: ", strlen("+CMGS: ")) == 0)
    {
      sim_curr_state = sim_next_state;
//      RS485_SendStr("Got +CMGS: \r\n");
    }
//    // new sms coming
//    else if (memcmp(sim_temp_buff, "+CMTI: \"SM\",", strlen("+CMTI: \"SM\",")) == 0)
//    {
//      // we need scan sms number
//      sscanf(sim_temp_buff, "+CMTI: \"SM\",%d\r\n", &sms_number);
//      sim_curr_state = sim_next_state;
//    }
//    // sms content read
//    else if (memcmp(sim_temp_buff, "+CMGR: \"REC UNREAD\",\"", strlen("+CMGR: \"REC UNREAD\",\"")) == 0)
//    {
//      sim_curr_state = sim_next_state;
//    }
//    // read sms content
//    else if (sms_new_message_available == 2)
//    {
//      memset(sms_content, 0, SMS_SIZE);
//      strcpy(sms_content, Sim900RxBuffer);
//      sms_content[strlen(Sim900RxBuffer)] = 0;
//      sms_new_message_available = 1;
//    }
//    else if (memcmp(sim_temp_buff, "AT+CMGD=", strlen("AT+CMGD=")) == 0)
//    {
//      sms_status = SIM900_SMS_DELETE_OK;
//    }
    else
    {
//      RS485_SendStr("Not processed\r\n");
    }
  }
  
  switch(sim_curr_state)
  {
  case sim_idle:
		if (sim900_power_on == 0)
		{
			sim_curr_state = sim_pow_unknown;
		}
		if (sim900_sim_plug == 0)
		{
			sim_curr_state = sim_plug_test;
		}
    break;
  case sim_wait:
    if (sim_wait_count == 0)
    {
      sim_curr_state = sim_idle; // timeout
    }
    else
    {
      sim_wait_count--;
    }
    break;
  case sim_delay:
    if (sim_delay_count == 0)
    {
      sim_curr_state = sim_next_state;
    }
    else
    {
      sim_delay_count--;
    }
    break;
  case sim_skip:
    if (sim_skip_count == 0)
    {
      sim_curr_state = sim_skip_state;
    }
    else
    {
      sim_skip_count--;
    }
    break;
  case sim_sms_receved:
    break;
  case sim_sms_sending: // setup charset
#if USE_SIM_HAL
    sim_hal_send_string("AT+CSCS=\"GSM\"\r");
#else
    USART1_SendStr("AT+CSCS=\"GSM\"\r");
#endif
    sim_curr_state = sim_wait;
    sim_next_state = sim_sms_sending_stage1;
    sim_wait_count = 10; // 1s
    break;
  case sim_sms_sending_stage1: // charset ok, setup sms
#if USE_SIM_HAL
    sim_hal_send_string("AT+CMGF=1\r");
#else
    USART1_SendStr("AT+CMGF=1\r");
#endif
    sim_curr_state = sim_wait;
    sim_next_state = sim_sms_sending_stage2;
    sim_wait_count = 10; // 1s
    break;
  case sim_sms_sending_stage2: // sms setup ok, send phone number
    memset(sim_temp_buff, 0, SMS_BUFF_SIZE);
    sprintf(sim_temp_buff, "AT+CMGS=\"%s\"\r", target_phone);
#if USE_SIM_HAL
    sim_hal_send_string(sim_temp_buff);
#else
    USART1_SendStr(sim_temp_buff);
#endif
    sim_curr_state = sim_wait;
    sim_next_state = sim_sms_sending_stage3;
    sim_wait_count = 10; // 1s
    break;
  case sim_sms_sending_stage3: // target phone number sent, send message
    memset(sim_temp_buff, 0, SMS_BUFF_SIZE);
    strcpy(sim_temp_buff, sms_send_buff);
#if USE_SIM_HAL
    sim_hal_send_string(sim_temp_buff);
    sim_hal_send_char(0x1a);
#else
    USART1_SendStr(sim_temp_buff);
    USART1_SendChar(0x1a);
#endif
    sim_curr_state = sim_wait;
    sim_next_state = sim_idle;
    sim_wait_count = 30; // 3s
    break;
  case sim_pow_unknown:
		memset(sim_temp_buff, 0, SMS_BUFF_SIZE);
//    sprintf(sim_temp_buff, "AT\r", target_phone);
#if USE_SIM_HAL
    sim_hal_send_string("AT\r");
#else
    USART1_SendStr("AT\r");
#endif
    sim_curr_state = sim_delay;
    sim_next_state = sim_pow_turn_on;
    sim_wait_count = 10; // 1s
    break;
  case sim_pow_turn_on:
		if (sim900_power_on == 0)
		{
#if USE_SIM_HAL
			sim_hal_power_high(); // turn on
#else
#endif
			sim_curr_state = sim_delay;
			sim_next_state = sim_pow_unknown;
			sim_wait_count = 10; // 1s
		}
		else
		{
			sim_curr_state = sim_idle;
			sim_next_state = sim_idle;
		}
    break;
  case sim_pow_reset:
    break;
	case sim_plug_test:
		break;
  default:
    break;
  }
  
  return sim_curr_state;
}
int Sim900_SendSMS(const char * sms, const char * phone_number)
{
  if (sim_curr_state != sim_idle)
    return -1;
	if (sim900_sim_plug == 0) // sim not inserted
		return -1;
  memset(sms_send_buff, 0, SMS_BUFF_SIZE);
  strcpy(sms_send_buff, sms);
  memset(target_phone, 0, 15);
  strcpy(target_phone, phone_number);
  sim_curr_state = sim_sms_sending;
  return 0;
}
int Sim900_ReceiveSMS(char * sms_buff)
{
  
  return 0;
}
int Sim900_SMSAvailabe(void)
{
  
  return 0;
}
