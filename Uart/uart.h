#ifndef _UART_H_
#define _UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mhscpu_uart.h"
#include <stdio.h>


//| Stop Bit 1 | Data Bit 8 |
#define LCR_DATA8_STOP1 (0x03)

#define COMM_MAX 2
//#define COMMBUFLEN 5300
#define OPENED	          1
#define CLOSED	          0

#define COMM_OK			     0x00
#define COMM_OVR_ERR	     0x01	// overrun error
#define COMM_PRT_ERR	     0x02	//  parity error
#define COMM_FRM_ERR	     0x03	//  framing error
#define COMM_OVF_ERR	     0x04	//  overflow error
#define COMM_OTR_ERR	     0x05	//  others error
#define COMM_PARAM_ERR	 	 0xf0
#define COMM_OPEN_ERR		 0xf1	//not opened
#define COMM_TIMEOUT_ERR	 0xf2	//timeout error


int fputc(int ch, FILE *f);

#define COMMON_ERR_UNKNOWN    (0x7087 << 16)

#define	K_COBRA_LOCAL_BUF_SIZE_IN_BYTES	3000//520



#define RECV_INT  (BIT0)
#define SEND_INT  (BIT1 | BIT7)


//#define UART_SEND_BUF_SIZE      256
//#define UART_RECV_BUF_SIZE      256

typedef struct
{
	/** Read data buffer */
	unsigned char								buffer[K_COBRA_LOCAL_BUF_SIZE_IN_BYTES];
	/** Relative offset on next free room for data to receive */
	unsigned int								ofst_free;
	/** Relative offset on last data to read */
	unsigned int								ofst_last;

} t_uart_context;

int PortOpen(unsigned char port,unsigned int baudrate);
void PortSend(unsigned char port,unsigned char ch);
unsigned char PortRecv(int port, unsigned char * ch,unsigned long ms);
void PortClose(int port);
unsigned char PortReset(int port);
int PortCheck(int port);




#ifdef __cplusplus
}
#endif




#endif
