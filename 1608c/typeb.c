#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mh523.h"
#include "uart.h"
#include "mhscpu.h"
//#include "gpio.h"
//#include "timer.h"
#include "iso14443_4.h"
#include "spi.h"
#include "user.h"

#define DelaymS    delay_ms
#define PcdAntennaOff    pcd_antenna_off
#define ReadRawRC      read_reg
#define SetBitMask     set_bit_mask
#define WriteRawRC     write_reg


void typeB(void)
{
  unsigned int i;
  unsigned char j,card_type,temp;
  unsigned char datbuf[128]; 
 
    pcd_config('B');//init_typeB();                                                        // ³õÊ¼»¯RC522½Ó¿Ú
    datbuf[0]=0x05;
	datbuf[1]=0x00;
	datbuf[2]=0x00;
	i= transceive( datbuf, 3);
	temp=0;
	if(i>1)
	{
	   if((datbuf[7]==0x00)&&\
	   	  (datbuf[8]==0x81)&&\
		  (datbuf[9]==0x00)&&\
		  (datbuf[10]==0x70)&&\
		  (datbuf[11]==0xc0)) 
	    //   SendString("»ª´ó\r\n"); 
		  card_type=0x10;								 
	   else if((datbuf[7]==0x86)&&\
	   	  (datbuf[8]==0x0c)&&\
		  (datbuf[9]==0x00)&&\
		  (datbuf[10]==0x80)&&\
		  (datbuf[11]==0x80)) 
	     // SendString("Çå»ª\r\n"); 
		   card_type=0x20;
	   else if((datbuf[7]==0x86)&&\
	   	  (datbuf[8]==0x05)&&\
		  (datbuf[9]==0x00)&&\
		  (datbuf[10]==0x80)&&\
		  (datbuf[11]==0x80)) 
	      // SendString("»ªºç\r\n");
		   card_type=0x30; 
	   else if((datbuf[7]==0x86)&&\
	   	  (datbuf[8]==0x07)&&\
		  (datbuf[9]==0x00)&&\
		  (datbuf[10]==0x80)&&\
		  (datbuf[11]==0x90)) 
	      // SendString("´óÌÆ\r\n"); 
		  card_type=0x40;
	   else
	      // SendString("´íÎó\r\n");
		   card_type=0xaa;
		if((card_type==0x10)||\
		   (card_type==0x20)||\
		   (card_type==0x30)||\
		   (card_type==0x40))
		{
			datbuf[0]=0x1d;		   //1d  00  00  00  00  00  08  01  08  
			datbuf[1]=0x00;
			datbuf[2]=0x00;
			datbuf[3]=0x00;
			datbuf[4]=0x00;
			datbuf[5]=0x00;
			datbuf[6]=0x08;
			datbuf[7]=0x01;
			datbuf[8]=0x08;
			i= transceive( datbuf, 9);
			if(i>1)
			{
//				SendString("Ñ¡¿¨³É¹¦£¡\r\n");
//				UART0_TxByte(datbuf[0]);
				datbuf[0]=0x00;		   //00  36  00  00  08 
				datbuf[1]=0x36;
				datbuf[2]=0x00;
				datbuf[3]=0x00;
				datbuf[4]=0x08;
				i= transceive(datbuf, 5);
				if(i>0)
				{
					//UART0_TxByte(0x02);		//Êý¾ÝÖ¡Í·
					printf("%02x ", 0x02);

					for(j=0;j<i-1;j++)
					{
						temp=temp+datbuf[j];	   //Êý¾ÝÐ£ÑéÎ»
					//	UART0_TxByte(datbuf[j]);	
						printf("%02x ", datbuf[j]);
					}
														  
				//	UART0_TxByte(card_type);		// ³§¼Ò´úÂë 
				//	UART0_TxByte(0x03);		//Êý¾ÝÖ¡Î²
					
					printf("%02x ", card_type);
					printf("%02x ", 0x03);
					printf("\r\n");
				}
loopB:
				datbuf[0]=0x1d;		   //1d  00  00  00  00  00  08  01  08  
				datbuf[1]=0x00;
				datbuf[2]=0x00;
				datbuf[3]=0x00;
				datbuf[4]=0x00;
				datbuf[5]=0x00;
				datbuf[6]=0x08;
				datbuf[7]=0x01;
				datbuf[8]=0x08;
				i= transceive( datbuf, 9);
				while(i==0x03)
				goto loopB;

			}
		}  
   
	} 
	PcdAntennaOff();
	DelaymS(5);
	//PcdAntennaOn();

} 


void SetTimeOut(unsigned int uiMicroSeconds)
{
  unsigned int RegVal;
  unsigned char TmpVal;

  RegVal = uiMicroSeconds / 100;

 // RcModifyReg(TModeReg, 1, 0x80);
  SetBitMask(TModeReg, 0x80);
  
  WriteRawRC(TPrescalerReg, 0xa6);
  
  TmpVal  = ReadRawRC(TModeReg);
  TmpVal &= 0xf0;
  TmpVal |= 0x02;
  WriteRawRC(TModeReg, TmpVal); // 82

  WriteRawRC(TReloadRegL, ((unsigned char)(RegVal&0xff)));
  WriteRawRC(TReloadRegH, ((unsigned char)((RegVal>>8)&0xff)));
}



int transceive(unsigned char* datbuf, unsigned char datlen)
{
    unsigned char i, j;
    unsigned char RegVal;
    unsigned char waitForComm;
    unsigned char status = 0x0000;
  
    /*************** initialize ***************/
    SetTimeOut(20000);

    //RcModifyReg(TxModeReg, 1, 0x80);
    //RcModifyReg(RxModeReg, 1, 0x80);
	SetBitMask(TModeReg, 0x80);
	SetBitMask(RxModeReg, 0x80);

    WriteRawRC(BitFramingReg, 0);

    WriteRawRC(CommandReg, PCD_IDLE);
    WriteRawRC(FIFOLevelReg, BIT7);
    WriteRawRC(ComIrqReg, 0x7f);
    WriteRawRC(CommandReg, PCD_TRANSCEIVE);

    for(i=0;i<datlen;i++) {WriteRawRC(FIFODataReg, datbuf[i]); }
    WriteRawRC(BitFramingReg, 0x80);

    waitForComm = TxIRq | TimerIRq;
    do{
        for(i=0;i<100;i++);
        RegVal = ReadRawRC(ComIrqReg);
    }while(!(waitForComm & RegVal));                              // Wait Tx complete or timeout

    datlen  = 0;
    WriteRawRC(ComIrqReg, 0x7f);
    waitForComm = RxIRq | TimerIRq;
    do
    {
        RegVal = ReadRawRC(ComIrqReg);

        j = ReadRawRC(FIFOLevelReg);
		for(i=0;i<j;i++)
		{
            datbuf[datlen] = ReadRawRC(FIFODataReg);
			datlen++;
		}
	}while(!(waitForComm & RegVal));

    if(RegVal & RxIRq)
    {
	    datbuf[datlen] = 0x00;
        datlen ++;
    }
	else
	{
	    datbuf[0x00] = 0xee;
	    datlen = 0x01;
	}

    return datlen;
}
