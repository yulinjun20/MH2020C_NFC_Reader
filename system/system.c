#include "mhscpu_gpio.h"
#include "mhscpu.h"
#include "mhscpu_it.h"
//#include "My_Typedef.h"
#include "spi.h"
#include "mhscpu_exti.h"
#include "user.h"


#define MH2020C_APP_MAXVER 0x01
#define MH2020C_APP_MIDVER 0x00
#define MH2020C_APP_MINVER 0x00

extern unsigned int g_CPU_Type;
extern unsigned int g_RF_Module;

int Lib_GetVersion(unsigned char *version)
{
        version[0] = MH2020C_APP_MAXVER;
        version[1] = MH2020C_APP_MIDVER;
        version[2] = MH2020C_APP_MINVER;
	      
	 flash_read(&version[3], BOOTVERSION_ADDR, 3);
                
        return 0;
}


unsigned char atohex(char* a)
{
	unsigned char chRet;
	unsigned char low;
	unsigned char high;
	char cHigh = a[0];
	char cLow = a[1];

	//	高位
	if (cHigh>='0' && cHigh<='9')
	{
		high = (unsigned char)(cHigh-48);
	}
	else if (cHigh>='A' && cHigh<='F')
	{
		high = cHigh-55;
	}
	else if (cHigh>='a' && cHigh<='f')
	{
		high = cHigh-87;
	}

	//	低位
	if (cLow>='0' && cLow<='9')
	{
		low = (unsigned char)(cLow-48);
	}
	else if (cLow>='A' && cLow<='F')
	{
		low = cLow-55;
	}
	else if (cLow>='a' && cLow<='f')
	{
		low = cLow-87;
	}

	chRet = high;
	chRet = chRet<<4;
	chRet+= low;

	return chRet;
}

int StrToHex(char* chArray,int Length,unsigned char* cHex)
{
	char chTemp[2];
	int i;
	for (i=0; i<Length/2; i++)
	{
		memcpy(chTemp,&chArray[i*2],2);
		cHex[i] = atohex(chTemp);
	}

	return Length/2;
}

void GetUSN(unsigned char *usn)
{
//MH2020C存放唯一序列号地址是:0x4000801E,一共16字节
   
 	memcpy(usn, (unsigned char*)0x4000801E, 16);
}



void RF_Rst_Low(void)
{
    if(g_CPU_Type == MH2020C_QFN48)
    {
           GPIO_ResetBits(GPIOB, GPIO_Pin_15);
    }
    if(g_CPU_Type == MH2020C_QFN32)
    {
           GPIO_ResetBits(GPIOB, GPIO_Pin_6);
    }
}

void RF_Rst_High(void)
{
    if(g_CPU_Type == MH2020C_QFN48)
    {
           GPIO_SetBits(GPIOB, GPIO_Pin_15);
    }
    if(g_CPU_Type == MH2020C_QFN32)
    {
           GPIO_SetBits(GPIOB, GPIO_Pin_6);
    }
}

uint8_t Read_BusyPin(void)
{
    if(g_CPU_Type == MH2020C_QFN48)
    {
          return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14);
    }
    if(g_CPU_Type == MH2020C_QFN32)
    {
          return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
    }
}


void RF_Reset(void)
{
     RF_Rst_Low();
     delay_ms(10);
     RF_Rst_High();
      delay_ms(10);
}

#define QFN48_RF_INT_GPIO      GPIOD
#define QFN48_RF_INT_GPIO_PIN  GPIO_Pin_5

#define QFN32_RF_INT_GPIO     GPIOB
#define QFN32_RF_INT_GPIO_PIN     GPIO_Pin_8


void pn5180_irq_enable(void)
{
    if(g_CPU_Type == MH2020C_QFN48)
    {
        EXTI_LineConfig(EXTI_Line3, QFN48_RF_INT_GPIO_PIN, EXTI_Trigger_Rising);
    }
    if(g_CPU_Type == MH2020C_QFN32)
    {
          EXTI_LineConfig(EXTI_Line1, QFN32_RF_INT_GPIO_PIN, EXTI_Trigger_Rising);
    }
}

void pn5180_irq_disable(void)
{
    if(g_CPU_Type == MH2020C_QFN48)
    {
        EXTI_LineConfig(EXTI_Line3, QFN48_RF_INT_GPIO_PIN, EXTI_Trigger_Off);
    }
    if(g_CPU_Type == MH2020C_QFN32)
    {
        EXTI_LineConfig(EXTI_Line1, QFN32_RF_INT_GPIO_PIN, EXTI_Trigger_Off);
    }
}

int pn5180_clear_irq_flag(void)
{
    if(g_CPU_Type == MH2020C_QFN32)
    {
	EXTI_ClearITPendingBit(EXTI_Line3);
       NVIC_ClearPendingIRQ(EXTI3_IRQn);
    }
    if(g_CPU_Type == MH2020C_QFN48)
    {
	EXTI_ClearITPendingBit(EXTI_Line1);
       NVIC_ClearPendingIRQ(EXTI1_IRQn);
    }
	return 0;
}
/*
void EXTI1_IRQHandler(void)
{
		unsigned int status;
 //   printf("EXTI3_IRQHandler\r\n");
    status = EXTI_GetITLineStatus(EXTI_Line1);
    EXTI_ClearITPendingBit(EXTI_Line1);
    NVIC_ClearPendingIRQ(EXTI1_IRQn);
	  if(status & QFN32_RF_INT_GPIO_PIN)
		{
			Rfid_IRQHandler();
		}
}*/

/*
void EXTI3_IRQHandler(void)
{
		unsigned int status;
 //   printf("EXTI3_IRQHandler\r\n");
    status = EXTI_GetITLineStatus(EXTI_Line3);
    EXTI_ClearITPendingBit(EXTI_Line3);
    NVIC_ClearPendingIRQ(EXTI3_IRQn);
	  if(status & RF_INT_GPIO_PIN)
		{
			Rfid_IRQHandler();
		}
}*/

void s_PiccInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

     if(g_CPU_Type == MH2020C_QFN48)
     {

            //  RF_RST
            GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
            GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
            GPIO_Init(GPIOB,&GPIO_InitStruct);
						
			      /****txz_test****/
						//RF_INT
						GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
        	  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
        	  GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
        	  GPIO_Init(GPIOC,&GPIO_InitStruct);
						/*******/
			 
              //RF_CS
            GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
            GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
            GPIO_Init(GPIOB,&GPIO_InitStruct);

            init_spi();
            SPI_CS_HIGH();
            RF_Reset();
            g_RF_Module = RF_MH1608;
            ReadMH1608_Para();
            pcd_antenna_off();
            rfid_init();
            ReadMH1608_Para();//
						
     }
     
     if(g_CPU_Type == MH2020C_QFN32)
     {
            // RF_RST
            GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
            GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
            GPIO_Init(GPIOB,&GPIO_InitStruct);

            //RF_CS
            GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
            GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
            GPIO_Init(GPIOB,&GPIO_InitStruct);
            init_spi();
            SPI_CS_HIGH();
            RF_Reset();
            g_RF_Module = RF_MH1608;
            ReadMH1608_Para();
            pcd_antenna_off();
            rfid_init();
     }

     
}

