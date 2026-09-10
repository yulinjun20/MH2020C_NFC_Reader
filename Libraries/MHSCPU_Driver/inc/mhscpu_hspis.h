/************************ (C) COPYRIGHT Megahuntmicro ****************************************************
 * @file                : mhscpu_hspis.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 18-September-2019
 * @brief               : This file contains all the functions prototypes for the HSPIS firmware library
 *********************************************************************************************************/
 
#ifndef __MHSCPU_HSPIS_H
#define __MHSCPU_HSPIS_H
#ifdef __cplusplus
extern
{
#endif

#include "mhscpu.h"
	
typedef struct
{
	uint8_t HSPIS_Sample;
	uint8_t HSPIS_RXEnable;
	uint8_t HSPIS_TXEnable;
	uint8_t HSPIS_RXFIFOFullThreshold;
	uint8_t HSPIS_TXFIFOEmptyThreshold;
	
}HSPIS_InitTypeDef;


typedef struct 
{
	uint8_t HSPIS_DMAReceiveEnable;
	uint8_t HSPIS_DMATransmitEnable;
	uint8_t HSPIS_DMAReceiveLevel;
	uint8_t HSPIS_DMATransmitLevel;
	
}HSPIS_DMAInitTypeDef;
	

/** @defgroup HSPIS_Sample
  * @{
  */
  
#define HSPIS_SAMPLE_MODE_0											(0x00)
#define HSPIS_SAMPLE_MODE_1											(0x01)
#define HSPIS_SAMPLE_MODE_2											(0x02)
#define HSPIS_SAMPLE_MODE_3											(0x03)

#define IS_HSPIS_SAMPLE(SAMPLE)										(((SAMPLE) == HSPIS_SAMPLE_MODE_0) || \
																	((SAMPLE) == HSPIS_SAMPLE_MODE_1)  || \
																	((SAMPLE) == HSPIS_SAMPLE_MODE_2)  || \
																	(SAMPLE) == HSPIS_SAMPLE_MODE_3)
/**
  * @}
  */
  

/** @defgroup HSPIS_TXFIFOEmptyThreshold
  * @{
  */
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_0		 						((uint32_t)0x0000)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_1       				 		((uint32_t)0x0001)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_2        						((uint32_t)0x0002)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_3        						((uint32_t)0x0003)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_4        						((uint32_t)0x0004)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_5        						((uint32_t)0x0005)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_6        						((uint32_t)0x0006)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_7        						((uint32_t)0x0007)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_8        						((uint32_t)0x0008)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_9        						((uint32_t)0x0009)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_10       						((uint32_t)0x000A)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_11       						((uint32_t)0x000B)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_12       						((uint32_t)0x000C)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_13       						((uint32_t)0x000D)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_14       						((uint32_t)0x000E)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_15       						((uint32_t)0x000F)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_31		  						((uint32_t)0x001F)
#define HSPIS_TX_FIFO_EMPTY_THRESHOLD_63       						((uint32_t)0x003F)

#define IS_HSPIS_TX_FIFO_EMPTY_THRESHOLD(THRESHOLD)					((((int32_t)(THRESHOLD)) >= HSPIS_TX_FIFO_EMPTY_THRESHOLD_0) && \
																	(((int32_t)(THRESHOLD)) <= HSPIS_TX_FIFO_EMPTY_THRESHOLD_63))
/**
  * @}
  */ 


/** @defgroup HSPIS_TXFIFOFullThreshold 
  * @{
  */
#define HSPIS_TX_FIFO_FULL_THRESHOLD_1       							((uint32_t)0x0001)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_2        					 	((uint32_t)0x0002)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_3       							((uint32_t)0x0003)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_4        						((uint32_t)0x0004)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_5        						((uint32_t)0x0005)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_6        						((uint32_t)0x0006)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_7        						((uint32_t)0x0007)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_8        						((uint32_t)0x0008)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_9        						((uint32_t)0x0009)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_10       						((uint32_t)0x000A)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_11       						((uint32_t)0x000B)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_12       						((uint32_t)0x000C)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_13       						((uint32_t)0x000D)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_14       						((uint32_t)0x000E)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_15       						((uint32_t)0x000F)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_16       						((uint32_t)0x0010)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_32       						((uint32_t)0x0020)
#define HSPIS_TX_FIFO_FULL_THRESHOLD_64       						((uint32_t)0x0040)


#define IS_HSPIS_TX_FIFO_FULL_THRESHOLD(THRESHOLD)					((((int32_t)(THRESHOLD)) >= HSPIS_TX_FIFO_FULL_THRESHOLD_1) && \
																	(((int32_t)(THRESHOLD)) <= HSPIS_TX_FIFO_FULL_THRESHOLD_64))														
/**
  * @}
  */ 
  
  
 /** @defgroup HSPIS_RXFIFOEmptyThreshold
  * @{
  */
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_0		 						((uint32_t)0x0000)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_1       				 		((uint32_t)0x0001)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_2        						((uint32_t)0x0002)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_3        						((uint32_t)0x0003)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_4        						((uint32_t)0x0004)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_5        						((uint32_t)0x0005)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_6        						((uint32_t)0x0006)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_7        						((uint32_t)0x0007)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_8        						((uint32_t)0x0008)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_9        						((uint32_t)0x0009)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_10       						((uint32_t)0x000A)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_11       						((uint32_t)0x000B)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_12       						((uint32_t)0x000C)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_13       						((uint32_t)0x000D)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_14       						((uint32_t)0x000E)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_15       						((uint32_t)0x000F)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_31		  						((uint32_t)0x001F)
#define HSPIS_RX_FIFO_EMPTY_THRESHOLD_63       						((uint32_t)0x003F)

#define IS_HSPIS_RX_FIFO_EMPTY_THRESHOLD(THRESHOLD)					((((int32_t)(THRESHOLD)) >= HSPIS_RX_FIFO_EMPTY_THRESHOLD_0) && \
																	(((int32_t)(THRESHOLD)) <= HSPIS_RX_FIFO_EMPTY_THRESHOLD_63))															
/**
  * @}
  */ 

/** @defgroup HSPIS_RXFIFOFullThreshold 
  * @{
  */
#define HSPIS_RX_FIFO_FULL_THRESHOLD_1       						 	((uint32_t)0x0001)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_2        					 	((uint32_t)0x0002)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_3       							((uint32_t)0x0003)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_4        						((uint32_t)0x0004)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_5        						((uint32_t)0x0005)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_6        						((uint32_t)0x0006)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_7        						((uint32_t)0x0007)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_8        						((uint32_t)0x0008)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_9        						((uint32_t)0x0009)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_10       						((uint32_t)0x000A)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_11       						((uint32_t)0x000B)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_12       						((uint32_t)0x000C)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_13       						((uint32_t)0x000D)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_14       						((uint32_t)0x000E)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_15       						((uint32_t)0x000F)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_16       						((uint32_t)0x0010)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_32       						((uint32_t)0x0020)
#define HSPIS_RX_FIFO_FULL_THRESHOLD_64       						((uint32_t)0x0040)


#define IS_HSPIS_RX_FIFO_FULL_THRESHOLD(THRESHOLD)					((((int32_t)(THRESHOLD)) >= HSPIS_RX_FIFO_FULL_THRESHOLD_1) && \
																	(((int32_t)(THRESHOLD)) <= HSPIS_RX_FIFO_FULL_THRESHOLD_64))
														
/**
  * @}
  */ 
  

/** @defgroup HSPIS_DMAReceiveLevel 
  * @{
  */
#define HSPIS_DMA_RECEIVE_LEVEL_1				    					((uint32_t)0x0001)
#define HSPIS_DMA_RECEIVE_LEVEL_2			        					((uint32_t)0x0002)
#define HSPIS_DMA_RECEIVE_LEVEL_3										((uint32_t)0x0003)
#define HSPIS_DMA_RECEIVE_LEVEL_4										((uint32_t)0x0004)
#define HSPIS_DMA_RECEIVE_LEVEL_5										((uint32_t)0x0005)
#define HSPIS_DMA_RECEIVE_LEVEL_6										((uint32_t)0x0006)
#define HSPIS_DMA_RECEIVE_LEVEL_7										((uint32_t)0x0007)
#define HSPIS_DMA_RECEIVE_LEVEL_8										((uint32_t)0x0008)
#define HSPIS_DMA_RECEIVE_LEVEL_9										((uint32_t)0x0009)
#define HSPIS_DMA_RECEIVE_LEVEL_10									((uint32_t)0x000A)
#define HSPIS_DMA_RECEIVE_LEVEL_11									((uint32_t)0x000B)
#define HSPIS_DMA_RECEIVE_LEVEL_12									((uint32_t)0x000C)
#define HSPIS_DMA_RECEIVE_LEVEL_13									((uint32_t)0x000D)
#define HSPIS_DMA_RECEIVE_LEVEL_14									((uint32_t)0x000E)
#define HSPIS_DMA_RECEIVE_LEVEL_15									((uint32_t)0x000F)
#define HSPIS_DMA_RECEIVE_LEVEL_16									((uint32_t)0x0010)
#define HSPIS_DMA_RECEIVE_LEVEL_32									((uint32_t)0x0020)
#define HSPIS_DMA_RECEIVE_LEVEL_64									((uint32_t)0x0040)

#define IS_HSPIS_DMA_RECEIVE_LEVEL(LEVEL)								((((int32_t)(LEVEL)) >= HSPIS_DMA_RECEIVE_LEVEL_1) && \
																	(((int32_t)(LEVEL)) <= HSPIS_DMA_RECEIVE_LEVEL_64))
/**
  * @}
  */ 

/** @defgroup HSPIS_DMATransmitLevel 
  * @{
  */
#define HSPIS_DMA_TRANSMIT_LEVEL_0									((uint32_t)0x0000)
#define HSPIS_DMA_TRANSMIT_LEVEL_1									((uint32_t)0x0001)
#define HSPIS_DMA_TRANSMIT_LEVEL_2									((uint32_t)0x0002)
#define HSPIS_DMA_TRANSMIT_LEVEL_3									((uint32_t)0x0003)
#define HSPIS_DMA_TRANSMIT_LEVEL_4									((uint32_t)0x0004)
#define HSPIS_DMA_TRANSMIT_LEVEL_5									((uint32_t)0x0005)
#define HSPIS_DMA_TRANSMIT_LEVEL_6									((uint32_t)0x0006)
#define HSPIS_DMA_TRANSMIT_LEVEL_7									((uint32_t)0x0007)
#define HSPIS_DMA_TRANSMIT_LEVEL_8									((uint32_t)0x0008)
#define HSPIS_DMA_TRANSMIT_LEVEL_9									((uint32_t)0x0009)
#define HSPIS_DMA_TRANSMIT_LEVEL_10									((uint32_t)0x000A)
#define HSPIS_DMA_TRANSMIT_LEVEL_11									((uint32_t)0x000B)
#define HSPIS_DMA_TRANSMIT_LEVEL_12									((uint32_t)0x000C)
#define HSPIS_DMA_TRANSMIT_LEVEL_13									((uint32_t)0x000D)
#define HSPIS_DMA_TRANSMIT_LEVEL_14									((uint32_t)0x000E)
#define HSPIS_DMA_TRANSMIT_LEVEL_15									((uint32_t)0x000F)
#define HSPIS_DMA_TRANSMIT_LEVEL_31									((uint32_t)0x001F)
#define HSPIS_DMA_TRANSMIT_LEVEL_63									((uint32_t)0x003F)

#define IS_HSPIS_DMA_TRANSMIT_LEVEL(LEVEL)							((((int32_t)(LEVEL)) >= HSPIS_DMA_TRANSMIT_LEVEL_0) && \
																	(((int32_t)(LEVEL)) <= HSPIS_DMA_TRANSMIT_LEVEL_63))
/**
  * @}
  */ 
  
/** @defgroup HSPIS FIFO Reset
  * @{
  */


#define HSPIS_FIFO_TX													((uint32_t)0x0080)
#define HSPIS_FIFO_RX													((uint32_t)0x0040)

/**
  * @}
  */ 
  
/** @defgroup HSPIS_Interrupt Mask and Status Flag 
  * @{
  */
#define HSPIS_IT_EN													((uint32_t)0x000004)
#define HSPIS_IT_RX													((uint32_t)0x200000)
#define HSPIS_IT_TX													((uint32_t)0x400000)

#define HSPIS_IT_ID_RXF												((uint32_t)0x0080)	 
#define HSPIS_IT_ID_RXTF												((uint32_t)0x0040)      							
#define HSPIS_IT_ID_RXE												((uint32_t)0x0020)	 	 

#define HSPIS_IT_ID_TXF												((uint32_t)0x0008)
#define HSPIS_IT_ID_TXE												((uint32_t)0x0002)
#define HSPIS_IT_ID_TXTF												((uint32_t)0x0001)

#define HSPIS_FLAG_RXE												((uint32_t)0x0400)
#define HSPIS_FLAG_RXF												((uint32_t)0x1000)
#define HSPIS_FLAG_RXTF												((uint32_t)0x2000)

#define HSPIS_FLAG_TXE												((uint32_t)0x0004)
#define HSPIS_FLAG_TXF												((uint32_t)0x0010)
#define HSPIS_FLAG_TXTF												((uint32_t)0x0020)		

/**
  * @}
  */ 
 
void HSPIS_Init(HSPIS_InitTypeDef* HSPIS_InitStruct);
void HSPIS_DMAInit(HSPIS_DMAInitTypeDef* QHSPIS_DMAInitStruct); 
void HSPIS_FIFOReset(uint32_t HSPIS_FIFO);
void HSPIS_ITConfig(uint32_t HSPIS_IT, FunctionalState NewState);
void HSPIS_SendData(uint8_t Data);
uint8_t HSPIS_ReceiveData(void);
FlagStatus HSPIS_GetFlagStatus(uint32_t HSPIS_Flag);
ITStatus HSPIS_GetITStatus(uint32_t HSPIS_IT);

#ifdef __cplusplus
}
#endif
#endif

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE**************************************/
