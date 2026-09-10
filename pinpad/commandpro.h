#ifndef _COMMANDPRO_H_
#define _COMMANDPRO_H_


#define CMD_MAX_BUFLEN		5120

#define PICC_CODE                   0xba  //0xc3
#define PICC_CODE2                  0xc3



#define SYS_CODE                    0xd1

#define CMD_COMMAND     0x30
#define CMD_ADMSET         0x20
#define CMD_VERSION				 0x1e
#define CMD_KEY				 0x1f
#define CMD_ENTER                0x0d
#define CMD_OTHER       0xee
#define CMD_CANCEL		0xff
#define CMD_OK			0x00
#define CMD_ABORT		0x01


int Picc_Pro(void);
int System_Pro(void);

#endif



