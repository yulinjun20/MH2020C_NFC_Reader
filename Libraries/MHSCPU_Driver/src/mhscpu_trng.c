/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_trng.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the TRNG firmware functions
 *****************************************************************************/

/* Includes -----------------------------------------------------------------*/ 
#include "mhscpu_trng.h"


/**
  * @method	TRNG_Get
  * @brief	
  * @param	rand
  * @param	TRNGx
  * @retval 
  */
uint32_t TRNG_Get(uint32_t rand[4], TRNG_ChannelTypeDef TRNGx)
{
	uint32_t ret = 1;
	assert_param(IS_TRNG_CHANNEL(TRNGx));
	
	if(TRNG->RNG_CSR & TRNG_RNG_CSR_S128_TRNG0_MASK)
	{
		rand[0] = TRNG->RNG_DATA;
		rand[1] = TRNG->RNG_DATA;
		rand[2] = TRNG->RNG_DATA;
		rand[3] = TRNG->RNG_DATA;
		ret = 0;
	}
	TRNG->RNG_CSR &= ~TRNG_RNG_CSR_S128_TRNG0_MASK; 
	return ret;
}

/**
  * @method	TRNG_SetPseudoRandom
  * @brief	
  * @param	PseudoRandom
  * @retval 
  */
void TRNG_SetPseudoRandom(uint32_t PseudoRandom)
{
	TRNG->RNG_PN = PseudoRandom;
}

/**
  * @method	TRNG_DirectOutANA
  * @brief	
  * @param	TRNGx
  * @param	NewState
  * @retval 
  */
void TRNG_DirectOutANA(TRNG_ChannelTypeDef TRNGx, FunctionalState NewState)
{
	assert_param(IS_TRNG_CHANNEL(TRNGx));
	assert_param(IS_FUNCTIONAL_STATE(NewState));
	
	if(NewState != DISABLE)
	{
		TRNG->RNG_ANA |= TRNG_RNG_ANA_ANA_OUT_TRNG0_MASK;
	}
	else
	{
		TRNG->RNG_ANA &= ~TRNG_RNG_ANA_ANA_OUT_TRNG0_MASK;
	}
}

/**
  * @method	TRNG_ITConfig
  * @brief	
  * @param	NewState
  * @retval 
  */
void TRNG_ITConfig(FunctionalState NewState)
{
	if(NewState != DISABLE)
	{
		TRNG->RNG_CSR |= TRNG_RNG_CSR_INTP_EN_MASK;
	}
	else
	{
		TRNG->RNG_CSR &= ~TRNG_RNG_CSR_INTP_EN_MASK;
	}
}
/**
  * @brief  获取中断状态
  * @param  TRNG_IT:
  *         TRNG_IT_RNG0_S128
  *         TRNG_IT_RNG0_ATTACK
  * @retval None
  */
ITStatus TRNG_GetITStatus(uint32_t TRNG_IT)
{
	ITStatus bitstatus = RESET;
    assert_param(IS_TRNG_GET_IT(TRNG_IT));
	
    if(((TRNG->RNG_CSR) & TRNG_IT) != (uint32_t)RESET)
	{
		bitstatus = SET;
	}
	else
	{
		bitstatus = RESET;
	}
    return  bitstatus;
}

/**
  * @brief  清除中断标志位
  * @param  TRNG_IT:
  *         TRNG_IT_RNG0_S128
  *         TRNG_IT_RNG0_ATTACK
  * @retval None
  */
void TRNG_ClearITPendingBit(uint32_t TRNG_IT)
{
    assert_param(IS_TRNG_GET_IT(TRNG_IT));
	
    TRNG->RNG_CSR &= ~TRNG_IT;
}

/**
  * @brief  启动生成随机数
  * @param  
  * @retval None
  */
 void TRNG_Start(TRNG_ChannelTypeDef TRNGx)
{
	assert_param(IS_TRNG_CHANNEL(TRNGx));
	TRNG->RNG_ANA &= ~TRNG_RNG_ANA_PD_TRNG0_MASK;
	TRNG->RNG_CSR &= ~TRNG_RNG_CSR_S128_TRNG0_MASK;
}

/**
  * @brief  停止产生随机数
  * @param  
  * @retval None
  */
void TRNG_Stop(TRNG_ChannelTypeDef TRNGx)
{
	assert_param(IS_TRNG_CHANNEL(TRNGx));
	
	TRNG->RNG_ANA |= TRNG_RNG_ANA_PD_TRNG0_MASK;
}

/**
  * @brief  随机数采样沿选择
  * @param  
  * @retval None
  */
void TRNG_Trigger_Choose(uint32_t TRIGGER)
{
    assert_param(IS_TRNG_GET_TRIGGER(TRIGGER));
    switch(TRIGGER)
    {
    case Trigger_Falling:
        
        TRNG->RNG_CSR |= TRNG_RNG_CSR_TRIGGER;
        break;
    case Trigger_Rising:
        TRNG->RNG_CSR &= ~TRNG_RNG_CSR_TRIGGER;
        
        break;
    }
}

/**
  * @brief  开启/关闭冯诺依曼
  * @param  
  * @retval None
  */
void TRNG_Neumann(FunctionalState NewState)
{
	if (DISABLE != NewState)
	{
		TRNG->RNG_ANA &= ~TRNG_RNG_ANA_Neumann_MASK;
	}
	else
	{
		
        TRNG->RNG_ANA |= TRNG_RNG_ANA_Neumann_MASK;
	}
}


void TRNG_LFSR(FunctionalState NewState)
{
	if (DISABLE != NewState)
	{
		TRNG->RNG_ANA |= TRNG_RNG_ANA_LFSR_XOR_FB_MASK;
	}
	else
	{
		
        TRNG->RNG_ANA &= ~ TRNG_RNG_ANA_LFSR_XOR_FB_MASK;
	}
}
/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
