/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_otp.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the OTP firmware library
 *****************************************************************************/

#ifndef __MHSCPU_OTP_H
#define __MHSCPU_OTP_H

#ifdef __cplusplus
extern "C" {
#endif
	
/* Includes ------------------------------------------------------------------*/
#include "mhscpu.h"

/* Exported types ------------------------------------------------------------*/    
typedef enum
{
    OTP_WriteNoDone = 0,
    OTP_WriteDone = 1,
}OTP_WrieDoneTypeDef;

typedef enum
{
    OTP_ReadNotReady = 0,
    OTP_ReadReady = 1,
}OTP_ReadReadyTypeDef;

typedef enum
{
    OTP_UnLocked = 0,
    OTP_Locked = 1,
}OTP_LockTypeDef;
 
typedef enum
{
    OTP_Complete = 0,
    OTP_ReadOnProgramOrSleep = 1,              //在编程、休眠状态下对OTP进行读操作
    OTP_ProgramInROBlock = 2,                  //对只读取进行编程
    OTP_ProgramOutOfAddr = 3,                  //编程范围超过OTP范围
    OTP_ProgramOnSleep = 4,                    //在休眠状态进行编程操作
    OTP_WakeUpOnNoSleep = 5,                   //在非休眠状态下进行唤醒操作
    OTP_TimeOut = 6,                           //OTP完成标志位超时
    OTP_DataWrong = 7,  
}OTP_StatusTypeDef;
#define IS_OTP_ADDRESS(ADDRESS) (((ADDRESS) > OTP_BASE - 1) && ((ADDRESS) < OTP_BASE + MHSCPU_OTP_SIZE))
     

void OTP_Unlock(void);
void OTP_Lock(void);
void OTP_ClearStatus(void);

void OTP_Init(void);

void OTP_SetProtect(uint32_t u32Addr);
OTP_StatusTypeDef OTP_UnProtect(uint32_t u32Addr);

OTP_LockTypeDef OTP_IsProtect(uint32_t u32Addr);
OTP_LockTypeDef OTP_IsProtectLock(uint32_t u32Addr);
OTP_StatusTypeDef OTP_WriteByte(uint32_t addr, uint8_t w);


#ifdef __cplusplus
}
#endif

#endif

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
