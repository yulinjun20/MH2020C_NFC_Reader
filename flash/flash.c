#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mhscpu.h"
#include "mhscpu_it.h"
#include "mhscpu_qspi.h"
#include "user.h"
#include "flash.h"


#define FLASH_SIZE_BYTES       (1024*1024)

#define FLASH_START_ADDR 	    0x1000000U

#define FLASH_SECTOR_SIZE 	    (0x1000)
#define FLASH_SECTOR_NUM 	    (FLASH_SIZE_BYTES / FLASH_SECTOR_SIZE)
#define FLASH_PAGE_NUM 	        (FLASH_SECTOR_NUM * 16)         

#define FLASH_ARRAY_ADDR		0x1008000U


typedef enum
{
	DATA_TYPE_ALL_ZERO = 0x0,
	DATA_TYPE_ALL_ONE,
	DATA_TYPE_A5A5A5A5,
	DATA_TYPE_ADDRESS_SELF,
    DATA_TYPE_00TOFF,
}FLASH_TEST_DATA_TYPE;

//unsigned char mh_buffer[4096];


#define FLASH_SIZE                              (1UL << 20)

#define IS_FLASH_ADDRESS(ADDRESS) (((ADDRESS) > MHSCPU_FLASH_BASE - 1) && ((ADDRESS) < MHSCPU_FLASH_BASE + FLASH_SIZE))

int flash_init()
{
	return 0;
}


/**
  * @brief  Flash Erase Sector.
  * @param  cmdParam:      pointer to a QSPI_CommandTypeDef structure that contains the configuration information. 
  * @param  sectorAddress: The sector address to be erased
  * @retval FLASH Status:  The returned value can be: QSPI_STATUS_ERROR, QSPI_STATUS_OK
  */
#define ROM_QSPI_EraseSector    (*((uint8_t (*)(QSPI_CommandTypeDef *cmdParam, uint32_t sectorAddress))(*(uint32_t *)0x0024)))

/**
  * @brief  Flash Program Interface.
  * @param  cmdParam:      pointer to a QSPI_CommandTypeDef structure that contains the configuration information. 
  * @param  DMA_Channelx:  DMA_Channel_0
  * @param  addr:          specifies the address to be programmed.
  * @param  size:          specifies the size to be programmed.
  * @param  buffer:        pointer to the data to be programmed, need word aligned
  * @retval FLASH Status:  The returned value can be: QSPI_STATUS_ERROR, QSPI_STATUS_OK
  */
#define ROM_QSPI_ProgramPage    (*((uint8_t (*)(QSPI_CommandTypeDef *cmdParam, DMA_TypeDef *DMA_Channelx, uint32_t addr, uint32_t size, uint8_t *buffer))(*(uint32_t *)0x0028)))   


uint8_t  FLASH_EraseSector(uint32_t sectorAddress)
{
    uint8_t ret;
    
	__disable_irq();
	__disable_fault_irq();	

    ret = ROM_QSPI_EraseSector(NULL, sectorAddress);
    
	__enable_fault_irq();
	__enable_irq();

    return ret;
}

uint8_t FLASH_ProgramPage(uint32_t addr, uint32_t size, uint8_t *buffer)
{
    uint8_t ret;
	QSPI_CommandTypeDef cmdType;
    
    cmdType.Instruction = QUAD_INPUT_PAGE_PROG_CMD;
    cmdType.BusMode = QSPI_BUSMODE_114;	      
    cmdType.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;     
    
	__disable_irq();
	__disable_fault_irq();	

    ret = ROM_QSPI_ProgramPage(&cmdType, DMA_Channel_0, addr, size, buffer);
//	ret = ROM_QSPI_ProgramPage(&cmdType, NULL, addr, size, buffer);
    
	__enable_fault_irq();
	__enable_irq();

    return ret;    
}

static int EraseCheck(uint32_t addr, uint32_t pagNum)
{
	uint32_t i = 0;
	uint8_t erase_Buf[X25Q_PAGE_SIZE];

	memset(erase_Buf, 0xFF, X25Q_PAGE_SIZE);	
 
	CACHE_CleanAll(CACHE);
	for (i = 0; i < pagNum; i++)
	{	
		if (-1 == DataCheck(erase_Buf, (uint8_t *)(addr + i * X25Q_PAGE_SIZE), sizeof(erase_Buf)))
		{
			printf("Erase Check failed!\n");
            while(1);
			return -1;
		}
	}	
	
	return 0;
}

static int FlashTest(uint32_t pagNum, uint32_t dataType)
{
	uint32_t i = 0, j = 0;
	uint32_t write_Buf[X25Q_PAGE_SIZE/4] = {0};
    uint8_t data_buf_00toFF[X25Q_PAGE_SIZE] = {0};
	uint32_t program_addr = FLASH_START_ADDR;
	
    for (i = 0; i < X25Q_PAGE_SIZE; i++)
    {
        data_buf_00toFF[i] = i;
    }
	
	switch (dataType)
	{
		case DATA_TYPE_ALL_ONE:
			memset(write_Buf, 0xFF, sizeof(write_Buf));			
			break;
	
		case DATA_TYPE_A5A5A5A5:
			memset(write_Buf, 0xA5, sizeof(write_Buf));
			break;
		
        case DATA_TYPE_00TOFF:
			memset(write_Buf, 0x00, sizeof(write_Buf));
            memcpy(write_Buf, data_buf_00toFF, sizeof(data_buf_00toFF));
            break;
        
		case DATA_TYPE_ADDRESS_SELF:
			break;
			
		case DATA_TYPE_ALL_ZERO:
		default:			
			memset(write_Buf, 0x00, sizeof(write_Buf));
			break;
	}

	printf("Flash Programe data: \n");
	for (i = 0; i < pagNum; i++)
	{
		if (DATA_TYPE_ADDRESS_SELF == dataType)
		{
			for (j = 0; j < X25Q_PAGE_SIZE/4; j++)
			{
				write_Buf[j] = program_addr + i * X25Q_PAGE_SIZE + j;
			}
		}

		FLASH_ProgramPage(program_addr + i * X25Q_PAGE_SIZE, sizeof(write_Buf), (uint8_t*)(write_Buf));	
	}

	printf("Flash Read Data From Cache Forward Start\n");
	for (i = 0; i < pagNum; i++)
	{
		if (DATA_TYPE_ADDRESS_SELF == dataType)
		{
			for (j = 0; j < X25Q_PAGE_SIZE/4; j++)
			{
				write_Buf[j] = program_addr + i * X25Q_PAGE_SIZE + j;
			}
		}
		
		CACHE_CleanAll(CACHE);
		if (-1 == DataCheck(write_Buf, (uint8_t *)(program_addr + i * X25Q_PAGE_SIZE), sizeof(write_Buf)))
		{
			printf("Addr %#x data[Cache] check error!\n", program_addr + i * X25Q_PAGE_SIZE);
            while(1);
			return -1;
		}
	}	
	
	printf("Flash Read Data From Cache Reverse Start\n");
	for (i = 0; i < pagNum; i++)
	{
		if (DATA_TYPE_ADDRESS_SELF == dataType)
		{
			for (j = 0; j < X25Q_PAGE_SIZE/4; j++)
			{
				write_Buf[j] = program_addr + i * X25Q_PAGE_SIZE + j;
			}
		}

		CACHE_CleanAll(CACHE);
		if (-1 == DataCheckReverse(write_Buf, (uint8_t *)(program_addr + i * X25Q_PAGE_SIZE), sizeof(write_Buf)))
		{
			printf("Addr %#x data[Cache] check error!\n", program_addr + i * X25Q_PAGE_SIZE);
            while(1);
			return -1;
		}
	}		
	
	printf("Flash Read Data Check OK \n");	
	return 0;
}


void DataPrintf(void *buf, uint32_t bufsize)
{
	uint32_t i = 0;
	uint8_t *pBuf = (uint8_t *)buf;
	
	if (0 != bufsize)
	{
		for (i = 0; i < bufsize; i++)
		{
			if (0 != i && 0 == i%16)
			{
				printf(" \r\n");			
			}

			printf("%02X ", pBuf[i]);
		}
	}
	printf("\n");
}

static int DataCheck(void *src, void *dst, uint32_t size)
{
	uint8_t *pSrc = (uint8_t *)src, *pDst = (uint8_t *)dst;
	
	if (memcmp(pDst, pSrc, size))
	{
		DataPrintf(pDst, size);
		return -1;
	}

	return 0;
}

static int DataCheckReverse(void *src, void *dst, uint32_t size)
{
	uint32_t i;
	uint8_t *pSrc = (uint8_t *)src, *pDst = (uint8_t *)dst;

	for (i = size - 1; i > 0; i--)
	{
		if (pSrc[i] != pDst[i])
		{
			return -1;
		}
	}
	
	return 0;
}



void WordToBytes(WORD *pdwIn, int iLen, BYTE *pbyOut)
{
    int i;

    for (i=0; i<iLen; i++)
    {
        pbyOut[i*2  ] = (BYTE)(pdwIn[i]>>8);
        pbyOut[i*2+1] = (BYTE)(pdwIn[i]);
    }
}

void ByteToWord(BYTE *pbyIn, int iLen, WORD *pdwOut)
{
    int i;
    for (i=0; i<iLen; i+=2)
    {
        pdwOut[i/2] = pbyIn[i]*256+pbyIn[i+1];
   
    }
}
void ByteToDword(BYTE *pbyIn, uint32_t iLen, DWORD *pdwOut)
{
    uint32_t i;
    for (i=0; i<iLen; i+=4)
    {
        pdwOut[i/4] = (pbyIn[i]*256+pbyIn[i+1])*65536
            +pbyIn[i+2]*256+pbyIn[i+3];
    }
}

void DwordToBytes(DWORD *pdwIn, uint32_t iLen, BYTE *pbyOut)
{
    uint32_t i;

    for (i=0; i<iLen; i++)
    {
        pbyOut[i*4  ] = (BYTE)(pdwIn[i]>>24);
        pbyOut[i*4+1] = (BYTE)(pdwIn[i]>>16);
        pbyOut[i*4+2] = (BYTE)(pdwIn[i]>>8);
        pbyOut[i*4+3] = (BYTE)pdwIn[i];
    }
}
unsigned char toupper(unsigned char c)
{
	if((c >= 'a') && (c <= 'z'))
	c = c - ('a' - 'A');
	return c;
}
/*------------------------------------------------
         Two ASCII Bytes to One Hex Byte
 ------------------------------------------------*/
void vTwoOne(unsigned char *in, unsigned short in_len, unsigned char *out)
{
    unsigned char tmp;
    unsigned short i;

    for(i=0;i<in_len;i+=2)
    {
        tmp = in[i];
        if(tmp > '9')
            tmp = toupper(tmp) - ('A' - 0x0A);
        else
            tmp &= 0x0f;
        tmp <<= 4;
        out[i/2]=tmp;

        tmp=in[i+1];
        if(tmp>'9')
            tmp = toupper(tmp) - ('A' - 0x0A);
        else
            tmp &= 0x0f;
        out[i/2]+=tmp;
    }
}



/*--------------------------------------------
        One Hex Byte To Two Ascii Bytes
 ----------------------------------------------*/

void vOneTwo(unsigned char *in, unsigned short lc, unsigned char *out)
{
    unsigned char ucHexToChar[17];
    unsigned short nCounter;

    memcpy(ucHexToChar,"0123456789ABCDEF",16);
    for(nCounter = 0; nCounter < lc; nCounter++)
    {
        out[2*nCounter]   = ucHexToChar[(in[nCounter] >> 4)];
        out[2*nCounter+1] = ucHexToChar[(in[nCounter] & 0x0F)];
    }
    return;
}
int eraseFlashPage(int pageAddress)
{
		FLASH_EraseSector(MHSCPU_FLASH_BASE + pageAddress);  //FLASH_START_ADDR
		return 0;
}
int flash_erase(uint32_t page)
{
		FLASH_EraseSector(MHSCPU_FLASH_BASE + page*FLASH_SECTOR_SIZE);  //FLASH_START_ADDR
		return 0;
}
int flash_write( uint32_t addr, uint8_t *data, uint32_t size)
{
		FLASH_ProgramPage(MHSCPU_FLASH_BASE + addr, size, data);	 // FLASH_START_ADDR
		return 0;
}

int flash_read(uint8_t *data, uint32_t addr, uint32_t size )
{
               CACHE_CleanAll(CACHE);
		memcpy(data,(uint8_t *)(MHSCPU_FLASH_BASE + addr),size);  //FLASH_START_ADDR
		return 0;
}


int flash_write_witherase( uint32_t u32Addr, uint8_t *data, uint32_t len)
{
    uint32_t Page;
    uint32_t PageOffSet;
//    uint32_t i;
    uint32_t remain_len;
    uint32_t size;
    uint8_t *p;
    int iRet;
    unsigned char mh_buffer[4096];
    p = data;
    if (0 == len)
    {
        return 0;
    }
    
    assert_param((IS_FLASH_ADDRESS(FLASH_START_ADDR + u32Addr) && IS_FLASH_ADDRESS(FLASH_START_ADDR + u32Addr + len - 1)));
//  assert_param(0 == (u32Addr & 0x03));
    
   
    remain_len = len;
    size = 0;
    Page = u32Addr / FLASH_IAP_PAGE_SIZE;
    PageOffSet = u32Addr % FLASH_IAP_PAGE_SIZE;
 //   printf("page=%d,offset=%d\r\n", Page, PageOffSet);
    
    memset(mh_buffer, 0xff, sizeof(mh_buffer));
    flash_read(mh_buffer, Page * FLASH_IAP_PAGE_SIZE, FLASH_IAP_PAGE_SIZE);
    if(remain_len >= (FLASH_IAP_PAGE_SIZE - PageOffSet))
    {
        size = FLASH_IAP_PAGE_SIZE - PageOffSet;
    }
    else
    {
        size = remain_len;
    }
  //  printf("size=%d, remain_len=%d\r\n",size, remain_len);
    
    memcpy(&mh_buffer[PageOffSet], p, size); 
    
    iRet = flash_erase(Page);
    if(iRet)
    {
         return iRet;
    }
//    printf("mh_buffer:%02x %02x %02x %02x %02x\r\n", mh_buffer[0], mh_buffer[1], mh_buffer[2], mh_buffer[3], mh_buffer[4]);
    iRet = flash_write(Page * FLASH_IAP_PAGE_SIZE, mh_buffer, FLASH_IAP_PAGE_SIZE);
    if(iRet)
    {
         return iRet;
    }
 //   printf("aaaaaaa\r\n");
 //   return 0;
    remain_len = remain_len - size;
    p += size;

    Page++;
    PageOffSet = 0;
    while(remain_len >= FLASH_IAP_PAGE_SIZE)
    {
//    printf("000000000\r\n");
        memcpy(&mh_buffer[0], p, FLASH_IAP_PAGE_SIZE);
        iRet = flash_erase(Page);
        if(iRet)
        {
             return iRet;
        }
        iRet = flash_write(Page * FLASH_IAP_PAGE_SIZE, mh_buffer, FLASH_IAP_PAGE_SIZE);
        if(iRet)
        {
             return iRet;
        }
        remain_len -= FLASH_IAP_PAGE_SIZE; 
        p = p + FLASH_IAP_PAGE_SIZE;

        Page++;
        PageOffSet = 0;
    }
    
    if(remain_len)
    {
 //   printf("111111111111111\r\n");
        memset(mh_buffer, 0xff, sizeof(mh_buffer));
        flash_read(mh_buffer, Page * FLASH_IAP_PAGE_SIZE, FLASH_IAP_PAGE_SIZE);
        memcpy(&mh_buffer[0], p, remain_len);
        iRet = flash_erase(Page);
        if(iRet)
        {
           return iRet;
        }
        iRet = flash_write(Page * FLASH_IAP_PAGE_SIZE, mh_buffer, FLASH_IAP_PAGE_SIZE);
        if(iRet)
        {
           return iRet;
        }
    }

    
    return 0;
}


