#include <stdio.h>
#include "mhscpu.h"
#include "spi.h"
#include "mhscpu_spi.h"

static SPI_TypeDef *NFC_SPI = SPIM0;

void spi_config(void)
{
    /// Mode 0, Polling
	SPI_InitTypeDef SPI_InitStructure;

    GPIO_PinRemapConfig(GPIOB, GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5, GPIO_Remap_0);   

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;

    SPI_InitStructure.SPI_NSS = SPI_NSS_0;
    
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;//SPI_BaudRatePrescaler_64;//SPI_BaudRatePrescaler_32;//SPI_BaudRatePrescaler_16;
	SPI_InitStructure.SPI_RXFIFOFullThreshold = SPI_RXFIFOFullThreshold_1;
	SPI_InitStructure.SPI_TXFIFOEmptyThreshold = SPI_TXFIFOEmptyThreshold_0;
    
	SPI_Init(NFC_SPI, &SPI_InitStructure);    
	SPI_Cmd(NFC_SPI, ENABLE);

}

uint8_t spi_write_byte(uint8_t write_data)
{ 
    uint8_t state = 0;
    uint16_t u16_time_out_counter = 4000;
    
    SPI_SendData(NFC_SPI, write_data);
    
    while((RESET == SPI_GetFlagStatus(NFC_SPI, SPI_FLAG_RXNE)) && u16_time_out_counter)
    {
        u16_time_out_counter--;
    }
    
    if (!u16_time_out_counter)
    {
        state = 1;
    }
    
    SPI_ReceiveData(NFC_SPI);
	
    return state;
}

uint8_t spi_read_byte(void)
{ 
    uint8_t read_byte = 0;
    uint16_t u16_time_out_counter = 4000;
    
    SPI_SendData(NFC_SPI, 0xFF);
    
    while((RESET == SPI_GetFlagStatus(NFC_SPI, SPI_FLAG_RXNE)) && u16_time_out_counter)
    {
        u16_time_out_counter--;
    }
    
    if (!u16_time_out_counter)
    {
        printf("spi_read_byte failed!\r\n");
    }
    
    read_byte = SPI_ReceiveData(NFC_SPI);
    
    return read_byte;
}

int spi_write(unsigned char *data, unsigned int length)
{
	unsigned int							size;
	volatile unsigned char					*ptr = (unsigned char*)data;
    
    uint8_t state = 0;
    uint16_t u16_time_out_counter = 4000;
    size = length;
    while(size)
    {
        SPI_SendData(NFC_SPI, *ptr);
        
        while((RESET == SPI_GetFlagStatus(NFC_SPI, SPI_FLAG_RXNE)) && u16_time_out_counter)
        {
            u16_time_out_counter--;
        }
        
        if (!u16_time_out_counter)
        {
            state = 1;
            break;
        }
        
       SPI_ReceiveData(NFC_SPI);
       ptr++;
	size--;
   }
	
    return state;
}

int spi_read(unsigned char *data, unsigned int length)
{
	unsigned int							size;
	volatile unsigned char					*ptr = (unsigned char*)data;
       uint8_t state = 0;
       uint16_t u16_time_out_counter = 4000;
    
	size = length;
	while(size)
	{
            	  SPI_SendData(NFC_SPI, 0xFF);
                
                while((RESET == SPI_GetFlagStatus(NFC_SPI, SPI_FLAG_RXNE)) && u16_time_out_counter)
                {
                    u16_time_out_counter--;
                }
                
                if (!u16_time_out_counter)
                {
                    printf("spi_read_byte failed!\r\n");
                }
                
               *ptr = SPI_ReceiveData(NFC_SPI);
		//*ptr = spi_read_byte();
		ptr++;
		size--;
	}
	return 0;
}

/**
 ****************************************************************
 * @brief write_reg() 
 *
 * 写芯片的寄存器
 *
 * @param:  addr 寄存器地址
 ****************************************************************
 */
void write_reg(uint8_t addr, uint8_t val)
{
	uint8_t c;

	//最低位空闲，有效数据域为bit1~bit6
	addr <<= 1;
	
	//地址最高位为1代表读，为0代表写；
	c = addr & ~(READ_REG_CTRL);

	SPI_CS_LOW();						
	spi_write_byte(c);
	spi_write_byte(val);
	SPI_CS_HIGH();
}

/**
 ****************************************************************
 * @brief read_reg() 
 *
 * 读芯片的寄存器
 *
 * @param: addr 寄存器地址
 * @return: c 寄存器的值
 ****************************************************************
 */
uint8_t read_reg(uint8_t addr)
{
	uint8_t c;
	
	//最低位空闲，有效数据域为bit1~bit6
	addr <<= 1;
	
	//地址最高位为1代表读，为0代表写；
	c = addr | READ_REG_CTRL;

	SPI_CS_LOW();	
	c = spi_write_byte(c);	
       if(c!=0)
       {
            return c;
       }
	c = spi_read_byte();	
	SPI_CS_HIGH();	

	return c;
}

