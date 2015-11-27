#include "sim900.h"
#include "sim_hal.h"
#include <string.h>

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
  sim_pow_on_stage1,
  sim_pow_on_stage2,
  sim_pow_on_stage3,
} SimState_t;

int sms_available = 0;
#define SMS_BUFF_SIZE   128
char sms_send_buff[SMS_BUFF_SIZE];
char sms_recv_buff[SMS_BUFF_SIZE];
char sms_temp_buff[SMS_BUFF_SIZE];

int sim_delay_count = 0;
int sim_wait_count = 0;
int sim_skip_count = 0;

SimState_t sim_curr_state = sim_idle, sim_next_state = sim_idle, sim_skip_state = sim_idle;

// sim900 module status
char sim900_power_on = 0;
char sim900_sim_plug = 0;

int Sim900_Process(void)
{
  if (sim_hal_Available())
  {
    sim_hal_GetData(sim_temp_buff, sim_hal_Available());
    
    // process response message here
    // with any response message, it mean power is on 
    sim900_power_on = 1;
    
    //printf("SIM900 received message: %s.\n", Sim900RxBuffer);
    
    // common response
    if (memcmp("\r\n", Sim900RxBuffer, strlen("\r\n")) == 0)
    {
      if (sim_curr_state == sim_sms_sending_stage1)
      {
        sim_curr_state = sim_next_state;
      }
    }
    else if (memcmp("OK\r\n", Sim900RxBuffer, strlen("OK\r\n")) == 0)
    {
      // switch to next state
      sim_curr_state = sim_next_state;
    }
    
    // power and sim
    else if (memcmp("NORMAL POWER DOWN\r\n", Sim900RxBuffer, strlen("NORMAL POWER DOWN\r\n")) == 0)
    {
      sim900_power_on = 0;
    }
    else if (memcmp("RDY\r\n", Sim900RxBuffer, strlen("RDY\r\n")) == 0)
    {
      sim900_power_on = 1;
    }
    else if (memcmp(Sim900RxBuffer, "+CPIN: READY\r\n", strlen("+CPIN: READY\r\n")) == 0)
    {
      sim900_sim_plug = 1;
    }
    else if (memcmp(Sim900RxBuffer, "+CPIN: NOT INSERTED\r\n", strlen("+CPIN: NOT INSERTED\r\n")) == 0)
    {
      sim900_sim_plug = 0;
    }
    else if (memcmp(Sim900RxBuffer, "Call Ready\r\n", strlen("Call Ready\r\n")) == 0)
    {
      //        CallReady = SIM900_CALL_OK;
    }
    
    // SMS
    // setting charset
    else if (memcmp(Sim900RxBuffer, "AT+CSCS=\"GSM\"\r\n", strlen("AT+CSCS=\"GSM\"\r\n")) == 0)
    {
      sim_curr_state = sim_next_state;
    }
    // sms not setup
    else if (memcmp(Sim900RxBuffer, "+CMS ERROR: operation not allowed\r\n", strlen("+CMS ERROR: operation not allowed\r\n")) == 0)
    {
      sim_curr_state = sim_idle;
    }
    // sms setup ok
    else if (memcmp(Sim900RxBuffer, "AT+CMGF=1\r\n", strlen("AT+CMGF=1\r\n")) == 0)
    {
      sim_curr_state = sim_next_state;
    }
    // send number ok
    else if (memcmp(Sim900RxBuffer, "AT+CMGS=\"", strlen("AT+CMGS=\"")) == 0)
    {
      sim_curr_state = sim_next_state;
    }
    // send sms done
    else if (memcmp(Sim900RxBuffer, "+CMGS: ", strlen("+CMGS: ")) == 0)
    {
      sim_curr_state = sim_next_state;
    }
    // new sms coming
    else if (memcmp(Sim900RxBuffer, "+CMTI: \"SM\",", strlen("+CMTI: \"SM\",")) == 0)
    {
      // we need scan sms number
      sscanf(Sim900RxBuffer, "+CMTI: \"SM\",%d\r\n", &sms_number);
      sim_curr_state = sim_next_state;
    }
    // sms content read
    else if (memcmp(Sim900RxBuffer, "+CMGR: \"REC UNREAD\",\"", strlen("+CMGR: \"REC UNREAD\",\"")) == 0)
    {
      sim_curr_state = sim_next_state;
    }
    // read sms content
    else if (sms_new_message_available == 2)
    {
      memset(sms_content, 0, SMS_SIZE);
      strcpy(sms_content, Sim900RxBuffer);
      sms_content[strlen(Sim900RxBuffer)] = 0;
      sms_new_message_available = 1;
    }
    else if (memcmp(Sim900RxBuffer, "AT+CMGD=", strlen("AT+CMGD=")) == 0)
    {
      sms_status = SIM900_SMS_DELETE_OK;
    }
    else
    {
    }
  }
  
  switch(sim_curr_state)
  {
  case sim_idle:
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
  case sim_sms_receve:
    break;
  case sim_sms_sending: // setup charset
    sim_hal_sendStr("AT+CSCS=\"GSM\"\r");
    sim_curr_state = sim_wait;
    sim_next_state = sim_sms_sending_stage1;
    sim_wait_count = 10; // 1s
    break;
  case sim_sms_sending_stage1: // charset ok, setup sms
    sim_hal_sendStr("AT+CMGF=1\r");
    sim_curr_state = sim_wait;
    sim_next_state = sim_sms_sending_stage2;
    sim_wait_count = 10; // 1s
    break;
  case sim_sms_sending_stage2: // sms setup ok, send phone number
    
    break;
  case sim_sms_sending_stage3:
    break;
  case sim_pow_on_stage1:
    break;
  case sim_pow_on_stage2:
    break;
  case sim_pow_on_stage3:
    break;
  default:
    break;
  }
  
  return sim_curr_state;
}
int Sim900_SendSMS(const char * sms, const char * phone_number)
{
  
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
