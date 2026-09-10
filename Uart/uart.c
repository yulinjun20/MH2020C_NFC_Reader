#include "uart.h"
#include <stdio.h>
#include "user.h"
unsigned char Comm_used[COMM_MAX] = {CLOSED};
t_uart_context context[COMM_MAX];


void Uart_TxRxFIFO_Reset(UART_TypeDef* UARTx);

int uart_init(unsigned char port,unsigned int baudrate)
{
  //  int											result = COMMON_ERR_UNKNOWN;
    UART_InitTypeDef                            uart;
    UART_FIFOInitTypeDef UART_FIFOInitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;

    UART_StructInit(&uart);
    UART_FIFOStructInit(&UART_FIFOInitStruct);

    uart.UART_BaudRate = baudrate; 
    uart.UART_Parity = UART_Parity_No;
    uart.UART_StopBits = UART_StopBits_1;
    uart.UART_WordLength = UART_WordLength_8b;

    UART_FIFOInitStruct.FIFO_Enable = ENABLE;
    UART_FIFOInitStruct.FIFO_DMA_Mode = UART_FIFO_DMA_Mode_1;
    UART_FIFOInitStruct.FIFO_RX_Trigger = UART_FIFO_RX_Trigger_1_4_Full;
    UART_FIFOInitStruct.FIFO_TX_Trigger = UART_FIFO_TX_Trigger_2_Chars;
    UART_FIFOInitStruct.FIFO_TX_TriggerIntEnable = ENABLE;
    
    if(port == 0)
    {
			  SYSCTRL->SOFT_RST1 |= 0x01;
        UART_Init(UART0, &uart);
        GPIO_PinRemapConfig(GPIOA, GPIO_Pin_0 | GPIO_Pin_1, GPIO_Remap_0);
        Uart_TxRxFIFO_Reset(UART0);
         //--------------------------
        UART_FIFOInit(UART0, &UART_FIFOInitStruct);
        UART_FIFOCmd(UART0, ENABLE);
        //----------------------------
    //    UART_ITConfig(UART0, RECV_INT | SEND_INT, ENABLE);
        UART_ITConfig(UART0, RECV_INT , ENABLE);
   //     NVIC_EnableIRQ(UART0_IRQn);
		
        NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);

		NVIC_InitStructure.NVIC_IRQChannel = UART0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

	   UART_ITConfig(UART0, SEND_INT , DISABLE);
    }
    else if(port == 1)
    {
			  SYSCTRL->SOFT_RST1 |= 0x02;
        UART_Init(UART1, &uart);
       // GPIO_PinRemapConfig(GPIOD, GPIO_Pin_4 | GPIO_Pin_5, GPIO_Remap_0);
        if(g_CPU_Type == MH2020C_QFN32)
        {
            GPIO_PinRemapConfig(GPIOC, GPIO_Pin_0 | GPIO_Pin_1, GPIO_Remap_3);
        }
        if(g_CPU_Type == MH2020C_QFN48)
        {
            GPIO_PinRemapConfig(GPIOB, GPIO_Pin_12 | GPIO_Pin_13, GPIO_Remap_3);
        }
        Uart_TxRxFIFO_Reset(UART1);
        //--------------------------
        UART_FIFOInit(UART1, &UART_FIFOInitStruct);
        UART_FIFOCmd(UART1, ENABLE);
        //----------------------------
      //  UART_ITConfig(UART1, RECV_INT | SEND_INT, ENABLE);
   //     UART_ITConfig(UART1, RECV_INT , ENABLE);
        UART_ITConfig(UART1, UART_IT_RX_RECVD, ENABLE);//--------------
    //    NVIC_EnableIRQ(UART1_IRQn);

	    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);

		NVIC_InitStructure.NVIC_IRQChannel = UART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		UART_ITConfig(UART1, SEND_INT , DISABLE);
    }
	
	return 0;
}

void Uart_TxRxFIFO_Reset(UART_TypeDef* UARTx)
{
     /* FCR Write Only So Here we Use FCR Shadow Register SRR to protect FCR's value*/
	UARTx->SRR = 6;
}


void Uart_RecvInt_Enable(unsigned char port)
{
    switch(port)
    {
        case 0:
            UART_ITConfig(UART0, RECV_INT, ENABLE);
        break;
        case 1:
            UART_ITConfig(UART1, RECV_INT, ENABLE);
        break;
        default:
            break;
    }
}

void Uart_RecvInt_Disable(unsigned char port)
{
    switch(port)
    {
        case 0:
            UART_ITConfig(UART0, RECV_INT, DISABLE);
        break;
        case 1:
            UART_ITConfig(UART1, RECV_INT, DISABLE);
        break;
        default:
            break;
    }
}

typedef enum
{
    //Clear to send or data set ready or ring indicator or data carrier detect.
    //Note that if auto flow control mode is enabled, a change in CTS(that is, DCTS set)does not cause an interrupt.
    MODEM_STATUS    = 0x00,
    //None(No interrupt occur)
    NONE            = 0x01,
    //Transmitter holding register empty(Prog.
    //THRE Mode disabled)or XMIT FIFO at or below threshold(Prog. THRE Mode enable)
    TRANS_EMPTY     = 0x02,
    //Receive data available or FIFO trigger level reached
    RECV_DATA       = 0x04,
    //Overrun/parity/framing errors or break interrupt
    RECV_STATUS     = 0x06,
    //UART_16550_COMPATIBLE = NO and master has tried to write to the 
    //Line Control Register while the DW_apb_uart is busy(USR[0] is set to1)
    BUSY_DETECT     = 0x07,
    //No char in or out of RCVR FIFO during the last 4 character times 
    //and there is at least 1 character in it during this time
    CHAR_TIMEOUT    = 0x0C
} INT_FLAG;

void UART0_IRQHandler(void)
{
//   int i;
   unsigned int						isr;
   isr = (UART0->OFFSET_8.IIR & 0x0F);
   
   if(isr == RECV_DATA || isr == CHAR_TIMEOUT)
   {
      // printf("isr=%d,%d\r\n",isr,UART0->RFL);
     //  for (i = 0; i < 16 && UART0->RFL != 0; i++)
     //if(UART_GetLineStatus(UART0) & UART_LINE_STATUS_RX_RECVD)
       while(UART_IsRXFIFONotEmpty(UART0))
       {
             /** Get character */
		     context[0].buffer[context[0].ofst_free++] = (unsigned char)UART0->OFFSET_0.RBR;
		     /** update indexes */
			// printf("%02x",context[0].buffer[context[0].ofst_free - 1]);
		     context[0].ofst_free %= K_COBRA_LOCAL_BUF_SIZE_IN_BYTES;

             //PortSend(0, context[0].buffer[context[0].ofst_free - 1]);
             
       }
   }

}

void UART1_IRQHandler(void)
{
//   int i;
   unsigned int	  isr;
   unsigned char      ch;

   
   isr = (UART1->OFFSET_8.IIR & 0x0F);
   
   if(isr == RECV_DATA || isr == CHAR_TIMEOUT)
   {
 //      printf("isr=%d,%d\r\n",isr,UART0->RFL);
     //  for (i = 0; i < 16 && UART0->RFL != 0; i++)
   //    if(UART_GetLineStatus(UART1) & UART_LINE_STATUS_RX_RECVD)
       while(UART_IsRXFIFONotEmpty(UART1))
       {
                    /** Get character */
		     ch = (unsigned char)UART1->OFFSET_0.RBR;
		     context[1].buffer[context[1].ofst_free++] = ch;//(unsigned char)UART1->OFFSET_0.RBR;
		     /** update indexes */
		     context[1].ofst_free %= K_COBRA_LOCAL_BUF_SIZE_IN_BYTES;

         //    PortSend(1, context[1].buffer[context[1].ofst_free - 1]);
        //     printf("-%c", ch);
       }
   }
}

int PortOpen(unsigned char port,unsigned int baudrate)
{	

	if(port>=COMM_MAX) return COMM_PARAM_ERR;
	Comm_used[port] = OPENED; 

        uart_init(port,baudrate);
        context[port].ofst_free = context[port].ofst_last = 0;
        memset(context[port].buffer,0,sizeof(context[port].buffer));
      
       return COMM_OK;
}

void PortSend(unsigned char port,unsigned char ch)
{
	switch(port)
	{
		case 0:
                    while(!UART_IsTXEmpty(UART0));
        	        UART_SendData(UART0, (uint8_t) ch);
			break;
		case 1:
			while(!UART_IsTXEmpty(UART1));
	               UART_SendData(UART1, (uint8_t) ch);
			break;
		default:
			break;        
	}
}

void Uart_Send(unsigned char port, unsigned char *data, unsigned int len)
{
    int i;
    
    for(i = 0; i < len; i++)
    {
         PortSend(port, data[i]);
    }
}

unsigned char PortRecv(int port, unsigned char * ch,unsigned long ms)
{
    unsigned long i;

    if(port>=COMM_MAX) return COMM_PARAM_ERR;
    
    if(Comm_used[port] == CLOSED)
        return COMM_OPEN_ERR;
    i = ms;// * 20000;
    while(i--)
    {
        if(context[port].ofst_free != context[port].ofst_last) 
        {
            *ch = context[port].buffer[context[port].ofst_last];
			//printf("<%c> ",context[port].buffer[context[port].ofst_last]);
            context[port].ofst_last = (context[port].ofst_last+1)%K_COBRA_LOCAL_BUF_SIZE_IN_BYTES;
            return 0;
        }
        delay_ms(1);
    }
    return COMM_TIMEOUT_ERR;
}

void PortClose(int port)
{
    Comm_used[port] = CLOSED;
	if(port==0)
		UART_DeInit(UART0);//M_MML_UART_INTERRUPT_DISABLE(MML_UART_DEV0);
	else if(port==1)
		UART_DeInit(UART1);//M_MML_UART_INTERRUPT_DISABLE(MML_UART_DEV1);
    return;
}

unsigned char PortReset(int port)
{
    if(Comm_used[port] == CLOSED)
        return COMM_OPEN_ERR;
	
	context[port].ofst_free = context[port].ofst_last = 0;
	memset(context[port].buffer,0xff,K_COBRA_LOCAL_BUF_SIZE_IN_BYTES);
    return COMM_OK;
}

int PortCheck(int port)
{
    if(Comm_used[port] == CLOSED)
        return COMM_OPEN_ERR;

	if(context[port].ofst_free!=context[port].ofst_last) 
		return 0;

    return COMM_OTR_ERR;
}

const unsigned int gc_dwCrc32Table[256]={
    0x00000000,0x77073096,0xee0e612c,0x990951ba,
    0x076dc419,0x706af48f,0xe963a535,0x9e6495a3,
    0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,
    0x09b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,
    0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,
    0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,
    0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,
    0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5,
    0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,
    0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,
    0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940,
    0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,
    0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116,
    0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,
    0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,
    0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,
    0x76dc4190,0x01db7106,0x98d220bc,0xefd5102a,
    0x71b18589,0x06b6b51f,0x9fbfe4a5,0xe8b8d433,
    0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818,
    0x7f6a0dbb,0x086d3d2d,0x91646c97,0xe6635c01,
    0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,
    0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,
    0x65b0d9c6,0x12b7e950,0x8bbeb8ea,0xfcb9887c,
    0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,
    0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,
    0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,
    0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,
    0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,
    0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,
    0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
    0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,
    0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,
    0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a,
    0xead54739,0x9dd277af,0x04db2615,0x73dc1683,
    0xe3630b12,0x94643b84,0x0d6d6a3e,0x7a6a5aa8,
    0xe40ecf0b,0x9309ff9d,0x0a00ae27,0x7d079eb1,
    0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,
    0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,
    0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,
    0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,
    0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252,
    0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
    0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,
    0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79,
    0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,
    0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,
    0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,
    0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,
    0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a,
    0x9c0906a9,0xeb0e363f,0x72076785,0x05005713,
    0x95bf4a82,0xe2b87a14,0x7bb12bae,0x0cb61b38,
    0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21,
    0x86d3d2d4,0xf1d4e242,0x68ddb3f8,0x1fda836e,
    0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,
    0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,
    0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,
    0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,
    0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,
    0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,
    0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
    0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,
    0xbad03605,0xcdd70693,0x54de5729,0x23d967bf,
    0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,
    0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d,
};

void QuickCrc32(unsigned char *pbyDataIn, unsigned int dwDataLen, unsigned char abyCrcOut[4])
{
    unsigned int dwCrc;
    unsigned int i;
    dwCrc = 0xffffffff;
    for (i = 0; i < dwDataLen; i++)
    {
        dwCrc = gc_dwCrc32Table[((unsigned char)dwCrc^pbyDataIn[i])&0xff] ^ (dwCrc>>8);
    }
    dwCrc = dwCrc^0xffffffff;
    abyCrcOut[0] = (unsigned char )(dwCrc>>24);
    abyCrcOut[1] = (unsigned char )(dwCrc>>16);
    abyCrcOut[2] = (unsigned char )(dwCrc>>8);
    abyCrcOut[3] = (unsigned char )(dwCrc);
}


unsigned short g_awhalfCrc16CCITT[16]={  
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};
static unsigned char g_pbySendBuff[100];
void Crc16CCITT(unsigned char *pbyDataIn, unsigned int dwDataLen, unsigned char abyCrcOut[2])
{
    unsigned short wCrc = 0;
    unsigned char byTemp;
    while (dwDataLen-- != 0)
    {
        byTemp = ((unsigned char)(wCrc>>8))>>4;
        wCrc <<= 4; 
        wCrc ^= g_awhalfCrc16CCITT[byTemp^(*pbyDataIn/16)]; 
        byTemp = ((unsigned char)(wCrc>>8))>>4;
        wCrc <<= 4;
        wCrc ^= g_awhalfCrc16CCITT[byTemp^(*pbyDataIn&0x0f)]; 
        pbyDataIn++; 
    }
    abyCrcOut[0] = wCrc/256;
    abyCrcOut[1] = wCrc%256;    
}

void printhex(char *s,unsigned char *data,int len)
{
        int i = 0;
  //      char out[1024]={0};
  //      out[0]=' ';  
        printf("[%d][%s]:",len,s);
        for(i=0;i<len;i++)
        {
            printf("%02x ", data[i]);
        }
        printf("\r\n");
         
     /*   
        for(i=0;i<len;i++){
            sprintf(out,"%s%02x",out,data[i]);
        }
        sprintf(out,"%s\n",out);
        for(i=0;i<strlen(out);i++)
            PortSend(0,out[i]);
       */
}


