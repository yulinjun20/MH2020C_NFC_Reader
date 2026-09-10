/**************************************************************************//**
 * @file     mhscpu_sysctrl.c
 * @brief    This file provides all the system firmware function.
 * @version  V1.00
 * @date     11. April 2015
 *
 * @note
 *
 ******************************************************************************/
/* Copyright (c) 2012 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/
#include "mhscpu_sysctrl.h"
#include "mhscpu_exti.h"


#define SYSCTRL_FREQ_SEL_HCLK_DIV_EN                ((uint32_t)0x0008)
#define SYSCTRL_FREQ_SEL_XTAL_Pos                   (4)
#define SYSCTRL_FREQ_SEL_XTAL_Mask                  (0x1F << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_HCLK_DIV_Pos               (2)
#define SYSCTRL_FREQ_SEL_HCLK_DIV_Mask              (0x01 << SYSCTRL_FREQ_SEL_HCLK_DIV_Pos)
#define SYSCTRL_FREQ_SEL_PCLK_DIV_Pos               (0)
#define SYSCTRL_FREQ_SEL_PCLK_DIV_Mask              (0x01 << SYSCTRL_FREQ_SEL_PCLK_DIV_Pos)


#define SYSCTRL_FREQ_SEL_XTAL_54Mhz                 (0x02 << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_60Mhz                 (0x01 << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_72Mhz                 (0x00 << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_108Mhz				(0X06 << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_120Mhz                (0x05 << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_144Mhz				(0X04 << SYSCTRL_FREQ_SEL_XTAL_Pos)

#define SYSCTRL_FREQ_SEL_HCLK_DIV_1_2               (0x00 << SYSCTRL_FREQ_SEL_HCLK_DIV_Pos)
#define SYSCTRL_FREQ_SEL_HCLK_DIV_1_4               (0x01 << SYSCTRL_FREQ_SEL_HCLK_DIV_Pos)

#define SYSCTRL_FREQ_SEL_PCLK_DIV_1_2               (0x00 << SYSCTRL_FREQ_SEL_PCLK_DIV_Pos)
#define SYSCTRL_FREQ_SEL_PCLK_DIV_1_4               (0x01 << SYSCTRL_FREQ_SEL_PCLK_DIV_Pos)

/**
  * @brief  Enables or disables the AHB peripheral clock.
  * @param  SYSCTRL_AHBPeriph: specifies the AHB peripheral to gates its clock.
  *   
  *   For @b this parameter can be any combination
  *   of the following values:        
  *     @arg SYSCTRL_AHBPeriph_DMA
  *     @arg SYSCTRL_AHBPeriph_USB
  *     @arg SYSCTRL_ASYMC_CRYPT
  *     @arg SYSCTRL_SYMC_CRYPT
  *
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SYSCTRL_AHBPeriphClockCmd(uint32_t SYSCTRL_AHBPeriph, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SYSCTRL_AHB_PERIPH(SYSCTRL_AHBPeriph));
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    
    if (NewState != DISABLE)
    {
        SYSCTRL->CG_CTRL2 |= SYSCTRL_AHBPeriph;
    }
    else
    {
        SYSCTRL->CG_CTRL2 &= ~SYSCTRL_AHBPeriph;
    }
}

/**
  * @brief  Enables or disables the APB peripheral clock.
  * @param  SYSCTRL_APBPeriph: specifies the APB peripheral to gates its clock.
  *   
  *   For @b this parameter can be any combination
  *   of the following values:        
  *     @arg SYSCTRL_APBPeriph_TRNG
  *     @arg SYSCTRL_APBPeriph_ADC
  *     @arg SYSCTRL_APBPeriph_CRC
  *     @arg SYSCTRL_APBPeriph_BPU
  *     @arg SYSCTRL_APBPeriph_TIMM0
  *     @arg SYSCTRL_APBPeriph_GPIO
  *     @arg SYSCTRL_APBPeriph_SCI0
  *     @arg SYSCTRL_APBPeriph_SPI2
  *     @arg SYSCTRL_APBPeriph_SPI1
  *     @arg SYSCTRL_APBPeriph_SPI0
  *     @arg SYSCTRL_APBPeriph_UART2
  *     @arg SYSCTRL_APBPeriph_UART1
  *     @arg SYSCTRL_APBPeriph_UART0
  *
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SYSCTRL_APBPeriphClockCmd(uint32_t SYSCTRL_APBPeriph, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SYSCTRL_APB_PERIPH(SYSCTRL_APBPeriph));
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    
    if (NewState != DISABLE)
    {
        SYSCTRL->CG_CTRL1 |= SYSCTRL_APBPeriph;
    }
    else
    {
        SYSCTRL->CG_CTRL1 &= ~SYSCTRL_APBPeriph;
    }
}

/**
  * @brief  Enables or disables the AHB peripheral reset.
  * @param  SYSCTRL_AHBPeriph: specifies the AHB peripheral to reset(CM3 reset/GLB Reset).
  *   
  *   For @b this parameter can be any combination
  *   of the following values:        
  *     @arg SYSCTRL_AHBPeriph_DMA
  *     @arg SYSCTRL_AHBPeriph_USB
  *     @arg SYSCTRL_ASYMC_CRYPT
  *     @arg SYSCTRL_SYMC_CRYPT
  *     @arg SYSCTRL_GLB_RESET
  *     @arg SYSCTRL_CM3_RESET
  *     @arg SYSCTRL_CACHE_RESET
  *     @arg SYSCTRL_OTP_RESET

  * @param  NewState: new state of the specified peripheral reset.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SYSCTRL_AHBPeriphResetCmd(uint32_t SYSCTRL_AHBPeriph, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SYSCTRL_AHB_PERIPH_RESET(SYSCTRL_AHBPeriph));
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    
    /* LOCK_R enable reset */
    SYSCTRL->LOCK_R &= ~(SYSCTRL_AHBPeriph & 0xF0000000);
    if (NewState != DISABLE)
    {
        SYSCTRL->SOFT_RST2 |= SYSCTRL_AHBPeriph;
    }
    else
    {
        SYSCTRL->SOFT_RST2 &= ~SYSCTRL_AHBPeriph;
    }
    /* LOCK_R disable reset */
    SYSCTRL->LOCK_R |= (SYSCTRL_AHBPeriph & 0xF0000000);
}

/**
  * @brief  Enables or disables the APB peripheral reset.
  * @param  SYSCTRL_APBPeriph: specifies the APB peripheral to reset.
  *   
  *   For @b this parameter can be any combination
  *   of the following values:        
  *     @arg SYSCTRL_APBPeriph_TRNG
  *     @arg SYSCTRL_APBPeriph_ADC
  *     @arg SYSCTRL_APBPeriph_CRC
  *     @arg SYSCTRL_APBPeriph_TIMM0
  *     @arg SYSCTRL_APBPeriph_GPIO
  *     @arg SYSCTRL_APBPeriph_SCI0
  *     @arg SYSCTRL_APBPeriph_SPI2
  *     @arg SYSCTRL_APBPeriph_SPI1
  *     @arg SYSCTRL_APBPeriph_SPI0
  *     @arg SYSCTRL_APBPeriph_UART2
  *     @arg SYSCTRL_APBPeriph_UART1
  *     @arg SYSCTRL_APBPeriph_UART0
  *
  * @param  NewState: new state of the specified peripheral reset.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SYSCTRL_APBPeriphResetCmd(uint32_t SYSCTRL_APBPeriph, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SYSCTRL_APB_PERIPH(SYSCTRL_APBPeriph));
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    
    if (NewState != DISABLE)
    {
        SYSCTRL->SOFT_RST1 |= SYSCTRL_APBPeriph;
    }
    else
    {
        SYSCTRL->SOFT_RST1 &= ~SYSCTRL_APBPeriph;
    }
}

#if defined ( __CC_ARM )
__STATIC_INLINE __ASM void SYSCTRL_Sleep(void)
{
	CPSID i;
	NOP;
	NOP;
	NOP;
	NOP;
	WFI;
	NOP;
	NOP;
	NOP;
	NOP;
	CPSIE i;
	BX LR;
}
#elif defined (__GNUC__)
void SYSCTRL_Sleep(void)
{
	asm("CPSID i");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("wfi");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("CPSIE i");
	asm("BX LR");
}
#elif defined (__ICCARM__)
void SYSCTRL_Sleep(void)
{
    __ASM(
    "CPSID i\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "WFI\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "CPSIE i\n"
    "BX LR"
    );
}
#endif

/**
* @brief
* @param
* @retval
**/
void SYSCTRL_EnterSleep(SleepMode_TypeDef SleepMode)
{
    uint32_t rng;
    assert_param(IS_ALL_SLEEP_MODE(SleepMode));

    if (SleepMode == SleepMode_CpuSleep)
    {
        SYSCTRL->FREQ_SEL = (SYSCTRL->FREQ_SEL & ~(SYSCTRL_FREQ_SEL_POWERMODE_Mask)) | SYSCTRL_FREQ_SEL_POWERMODE_CLOSE_CPU;
        while (!(SYSCTRL->FREQ_SEL & (~SYSCTRL_FREQ_SEL_POWERMODE_CLOSE_CPU)));
        SYSCTRL_Sleep();
    }
    else if (SleepMode == SleepMode_DeepSleep)
    {
        SYSCTRL->FREQ_SEL = (SYSCTRL->FREQ_SEL & ~(SYSCTRL_FREQ_SEL_POWERMODE_Mask)) | SYSCTRL_FREQ_SEL_POWERMODE_CLOSE_CPU_MEM;
        rng = TRNG->RNG_ANA;
        TRNG->RNG_ANA = rng | TRNG_RNG_ANA_PD_TRNG0_MASK;
        SYSCTRL_Sleep();
        TRNG->RNG_ANA = rng;
    }
    else if (SleepMode == SleepMode_PowerDown)
    {
        SYSCTRL->FREQ_SEL = (SYSCTRL->FREQ_SEL & ~(SYSCTRL_FREQ_SEL_POWERMODE_Mask)) | SYSCTRL_FREQ_SEL_POWERMODE_CLOSE_ALL_BUT_IO;
        rng = TRNG->RNG_ANA;
        TRNG->RNG_ANA = rng | TRNG_RNG_ANA_PD_TRNG0_MASK;
        SYSCTRL_Sleep();
        TRNG->RNG_ANA = rng;
    }	
}

/**
  * @brief  Select System clock source
  * @param  source_select：System clock source value。
  * @retval None
  */
void SYSCTRL_SYSCLKSourceSelect(SYSCLK_SOURCE_TypeDef source)
{
    assert_param(IS_SYSCLK_SOURCE(source));
    switch (source)
    {
        case SELECT_EXT12M:
            SYSCTRL->FREQ_SEL &= ~BIT(9);
            break;
        case SELECT_INC12M:
            SYSCTRL->FREQ_SEL |= BIT(9);
            break;
    }
}

/*  * @brief  Set System clock Freq
  * @param  SYSCLK_Freq：System clock set value。
  * @retval None
  */
void SYSCTRL_PLLConfig(SYSCTRL_PLL_TypeDef SYSCLK_Freq)
{
    assert_param(IS_PLL_FREQ(SYSCLK_Freq));
	
    switch((uint32_t)SYSCLK_Freq)
    {
    case SYSCTRL_PLL_54MHz:
        SYSCTRL->FREQ_SEL = (SYSCTRL->FREQ_SEL & ~SYSCTRL_FREQ_SEL_XTAL_Mask) | SYSCTRL_FREQ_SEL_XTAL_54Mhz;
        break;	
	case SYSCTRL_PLL_60MHz:
		SYSCTRL->FREQ_SEL = (SYSCTRL->FREQ_SEL & ~SYSCTRL_FREQ_SEL_XTAL_Mask) | SYSCTRL_FREQ_SEL_XTAL_60Mhz; 
        break;
    case SYSCTRL_PLL_72MHz:
		SYSCTRL->FREQ_SEL = (SYSCTRL->FREQ_SEL & ~SYSCTRL_FREQ_SEL_XTAL_Mask) | SYSCTRL_FREQ_SEL_XTAL_72Mhz; 
        break;
	  case SYSCTRL_PLL_108MHz:
        SYSCTRL->FREQ_SEL = (SYSCTRL->FREQ_SEL & ~SYSCTRL_FREQ_SEL_XTAL_Mask) | SYSCTRL_FREQ_SEL_XTAL_108Mhz;
        break;	
    case SYSCTRL_PLL_120MHz:
		SYSCTRL->FREQ_SEL = (SYSCTRL->FREQ_SEL & ~SYSCTRL_FREQ_SEL_XTAL_Mask) | SYSCTRL_FREQ_SEL_XTAL_120Mhz; 
        break;
    case SYSCTRL_PLL_144MHz:
		SYSCTRL->FREQ_SEL = (SYSCTRL->FREQ_SEL & ~SYSCTRL_FREQ_SEL_XTAL_Mask) | SYSCTRL_FREQ_SEL_XTAL_144Mhz; 
        break;
    }
}

/**
  * @brief  Set System HCLK Div 
  * @param  HCLK_Div：Div value
  * @retval None
  */
void SYSCTRL_HCLKConfig(uint32_t HCLK_Div)
{
    assert_param(IS_GET_SYSCTRL_HCLK_DIV(HCLK_Div));
    switch(HCLK_Div)
    {
    case SYSCTRL_HCLK_Div2:
        SYSCTRL->FREQ_SEL |= SYSCTRL_FREQ_SEL_HCLK_DIV_EN;
        SYSCTRL->FREQ_SEL &= ~SYSCTRL_FREQ_SEL_HCLK_DIV_Mask;
        SYSCTRL->FREQ_SEL |= SYSCTRL_FREQ_SEL_HCLK_DIV_1_2;
        break;
    case SYSCTRL_HCLK_Div4:
        SYSCTRL->FREQ_SEL |= SYSCTRL_FREQ_SEL_HCLK_DIV_EN;
        SYSCTRL->FREQ_SEL &= ~SYSCTRL_FREQ_SEL_HCLK_DIV_Mask;
        SYSCTRL->FREQ_SEL |= SYSCTRL_FREQ_SEL_HCLK_DIV_1_4;
        break;
    case SYSCTRL_HCLK_Div_None:
        SYSCTRL->FREQ_SEL &= ~SYSCTRL_FREQ_SEL_HCLK_DIV_EN;
        SYSCTRL->FREQ_SEL &= ~SYSCTRL_FREQ_SEL_HCLK_DIV_Mask;
        break;
    }
}

/**
  * @brief  Set System PCLK Div 
  * @param  PCLK_Div：Div value
  * @retval None
  */
void SYSCTRL_PCLKConfig(uint32_t PCLK_Div)
{
    assert_param(IS_GET_SYSCTRL_PCLK_DIV(PCLK_Div));
    switch(PCLK_Div)
    {
    case SYSCTRL_PCLK_Div2:
        SYSCTRL->FREQ_SEL &= ~SYSCTRL_FREQ_SEL_PCLK_DIV_Mask;
        SYSCTRL->FREQ_SEL |= SYSCTRL_FREQ_SEL_PCLK_DIV_1_2;
        break;
    case SYSCTRL_PCLK_Div4:
        SYSCTRL->FREQ_SEL &= ~SYSCTRL_FREQ_SEL_PCLK_DIV_Mask;
        SYSCTRL->FREQ_SEL |= SYSCTRL_FREQ_SEL_PCLK_DIV_1_4;
        break;
    }
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void SYSCTRL_GetClocksFreq(SYSCTRL_ClocksTypeDef* SYSCTRL_Clocks)
{
    /* 获取系统时钟 */
	if (SYSCTRL_FREQ_SEL_XTAL_54Mhz == (SYSCTRL->FREQ_SEL & SYSCTRL_FREQ_SEL_XTAL_Mask))
    {
        SYSCTRL_Clocks->PLL_Frequency = 54000000;
    }
    else if (SYSCTRL_FREQ_SEL_XTAL_60Mhz == (SYSCTRL->FREQ_SEL & SYSCTRL_FREQ_SEL_XTAL_Mask))
    {
        SYSCTRL_Clocks->PLL_Frequency = 60000000;
    }
    else if (SYSCTRL_FREQ_SEL_XTAL_72Mhz == (SYSCTRL->FREQ_SEL & SYSCTRL_FREQ_SEL_XTAL_Mask))
    {
        SYSCTRL_Clocks->PLL_Frequency = 72000000;
    }
	else if (SYSCTRL_FREQ_SEL_XTAL_108Mhz == (SYSCTRL->FREQ_SEL & SYSCTRL_FREQ_SEL_XTAL_Mask))
	{
		SYSCTRL_Clocks->PLL_Frequency = 108000000;
	}
	else if (SYSCTRL_FREQ_SEL_XTAL_120Mhz == (SYSCTRL->FREQ_SEL & SYSCTRL_FREQ_SEL_XTAL_Mask))
	{
		SYSCTRL_Clocks->PLL_Frequency = 120000000;
	}
	else if (SYSCTRL_FREQ_SEL_XTAL_144Mhz == (SYSCTRL->FREQ_SEL & SYSCTRL_FREQ_SEL_XTAL_Mask))
	{
		SYSCTRL_Clocks->PLL_Frequency = 144000000;
	}
    
    /* HCLK */
    SYSCTRL_Clocks->HCLK_Frequency = SYSCTRL->HCLK_1MS_VAL * 1000;
    
    /* PCLK */
    SYSCTRL_Clocks->PCLK_Frequency = SYSCTRL->PCLK_1MS_VAL * 1000;
}

void SYSCTRL_HSPIModeSwitch(HSPIMode_Typedef HSPIMode)
{
	if (HSPI_MASTER == HSPIMode)
	{
		SYSCTRL->PHER_CTRL &= ~BIT(27);
	}
	else
	{
		SYSCTRL->PHER_CTRL |= BIT(27);
		HSPIM->CR0 &= ~BIT(24);
	}
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
