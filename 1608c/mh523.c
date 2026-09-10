/**
 ****************************************************************
 * @file rc523.c
 *
 * @brief  rc523 driver.
 *
 * @author 
 *
 * 
 ****************************************************************
 */ 

/*
 * INCLUDE FILES
 ****************************************************************
 */
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
#define WATER_LEVEL	     16
#define FIFO_SIZE	     64
#define FSD              256 //Frame Size for proximity coupling Device


#define	READ_REG_CTRL	 0x80
#define	TP_FWT_302us	 2048
#define TP_dFWT	         192 

#define MAX_RX_REQ_WAIT_MS	     5000 // 命令等待超时时间100ms

static u32 XDATA g_polling_cnt = 0;
transceive_buffer XDATA mf_com_data;

unsigned char gucTypeB_ModGsp = 0x1f;
unsigned char gucMH1608_Version = 0;

unsigned char gucTypeB_Reg_26H = 0x48;
unsigned char gucTypeB_Reg_27H = 0xF8;
unsigned char gucTypeB_Reg_28H = 0x3F;
unsigned char gucTypeB_Reg_18H = 0x60;

void ReadMH1608_Para(void);


/*
static GPIO_TypeDef* LED_GPIOx = GPIOB;

#define	NFC_PD_GPIOX	  GPIOA
#define	NFC_PD_GPIO_PIN	  GPIO_Pin_11
void pcd_poweron(void)
{
	int i;
	
	GPIO_SetBits(NFC_PD_GPIOX, NFC_PD_GPIO_PIN); 
	for (i = 0; i < 2000; i++);
}*/

/**
 ****************************************************************
 * @brief pcd_init() 
 *
 * 初始化芯片
 *
 * @param:    
 * @return: 
 * @retval: 
 ****************************************************************
 */
void pcd_init(void)
{
	//pcd_poweron();
 /*       if(g_CPU_Type == MH2020C_QFN32)
        {   
            GPIO_ResetBits(GPIOB, GPIO_Pin_6);
            delay_ms(15);
            GPIO_SetBits(GPIOB, GPIO_Pin_6);
        }
        if(g_CPU_Type == MH2020C_QFN48)
        {   
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            delay_ms(15);
            GPIO_SetBits(GPIOB, GPIO_Pin_15);
        }
	delay_ms(5);
	*/
	RF_Reset();
//	pcd_reset();
//	delay_ms(25);
	  pcd_config('A');	

    pcd_antenna_off();
}

unsigned char Get_MH1608_Version(void)
{
     return gucMH1608_Version;
}

/**
 ****************************************************************
 * @brief pcd_config() 
 *
 * 配置芯片的A/B模式
 *
 * @param: u8 type   
 * @return: 
 * @retval: 
 ****************************************************************
 */
int pcd_config(u8 type)
{
       unsigned char cRFCfg_26h;
       unsigned char cGsN_27h;
       unsigned char cCWGsP_28h;
       unsigned char cModgsp_29h;
       unsigned char cRxThreshold_18h;
       pcd_reset();
       
       cRxThreshold_18h = gucTypeB_Reg_18H;
       cRFCfg_26h = gucTypeB_Reg_26H;
       cGsN_27h = gucTypeB_Reg_27H;
       cCWGsP_28h = gucTypeB_Reg_28H;
       cModgsp_29h = gucTypeB_ModGsp;
       
	pcd_antenna_off();
	delay_ms(4);//17

	if ('A' == type)
	{
		clear_bit_mask(Status2Reg, BIT3);
		clear_bit_mask(ComIEnReg, BIT7); // 高电平
		write_reg(ModeReg,0x3D);	// 11 // CRC seed:6363
		write_reg(RxSelReg, 0x86);//RxWait
		write_reg(RFCfgReg, 0x58); // 
		write_reg(TxASKReg, 0x40);//15  //typeA
		write_reg(TxModeReg, 0x00);//12 //Tx Framing A
		write_reg(RxModeReg, 0x00);//13 //Rx framing A
		write_reg(0x0C, 0x10);	//^_^

		write_reg(CWGsPReg, 0x3F);	//3F
		write_reg(ModGsPReg, cModgsp_29h);//0x1f//0x12	//调制指数	
		//clear_bit_mask(ComIEnReg, BIT2);// disable LoAlertIRq
		//write_reg(ComIrqReg, BIT2); // clear LoAlertIRq
		//兼容配置
		{
			u8 backup, adc;
			backup = read_reg(0x37);
			write_reg(0x37, 0x00);	
			adc = read_reg(0x37);
	  	//printf(" a:adc=0x%02x\r\n", adc);	
	              gucMH1608_Version = adc;
			if (adc == 0x11)
			{
				write_reg(0x26, 0x48);
				write_reg(0x37, 0x5E);
				write_reg(0x17, 0x88);
				write_reg(0x38, 0x6B);			
				write_reg(0x3A, 0x23);
				write_reg(0x29, cModgsp_29h);//0x12//0x0F); //调制指数 
				write_reg(0x3b, 0x25);
			}			
			else if (adc == 0x12)
			{
				// 以下寄存器必须按顺序配置
				write_reg(0x37, 0x5E);
				write_reg(0x26, 0x48);
				write_reg(0x17, 0x88);
				write_reg(0x29, cModgsp_29h);//0x12//0x0F); //调制指数	
				write_reg(0x35, 0xED);
				write_reg(0x3b, 0xA5);
				write_reg(0x37, 0xAE);
				write_reg(0x3b, 0x72);

			}
			else if (adc == 0x15)
			{
				// 以下寄存器必须按顺序配置
				write_reg(0x37, 0x5E);
				write_reg(0x26, 0x48);
				write_reg(0x17, 0x88);
				write_reg(0x29, cModgsp_29h);//0x12//0x0F); //调制指数	
				write_reg(0x35, 0xAC);
				write_reg(0x3b, 0xA5);
				write_reg(0x37, 0xAE);
				write_reg(0x3b, 0x72);
				write_reg(0x31, 0x54);
			}
			write_reg(0x37, backup);
   //         printf("[29h]=%02x\r\n",read_reg(0x29));
		}
			
	}
	else if ('B' == type)
	{
		write_reg(Status2Reg, 0x00);	//清MFCrypto1On
		clear_bit_mask(ComIEnReg, BIT7);// 高电平触发中断
		write_reg(ModeReg, 0x3F);	// CRC seed:FFFF
		write_reg(RxSelReg, 0x88);	//RxWait
		write_reg(RxThresholdReg, cRxThreshold_18h);
        
		//Tx
		write_reg(GsNReg, cGsN_27h);	//0xf8//调制系数
		write_reg(CWGsPReg, cCWGsP_28h);	// 0x3f
		write_reg(ModGsPReg, cModgsp_29h);//0x1f//0x12	//调制指数
		write_reg(AutoTestReg, 0x00);
		write_reg(TxASKReg, 0x00);	// typeB
		write_reg(TypeBReg, 0x13);
		write_reg(TxModeReg, 0x83);	//Tx Framing B
		write_reg(RxModeReg, 0x83);	//Rx framing B
		write_reg(BitFramingReg, 0x00);	//TxLastBits=0

		//兼容配置
		{
			u8 backup, adc;
			backup = read_reg(0x37);
			write_reg(0x37, 0x00);
			adc = read_reg(0x37);
                     gucMH1608_Version = adc;
//printf(" b:adc=0x%02x\r\n", adc);
			if (adc == 0x11)
			{
				write_reg(0x37, 0x5E);
				write_reg(0x17, 0x88);
				write_reg(0x3A, 0x23);		
				write_reg(0x38, 0x6B);	
				write_reg(0x29, cModgsp_29h);//0x1f//0x12//0x0F); //调制指数 
				write_reg(0x3b, 0x25);
			}
			else if (adc == 0x12)
			{	
				write_reg(0x37, 0x5E);
				write_reg(0x26, cRFCfg_26h);
				write_reg(0x17, 0x88);
				write_reg(0x29, cModgsp_29h);//0x1f//0x12
				write_reg(0x35, 0xED);
				write_reg(0x3b, 0xA5);
				write_reg(0x37, 0xAE);
				write_reg(0x3b, 0x72);
			}
			else if (adc == 0x15)
			{
				// 以下寄存器必须按顺序配置
				write_reg(0x37, 0x5E);
				write_reg(0x26, cRFCfg_26h);
				write_reg(0x17, 0x88);
				write_reg(0x29, cModgsp_29h);//0x12//0x0F); //调制指数	
				write_reg(0x35, 0xAC);
				write_reg(0x3b, 0xA5);
				write_reg(0x37, 0xAE);
				write_reg(0x3b, 0x72);
				write_reg(0x31, 0x54);
			}
			write_reg(0x37, backup);
   //         printf("[29h]=%02x\r\n",read_reg(0x29));
		}
	}
	else
	{
		return USER_ERROR;
	}
	
	pcd_antenna_on();
	delay_ms(10);//12
	
	return MI_OK;
}

int pcd_config2(u8 type)
{
       unsigned char cRFCfg_26h;
       unsigned char cGsN_27h;
       unsigned char cCWGsP_28h;
       unsigned char cModgsp_29h;
       unsigned char cRxThreshold_18h;
       pcd_reset();

       cRxThreshold_18h = gucTypeB_Reg_18H;
       cRFCfg_26h = gucTypeB_Reg_26H;
       cGsN_27h = gucTypeB_Reg_27H;
       cCWGsP_28h = gucTypeB_Reg_28H;
       cModgsp_29h = gucTypeB_ModGsp;
	pcd_antenna_off();
	delay_ms(4);//17

	if ('A' == type)
	{
		clear_bit_mask(Status2Reg, BIT3);
		clear_bit_mask(ComIEnReg, BIT7); // 高电平
		write_reg(ModeReg,0x3D);	// 11 // CRC seed:6363
		write_reg(RxSelReg, 0x86);//RxWait
		write_reg(RFCfgReg, 0x58); // 
		write_reg(TxASKReg, 0x40);//15  //typeA
		write_reg(TxModeReg, 0x00);//12 //Tx Framing A
		write_reg(RxModeReg, 0x00);//13 //Rx framing A
		write_reg(0x0C, 0x10);	//^_^

		write_reg(CWGsPReg, 0x3F);	//3F
		write_reg(ModGsPReg, cModgsp_29h);//0x1f//0x12	//调制指数	
		//clear_bit_mask(ComIEnReg, BIT2);// disable LoAlertIRq
		//write_reg(ComIrqReg, BIT2); // clear LoAlertIRq
		//兼容配置
		{
			u8 backup, adc;
			backup = read_reg(0x37);
			write_reg(0x37, 0x00);	
			adc = read_reg(0x37);
	//	printf(" a:adc=0x%02x\r\n", adc);	
	              gucMH1608_Version = adc;
			if (adc == 0x11)
			{
				write_reg(0x26, 0x48);
				write_reg(0x37, 0x5E);
				write_reg(0x17, 0x88);
				write_reg(0x38, 0x6B);			
				write_reg(0x3A, 0x23);
				write_reg(0x29, cModgsp_29h);//0x12//0x0F); //调制指数 
				write_reg(0x3b, 0x25);
			}			
			else if (adc == 0x12)
			{
				// 以下寄存器必须按顺序配置
				write_reg(0x37, 0x5E);
				write_reg(0x26, 0x48);
				write_reg(0x17, 0x88);
				write_reg(0x29, cModgsp_29h);//0x12//0x0F); //调制指数	
				write_reg(0x35, 0xED);
				write_reg(0x3b, 0xA5);
				write_reg(0x37, 0xAE);
				write_reg(0x3b, 0x72);

			}
			else if (adc == 0x15)
			{
				// 以下寄存器必须按顺序配置
				write_reg(0x37, 0x5E);
				write_reg(0x26, 0x48);
				write_reg(0x17, 0x88);
				write_reg(0x29, cModgsp_29h);//0x12//0x0F); //调制指数	
				write_reg(0x35, 0xAC);
				write_reg(0x3b, 0xA5);
				write_reg(0x37, 0xAE);
				write_reg(0x3b, 0x72);
				write_reg(0x31, 0x54);
			}
			write_reg(0x37, backup);
   //         printf("[29h]=%02x\r\n",read_reg(0x29));
		}
			
	}
	else if ('B' == type)
	{
		write_reg(Status2Reg, 0x00);	//清MFCrypto1On
		clear_bit_mask(ComIEnReg, BIT7);// 高电平触发中断
		write_reg(ModeReg, 0x3F);	// CRC seed:FFFF
		write_reg(RxSelReg, 0x88);	//RxWait
		write_reg(RxThresholdReg, cRxThreshold_18h);
		//Tx
		write_reg(GsNReg, cGsN_27h);//0xf8	//调制系数
		write_reg(CWGsPReg, cCWGsP_28h);	// 0c3f
		write_reg(ModGsPReg, cModgsp_29h);//0x1f//0x12	//调制指数
		write_reg(AutoTestReg, 0x00);
		write_reg(TxASKReg, 0x00);	// typeB
		write_reg(TypeBReg, 0x13);
		write_reg(TxModeReg, 0x83);	//Tx Framing B
		write_reg(RxModeReg, 0x83);	//Rx framing B
		write_reg(BitFramingReg, 0x00);	//TxLastBits=0

		//兼容配置
		{
			u8 backup, adc;
			backup = read_reg(0x37);
			write_reg(0x37, 0x00);
			adc = read_reg(0x37);
                     gucMH1608_Version = adc;
//printf(" b:adc=0x%02x\r\n", adc);
			if (adc == 0x11)
			{
				write_reg(0x37, 0x5E);
				write_reg(0x17, 0x88);
				write_reg(0x3A, 0x23);		
				write_reg(0x38, 0x6B);	
				write_reg(0x29, cModgsp_29h);//0x1f//0x12//0x0F); //调制指数 
				write_reg(0x3b, 0x25);
			}
			else if (adc == 0x12)
			{	
				write_reg(0x37, 0x5E);
				write_reg(0x26, cRFCfg_26h);
				write_reg(0x17, 0x88);
				write_reg(0x29, cModgsp_29h);//0x1f//0x12
				write_reg(0x35, 0xED);
				write_reg(0x3b, 0xA5);
				write_reg(0x37, 0xAE);
				write_reg(0x3b, 0x72);
			}
			else if (adc == 0x15)
			{
				// 以下寄存器必须按顺序配置
				write_reg(0x37, 0x5E);
				write_reg(0x26, cRFCfg_26h);
				write_reg(0x17, 0x88);
				write_reg(0x29, cModgsp_29h);//0x12//0x0F); //调制指数	
				write_reg(0x35, 0xAC);
				write_reg(0x3b, 0xA5);
				write_reg(0x37, 0xAE);
				write_reg(0x3b, 0x72);
				write_reg(0x31, 0x54);
			}
			write_reg(0x37, backup);
   //         printf("[29h]=%02x\r\n",read_reg(0x29));
		}
	}
	else
	{
		return USER_ERROR;
	}
	
//	pcd_antenna_on();
       pcd_antenna_off();

	delay_ms(10);//12
	
	return MI_OK;
}


/**
 ****************************************************************
 * @brief pcd_com_transceive() 
 *
 * 通过芯片和ISO14443卡通讯
 *
 * @param: pi->mf_command = 芯片命令字
 * @param: pi->mf_length  = 发送的数据长度
 * @param: pi->mf_data[]  = 发送数据
 * @return: status 值为MI_OK:成功
 * @retval: pi->mf_length  = 接收的数据BIT长度
 * @retval: pi->mf_data[]  = 接收数据
 ****************************************************************
 */
int pcd_com_transceive(struct transceive_buffer *pi)
{
	u8 XDATA recebyte;
	int XDATA status;
	u8 XDATA irq_en;
	u8 XDATA wait_for;
	u8 XDATA last_bits;
	u16 XDATA j;
	u8 XDATA val;
	u8 XDATA err;
	
	u8 irq_inv = 0;
	u16  len_rest;
	u8  len;
    int i;
	
	len = 0;
	len_rest = 0;
	err = 0;
	recebyte = 0;
	irq_en = 0;
	wait_for = 0;

	switch (pi->mf_command)
	{
	  case PCD_IDLE:
	     irq_en   = 0x00;
	     wait_for = 0x00;
	     break;
	  case PCD_AUTHENT:    
		irq_en = IdleIEn | TimerIEn;
		wait_for = IdleIRq;
		break;
	  case PCD_RECEIVE:
	     irq_en   = RxIEn | IdleIEn;
	     wait_for = RxIRq;
	     recebyte=1;
	     break;
	  case PCD_TRANSMIT:
	     irq_en   = TxIEn | IdleIEn;
	     wait_for = TxIRq;
	     break;
	  case PCD_TRANSCEIVE:   
		 irq_en = RxIEn | IdleIEn | TimerIEn | TxIEn;
	     wait_for = RxIRq;
	     recebyte=1;
	     break;
	  default:
	     pi->mf_command = MI_UNKNOWN_COMMAND;
	     break;
	}
   
	if (pi->mf_command != MI_UNKNOWN_COMMAND
		&& (((pi->mf_command == PCD_TRANSCEIVE || pi->mf_command == PCD_TRANSMIT) && pi->mf_length > 0)
		|| (pi->mf_command != PCD_TRANSCEIVE && pi->mf_command != PCD_TRANSMIT))
		)
	{		
		write_reg(CommandReg, PCD_IDLE);
		
		irq_inv = read_reg(ComIEnReg) & BIT7;
		write_reg(ComIEnReg, irq_inv |irq_en | BIT0);//使能Timer 定时器中断
		write_reg(ComIrqReg, 0x7F); //Clear INT
		write_reg(DivIrqReg, 0x7F); //Clear INT
		//for(i=0;i<0x3F;i++)
		{
			//if(i%8==0) lite_printf("\r\n");
			//lite_printf("0x%02x ",read_reg(i));
			
		}
		//Flush Fifo
		set_bit_mask(FIFOLevelReg, BIT7);
		if (pi->mf_command == PCD_TRANSCEIVE || pi->mf_command == PCD_TRANSMIT || pi->mf_command == PCD_AUTHENT)
		{
			#if (NFC_DEBUG)
			printf(" PCD_tx:");
			#endif
			for (j = 0; j < pi->mf_length; j++)
			{
				
				#if (NFC_DEBUG)
				printf("%02x ", (u16)pi->mf_data[j]);
				#endif
			}
			#if (NFC_DEBUG)
			printf("\n");
			#endif
		
			len_rest = pi->mf_length;
			if (len_rest >= FIFO_SIZE)
			{
				len = FIFO_SIZE;
			}else
			{
				len = len_rest;
			}
			
			for (j = 0; j < len; j++)
			{
				write_reg(FIFODataReg, pi->mf_data[j]);
			}
			len_rest -= len;//Rest bytes
			if (len_rest != 0)
			{
				write_reg(ComIrqReg, BIT2); // clear LoAlertIRq
				set_bit_mask(ComIEnReg, BIT2);// enable LoAlertIRq
			}

			write_reg(CommandReg, pi->mf_command);
			if (pi->mf_command == PCD_TRANSCEIVE)
		    {    
				set_bit_mask(BitFramingReg,0x80);  
			}
		
			while (len_rest != 0)
			{
				while(INT_PIN == 0);//Wait LoAlertIRq		
				if (len_rest > (FIFO_SIZE - WATER_LEVEL))
				{
					len = FIFO_SIZE - WATER_LEVEL;
				}
				else
				{
					len = len_rest;
				}
				for (j = 0; j < len; j++)
				{
					write_reg(FIFODataReg, pi->mf_data[pi->mf_length - len_rest + j]);
				}

				write_reg(ComIrqReg, BIT2);//在write fifo之后，再清除中断标记才可以
			
				//printf("\n8 comirq=%02bx,ien=%02bx,INT= %bd \n", read_reg(ComIrqReg), read_reg(ComIEnReg), (u8)INT_PIN);
				len_rest -= len;//Rest bytes
				if (len_rest == 0)
				{
					clear_bit_mask(ComIEnReg, BIT2);// disable LoAlertIRq
					//printf("\n9 comirq=%02bx,ien=%02bx,INT= %bd \n", read_reg(ComIrqReg), read_reg(ComIEnReg), (u8)INT_PIN);
				}
			}
			//Wait TxIRq
			while (INT_PIN == 0)
			{
		
			}
			val = read_reg(ComIrqReg);
			if (val & TxIRq)
			{
				write_reg(ComIrqReg, TxIRq);
			}
		}
		if (PCD_RECEIVE == pi->mf_command)
		{	
			set_bit_mask(rControlReg, BIT6);// TStartNow
		}
	
		len_rest = 0; // bytes received
		write_reg(ComIrqReg, BIT3); // clear HoAlertIRq
		set_bit_mask(ComIEnReg, BIT3); // enable HoAlertIRq

		//等待命令执行完成
		while(INT_PIN == 0)
		{
	
		}
		
		while(1)
		{
			
			while(0 == INT_PIN)
			{
		
			}
			val = read_reg(ComIrqReg);
			if ((val & BIT3) && !(val & BIT5))
			{
				if (len_rest + FIFO_SIZE - WATER_LEVEL > 255)
				{
					#if (NFC_DEBUG)
					printf("AF RX_LEN > 255B\n");
					#endif
					break;
				}
		        for (j = 0; j <FIFO_SIZE - WATER_LEVEL; j++)
		        {
					pi->mf_data[len_rest + j] = read_reg(FIFODataReg);
		        }
				write_reg(ComIrqReg, BIT3);//在read fifo之后，再清除中断标记才可以
				len_rest += FIFO_SIZE - WATER_LEVEL; 
			}
			else
			{
				clear_bit_mask(ComIEnReg, BIT3);//disable HoAlertIRq
				break;
			}			
		}


		val = read_reg(ComIrqReg);
		#if (NFC_DEBUG)
		printf(" INT:fflvl=%d,rxlst=%02x ,ien=%02x,cirq=%02x\n", (u16)read_reg(FIFOLevelReg),read_reg(rControlReg)&0x07,read_reg(ComIEnReg), val);//XU
		#endif
		write_reg(ComIrqReg, val);// 清中断
		
		if (val & BIT0)
		{//发生超时
			status = MI_NOTAGERR;
		//	printf(" Time_out!!\n");
		}
		else
		{
			err = read_reg(ErrorReg);
			
			status = MI_COM_ERR;
			if ((val & wait_for) && (val & irq_en))
			{
				if (!(val & ErrIRq))
				 {//指令执行正确
				    status = MI_OK;

				    if (recebyte)
				    {
						val = 0x7F & read_reg(FIFOLevelReg);
				      	last_bits = read_reg(rControlReg) & 0x07;
						if (len_rest + val > MAX_TRX_BUF_SIZE)
						{//长度过长超出缓存
							status = MI_COM_ERR;
							#if (NFC_DEBUG)
							printf("RX_LEN > 255B\n");
							#endif
						}
						else
						{	
							if (last_bits && val) //防止spi读错后 val-1成为负值
					        {
					           pi->mf_length = (val-1)*8 + last_bits;
					        }
					        else
					        {
					           pi->mf_length = val*8;
					        }
							pi->mf_length += len_rest*8;

							#if (NFC_DEBUG)
							printf(" RX:len=%02x,dat:", (u16)pi->mf_length);
							#endif
					        if (val == 0)
					        {
					           val = 1;
					        }
					        for (j = 0; j < val; j++)
					        {
								pi->mf_data[len_rest + j] = read_reg(FIFODataReg);
					        }					

							#if (NFC_DEBUG)
						    for (j = 0; j < pi->mf_length/8 + !!(pi->mf_length%8); j++)
					        {
							//	if (j > 4)
							//	{
							//		printf("..");
							//		break;
							//	}
							//	else
							//	{
									printf("%02X ", (u16)pi->mf_data[j]);
							//	}
								//printf("%02X ", (u16)pi->mf_data[j]);
					        }
							//printf("l=%d", pi->mf_length/8 + !!(pi->mf_length%8));
							printf("\n");
							#endif
						}
				    }
				 }					
				 else if ((err & CollErr) && (!(read_reg(CollReg) & BIT5)))
				 {//a bit-collision is detected				 	
				    status = MI_COLLERR;
				    if (recebyte)
				    {
						val = 0x7F & read_reg(FIFOLevelReg);
				      	last_bits = read_reg(rControlReg) & 0x07;
						if (len_rest + val > MAX_TRX_BUF_SIZE)
						{//长度过长超出缓存
							#if (NFC_DEBUG)
							printf("COLL RX_LEN > 255B\n");
							#endif
						}
						else
						{
					        if (last_bits && val) //防止spi读错后 val-1成为负值
					        {
					           pi->mf_length = (val-1)*8 + last_bits;
					        }
					        else
					        {
					           pi->mf_length = val*8;
					        }		
							pi->mf_length += len_rest*8;
							#if (NFC_DEBUG)
							printf(" RX: pi_cmd=%02x,pi_len=%02x,pi_dat:", (u16)pi->mf_command, (u16)pi->mf_length);
							#endif
					        if (val == 0)
					        {
					           val = 1;
					        }
							for (j = 0; j < val; j++)
					        {
								pi->mf_data[len_rest + j +1] = read_reg(FIFODataReg);				
					        }				
							#if (NFC_DEBUG)
					        for (j = 0; j < pi->mf_length/8 + !!(pi->mf_length%8); j++)
					        {
								printf("%02X ", (u16)pi->mf_data[j+1]);
					        }
							printf("\n");
							#endif
						}
				    }
					pi->mf_data[0] = (read_reg(CollReg) & CollPos);
					if (pi->mf_data[0] == 0)
					{
						pi->mf_data[0] = 32;
					}
				#if(NFC_DEBUG)
					printf("\n COLL_DET pos=%02x\n", (u16)pi->mf_data[0]);
				#endif
					pi->mf_data[0]--;// 与之前版本有点映射区别，为了不改变上层代码，这里直接减一；

				}
				else if ((err & CollErr) && (read_reg(CollReg) & BIT5))
				{
					//printf("COLL_DET,but CollPosNotValid=1\n");		
				}
				//else if (err & (CrcErr | ParityErr | ProtocolErr))
				else if (err & (ProtocolErr))
				{
					#if (NFC_DEBUG)
					printf("protocol err=%02x\n", err);
					#endif
					status = MI_FRAMINGERR;				
				}
				else if ((err & (CrcErr | ParityErr)) && !(err &ProtocolErr) )
				{
					//EMV  parity err EMV 307.2.3.4		
					val = 0x7F & read_reg(FIFOLevelReg);
			      	last_bits = read_reg(rControlReg) & 0x07;
					if (len_rest + val > MAX_TRX_BUF_SIZE)
					{//长度过长超出缓存
						status = MI_COM_ERR;
						#if (NFC_DEBUG)
						printf("RX_LEN > 255B\n");
						#endif
					}
					else
					{
				        if (last_bits && val)
				        {
				           pi->mf_length = (val-1)*8 + last_bits;
				        }
				        else
				        {
				           pi->mf_length = val*8;
				        }
						pi->mf_length += len_rest*8;
					}
					#if (NFC_DEBUG)
					printf("crc-parity err=%02x\n", err);
					printf("l=%d\n", pi->mf_length );
					#endif


					
					status = MI_INTEGRITY_ERR;
				}				
				else
				{
					#if (NFC_DEBUG)
					printf("unknown ErrorReg=%02x\n", err);
					#endif
					status = MI_INTEGRITY_ERR;
				}
			}
			else
			{   
				status = MI_COM_ERR;
				#if (NFC_DEBUG)
				printf(" MI_COM_ERR\n");
				#endif
			}
		}
	
 		set_bit_mask(rControlReg, BIT7);// TStopNow =1,必要的；
		write_reg(ComIrqReg, 0x7F);// 清中断0
		write_reg(DivIrqReg, 0x7F);// 清中断1
		clear_bit_mask(ComIEnReg, 0x7F);//清中断使能,最高位是控制位
		clear_bit_mask(DivIEnReg, 0x7F);//清中断使能,最高位是控制位
		write_reg(CommandReg, PCD_IDLE);

	}
	else
	{
		status = USER_ERROR;
		#if (NFC_DEBUG)
		printf("USER_ERROR\n");
		#endif
	}
	#if (NFC_DEBUG)
		printf(" pcd_com: sta=%d,err=%02x\n", status, err);
	#endif	
	return status;
}

void pcd_reset(void)
{	
#if(NFC_DEBUG)
	printf("pcd_reset\n");
#endif
	write_reg(CommandReg, PCD_RESETPHASE); //软复位数字芯片
}

void pcd_antenna_on(void)
{
	write_reg(TxControlReg, read_reg(TxControlReg) | 0x03); //Tx1RFEn=1 Tx2RFEn=1
}

void pcd_antenna_off(void)
{
	write_reg(TxControlReg, read_reg(TxControlReg) & (~0x03));
}
/////////////////////////////////////////////////////////////////////
//设置PCD定时器
//input:fwi=0~15
/////////////////////////////////////////////////////////////////////
void pcd_set_tmo(u8 fwi)
{
/*	write_reg(TPrescalerReg, (TP_FWT_302us) & 0xFF);
	write_reg(TModeReg, BIT7 | (((TP_FWT_302us)>>8) & 0xFF));

	write_reg(TReloadRegL, (1 << fwi)  & 0xFF);
	write_reg(TReloadRegH, ((1 << fwi)  & 0xFF00) >> 8);*/

    unsigned int receiveTimeout;
    u16 nrtValue;

	receiveTimeout = ((4096UL) << fwi) + 49152;
	
	if (receiveTimeout > (unsigned int)(0x003FFFC0))
	{
		// Use 1:4096 prescaler. 
		nrtValue = (u16) (receiveTimeout >> 12);

		// Round up if necessary. 
		if ((((u16) receiveTimeout) & 0x0FFF) != 0)
			nrtValue += 1;

        write_reg(TPrescalerReg, (TP_FWT_302us) & 0xFF);
	    write_reg(TModeReg, BIT7 | (((TP_FWT_302us)>>8) & 0xFF));
		//as3911ModifyRegister(AS3911_REG_GPT_CONF, 0x03, 0x03);
	}
	else
	{
		// Use 1:64 prescaler. 
		nrtValue = (u16) (receiveTimeout >> 6);

		// Round up if necessary. 
		if ((((u16) receiveTimeout) & 0x03F) != 0)
			nrtValue += 1;

        write_reg(TPrescalerReg, (32) & 0xFF);
	    write_reg(TModeReg, BIT7 | (((32)>>8) & 0xFF));
		//as3911ModifyRegister(AS3911_REG_GPT_CONF, 0x03, 0x02);
	}

	write_reg(TReloadRegL, nrtValue & 0xff);
	write_reg(TReloadRegH, (nrtValue >> 8) & 0xff);
/*
    write_reg(TPrescalerReg, (TP_FWT_302us) & 0xFF);
	write_reg(TModeReg, BIT7 | (((TP_FWT_302us)>>8) & 0xFF));

	write_reg(TReloadRegL, ((1 << (fwi))+12)  & 0xFF);
	write_reg(TReloadRegH, (((1 << (fwi))+12)  & 0xFF00) >> 8);*/
}



void pcd_delay_sfgi(u8 sfgi)
{
	//SFGT = (SFGT+dSFGT) = [(256 x 16/fc) x 2^SFGI] + [384/fc x 2^SFGI] 
	//dSFGT =  384 x 2^FWI / fc
		write_reg(TPrescalerReg, (TP_FWT_302us + TP_dFWT) & 0xFF);
		write_reg(TModeReg, BIT7 | (((TP_FWT_302us + TP_dFWT)>>8) & 0xFF)); 

		if (sfgi > 14 || sfgi < 1)
		{//FDTA,PCD,MIN = 6078 * 1 / fc
			sfgi = 1;
		}

		write_reg(TReloadRegL, (1 << sfgi) & 0xFF);
		write_reg(TReloadRegH, ((1 << sfgi) >> 8) & 0xFF);

		write_reg(ComIrqReg, 0x7F);//清除中断
		write_reg(ComIEnReg, BIT0);
		clear_bit_mask(TModeReg,BIT7);// clear TAuto
		set_bit_mask(rControlReg,BIT6);// set TStartNow
		
		while(!INT_PIN);// wait new INT
		//set_bit_mask(TModeReg,BIT7);// recover TAuto
		pcd_set_tmo(g_pcd_module_info.uiFwi); //recover timeout set
		
}


void pcd_lpcd_config_start(u8 delta, u32 t_inactivity_ms, u8 skip_times, u8 t_detect_us)
{
	u8 XDATA WUPeriod;
	u8 XDATA SwingsCnt;
#if (NFC_DEBUG)
	printf("pcd_lpcd_config_start\n");
#endif
	WUPeriod = t_inactivity_ms * 32.768 / 256  + 0.5;
	SwingsCnt = t_detect_us * 27.12 / 2 / 16 + 0.5;

	write_reg(0x01,0x0F); //先复位寄存器再进行lpcd

	write_reg(0x14, 0x8B);	// Tx2CW = 1 ，continue载波发射打开
	write_reg(0x37, 0x00);//恢复版本号
	write_reg(0x37, 0x5e);	// 打开私有寄存器保护开关
	write_reg(0x3c, 0x30 | delta);	//设置Delta[3:0]的值, 开启32k
	write_reg(0x3d, WUPeriod);	//设置休眠时间	
	write_reg(0x3e, 0x80 | ((skip_times & 0x07) << 4) | (SwingsCnt & 0x0F));	//开启LPCD_en设置,跳过探测次数，探测时间
	write_reg(0x37, 0x00);	// 关闭私有寄存器保护开关
	write_reg(0x03, 0x20);	//打开卡探测中断使能
	write_reg(0x01, 0x10);	//PCD soft powerdown

	//具体应用相关，本示例工程配置为高电平为有中断
	clear_bit_mask(0x02, BIT7);
}

/*
	lpcd功能开始函数
*/
void pcd_lpcd_start(void)
{
#if (NFC_DEBUG)
	printf("pcd_lpcd_start\n");
#endif

	write_reg(0x01,0x0F); //先复位寄存器再进行lpcd
	
	write_reg(0x37, 0x00);//恢复版本号
	if (read_reg(0x37) == 0x10)
	{
		write_reg(0x01, 0x00);	// idle
	}
	write_reg(0x14, 0x8B);	// Tx2CW = 1 ，continue载波发射打开
	
	write_reg(0x37, 0x5e);	// 打开私有寄存器保护开关

	//write_reg(0x3c, 0x30);	//设置Delta[3:0]的值, 开启32k //0 不能使用
	//write_reg(0x3c, 0x31);	//设置Delta[3:0]的值, 开启32k
	//write_reg(0x3c, 0x32);	//设置Delta[3:0]的值, 开启32k
	//write_reg(0x3c, 0x33);	//设置Delta[3:0]的值, 开启32k
	//write_reg(0x3c, 0x34);	//设置Delta[3:0]的值, 开启32k
	//write_reg(0x3c, 0x35);	//设置Delta[3:0]的值, 开启32k XU
	write_reg(0x3c, 0x37);	//设置Delta[3:0]的值, 开启32k XU
	//write_reg(0x3c, 0x3A);	//设置Delta[3:0]的值, 开启32k XU
	//write_reg(0x3c, 0x3F);	//设置Delta[3:0]的值, 开启32k XU
	
	write_reg(0x3d, 0x0d);	//设置休眠时间	
	write_reg(0x3e, 0x95);	//设置连续探测次数，开启LPCD_en
	write_reg(0x37, 0x00);	// 关闭私有寄存器保护开关
	write_reg(0x03, 0x20);	//打开卡探测中断使能
	write_reg(0x01, 0x10);	//PCD soft powerdown		

	//具体应用相关，配置为高电平为有中断
	clear_bit_mask(0x02, BIT7); 
}

void pcd_lpcd_end(void)
{
#if (NFC_DEBUG)
	printf("pcd_lpcd_end\n");
#endif
	write_reg(0x01,0x0F); //先复位寄存器再进行lpcd
}

u8 pcd_lpcd_check(void)
{
	if (INT_PIN && (read_reg(DivIrqReg) & BIT5)) //TagDetIrq
	{
		write_reg(DivIrqReg, BIT5); //清除卡检测到中断
		pcd_lpcd_end();
		return TRUE;
	}
	return FALSE;
}


void pcd_set_rate(u8 rate)
{
	u8 val,rxwait;
	switch(rate)
	{
		case '1':
			clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
			clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
			write_reg(ModWidthReg, 0x26);//Miller Pulse Length

			write_reg(RxSelReg, 0x88);
			
			break;

		case '2':
			clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
			set_bit_mask(TxModeReg, BIT4);
			clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
			set_bit_mask(RxModeReg, BIT4);
			write_reg(ModWidthReg, 0x12);//Miller Pulse Length
			//rxwait相对于106基本速率需增加相应倍数
			val = read_reg(RxSelReg);
			rxwait = ((val & 0x3F)*2);
			if (rxwait > 0x3F)
			{
				rxwait = 0x3F;
			}			
			write_reg(RxSelReg,(rxwait | (val & 0xC0)));
			
			break;

		case '4':			
			clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
			set_bit_mask(TxModeReg, BIT5);
			clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
			set_bit_mask(RxModeReg, BIT5);
			write_reg(ModWidthReg, 0x0A);//Miller Pulse Length
			//rxwait相对于106基本速率需增加相应倍数
			val = read_reg(RxSelReg);
			rxwait = ((val & 0x3F)*4);
			if (rxwait > 0x3F)
			{
				rxwait = 0x3F;
			}			
			write_reg(RxSelReg,(rxwait | (val & 0xC0)));	

			break;
			
			
		default:
			clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
			clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
			write_reg(ModWidthReg, 0x26);//Miller Pulse Length
			
			break;
	}

	{//不同速率选择不同带宽
		u8 adc;
		write_reg(0x37, 0x00);
		adc = read_reg(0x37);

		if (adc == 0x12)
		{
			write_reg(0x37, 0x5E);
			if (rate == '8' || rate == '4')//848k,424k
			{
				write_reg(0x3B, 0x25);
			}
			else if (rate == '2' || rate == '1')// 212k, 106k
			{
				write_reg(0x3B, 0xE5);
			}
			write_reg(0x37, 0x00);		
		}
	}
}


/**
 ****************************************************************
 * @brief set_bit_mask() 
 *
 * 将寄存器的某些bit位值1
 *
 * @param: reg 寄存器地址
 * @param: mask 需要置位的bit位
 ****************************************************************
 */
void set_bit_mask(u8 reg, u8 mask)  
{
	char XDATA tmp;

	tmp = read_reg(reg);
	write_reg(reg, tmp | mask);  // set bit mask
}


/**
 ****************************************************************
 * @brief clear_bit_mask() 
 *
 * 将寄存器的某些bit位清0
 *
 * @param: reg 寄存器地址
 * @param: mask 需要清0的bit位
 ****************************************************************
 */
void clear_bit_mask(u8 reg,u8 mask)  
{
	char XDATA tmp;

	tmp = read_reg(reg);
	write_reg(reg, tmp & ~mask);  // clear bit mask
}


//#define RF_EN_HIGH    GPIO_SetBits(GPIOD, GPIO_Pin_4)
//#define RF_EN_LOW     GPIO_ResetBits(GPIOD, GPIO_Pin_4)


void init_spi()
{
	spi_config();
}

#if 0
void s_PiccInit()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    unsigned char cTemp[10];
    unsigned char buf[20];
	//RF_EN
	
	
        //RF_RST
        if(g_CPU_Type == MH2020C_QFN48)
        {
        	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
        	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        	GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
        	GPIO_Init(GPIOB,&GPIO_InitStruct);
//--------------------PN5180--------------------------------
            //  RF_BUSY
               GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
        	 GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
        	 GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
        	 GPIO_Init(GPIOB,&GPIO_InitStruct);

             //RF_INT
               GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
        	 GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
        	 GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
        	 GPIO_Init(GPIOD,&GPIO_InitStruct);
              //RF_CS
       GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
    
             init_spi();
	      SPI_CS_HIGH();
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            delay_ms(15);
            GPIO_SetBits(GPIOB, GPIO_Pin_15);
            delay_ms(200);
             printf("picc init OK\r\n");
             return;
//-------------------------------------------------------------
        }
        
        if(g_CPU_Type == MH2020C_QFN32)
        {
            GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
            GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
            GPIO_Init(GPIOB,&GPIO_InitStruct);
        }
    
    //RF_CS
       GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
    
    //RF_INT
//    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
//	GPIO_Init(GPIOC,&GPIO_InitStruct);

       ReadMH1608_Para();
	init_spi();
	SPI_CS_HIGH();

	//RF_EN_HIGH;//RF_EN--------HIGH
	//delay_ms(200);
	
	pcd_init();//初始化pcd寄存器
	rfid_init();//初始化rfid本地变量
}
#endif

void ReadMH1608_Para(void)
{
    unsigned char cTemp[10];
    flash_read( &cTemp[0], TYPEB_PARA_CONFIG_ADDR, 9);
    //    printf("cTemp:%02x %02x %02x %02x %02x\r\n", cTemp[0],cTemp[1],cTemp[2],cTemp[3],cTemp[4]);
    if(*(unsigned int *)&cTemp[0] == TYPEB_PARA_CONFIG_FLAG)
    {
        if(cTemp[4] & 0x80)
        {
                gucTypeB_Reg_26H = 0x48;
        }
        else
        {
               gucTypeB_Reg_26H = cTemp[4];
        }

        if(cTemp[5] == 0xff)
        {
                gucTypeB_Reg_27H = 0xf8;
        }
        else
        {
               gucTypeB_Reg_27H = cTemp[5];
        }

        if(cTemp[6] == 0xff)
        {
                gucTypeB_Reg_28H = 0x3f;
        }
        else
        {
               gucTypeB_Reg_28H = cTemp[6];
        }
        
        if((cTemp[7] > 0) && (cTemp[7] < 0x3F))
        {
            gucTypeB_ModGsp = cTemp[7];
        }
        else
        {
             gucTypeB_ModGsp = 0x1f;
        }

        if(cTemp[8] == 0xff)
        {
                gucTypeB_Reg_18H = 0x60;
        }
        else
        {
               gucTypeB_Reg_18H = cTemp[8];
        }
   //     printf("----18h=%02x\r\n", gucTypeB_Reg_18H);
   }
   else
   {
       gucTypeB_Reg_26H = 0x48;
       gucTypeB_Reg_27H = 0xf8;
       gucTypeB_Reg_28H = 0x3f;
       gucTypeB_ModGsp = 0x1f;
       gucTypeB_Reg_18H = 0x60;

 /*     memset(buf, 0, sizeof(buf));
      *(unsigned int *)&buf[0] = TYPEB_PARA_CONFIG_FLAG;
      buf[4] = gucTypeB_Reg_26H;
      buf[5] = gucTypeB_Reg_27H;
      buf[6] = gucTypeB_Reg_28H;
      buf[7] = gucTypeB_ModGsp;
      buf[8] = gucTypeB_Reg_18H;
      
      eraseFlashPage(TYPEB_PARA_CONFIG_ADDR);
      flash_write(TYPEB_PARA_CONFIG_ADDR, buf, 9);*/
   }

}

