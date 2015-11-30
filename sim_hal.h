#ifndef _sim_hal_h_
#define _sim_hal_h_

#ifdef __cplusplus
extern "C" {
#endif
	
	
#include "stm32f10x.h"
	
	
	void sim_hal_init(int baudrate);
	void sim_hal_send_char(char c);
	void sim_hal_send_string(char Str[]);
	void sim_hal_send_numberic(int num);
	void sim_hal_send_float(float num);
	void sim_hal_send_byte(uint8_t b, BYTE_FORMAT f);
	int sim_hal_available(void);
	int sim_hal_get_data(char * buffer, int len);
	void sim_hal_flush(void);
	
	void sim_hal_power_low(void);
	void sim_hal_power_high(void);
	void sim_hal_reset_low(void);
	void sim_hal_reset_high(void);
	
#ifdef __cplusplus
}
#endif

#endif

