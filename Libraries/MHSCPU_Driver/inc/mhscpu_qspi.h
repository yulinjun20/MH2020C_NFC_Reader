/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_qspi.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the QSPI firmware library
 *****************************************************************************/

#ifndef __MHSCPU_QSPI_H
#define __MHSCPU_QSPI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "mhscpu.h"


/* Size of the flash */
#define X25Q_PAGE_SIZE				          0x100    

#define QSPI_SUPPORT_CHIP_JEDEC_ID_GD		  0xC8
#define QSPI_SUPPORT_CHIP_JEDEC_ID_WD		  0xEF
#define QSPI_SUPPORT_CHIP_JEDEC_ID_MXIC		  0xC2
#define QSPI_SUPPORT_CHIP_JEDEC_ID_EON        0x1C
#define QSPI_SUPPORT_CHIP_JEDEC_ID_ZBIT       0x5E
#define QSPI_SUPPORT_CHIP_JEDEC_ID_PUYA       0x85	
#define QSPI_SUPPORT_CHIP_JEDEC_ID_MICR		  0x20		//NO SUPPOR

/***Command Definitions***/
#define RESET_ENABLE_CMD                   	  0x66
#define RESET_MEMORY_CMD                      0x99

#define ENTER_QPI_MODE_CMD                    0x38
#define EXIT_QPI_MODE_CMD                     0xFF

// Identification Operations
#define READ_ID_CMD                           0x90
#define DUAL_READ_ID_CMD                      0x92
#define QUAD_READ_ID_CMD                      0x94
#define READ_JEDEC_ID_CMD                     0x9F

// Read Operations
#define READ_CMD                              0x03
#define FAST_READ_CMD                         0x0B
#define DUAL_OUT_FAST_READ_CMD     	          0x3B
#define DUAL_INOUT_FAST_READ_CMD              0xBB
#define QUAD_OUT_FAST_READ_CMD                0x6B
#define QUAD_INOUT_FAST_READ_CMD     	      0xEB

//Write Operations 
#define WRITE_ENABLE_CMD				      0x06
#define WRITE_DISABLE_CMD				      0x04

// Register Operations 
#define READ_STATUS_REG1_CMD			      0x05
#define READ_STATUS_REG2_CMD			      0x35
#define READ_STATUS_REG3_CMD			      0x15

#define WRITE_STATUS_REG1_CMD			      0x01
#define WRITE_STATUS_REG2_CMD			      0x31
#define WRITE_STATUS_REG3_CMD			      0x11


// Program Operations 
#define PAGE_PROG_CMD						  0x02
#define QUAD_INPUT_PAGE_PROG_CMD              0x32


// Erase Operations 
#define SECTOR_ERASE_CMD                      0x20
#define BLOCK_32K_ERASE_CMD					  0x52
#define BLOCK_64K_ERASE_CMD					  0xD8
#define CHIP_ERASE_CMD                        0xC7

#define PROG_ERASE_RESUME_CMD             	  0x7A
#define PROG_ERASE_SUSPEND_CMD           	  0x75

#define SET_BURST_WITH_WRAP				      0x77
#define RELEASE_FROM_DEEP_POWER_DOWN	      0xAB
#define DEEP_POWER_DOWN					      0xB9

/***End Cmd***/


#define QSPI_DEVICE_PARA_SAMPLE_DLY_Pos       15
#define QSPI_DEVICE_PARA_SAMPLE_PHA_Pos       14
#define QSPI_DEVICE_PARA_PROTOCOL_Pos         8
#define QSPI_DEVICE_PARA_DUMMY_CYCLE_Pos      4


#define QSPI_ENCRYPT_MODE_AES               ((uint32_t)0x01)
#define QSPI_ENCRYPT_MODE_SM4               ((uint32_t)0x02)
#define IS_QSPI_ENCRYPT_MODE(MODE)          (((MODE) == QSPI_ENCRYPT_MODE_AES) || \
                                             ((MODE) == QSPI_ENCRYPT_MODE_SM4))

typedef enum
{
	QSPI_BUSMODE_111 = 0x00,	//CMD-ADDR-DATA = 1-1-1
	QSPI_BUSMODE_114 = 0x01,	//CMD-ADDR-DATA = 1-1-4
	QSPI_BUSMODE_144 = 0x02,	//CMD-ADDR-DATA = 1-4-4
	QSPI_BUSMODE_444 = 0x03,	//CMD-ADDR-DATA = 4-4-4
}QSPI_BusModeTypeDef;


typedef enum
{
	QSPI_CMDFORMAT_CMD8						= 0x00,
	QSPI_CMDFORMAT_CMD8_RREG8				= 0x01,
	QSPI_CMDFORMAT_CMD8_RREG16				= 0x02,
	QSPI_CMDFORMAT_CMD8_RREG24				= 0x03,
	QSPI_CMDFORMAT_CMD8_DMY24_WREG8			= 0x04,
	QSPI_CMDFORMAT_CMD8_ADDR24_RREG8		= 0x05,
	QSPI_CMDFORMAT_CMD8_ADDR24_RREG16		= 0x06,
	QSPI_CMDFORMAT_CMD8_WREG8				= 0x07,
	QSPI_CMDFORMAT_CMD8_WREG16				= 0x08,
	QSPI_CMDFORMAT_CMD8_ADDR24				= 0x09,
	QSPI_CMDFORMAT_CMD8_ADDR24_RDAT			= 0x0A,
	QSPI_CMDFORMAT_CMD8_ADDR24_DMY_RDAT		= 0x0B,
	QSPI_CMDFORMAT_CMD8_ADDR24_M8_DMY_RDAT	= 0x0C,
	QSPI_CMDFORMAT_CMD8_ADDR24_PDAT			= 0x0D
}QSPI_CmdFormatTypeDef;


typedef enum
{
	QSPI_PROTOCOL_CLPL		= 0x00,
	QSPI_PROTOCOL_CHPH		= 0x03
}QSPI_ProtocolTypedef;

typedef enum
{
	QSPI_FREQSEL_HCLK_DIV2	= 0x01,
	QSPI_FREQSEL_HCLK_DIV3	= 0x02,
	QSPI_FREQSEL_HCLK_DIV4	= 0x03
}QSPI_FreqSelTypeDef;

typedef enum
{
	QSPI_STATUS_OK				=(0x00),
	QSPI_STATUS_ERROR			=(0x01),
	QSPI_STATUS_BUSY			=(0x02),
	QSPI_STATUS_NOT_SUPPORTED 	=(0x04),
	QSPI_STATUS_SUSPENDED		=(0x08)
} QSPI_StatusTypeDef;


typedef struct
{	
	uint8_t SampleDly;
	uint8_t SamplePha;	
	uint8_t ProToCol;
	uint8_t DummyCycles;
	uint8_t FreqSel;
	
}QSPI_InitTypeDef;

typedef struct
{
	uint8_t Instruction;       
	QSPI_BusModeTypeDef BusMode;
	QSPI_CmdFormatTypeDef CmdFormat;
	
}QSPI_CommandTypeDef;


#define QSPI_ENABLE_AES_CRYPT     0
#define QSPI_ENABLE_SM4_CRYPT     0


void QSPI_Init(QSPI_InitTypeDef *mhqspi);
void QSPI_SetLatency(uint32_t u32UsClk);

uint32_t QSPI_ReadID(QSPI_CommandTypeDef *cmdParam);

uint8_t QSPI_WriteParam(QSPI_CommandTypeDef *cmdParam, uint16_t wrData);

uint8_t QSPI_EraseSector(QSPI_CommandTypeDef *cmdParam, uint32_t SectorAddress);
uint8_t QSPI_EraseChip(QSPI_CommandTypeDef *cmdParam);
uint8_t Flash_BlockErase64K(QSPI_CommandTypeDef *cmdParam, uint32_t SectorAddress);
uint8_t Flash_BlockErase32K(QSPI_CommandTypeDef *cmdParam, uint32_t SectorAddress);

uint8_t QSPI_ProgramPage(QSPI_CommandTypeDef *cmdParam, DMA_TypeDef *DMA_Channelx, uint32_t adr, uint32_t sz, uint8_t *buf);
uint8_t QSPI_ProgramOnePage(QSPI_CommandTypeDef *cmdParam, uint32_t adr, uint32_t sz, uint8_t *buf);

uint8_t QSPI_SoftWareReset(QSPI_BusModeTypeDef BusMode);
uint8_t QSPI_SingleCommand(QSPI_CommandTypeDef *cmdParam);

#if (QSPI_ENABLE_AES_CRYPT || QSPI_ENABLE_SM4_CRYPT) 
uint8_t QSPI_ProgramPageCipher(QSPI_CommandTypeDef *cmdParam, DMA_TypeDef *DMA_Channelx, uint32_t adr, uint32_t sz, uint8_t *buf, uint32_t EncryptMode);
#endif

uint8_t QSPI_DeepPowerDown(QSPI_CommandTypeDef *cmdParam);
uint8_t QSPI_ReleaseDeepPowerDown(QSPI_CommandTypeDef *cmdParam);

uint8_t QSPI_Read(QSPI_CommandTypeDef *cmdParam, uint8_t* buf, uint32_t addr, uint32_t sz);


uint16_t QSPI_StatusReg(QSPI_CommandTypeDef *cmdParam);


#ifdef __cplusplus
}
#endif

#endif   ///< __MHSCPU_QSPI_H


/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
