#ifndef _thesis_h_
#define _thesis_h_

// my thesis error codes
#define THESIS_OK                     1
#define THESIS_PACKET_FAIL            2
#define THESIS_PACKET_CHECKSUM_FAIL   3
#define THESIS_PACKET_NULL            3
#define THESIS_CMD_NOT_SUPPORT        4
#define THESIS_PACKET_NOT_ENOUGH_LENGTH   5
#define THESIS_NOT_OWN_ID             6
#define THESIS_INVALID_UNIQUE_NUMBER  7
#define THESIS_FLASH_ERROR						8

  

#define THESIS_MSG_SIZE 128

extern int thesis_errn;
extern char thesis_err_msg[];

extern unsigned char thesis_need_to_send;
extern char thesis_sent_msg[];
extern int thesis_msg_len;

int ThesisInit(void);
int ThesisProcess(char * packet, int len);

#endif // _thesis_h_
