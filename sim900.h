#ifndef _sim900_h_
#define _sim900_h_

int Sim900_Process(void);
int Sim900_SendSMS(const char * sms, const char * phone_number);
int Sim900_ReceiveSMS(char * sms_buff);
int Sim900_SMSAvailabe(void);

#endif
