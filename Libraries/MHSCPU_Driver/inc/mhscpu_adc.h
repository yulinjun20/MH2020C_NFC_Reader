/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : mhscpu_adc.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 09/12/2016
 * Description          : ADC headfile.
 *****************************************************************************/
 
 
#ifndef __MHSCPU_ADC_H__
#define __MHSCPU_ADC_H__
 
 
#ifdef __cplusplus
extern "C" {
#endif
    
/* Include ------------------------------------------------------------------*/
#include "mhscpu.h" 
    
#define ADC_SAMP_EN_BIT                 BIT(6)    
#define ADC_START_BIT                   BIT(8)
#define ADC_DONE_FLAG                   BIT(0)    
#define ADC_IRQ_EN_BIT                  BIT(5)
#define ADC_FIFOOVERFLOWIRQ_EN_BIT      BIT(2)
#define ADC_FIFOOVERFLOWIRQ_FLAG        BIT(1)
#define ADC_FIFO_EN_BIT                 BIT(0)    
#define ADC_FIFO_RESET_BIT              BIT(1)
  
#define ADC_CR1_SAMPLE_RATE_POS                  3
#define ADC_CR1_SAMPLE_RATE_MASK                (0x03 << ADC_CR1_SAMPLE_RATE_POS)
#define ADC_CR1_CHANNEL_MASK                    (0x07)
    
#define ADC_CR2_REFTRIM_MASK                    (0x3F)
#define ADC_CR2_REFTRIM_OTPADDR                 (0x4000800B)
    
#define ADC_DATA_OFFSET_OTPADDR                 (0x4000800D)    

#define ADC_DATA_MASK                           (0x3FF)
    
typedef enum
{
    ADC_POLLING = 1,
    ADC_Interrupt = 2,
}ADC_ModeTypeDef;    
    
typedef enum
{
    ADC_OverFlow = 0,
    ADC_NoOverFlow = 1,
}ADC_OverFlowTypeDef;

/* ADC Channel select */  
typedef enum
{
    ADC_CHANNEL_0 = 0,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
	ADC_CHANNEL_CHARGE_VBAT,
}ADC_ChxTypeDef;
#define IS_ADC_CHANNEL(CHANNEL_NUM) (((CHANNEL_NUM) == ADC_CHANNEL_0) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_1) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_2) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_3) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_4) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_5) || \
									 ((CHANNEL_NUM) == ADC_CHANNEL_CHARGE_VBAT))
/* ADC Samp Select */
typedef enum
{
    ADC_SAMP_SEL_1M       	=              (uint32_t)(0x0000),
    ADC_SAMP_SEL_500K       =              (uint32_t)(0x0001),
    ADC_SAMP_SEL_250K       =              (uint32_t)(0x0002),
    ADC_SAMP_SEL_125K       =              (uint32_t)(0x0003)               ,
}ADC_SampTypeDef;
#define IS_ADC_SAMP(SAMP)           (((SAMP) == ADC_SAMP_SEL_125K)  || \
                                     ((SAMP) == ADC_SAMP_SEL_250K) || \
                                     ((SAMP) == ADC_SAMP_SEL_500K) || \
                                     ((SAMP) == ADC_SAMP_SEL_1M))
/* ADC Struct Define*/  
typedef struct _ADC_InitTypeDef
{
    ADC_ChxTypeDef              ADC_Channel;            /* ADC Channel select */
    ADC_SampTypeDef             ADC_SampSpeed;          /* ADC sampspeed select */
    FunctionalState             ADC_IRQ_EN;             /* ADC IRQ/Polling Select */
    FunctionalState             ADC_FIFO_EN;            /* ADC FIFO Enable Select */
} ADC_InitTypeDef;  


/* Exported constants -------------------------------------------------------*/ 
/* Exported macro -----------------------------------------------------------*/ 
/* Exported functions -------------------------------------------------------*/ 
void ADC_Init(ADC_InitTypeDef *ADC_InitStruct);
void ADC_StartCmd(FunctionalState NewState);
void ADC_FIFODeepth(uint32_t FIFO_Deepth);
void ADC_FIFOReset(void);
void ADC_ITCmd(FunctionalState NewState);
void ADC_FIFOOverflowITcmd(FunctionalState NewState);
ADC_ChxTypeDef ADC_GetChannel(void);
ADC_OverFlowTypeDef ADC_ISFIFOOverflow(void);

int32_t ADC_GetFIFOCount(void);
int32_t ADC_GetResult(void);
int32_t ADC_GetFIFOResult(uint16_t *ADCdata, uint32_t len);
uint32_t ADC_CalVoltage(uint32_t u32ADC_Value, uint32_t u32ADC_Ref_Value);
/* Exported variables -------------------------------------------------------*/
/* Exported types -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif   

#endif  /* __MHSCPU_ADC_H__ */
/************************ (C) COPYRIGHT 2016 Megahuntmicro ****END OF FILE****/
