/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_qspi.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the QSPI firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "mhscpu.h"
#include "mhscpu_qspi.h"
#include "mhscpu_dma.h"
#include "mhscpu_timer.h"
#include "mhscpu_cache.h"
#if (QSPI_ENABLE_AES_CRYPT)
#include "mh_aes.h"
#endif
#if (QSPI_ENABLE_SM4_CRYPT)
#include "mh_sm4.h"
#endif

typedef struct
{
    uint8_t Instruction;       
    QSPI_BusModeTypeDef BusMode;
    QSPI_CmdFormatTypeDef CmdFormat;
    uint32_t Address;
	
    uint32_t WrData;
    uint32_t RdData;
	
}MH_CommandTypeDef;


//命令完成中断标志置位的最大是就是编程命令耗时
//(PP_CMD(1B) + PP_ADDR(3B) + PP_MAX_BYTES(256B) + Cache_line(32B) + Cache_CMD(1B)+Cache_ADDR(3B)) * 8(bits) * 2(CPU主频最大是QSPI频率的2倍) 
//(1 + 3 + 256 + 32 + 1 + 3) * 8 * 2
#define MH_QSPI_TIMEOUT_DEFAULT_CNT	        (5000) //4736

#define MH_QSPI_ACCESS_REQ_ENABLE	        (0x00000001U)
#define MH_QSPI_FLASH_READY_ENABLE	        (0x0000006BU)


#define IS_PARAM_NOTNULL(PARAM)			    ((PARAM) != NULL)

#define IS_QSPI_ADDR(ADDR)					((((int32_t)(ADDR)) >= (uint32_t)(0x00000000)) && \
											(((int32_t)(ADDR)) <= (uint32_t)(0x00FFFFFF)))

#define IS_QSPI_ADDR_ADD_SZ(ADDR, SZ)		((((int32_t)((ADDR) + (SZ))) >= (uint32_t)(0x00000000)) && \
											(((int32_t)((ADDR) + (SZ))) <= (uint32_t)(0x01000000)))    

#define QSPI_RD_DATA_LEN_MAX		        (0x10)
#define QSPI_WR_DATA_LEN_MAX		        (0x04)
#define QSPI_DMA_RD_DATA_LEN_MAX		    (0x100)
#define QSPI_DMA_WR_DATA_LEN_MAX		    (0x100)

#define QSPI_WR_FIFO_DEPTH	                (0x8)

#define QSPI_RX_FIFO_LEVEL_MASK             (0x7)
#define QSPI_TX_FIFO_LEVEL_POS              (16)
#define QSPI_TX_FIFO_LEVEL_MASK             (0xF)

#define QSPI_FIFO_CNTL_FLUSH_TX_FIFO        (((uint32_t)0x80000000))
#define QSPI_FIFO_CNTL_FLUSH_RX_FIFO        (((uint32_t)0x00008000))

//实测各主频及分频下延时均为42ms
static void delay_40ms(void)
{
	int i, j;
	for(i = 7; i > 0; i --)
		for(j = SYSCTRL->HCLK_1MS_VAL; j > 0; j --);
}


static QSPI_StatusTypeDef MH_QSPI_Command(MH_CommandTypeDef *cmd, int32_t timeout)
{
	uint32_t i;
	
	QSPI_StatusTypeDef status = QSPI_STATUS_ERROR;
	
	assert_param(IS_PARAM_NOTNULL(cmd));	
	
	MHSCPU_MODIFY_REG32(&(QSPI->REG_WDATA), (QUADSPI_REG_WDATA), (cmd->WrData));
	MHSCPU_MODIFY_REG32(&(QSPI->ADDRES), (QUADSPI_ADDRESS_ADR), (cmd->Address << 8));
	MHSCPU_MODIFY_REG32(&(QSPI->FCU_CMD), 
						(QUADSPI_FCU_CMD_CODE | QUADSPI_FCU_CMD_BUS_MODE | QUADSPI_FCU_CMD_CMD_FORMAT | QUADSPI_FCU_CMD_ACCESS_REQ),
						(((uint32_t)(cmd->Instruction << 24)) |((uint32_t)( cmd->BusMode<< 8)) |((uint32_t)( cmd->CmdFormat << 4))| (MH_QSPI_ACCESS_REQ_ENABLE)));
	
	/* Wait For command done */
	for (i = 0; i < timeout; i += 4)
	{
		if (QSPI->INT_RAWSTATUS & BIT0)
		{		
			QSPI->INT_CLEAR = 0x01;
			status = QSPI_STATUS_OK;
			break;
		}
	}
	
	cmd->RdData = QSPI->REG_RDATA;

	return status;
}

uint32_t QSPI_SendCommand(QSPI_CommandTypeDef *cmdParam)
{
	MH_CommandTypeDef sCommand;

	sCommand.Instruction = cmdParam->Instruction;//READ_JEDEC_ID_CMD;    
	sCommand.BusMode = cmdParam->BusMode;//busMode;
	sCommand.CmdFormat = cmdParam->CmdFormat;//0x03U;
	
	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	return QSPI_STATUS_OK;
}

static QSPI_StatusTypeDef QSPI_WriteEnable(QSPI_BusModeTypeDef bus_mode)
{
	MH_CommandTypeDef sCommand;

	sCommand.Instruction = WRITE_ENABLE_CMD;       
	sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8;

	if (QSPI_BUSMODE_444 == bus_mode)
	{
		sCommand.BusMode = QSPI_BUSMODE_444;
	}
	else
	{
		sCommand.BusMode = QSPI_BUSMODE_111;
	}
	
	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	return QSPI_STATUS_OK;
}

static void QSPI_WriteEnableWithoutTimeout(QSPI_BusModeTypeDef bus_mode)
{
	MH_CommandTypeDef sCommand;

	sCommand.Instruction = WRITE_ENABLE_CMD;       
	sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8;
	if (QSPI_BUSMODE_444 == bus_mode)
	{
		sCommand.BusMode = QSPI_BUSMODE_444;
	}
	else
	{
		sCommand.BusMode = QSPI_BUSMODE_111;
	}
	
	MH_QSPI_Command(&sCommand, 0);
}


//PP,QPP,Sector Erase,Block Erase, Chip Erase, Write Status Reg, Erase Security Reg
static QSPI_StatusTypeDef QSPI_IsBusy(QSPI_BusModeTypeDef bus_mode)
{
	MH_CommandTypeDef sCommand;

	sCommand.Instruction = READ_STATUS_REG1_CMD;       
	sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8_RREG8;

	if (QSPI_BUSMODE_444 == bus_mode)
	{
		sCommand.BusMode = QSPI_BUSMODE_444;
	}
	else
	{
		sCommand.BusMode = QSPI_BUSMODE_111;
	}
	
	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
		
	if (sCommand.RdData & BIT0)
	{
		return QSPI_STATUS_ERROR;
	}
	
	return QSPI_STATUS_OK;
}

uint8_t QSPI_Read(QSPI_CommandTypeDef *cmdParam, uint8_t* buf, uint32_t addr, uint32_t sz)
{
	uint32_t read_times = 0,i = 0, j = 0, k = 0;
	uint8_t end_len = 0;
	MH_CommandTypeDef sCommand;

	while (CACHE->CACHE_CS & CACHE_IS_BUSY);
	
	addr &= (uint32_t)(0x00FFFFFF);
	assert_param(IS_QSPI_ADDR(addr));
	assert_param(IS_QSPI_ADDR_ADD_SZ(addr, sz));

	if(cmdParam == NULL)
	{
		sCommand.Instruction = READ_CMD;
		sCommand.BusMode = QSPI_BUSMODE_111;
		sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24_RDAT;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction;       
		sCommand.BusMode = cmdParam->BusMode;
		sCommand.CmdFormat = cmdParam->CmdFormat;
	}
	
	//由于芯片读取速率问题，每次最多读6*32bits的数据
	read_times = sz / QSPI_RD_DATA_LEN_MAX;
	end_len = sz % QSPI_RD_DATA_LEN_MAX;

	for (i = 0; i < read_times; i ++)
	{
        QSPI->FIFO_CNTL |= QSPI_FIFO_CNTL_FLUSH_RX_FIFO;
        QSPI->BYTE_NUM = QSPI_RD_DATA_LEN_MAX;
        
		sCommand.Address = addr;

		if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
		{
			return QSPI_STATUS_ERROR;
		}

		j = (QSPI->FIFO_CNTL & QSPI_RX_FIFO_LEVEL_MASK);
		for (k = 0; k < j; k++)
		{
			*(uint32_t *)(buf) = QSPI->RD_FIFO;
			buf += 4;
		}
		
		addr = addr + QSPI_RD_DATA_LEN_MAX;
	}
    
	QSPI->FIFO_CNTL |= QSPI_FIFO_CNTL_FLUSH_RX_FIFO;
	sCommand.Address = addr;
	if (end_len > 0)
	{
		QSPI->BYTE_NUM = end_len;
		
		if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
		{
			return QSPI_STATUS_ERROR;
		}

		j = (QSPI->FIFO_CNTL & QSPI_RX_FIFO_LEVEL_MASK);
		for (k = 0; k < j; k++)
		{
			*(uint32_t *)(buf) = QSPI->RD_FIFO;
			buf += 4;
		}
	}
	
	return QSPI_STATUS_OK;
}


static uint8_t QSPI_ProgramPage_Ex(QSPI_CommandTypeDef *cmdParam, uint32_t adr, uint32_t sz, uint8_t *buf)
{
	uint32_t i,j;
	uint32_t end_addr, current_size, current_addr, current_dat,loop_addr;
	MH_CommandTypeDef sCommand;

	adr &= (uint32_t)0x00FFFFFF;
	assert_param(IS_QSPI_ADDR(adr));
	assert_param(IS_QSPI_ADDR_ADD_SZ(adr, sz));
	
	current_addr = 0;

	while (current_addr <= adr)
	{
       	current_addr += X25Q_PAGE_SIZE;
	}
	current_size = current_addr - adr;

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > sz)
	{
		current_size = sz;
	}

    /* Initialize the adress variables */
	current_addr = adr;
	loop_addr = adr;
	end_addr = adr + sz;

	QSPI->FIFO_CNTL |= BIT31;
	QSPI->BYTE_NUM = QSPI_WR_DATA_LEN_MAX << 16;
	sCommand.Address = current_addr;

	if (cmdParam == NULL)
	{
		sCommand.Instruction = QUAD_INPUT_PAGE_PROG_CMD;       
		sCommand.BusMode = QSPI_BUSMODE_114;
		sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction;       
		sCommand.BusMode = cmdParam->BusMode;
		sCommand.CmdFormat = cmdParam->CmdFormat;
	}
	
	do
	{
		QSPI->BYTE_NUM = QSPI_WR_DATA_LEN_MAX << 16;
		
		for (i = 0; i < (current_size/QSPI_WR_DATA_LEN_MAX); i++)
		{
			current_dat = 0;
			current_dat |= *(buf + 3) << 24;
			current_dat |= *(buf + 2) << 16;
			current_dat |= *(buf + 1) << 8;
			current_dat |= *(buf + 0);
			
			QSPI->WR_FIFO = current_dat;
	
	        if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
	        {
				return QSPI_STATUS_ERROR;
	        }
	        
	        if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	        {
	        	return QSPI_STATUS_ERROR;
	        }
	        while (QSPI_IsBusy(sCommand.BusMode));
				
	        QSPI->FIFO_CNTL |= QSPI_FIFO_CNTL_FLUSH_TX_FIFO;
			buf += 4;

			current_addr += QSPI_WR_DATA_LEN_MAX;
			sCommand.Address = current_addr;
        }
		
		if (current_size % QSPI_WR_DATA_LEN_MAX > 0)
		{
			current_dat = 0;
			QSPI->FIFO_CNTL |= QSPI_FIFO_CNTL_FLUSH_TX_FIFO;
			
			for (j = (current_size % QSPI_WR_DATA_LEN_MAX); j > 0 ; j--)
			{
				current_dat <<= 8;
				current_dat |= *(buf + j - 1); 
			}
			QSPI->WR_FIFO = current_dat;
			QSPI->BYTE_NUM = ((current_size % QSPI_WR_DATA_LEN_MAX)) << 16;
		    
			if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
	       	{
				return QSPI_STATUS_ERROR;
			}
	        
			MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT);
			while(QSPI_IsBusy(sCommand.BusMode));
			
			buf += (current_size % QSPI_WR_DATA_LEN_MAX);
			sCommand.Address = (current_addr + current_size % QSPI_WR_DATA_LEN_MAX);
		}  
       	loop_addr += current_size;
		current_addr = loop_addr;
		current_size = ((loop_addr + X25Q_PAGE_SIZE) > end_addr) ? (end_addr - loop_addr) : X25Q_PAGE_SIZE;
	}while (loop_addr < end_addr);
	
	return QSPI_STATUS_OK;
}

uint8_t QSPI_ProgramOnePage(QSPI_CommandTypeDef *cmdParam, uint32_t adr, uint32_t sz, uint8_t *buf)
{
	uint32_t offset = 0, timeout;
	uint32_t *data = NULL;
	uint32_t current_size, current_addr;
	MH_CommandTypeDef sCommand;

	adr &= (uint32_t)0x00FFFFFF;
	assert_param(IS_QSPI_ADDR(adr));
	assert_param(IS_QSPI_ADDR_ADD_SZ(adr, sz));
	
	current_addr = 0;
	while (current_addr <= adr)
	{
       	current_addr += X25Q_PAGE_SIZE;
	}
	current_size = current_addr - adr;

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > sz)
	{
		current_size = sz;
	}

    /* Initialize the adress variables */
	current_addr = adr;

	QSPI->FIFO_CNTL |= QSPI_FIFO_CNTL_FLUSH_TX_FIFO;
	QSPI->BYTE_NUM = X25Q_PAGE_SIZE << 16;
	sCommand.Address = current_addr;
	
	data = (uint32_t *)buf;
	for (offset = 0; offset < QSPI_WR_FIFO_DEPTH; offset++)
	{
		QSPI->WR_FIFO = *(data++);
	}
	
	if (cmdParam == NULL)
	{
		sCommand.Instruction = QUAD_INPUT_PAGE_PROG_CMD;       
		sCommand.BusMode = QSPI_BUSMODE_114;
		sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction;       
		sCommand.BusMode = cmdParam->BusMode;
		sCommand.CmdFormat = cmdParam->CmdFormat;
	}

	QSPI_WriteEnableWithoutTimeout(sCommand.BusMode);
	MH_QSPI_Command(&sCommand, 0);

	while (offset < (X25Q_PAGE_SIZE >> 2))
	{
		if (((QSPI->FIFO_CNTL >> QSPI_TX_FIFO_LEVEL_POS) & QSPI_TX_FIFO_LEVEL_MASK) <= 4)
		{
			QSPI->WR_FIFO = *(data++);
			QSPI->WR_FIFO = *(data++);
			QSPI->WR_FIFO = *(data++);
			QSPI->WR_FIFO = *(data++);
			offset += 4;			
		}
	}
	
	/* Wait For CMD done */ 
	for (timeout = 0; timeout < MH_QSPI_TIMEOUT_DEFAULT_CNT; timeout += 4)
	{
		if (QSPI->INT_RAWSTATUS & BIT0)
		{
			QSPI->INT_CLEAR = BIT0;
			break;
		}
	}	
	
	while (QSPI_IsBusy(sCommand.BusMode));
	
	return QSPI_STATUS_OK;
}

static QSPI_StatusTypeDef QSPI_DMA_Configuration(DMA_TypeDef *DMA_Channelx, DMA_InitTypeDef *DMA_InitStruct)
{
	DMA_InitStruct->DMA_DIR = DMA_DIR_Memory_To_Peripheral;
	DMA_InitStruct->DMA_Peripheral = (uint32_t)(QSPI);
	DMA_InitStruct->DMA_PeripheralBaseAddr = (uint32_t)&(QSPI->WR_FIFO);
	DMA_InitStruct->DMA_PeripheralInc = DMA_Inc_Nochange;
	DMA_InitStruct->DMA_PeripheralDataSize = DMA_DataSize_Word;
	DMA_InitStruct->DMA_PeripheralBurstSize = DMA_BurstSize_4;
	
	DMA_InitStruct->DMA_MemoryInc = DMA_Inc_Increment;
	DMA_InitStruct->DMA_MemoryDataSize = DMA_DataSize_Word;
	DMA_InitStruct->DMA_MemoryBurstSize = DMA_BurstSize_4;
	DMA_InitStruct->DMA_BlockSize = QSPI_DMA_WR_DATA_LEN_MAX / 4;
	DMA_InitStruct->DMA_PeripheralHandShake = DMA_PeripheralHandShake_Hardware;

	if (DMA_Channelx == DMA_Channel_0)
	{
		SYSCTRL->DMA_CHAN &= ~0x1F;
		SYSCTRL->DMA_CHAN |= 0x1A << 0;
	}
	else
	{
		return	QSPI_STATUS_NOT_SUPPORTED;
	}
	return QSPI_STATUS_OK;
}

#define QSPI_DEBUG 	0

static uint8_t buf_aes_enc(uint8_t *cp_buf, uint32_t sz, uint8_t *sup_buf)
{
#if (QSPI_ENABLE_AES_CRYPT)	
	
	uint8_t is_sup = FALSE;
	uint32_t i,k,cnt_buf;
	MH_SYM_CRYPT_CALL callAes;
	int j;
	
	uint8_t key128Bak[] = {0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
	uint8_t ivBak[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
	uint8_t key128[16];
	uint8_t iv[16];
	
	uint8_t cipher[32];
	uint8_t plain[32];
											 
	memset(cipher, 0, sizeof(cipher));
	memset(plain, 0, sizeof(plain));
	
#if (QSPI_DEBUG)	
	printf("Plain:\r\n");
	for(i = 0; i < sz; i ++)
	{
		printf("%02X ",cp_buf[i]);
	}
	printf("\r\n");
#endif

	callAes.pu8In = plain;
	callAes.u32InLen = sizeof(plain);	
	callAes.pu8Out = cipher;
	callAes.u32OutLen = sizeof(cipher);
	callAes.pu8IV = iv;
	callAes.pu8Key = key128;
	callAes.u16Opt = MH_AES_OPT_MODE_ENCRYPT | MH_AES_OPT_KEY_128 | MH_AES_OPT_BLK_CBC;
	callAes.u32Crc = 0;	
	
	//Cache 32Bytes CBC;
	//set 0xFF to 32
	for(i = 0; i < sz/32; i ++)
	{
		//memcpy(plain, cp_buf + i * 32, 32);
		k = 0;
		for(j = 15; j >= 0; j --)
		{
			plain[k] = cp_buf[ i * 32 + j];
			plain[k + 16] = cp_buf[i * 32 + 16 + j];
			k += 1;
		}
		
		memcpy(key128, key128Bak, sizeof(key128));
		memcpy(iv, ivBak, sizeof(iv));		
				
		MHAES_EncDec(&callAes);		
		
		k = 0;
		for(j = 15; j >= 0; j --)
		{
			cp_buf[i * 32 + j] = cipher[k];
			cp_buf[i * 32 + 16 + j] = cipher[k + 16];
			k += 1;
		}
		//memcpy(cp_buf + i * 32, cipher, 32);
	}
	cnt_buf = i;
	
	if(sz % 32)
	{
		uint8_t  tmp;
		
		is_sup = TRUE;
		memset(plain, 0xFF, 32);
		memcpy(plain, cp_buf + cnt_buf * 32, sz%32);
		
		for(j = 0; j < 8; j ++)
		{
			tmp = plain[j];
			plain[j] = plain[15 - j];
			plain[15 - j] = tmp;

			tmp = plain[j + 16];
			plain[j + 16] = plain[31 - j];
			plain[31 - j] = tmp;
		}

		memcpy(key128, key128Bak, sizeof(key128));
		memcpy(iv, ivBak, sizeof(iv));
		MHAES_EncDec(&callAes);	
	
		k = 0;
		for(j = 15; j >= 0; j --)
		{
			sup_buf[j] = cipher[k];
			sup_buf[16 + j] = cipher[k + 16];
			k += 1;
		}
	}
#if (QSPI_DEBUG)
	printf("cipher: sz = %#x\r\n", sz);
	for(i = 0; i < (sz / 32) * 32; i ++)
	{
		printf("%02X ",cp_buf[i]);
	}
	for(i = 0; i < 32; i ++)
	{
		printf("%02X ",sup_buf[i]);
	}
	printf("\r\n");
	
	printf("AES ECB key128 test\n");
#endif

	return is_sup;
	
#else
	return FALSE;
#endif	
}
extern void print_Log(const char *pcStr, uint32_t u32Num);
static uint8_t buf_sm4_enc(uint8_t *cp_buf, uint32_t sz, uint8_t *sup_buf)
{
#if (QSPI_ENABLE_SM4_CRYPT)	
	uint8_t is_sup = FALSE;
	uint32_t i,k,cnt_buf;
    MH_SYM_CRYPT_CALL callSm4;
	int j;
	
	uint8_t key128Bak[] = {0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
	uint8_t ivBak[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

	uint8_t key128[16];
	uint8_t iv[16];
	
	uint8_t cipher[32];
	uint8_t plain[32];
											 
	memset(cipher, 0, sizeof(cipher));
	memset(plain, 0, sizeof(plain));
	
#if (QSPI_DEBUG)	
	printf("Plain:\r\n");
	for(i = 0; i < sz; i ++)
	{
		printf("%02X ",cp_buf[i]);
	}
	printf("\r\n");
#endif		
	
	callSm4.pu8In = plain;
	callSm4.u32InLen = sizeof(plain);	
	callSm4.pu8Out = cipher;
	callSm4.u32OutLen = sizeof(cipher);
	callSm4.pu8IV = iv;
	callSm4.pu8Key = key128;
	callSm4.u16Opt = MH_SM4_OPT_MODE_ENCRYPT | MH_SM4_OPT_BLK_CBC;
	callSm4.u32Crc = 0;		
	
	//Cache 32Bytes CBC;
	//set 0xFF to 32
	for(i = 0; i < sz/32; i ++)
	{
		//memcpy(plain, cp_buf + i * 32, 32);
		k = 0;
		for(j = 15; j >= 0; j --)
		{
			plain[k] = cp_buf[ i * 32 + j];
			plain[k + 16] = cp_buf[i * 32 + 16 + j];
			k += 1;
		}
		
		memcpy(key128, key128Bak, sizeof(key128));
		memcpy(iv, ivBak, sizeof(iv));					
		MHSM4_EncDec(&callSm4);	
	
		k = 0;
		for(j = 15; j >= 0; j --)
		{
			cp_buf[i * 32 + j] = cipher[k];
			cp_buf[i * 32 + 16 + j] = cipher[k + 16];
			k += 1;
		}
		//memcpy(cp_buf + i * 32, cipher, 32);
	}
	cnt_buf = i;
	
	if(sz % 32)
	{
		uint8_t  tmp;
		
		is_sup = TRUE;
		memset(plain, 0xFF, 32);
		memcpy(plain, cp_buf + cnt_buf * 32, sz%32);
		
		for(j = 0; j < 8; j ++)
		{
			tmp = plain[j];
			plain[j] = plain[15 - j];
			plain[15 - j] = tmp;

			tmp = plain[j + 16];
			plain[j + 16] = plain[31 - j];
			plain[31 - j] = tmp;
		}

		memcpy(key128, key128Bak, sizeof(key128));
		memcpy(iv, ivBak, sizeof(iv));
		MHSM4_EncDec(&callSm4);	
	
		k = 0;
		for(j = 15; j >= 0; j --)
		{
			sup_buf[j] = cipher[k];
			sup_buf[16 + j] = cipher[k + 16];
			k += 1;
		}
	}
#if (QSPI_DEBUG)
	printf("cipher: sz = %#x\r\n", sz);
	for(i = 0; i < (sz / 32) * 32; i ++)
	{
		printf("%02X ",cp_buf[i]);
	}
	for(i = 0; i < 32; i ++)
	{
		printf("%02X ",sup_buf[i]);
	}
	printf("\r\n");
	
	printf("SM4 CBC key128 test\n");
#endif
	
	return is_sup;
#else
	return FALSE;	
#endif	
}


uint8_t QSPI_SoftWareReset(QSPI_BusModeTypeDef BusMode)
{
	MH_CommandTypeDef sCommand;
	
	sCommand.Instruction = RESET_ENABLE_CMD;      
	sCommand.BusMode = BusMode;
	sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8;

	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	sCommand.Instruction = RESET_MEMORY_CMD;      
	sCommand.BusMode = BusMode;
	sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8;

	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	//复位后必须延时30ms以上
	delay_40ms();
	
	return QSPI_STATUS_OK;
}

uint8_t QSPI_SingleCommand(QSPI_CommandTypeDef *cmdParam)
{
	MH_CommandTypeDef sCommand;

	sCommand.Instruction = cmdParam->Instruction;      
	sCommand.BusMode = cmdParam->BusMode;
	sCommand.CmdFormat = cmdParam->CmdFormat;

	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	
	return QSPI_STATUS_OK;
}


uint8_t QSPI_DeepPowerDown(QSPI_CommandTypeDef *cmdParam)
{
	MH_CommandTypeDef sCommand;
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);
	
	if (NULL == cmdParam)
	{
		sCommand.Instruction = DEEP_POWER_DOWN;      
		sCommand.BusMode = QSPI_BUSMODE_111;
		sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction; 
		sCommand.BusMode 	= cmdParam->BusMode;
		sCommand.CmdFormat = cmdParam->CmdFormat;
	}
	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	return QSPI_STATUS_OK;
}

uint8_t QSPI_ReleaseDeepPowerDown(QSPI_CommandTypeDef *cmdParam)
{
	MH_CommandTypeDef sCommand;

	uint32_t clock_delay; 
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);
	
	if (NULL == cmdParam)
	{
		sCommand.Instruction 	= RELEASE_FROM_DEEP_POWER_DOWN;      
		sCommand.BusMode = QSPI_BUSMODE_111;
		sCommand.CmdFormat 	= QSPI_CMDFORMAT_CMD8;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction; 
		sCommand.BusMode 	= cmdParam->BusMode;
		sCommand.CmdFormat = cmdParam->CmdFormat;
	}
	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}

	//防止唤醒期间CACHE访问FLASH导致死机
	//延时100us
	for (clock_delay =0; clock_delay < (SYSCTRL->HCLK_1MS_VAL* 10 /1000); clock_delay += 4);
	
//		delay_40ms();
	
	return QSPI_STATUS_OK;
}

uint32_t QSPI_ReadID(QSPI_CommandTypeDef *cmdParam)
{
	MH_CommandTypeDef sCommand;
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);

	if (cmdParam == NULL)
	{
		sCommand.Instruction = READ_JEDEC_ID_CMD;    
		sCommand.BusMode = QSPI_BUSMODE_111;
		sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8_RREG24;//0x03U;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction;//READ_JEDEC_ID_CMD;    
		sCommand.BusMode = cmdParam->BusMode;//busMode;
		sCommand.CmdFormat = cmdParam->CmdFormat;//0x03U;
	}

	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	return (sCommand.RdData & 0x00FFFFFF);
}

uint16_t QSPI_StatusReg(QSPI_CommandTypeDef *cmdParam)
{
	MH_CommandTypeDef sCommand;

	assert_param(IS_PARAM_NOTNULL(cmdParam));
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);
	
	sCommand.Instruction = cmdParam->Instruction;
	sCommand.BusMode = cmdParam->BusMode;
	sCommand.CmdFormat = cmdParam->CmdFormat;
	
	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	return (sCommand.RdData & 0x0000FFFF);
}

uint8_t QSPI_WriteParam(QSPI_CommandTypeDef *cmdParam, uint16_t wrData)
{
	MH_CommandTypeDef sCommand;
	
	assert_param(IS_PARAM_NOTNULL(cmdParam));
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);

	sCommand.Instruction = cmdParam->Instruction;
	sCommand.BusMode = cmdParam->BusMode;
	sCommand.CmdFormat = cmdParam->CmdFormat;
	sCommand.WrData = wrData;
	
	if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}

	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	while(QSPI_IsBusy(sCommand.BusMode));
	
	return QSPI_STATUS_OK;
}

uint8_t QSPI_EraseSector(QSPI_CommandTypeDef *cmdParam, uint32_t SectorAddress)
{
	MH_CommandTypeDef sCommand;
	SectorAddress &= (uint32_t)(0x00FFFFFF);
	
	assert_param(IS_QSPI_ADDR(SectorAddress));
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);

	if (cmdParam == NULL)
	{
		sCommand.Instruction = SECTOR_ERASE_CMD;       
		sCommand.BusMode = QSPI_BUSMODE_111;
		sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction;       
		sCommand.BusMode = cmdParam->BusMode;
		sCommand.CmdFormat = cmdParam->CmdFormat;
	}

	sCommand.Address = SectorAddress;
	
	if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}

	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	while (QSPI_IsBusy(sCommand.BusMode));

	return QSPI_STATUS_OK;
}

uint8_t Flash_BlockErase32K(QSPI_CommandTypeDef *cmdParam, uint32_t SectorAddress)
{
	MH_CommandTypeDef sCommand;
	SectorAddress &= (uint32_t)(0x00FFFFFF);
	
	assert_param(IS_QSPI_ADDR(SectorAddress));
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);

	if (cmdParam == NULL)
	{
		sCommand.Instruction = BLOCK_32K_ERASE_CMD;       
		sCommand.BusMode = QSPI_BUSMODE_111;
		sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction;       
		sCommand.BusMode = cmdParam->BusMode;
		sCommand.CmdFormat = cmdParam->CmdFormat;
	}

	sCommand.Address = SectorAddress;
	
	if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}

	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	while (QSPI_IsBusy(sCommand.BusMode));

	return QSPI_STATUS_OK;	
}

uint8_t Flash_BlockErase64K(QSPI_CommandTypeDef *cmdParam, uint32_t SectorAddress)
{
	MH_CommandTypeDef sCommand;
	SectorAddress &= (uint32_t)(0x00FFFFFF);
	
	assert_param(IS_QSPI_ADDR(SectorAddress));
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);

	if (cmdParam == NULL)
	{
		sCommand.Instruction = BLOCK_64K_ERASE_CMD;       
		sCommand.BusMode = QSPI_BUSMODE_111;
		sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction;       
		sCommand.BusMode = cmdParam->BusMode;
		sCommand.CmdFormat = cmdParam->CmdFormat;
	}

	sCommand.Address = SectorAddress;
	
	if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}

	if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	while (QSPI_IsBusy(sCommand.BusMode));

	return QSPI_STATUS_OK;	
}

uint8_t QSPI_EraseChip(QSPI_CommandTypeDef *cmdParam)
{
	MH_CommandTypeDef sCommand;
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);
	
	if (cmdParam == NULL)
	{
		sCommand.Instruction = CHIP_ERASE_CMD;       
		sCommand.BusMode = QSPI_BUSMODE_111;
		sCommand.CmdFormat = QSPI_CMDFORMAT_CMD8;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction;       
		sCommand.BusMode = cmdParam->BusMode;
		sCommand.CmdFormat 	= cmdParam->CmdFormat;
	}

	while (QSPI_IsBusy(sCommand.BusMode));
	
	if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	if ((MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT)) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}

	while (QSPI_IsBusy(sCommand.BusMode));
	
	return QSPI_STATUS_OK;
}

uint8_t QSPI_ProgramPage(QSPI_CommandTypeDef *cmdParam, DMA_TypeDef *DMA_Channelx, uint32_t adr, uint32_t sz, uint8_t *buf)
{
	uint32_t i;
	uint32_t end_addr, current_size, current_addr = 0,loop_addr;
	DMA_InitTypeDef DMA_InitStruct;
	MH_CommandTypeDef sCommand;
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);
#if 1
	if (DMA_Channelx == NULL)
	{
		return QSPI_ProgramPage_Ex(cmdParam, adr, sz, buf);
	}
#endif
	adr &= (uint32_t)(0x00FFFFFF);
	assert_param(IS_QSPI_ADDR(adr));
	assert_param(IS_QSPI_ADDR_ADD_SZ(adr, sz));
	
	while (current_addr <= adr)
	{
	   current_addr += X25Q_PAGE_SIZE;
	}
	current_size = current_addr - adr;

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > sz)
	{
	   current_size = sz;
	}

	/* Initialize the adress variables */
	current_addr = adr;
	loop_addr = adr;
	end_addr = adr + sz;
	//printf("cur_addr  %#x , end_addr  %#x  sz =%#x \r\n",current_addr,end_addr,current_size);

	if(cmdParam == NULL)
	{
		sCommand.Instruction	= QUAD_INPUT_PAGE_PROG_CMD;       
		sCommand.BusMode    	= QSPI_BUSMODE_114;
		sCommand.CmdFormat  	= QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;
	}
	else
	{
		sCommand.Instruction	= cmdParam->Instruction;//0x02;//QUAD_INPUT_PAGE_PROG_CMD;       
		sCommand.BusMode     	= cmdParam->BusMode;//0;//QSPI_BUSMODE_114;
		sCommand.CmdFormat  	= cmdParam->CmdFormat;//QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;
	}
	
	if (QSPI_DMA_Configuration(DMA_Channelx, &DMA_InitStruct) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}
	
	do
	{
		for(i = 0; i < (current_size/QSPI_DMA_WR_DATA_LEN_MAX); i++)
	    {
			sCommand.Address = current_addr;
			QSPI->BYTE_NUM = QSPI_DMA_WR_DATA_LEN_MAX << 16;
			QSPI->FIFO_CNTL |= QSPI_FIFO_CNTL_FLUSH_TX_FIFO;
			
			DMA_InitStruct.DMA_BlockSize = (QSPI_DMA_WR_DATA_LEN_MAX / 4);			
			DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)buf;
			DMA_Init(DMA_Channelx, &DMA_InitStruct);
			//Enable DMA
			DMA_ChannelCmd(DMA_Channelx, ENABLE);

			QSPI->DMA_CNTL |= BIT0;

			if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
			{
				return QSPI_STATUS_ERROR;
			}			
			
			if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
			{
				return QSPI_STATUS_ERROR;
			}
			
	        while (QSPI_IsBusy(sCommand.BusMode));	
			
			QSPI->DMA_CNTL &= ~BIT0;
			DMA_ChannelCmd(DMA_Channelx,DISABLE);

			buf += QSPI_DMA_WR_DATA_LEN_MAX;
			current_addr += QSPI_DMA_WR_DATA_LEN_MAX;
			sCommand.Address = current_addr;   
		}
		
		if (current_size % QSPI_DMA_WR_DATA_LEN_MAX > 0)
		{	
			QSPI->FIFO_CNTL |= QSPI_FIFO_CNTL_FLUSH_TX_FIFO;
			sCommand.Address = current_addr;
			QSPI->BYTE_NUM = (current_size % QSPI_DMA_WR_DATA_LEN_MAX) << 16;
			
			DMA_InitStruct.DMA_BlockSize = ((current_size + 3) % QSPI_DMA_WR_DATA_LEN_MAX) >> 2;
			DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)buf;
			DMA_Init(DMA_Channelx, &DMA_InitStruct);

			DMA_ChannelCmd(DMA_Channelx,ENABLE);

			QSPI->DMA_CNTL |= BIT0;
			if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
			{
				return QSPI_STATUS_ERROR;
			}
		        
			if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
			{
				return QSPI_STATUS_ERROR;
			}
			while(QSPI_IsBusy(sCommand.BusMode));

			QSPI->DMA_CNTL &= ~BIT0;
			DMA_ChannelCmd(DMA_Channelx, DISABLE);			
			buf += current_size % QSPI_DMA_WR_DATA_LEN_MAX;
		}  

       	loop_addr += current_size;
       	current_addr = loop_addr;
 
		current_size = ((loop_addr + X25Q_PAGE_SIZE) > end_addr) ? (end_addr - loop_addr) : X25Q_PAGE_SIZE;
	} while (loop_addr < end_addr);
	
	//disable DMA
	DMA_ChannelCmd(DMA_Channelx,DISABLE);
	
	return QSPI_STATUS_OK;
}

#if (QSPI_ENABLE_AES_CRYPT || QSPI_ENABLE_SM4_CRYPT) 
uint8_t QSPI_ProgramPageCipher(QSPI_CommandTypeDef *cmdParam, DMA_TypeDef *DMA_Channelx, uint32_t adr, uint32_t sz, uint8_t *buf, uint32_t encryptMode)
{
	uint32_t i;
	uint32_t end_addr, current_size, current_addr = 0,loop_addr;
	DMA_InitTypeDef DMA_InitStruct;
	
	uint8_t mplain[32] = {0};

	MH_CommandTypeDef sCommand;
	
	while (CACHE->CACHE_CS & CACHE_IS_BUSY);
	
	adr &= (uint32_t)(0x00FFFFFF);
	
	assert_param(IS_QSPI_ADDR(adr));
	assert_param(IS_QSPI_ADDR_ADD_SZ(adr, sz));
	assert_param(IS_QSPI_ENCRYPT_MODE(encryptMode));

	if(DMA_Channelx == NULL)
	{
		DMA_Channelx = DMA_Channel_0;
	}
	
	while (current_addr <= adr)
	{
	   current_addr += X25Q_PAGE_SIZE;
	}
	current_size = current_addr - adr;

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > sz)
	{
	   current_size = sz;
	}

	/* Initialize the adress variables */
	current_addr = adr;
	loop_addr = adr;
	end_addr = adr + sz;

	if (QSPI_ENCRYPT_MODE_AES == encryptMode)
	{
		buf_aes_enc((uint8_t*)(buf), sz, (uint8_t *)mplain);	
	}
	else
	{			
		buf_sm4_enc((uint8_t*)(buf), sz, (uint8_t *)mplain);			
	}
	
	if (QSPI_DMA_Configuration(DMA_Channelx, &DMA_InitStruct) != QSPI_STATUS_OK)
	{
		return QSPI_STATUS_ERROR;
	}

	if(cmdParam == NULL)
	{
		sCommand.Instruction = QUAD_INPUT_PAGE_PROG_CMD;       
		sCommand.BusMode	 = QSPI_BUSMODE_114;
		sCommand.CmdFormat   = QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;
	}
	else
	{
		sCommand.Instruction = cmdParam->Instruction;      
		sCommand.BusMode     = cmdParam->BusMode;
		sCommand.CmdFormat   = cmdParam->CmdFormat;
	}
	
	do
	{
		for(i = 0; i < (current_size/QSPI_DMA_WR_DATA_LEN_MAX); i++)
	    {
			sCommand.Address = current_addr;
			QSPI->BYTE_NUM = QSPI_DMA_WR_DATA_LEN_MAX << 16;
			QSPI->FIFO_CNTL |= BIT31;

			DMA_InitStruct.DMA_BlockSize = (QSPI_DMA_WR_DATA_LEN_MAX / 4);	
			DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)buf;
			DMA_Init(DMA_Channelx, &DMA_InitStruct);

			//Enable DMA
			DMA_ChannelCmd(DMA_Channelx,ENABLE);
			
			QSPI->DMA_CNTL |= BIT0;

			if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
	        {
				return QSPI_STATUS_ERROR;
	        }
	        	
			if (MH_QSPI_Command(&sCommand, MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
			{
				return QSPI_STATUS_ERROR;
			}
			while(QSPI_IsBusy(sCommand.BusMode));

			QSPI->DMA_CNTL &= ~BIT0;
			DMA_ChannelCmd(DMA_Channelx, DISABLE);

			buf += QSPI_DMA_WR_DATA_LEN_MAX;
			current_addr += QSPI_DMA_WR_DATA_LEN_MAX;
			sCommand.Address = current_addr;
			    
		}
		if (current_size % QSPI_DMA_WR_DATA_LEN_MAX > 0)
		{	
			uint8_t cnt_div, cnt_rem;
			cnt_rem = current_size % 32;
			cnt_div = current_size /32;
			
			if (cnt_div)
			{
				QSPI->FIFO_CNTL |= BIT31;
				sCommand.Address= current_addr;
				QSPI->BYTE_NUM = ((cnt_div * 32)) << 16;
					
				DMA_InitStruct.DMA_BlockSize = (cnt_div * 32);
				DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)buf;
				DMA_Init(DMA_Channelx, &DMA_InitStruct);

				DMA_ChannelCmd(DMA_Channelx,ENABLE);

				QSPI->DMA_CNTL |= BIT0;
				if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
				{
					return QSPI_STATUS_ERROR;
				}
			        
				if (MH_QSPI_Command(&sCommand,MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
				{
					return QSPI_STATUS_ERROR;
				}
				while(QSPI_IsBusy(sCommand.BusMode));

				QSPI->DMA_CNTL &= ~BIT0;
				DMA_ChannelCmd(DMA_Channelx, DISABLE);
				
				buf += cnt_div * 32;
				current_addr += (cnt_div * 32);
				QSPI->ADDRES = current_addr << 8;
			}
			
			if (cnt_rem)
			{
				QSPI->FIFO_CNTL |= QSPI_FIFO_CNTL_FLUSH_TX_FIFO;
				sCommand.Address= current_addr;
				QSPI->BYTE_NUM =  32 << 16;
					
				DMA_InitStruct.DMA_BlockSize = 32;
				DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)mplain;
				DMA_Init(DMA_Channelx, &DMA_InitStruct);

				DMA_ChannelCmd(DMA_Channelx,ENABLE);

				QSPI->DMA_CNTL |= BIT0;
				if (QSPI_WriteEnable(sCommand.BusMode) != QSPI_STATUS_OK)
				{
					return QSPI_STATUS_ERROR;
				}
			        
				if (MH_QSPI_Command(&sCommand,MH_QSPI_TIMEOUT_DEFAULT_CNT) != QSPI_STATUS_OK)
				{
					return QSPI_STATUS_ERROR;
				}
				while(QSPI_IsBusy(sCommand.BusMode));

				QSPI->DMA_CNTL &= ~BIT0;
				DMA_ChannelCmd(DMA_Channelx, DISABLE);				
			}
		}  

       	loop_addr += current_size;
       	current_addr = loop_addr;
 
		current_size = ((loop_addr + X25Q_PAGE_SIZE) > end_addr) ? (end_addr - loop_addr) : X25Q_PAGE_SIZE;
	} while (loop_addr < end_addr);
	
	//disable DMA
	DMA_ChannelCmd(DMA_Channelx, DISABLE);
	
	return QSPI_STATUS_OK;
}
#endif


void QSPI_Init(QSPI_InitTypeDef *mhqspi)
{
	if (mhqspi == NULL)
	{
		QSPI->DEVICE_PARA = ((QSPI->DEVICE_PARA & ~0xFF) | 0x6A) | QUADSPI_DEVICE_PARA_SAMPLE_PHA;
	}
	else
	{
		QSPI->DEVICE_PARA  = (uint32_t)((QSPI->DEVICE_PARA & ~0xFFFF)
							 | ((mhqspi->SampleDly & 0x01) << QSPI_DEVICE_PARA_SAMPLE_DLY_Pos)
		                     | ((mhqspi->SamplePha & 0x01) << QSPI_DEVICE_PARA_SAMPLE_PHA_Pos)
							 | ((mhqspi->ProToCol & 0x03) << QSPI_DEVICE_PARA_PROTOCOL_Pos)
					         | ((mhqspi->DummyCycles & 0x0F) << QSPI_DEVICE_PARA_DUMMY_CYCLE_Pos)
							 | ((mhqspi->FreqSel & 0x03)
							 | QUADSPI_DEVICE_PARA_FLASH_READY));
	}
}

void QSPI_SetLatency(uint32_t u32UsClk)
{
	if (0 == u32UsClk)
	{
		QSPI->DEVICE_PARA = (QSPI->DEVICE_PARA & 0xFFFF) | ((SYSCTRL->HCLK_1MS_VAL*2/1000) << 16);
	}
	else
	{
		QSPI->DEVICE_PARA = (QSPI->DEVICE_PARA & 0xFFFF) | (u32UsClk << 16);
	}
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
