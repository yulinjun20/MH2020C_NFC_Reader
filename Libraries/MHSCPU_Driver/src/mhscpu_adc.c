/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : mhscpu_adc.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 05/28/2017
 * Description          : ADC module functions set.
 *****************************************************************************/
 
/* Include ------------------------------------------------------------------*/
#include "mhscpu_adc.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/	
/* Private macro ------------------------------------------------------------*/	
/* Private variables --------------------------------------------------------*/	
/* Ptivate function prototypes ----------------------------------------------*/	


/******************************************************************************
* Function Name  : ADC_Init
* Description    : 初始化ADC,初始化参考值
* Input          : ADC_InitStruct：要初始化的数据结构指针
* Return         : NONE
******************************************************************************/
void ADC_Init(ADC_InitTypeDef *ADC_InitStruct)
{
    assert_param(IS_ADC_CHANNEL(ADC_InitStruct->ADC_Channel));
    assert_param(IS_ADC_SAMP(ADC_InitStruct->ADC_SampSpeed));
    assert_param(IS_FUNCTIONAL_STATE(ADC_InitStruct->ADC_IRQ_EN));
    assert_param(IS_FUNCTIONAL_STATE(ADC_InitStruct->ADC_FIFO_EN));
	
    
    ADC0->ADC_CR2 = (ADC0->ADC_CR2 & ~(ADC_CR2_REFTRIM_MASK)) | *(uint8_t *)(ADC_CR2_REFTRIM_OTPADDR);
    /* Select ADC Channel */
    ADC0->ADC_CR1 = (ADC0->ADC_CR1 & ~(ADC_CR1_CHANNEL_MASK)) | ADC_InitStruct->ADC_Channel; 
    /* Select ADC Channel Samping */
    ADC0->ADC_CR1 = (ADC0->ADC_CR1 & ~(ADC_CR1_SAMPLE_RATE_MASK)) | (ADC_InitStruct->ADC_SampSpeed << ADC_CR1_SAMPLE_RATE_POS);
    /* Set ADC Interrupt */
    if (ENABLE == ADC_InitStruct->ADC_IRQ_EN )
    {
        NVIC_EnableIRQ(ADC0_IRQn);
        ADC0->ADC_CR1 |= ADC_IRQ_EN_BIT;
    }else
    {
        ADC0->ADC_CR1 &= ~ADC_IRQ_EN_BIT;
    }
    /* Set ADC FIFO */
    if (ENABLE == ADC_InitStruct->ADC_FIFO_EN)
    {
        ADC0->ADC_FIFO |= ADC_FIFO_EN_BIT;
    }else
    {
        ADC0->ADC_FIFO &= ~ADC_FIFO_EN_BIT;
    }
}

/******************************************************************************
* Function Name  : ADC_GetChannel
* Description    : 获取ADC当前使用的通道
* Input          : None
* Return         : ADC_ChxTypeDef
******************************************************************************/
ADC_ChxTypeDef ADC_GetChannel(void)
{
    return ADC0->ADC_CR1 & ADC_CR1_CHANNEL_MASK;
}

/******************************************************************************
* Function Name  : ADC_GetResult
* Description    : 立即获取ADC相应通道的值
* Input          : None
* Return         : 0:获取超时  Other:ADC值
******************************************************************************/
int32_t ADC_GetResult(void)
{
    while((!(ADC0->ADC_SR & ADC_DONE_FLAG)));
    return (ADC0->ADC_DATA & ADC_DATA_MASK) - *(int16_t *)(ADC_DATA_OFFSET_OTPADDR);
}

/******************************************************************************
* Function Name  : ADC_GetFIFOResult
* Description    : 立即获取ADC相应通道的值,有超时检测
* Input          : ADCdata:FIFO指针 
                   len:长度
* Return         : FIFO中ADC数据
******************************************************************************/
int32_t ADC_GetFIFOResult(uint16_t *ADCdata, uint32_t len)
{
    uint32_t i, adccount = 0;
    //获取长度大于FIFO门限个数
    if(NULL == ADCdata)
    {
        return 0;
    }

    adccount = ADC_GetFIFOCount();
    if(adccount > len)
    {
        adccount = len;
    }
    
    for(i = 0; i < adccount; i++)
    {
        ADCdata[i] = (ADC0->ADC_DATA & ADC_DATA_MASK) - *(int16_t *)(ADC_DATA_OFFSET_OTPADDR);
    }
    return adccount;
}

/******************************************************************************
* Function Name  : ADC_GetFIFOCount
* Description    : 获取FIFO中数据的个数
* Input          : None
* Return         : 0:未开启FIFO模式 other:FIFO中数据的个数
******************************************************************************/
int32_t ADC_GetFIFOCount(void)
{
    if ((ADC0->ADC_FIFO & ADC_FIFO_EN_BIT) != ADC_FIFO_EN_BIT)
    {
        return 0;
    }
    return ADC0->ADC_FIFO_FL & 0x1F;
}

/******************************************************************************
* Function Name  : ADC_FifoReset
* Description    : 重置ADC fifo
* Input          : None
* Return         : None
******************************************************************************/
void ADC_FIFOReset(void)
{
    ADC0->ADC_FIFO |= ADC_FIFO_RESET_BIT;
}

/******************************************************************************
* Function Name  : ADC_ITCmd
* Description    : 控制ADC中断的开关
* Input          : NewState：ADC中断开关参数，可以取ENABLE和DISABLE
* Output         : NONE
* Return         : NONE
******************************************************************************/
void ADC_ITCmd(FunctionalState NewState)
{
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    if(DISABLE != NewState)
	{
		ADC0->ADC_CR1 |= ADC_IRQ_EN_BIT;
	}
	else
	{
		ADC0->ADC_CR1 &= ~ADC_IRQ_EN_BIT;
	}
}

/******************************************************************************
* Function Name  : ADC_FIFOOverflowITcmd
* Description    : 控制ADC FIFO溢出中断的开关
* Input          : NewState：ENABLE/DISABLE
* Output         : NONE
* Return         : NONE
******************************************************************************/
void ADC_FIFOOverflowITcmd(FunctionalState NewState)
{
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    if(DISABLE != NewState)
	{
		ADC0->ADC_FIFO |= ADC_FIFOOVERFLOWIRQ_EN_BIT;
	}
	else
	{
		ADC0->ADC_FIFO &= ~ADC_FIFOOVERFLOWIRQ_EN_BIT;
	}
}

/******************************************************************************
* Function Name  : ADC_ISFIFOOverflow
* Description    : 获取ADC溢出中断是否置位
* Input          : NONE
* Output         : ADC_OverFlow/ADC_NoOverFlow
* Return         : NONE
******************************************************************************/
ADC_OverFlowTypeDef ADC_ISFIFOOverflow(void)
{
    if((ADC0->ADC_SR & ADC_FIFOOVERFLOWIRQ_FLAG) == ADC_FIFOOVERFLOWIRQ_FLAG)
    {
        return ADC_OverFlow;
    }
    else
    {
        return ADC_NoOverFlow;
    }
}

/******************************************************************************
* Function Name  : ADC_StartCmd
* Description    : ADC转换启动控制
* Input          : NewState：ADC转换启动开关参数，可以取ENABLE和DISABLE
* Output         : NONE
* Return         : NONE
******************************************************************************/
void ADC_StartCmd(FunctionalState NewState)
{
    if (DISABLE != NewState)
    {
        ADC0->ADC_CR1 |= ADC_SAMP_EN_BIT;
        ADC0->ADC_CR1 &= ~ADC_START_BIT;
    }
    else
    {
        ADC0->ADC_CR1 &= ~ADC_SAMP_EN_BIT;
        ADC0->ADC_CR1 |= ADC_START_BIT;
    }
}

/******************************************************************************
* Function Name  : ADC_FIFODeepth
* Description    : ADC fifo 深度设置
* Input          : FIFO_Deepth 设置fifo深度，最大值0x20
* Output         : NONE
* Return         : NONE
******************************************************************************/
void ADC_FIFODeepth(uint32_t FIFO_Deepth)
{
    ADC0->ADC_FIFO_THR = FIFO_Deepth - 1;
}

/******************************************************************************
* Function Name  : ADC_CalVoltage
* Description    : 计算转换后的电压值
* Input          : u32ADC_Value: ADC采集值
*                : u32ADC_Ref_Value: ADC参考电压
* Output         : NONE
* Return         : 计算转换后的电压值
******************************************************************************/
uint32_t ADC_CalVoltage(uint32_t u32ADC_Value, uint32_t u32ADC_Ref_Value)
{
    return (u32ADC_Value * u32ADC_Ref_Value / 1023);
}

/************************ (C) COPYRIGHT 2017 Megahuntmicro ****END OF FILE****/
