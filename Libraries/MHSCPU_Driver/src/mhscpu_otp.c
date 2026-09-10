/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_otp.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the OTP firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu_otp.h"


#define OTP_DONE_FLAG               BIT(0)
#define OTP_START                   BIT(0)
#define OTP_WAKEUPEN                BIT(1)

#define OTP_TIM_EN                  (0xA5)

#define OTP_EACHBIT_CONTROLBYTE     0x04

/*OTP unlock key*/
#define OTP_KEY1 				    (0xABCD00A5)
#define OTP_KEY2 				    (0x1234005A)

static uint32_t gu32OTP_Key1 = 0;
static uint32_t gu32OTP_Key2 = 0;

static OTP_ReadReadyTypeDef OTP_IsReadReady(void);
static OTP_WrieDoneTypeDef OTP_IsWriteDone(void);
static OTP_StatusTypeDef OTP_GetFlag(void);

/******************************************************************************
* Function Name  : OTP_Operate
* Description    : 启动对OTP的操作
* Input          : NONE
* Return         : NONE
******************************************************************************/
static void OTP_Operate(void)
{
    OTP->CS |= OTP_START;
}


/******************************************************************************
* Function Name  : OTP_Unlock
* Description    : 获取写操作用到的密钥
* Input          : NONE
* Return         : NONE
******************************************************************************/
void OTP_Unlock(void)
{
	gu32OTP_Key1 = OTP_KEY1;
	gu32OTP_Key2 = OTP_KEY2;
}

/******************************************************************************
* Function Name  : OTP_Lock
* Description    : 对写操作加锁
* Input          : NONE
* Return         : NONE
******************************************************************************/
void OTP_Lock(void)
{
	gu32OTP_Key1 = ~OTP_KEY1;
	gu32OTP_Key2 = ~OTP_KEY2;
	OTP->PROT = OTP_KEY2;
}

/******************************************************************************
* Function Name  : OTP_IsWriteDone
* Description    : 判断OTP写入操作是否完成
* Input          : NONE
* Return         : OTP_WrieDoneTypeDef:OTP_WriteDone/OTP_WriteNoDone
******************************************************************************/
static OTP_WrieDoneTypeDef OTP_IsWriteDone(void)
{
    if ((OTP->CS & OTP_DONE_FLAG) == OTP_DONE_FLAG)
    {
        return OTP_WriteNoDone;
    }
    else
    {
        return OTP_WriteDone;
    }
}

/******************************************************************************
 * Function Name  : OTP_GetFlag
 * Description    : 获取操作完成后的状态。
 * Input          : NONE
 * Return         : OTP_StatusTypeDef:
                    OTP_Complete此次操作无异常；
                    OTP_ReadOnProgramOrSleep在编程/休眠状态下对OTP进行读操作
                    OTP_ProgramIn_ROBlock对只读区域进行编程
                    OTP_ProgramOutOfAddr编程地址超出OTP范围
                    OTP_ProgramOnSleep在休眠状态下进行编程
                    OTP_WakeUpOnNoSleep在非休眠状态下进行唤醒操作
******************************************************************************/
static OTP_StatusTypeDef OTP_GetFlag(void)
{
    return (OTP_StatusTypeDef)((OTP->CS >> 1) & 0x7);
}

/******************************************************************************
 * Function Name  : OTP_ClearStatus
 * Description    : 清除状态寄存器的值
 * Input          : NONE
 * Return         : NONE
******************************************************************************/
void OTP_ClearStatus(void)
{
    OTP->CS &= ~(0x07 << 1);
}

/******************************************************************************
* Function Name  : WriteOtpByte
* Description    : OTP区编程
* Input          : u32Addr：编程地址
                   u32Data：写入的数据
* Return         : OTP_StatusTypeDef
******************************************************************************/
OTP_StatusTypeDef OTP_WriteByte(uint32_t addr, uint8_t w)
{
    uint32_t Delay = 0;
    OTP_StatusTypeDef otp_status;
    
    assert_param(IS_OTP_ADDRESS(addr));
    
    OTP->PDATA = w;
    OTP->ADDR = addr;
    
    OTP->PROT = gu32OTP_Key1;
    OTP->PROT = gu32OTP_Key2;
    
    OTP_Operate();
    
    Delay = 0xFFFFF;
    while((OTP_IsWriteDone() == OTP_WriteNoDone) && (0 != --Delay));
    if(0 == Delay)
    {
        return OTP_TimeOut;
    }
    
    otp_status = OTP_GetFlag();
    if (OTP_Complete != otp_status)
    {
        OTP_ClearStatus();
        return otp_status;
    }
    
    while (OTP_ReadNotReady == OTP_IsReadReady());

    if((*(uint8_t *)addr) != w)
    {
        return OTP_DataWrong;
    }
    
    return OTP_Complete;
}

/******************************************************************************
* Function Name  : OTP_UnProtect
* Description    : 设置对应OTP地址为可擦写
* Input          : u32Addr
* Return         : NONE
******************************************************************************/
OTP_StatusTypeDef OTP_UnProtect(uint32_t u32Addr)
{
	uint32_t pu32RO;
	assert_param(IS_OTP_ADDRESS(u32Addr));
    
    pu32RO = (u32Addr - OTP_BASE) / OTP_EACHBIT_CONTROLBYTE;
    if (pu32RO < 32)
    {
        if (!(OTP->ROL & BIT(pu32RO)))
        {
            OTP->RO &= ~BIT(pu32RO);
        }
        else
        {
            return OTP_DataWrong;
        }
    }
    else 
    {
        if (!(*(&(OTP->ROL1) + (pu32RO / 32 - 1) * 2) & BIT(pu32RO % 32)))
        {
            *(&(OTP->RO1) + (pu32RO / 32 - 1) * 2) &= ~BIT(pu32RO % 32);
        }
        else
        {
            return OTP_DataWrong;
        }
    }
		
	return OTP_Complete;
}


/******************************************************************************
* Function Name  : OTP_SetProtect
* Description    : 设置对应OTP地址为只读
* Input          : u32Addr
* Return         : NONE
******************************************************************************/
void OTP_SetProtect(uint32_t u32Addr)
{
    uint32_t pu32RO;
	assert_param(IS_OTP_ADDRESS(u32Addr));
    
    pu32RO = (u32Addr - OTP_BASE) / OTP_EACHBIT_CONTROLBYTE;
    
    if (pu32RO < 32)
    {
        OTP->RO |= BIT(pu32RO);
    }
    else 
    {
        *(&(OTP->RO1) + (pu32RO / 32 - 1) * 2) |= BIT(pu32RO % 32);
    }
}

/******************************************************************************
* Function Name  : OTP_IsReadReady
* Description    : 是否可以读操作，OTP处于编程/休眠状态时不可读
* Input          : void
* Return         : OTP_ReadReadyTypeDef:OTP_ReadReady/OTP_ReadNotReady
******************************************************************************/
static OTP_ReadReadyTypeDef OTP_IsReadReady(void)
{
    if(BIT(31) == (OTP->CS & BIT(31)))
    {
        return OTP_ReadReady;
    }
    else
    {
        return OTP_ReadNotReady;
    }
}

/******************************************************************************
* Function Name  : OTP_IsProtect
* Description    : 读取对应地址的只读锁是否为锁定状态
* Input          : u32Addr
* Return         : OTP_LockTypeDef:OTP_Locked/OTP_UnLocked
******************************************************************************/
OTP_LockTypeDef OTP_IsProtect(uint32_t u32Addr)
{
	uint32_t pu32RO;
    
    assert_param(IS_OTP_ADDRESS(u32Addr));
	
    pu32RO = (u32Addr - OTP_BASE) / OTP_EACHBIT_CONTROLBYTE;
    
    if (pu32RO < 32)
    {
        if(BIT(pu32RO) == (OTP->RO & BIT(pu32RO)))
        {
            return OTP_Locked;
        }
        else
        {
            return OTP_UnLocked;
        }
    }
    else 
    {
        if(BIT(pu32RO % 32) == (*(&(OTP->RO1) + (pu32RO / 32 - 1) * 2) & BIT(pu32RO % 32)))
        {
            return OTP_Locked;
        }
        else
        {
            return OTP_UnLocked;
        }
    }
}

/******************************************************************************
* Function Name  : OTP_IsProtectLock
* Description    : 读取对应地址的只读锁的硬件锁是否为锁定状态
* Input          : u32Addr
* Return         : OTP_LockTypeDef:OTP_Locked/OTP_UnLocked
******************************************************************************/
OTP_LockTypeDef OTP_IsProtectLock(uint32_t u32Addr)
{
	uint32_t pu32ROL;
    
    assert_param(IS_OTP_ADDRESS(u32Addr));
	
    pu32ROL = (u32Addr - OTP_BASE) / OTP_EACHBIT_CONTROLBYTE;
    
    if (pu32ROL < 32)
    {
        if(BIT(pu32ROL) == (OTP->ROL & BIT(pu32ROL)))
        {
            return OTP_Locked;
        }
        else
        {
            return OTP_UnLocked;
        }
    }
    else 
    {
        if(BIT(pu32ROL % 32) == (*(&(OTP->ROL1) + (pu32ROL / 32 - 1) * 2) & BIT(pu32ROL % 32)))
        {
            return OTP_Locked;
        }
        else
        {
            return OTP_UnLocked;
        }
    }
}

/******************************************************************************
* Function Name  : OTP_Init
* Description    : 打开OTP电源, 上电后需要等2us以后才能读取OTP内容
* Input          : 
* Return         : 
******************************************************************************/
void OTP_Init(void)
{
    uint32_t cycles_10ns = 0;
    uint32_t cycles_1us = 0;
	uint32_t Delay = 0xFFFFF;
	uint32_t rd_cycle = 0;
	uint32_t rd_byte_wait = 0;
    
	while (((OTP->CS & OTP_DONE_FLAG) == OTP_DONE_FLAG) && (0 != --Delay));
    
	if (0 == Delay)
    {
       	 return;
	}
    if (SYSCTRL->HCLK_1MS_VAL > 108000)
    {
        cycles_10ns = 2;
    }
    else
    {
        cycles_10ns = 1;
    }
    cycles_1us = (SYSCTRL->HCLK_1MS_VAL / 1000) + 1;
    
	OTP->TIM = (OTP->TIM & ~0x7FF) | (cycles_10ns << 8) | cycles_1us;
 	OTP->TIM_EN = 0xA5;
	
	if (SYSCTRL->HCLK_1MS_VAL >= 120000)
	{
		rd_cycle = 5;
	}
	else
	{
		rd_cycle = 4;
	}
	
	if (SYSCTRL->HCLK_1MS_VAL > 72000)
	{
		rd_byte_wait = 70 / (1000 / (SYSCTRL->HCLK_1MS_VAL / 1000)) + 1;
	}
	else
	{
		rd_byte_wait = 6;
	}
	
	OTP->RD_TIM = (OTP->RD_TIM & ~0xFFFFFFFF) | (rd_cycle << 16) | rd_byte_wait;
}

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
