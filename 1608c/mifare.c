/**
 ****************************************************************
 * @file mifare.c
 *
 * @brief  mifare protocol driver
 *
 * @author 
 *
 * 
 ****************************************************************
 */ 

/*
 * INCLUDE FILES
 ****************************************************************
 */	
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "uart.h"
#include "spi.h"
//#include "timer.h"
#include "mh523.h"
#include "mifare.h" 


/**
 ****************************************************************
 * @brief pcd_auth_state() 
 *
 * 功能：用存放在FIFO中的密钥和卡上的密钥进行验证
 *
 * @param: auth_mode=验证方式,0x60:验证A密钥,0x61:验证B密钥
 * @param: block=要验证的绝对块号
 * @param: psnr=序列号首地址
 * @return: status 值为MI_OK:成功
 *
 ****************************************************************
 */
int pcd_auth_state(u8 auth_mode, u8 block, u8 *psnr, u8 *pkey)
{
	int status;
	u8 i;
	
	transceive_buffer XDATA *pi;
	pi = &mf_com_data;

#if (NFC_DEBUG)
	printf("AUTH:\n");
#endif
	write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
	
	
	pcd_set_tmo(4);

	mf_com_data.mf_command = PCD_AUTHENT;
	mf_com_data.mf_length = 12;
	mf_com_data.mf_data[0] = auth_mode;
	mf_com_data.mf_data[1] = block;
	for (i = 0; i < 6; i++)
	{
		mf_com_data.mf_data[2+i] = pkey[i];
	}
	memcpy(&mf_com_data.mf_data[8], psnr, 4);

	status = pcd_com_transceive(pi);
	
	if (MI_OK == status)
	{
		if (read_reg(Status2Reg) & BIT3) //MFCrypto1On
		{
			status = MI_OK;
		}else
		{
			status = MI_AUTHERR;
		}
	}
	
	return status;

}



/**
 ****************************************************************
 * @brief pcd_read() 
 *
 * 功能：读mifare_one卡上一块(block)数据(16字节)
 *
 * @param: addr = 要读的绝对块号
 * @param: preaddata = 存放读出的数据缓存区的首地址
 * @return: status 值为MI_OK:成功
 * @retval: preaddata  读出的数据
 *
 ****************************************************************
 */
int pcd_read(u8 addr,u8 *preaddata)
{
    int status;
    
    transceive_buffer XDATA *pi;
    pi = &mf_com_data;
	
#if (NFC_DEBUG)
	printf("READ:\n");
#endif

	write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
	set_bit_mask(TxModeReg, BIT7); //使能发送crc
	set_bit_mask(RxModeReg, BIT7); //使能接收crc
	pcd_set_tmo(4);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = PICC_READ;
    mf_com_data.mf_data[1] = addr;

    status = pcd_com_transceive(pi);
    if (status == MI_OK)
    {
        if (mf_com_data.mf_length != 0x80)
        {
			status = MI_BITCOUNTERR;
		}
        else
        {
			memcpy(preaddata, &mf_com_data.mf_data[0], 16);
		}
    }
	
    return status;
}


/**
 ****************************************************************
 * @brief pcd_write() 
 *
 * 功能：写数据到卡上的一块
 *
 * @param: addr = 要写的绝对块号
 * @param: pwritedata = 存放写的数据缓存区的首地址
 * @return: status 值为MI_OK:成功
 *
 ****************************************************************
 */
int pcd_write(u8 addr,u8 *pwritedata)
{
    int status;
    
    transceive_buffer XDATA *pi;
    pi = &mf_com_data;
	
#if (NFC_DEBUG)
	printf("WRITE:\n");
#endif

    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
	set_bit_mask(TxModeReg, BIT7); //使能发送crc
	clear_bit_mask(RxModeReg, BIT7); //不使能接收crc
	
	pcd_set_tmo(5);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = PICC_WRITE;
    mf_com_data.mf_data[1] = addr;

    status = pcd_com_transceive(pi);
    if (status != MI_NOTAGERR)
    {
        if(mf_com_data.mf_length != 4)
        {
			status=MI_BITCOUNTERR;
		}
        else
        {
           mf_com_data.mf_data[0] &= 0x0F;
           switch (mf_com_data.mf_data[0])
           {
              case 0x00:
                 status = MI_NOTAUTHERR;
                 break;
              case 0x0A:
                 status = MI_OK;
                 break;
              default:
                 status = MI_CODEERR;
                 break;
           }
        }
    }
    if (status == MI_OK)
    {	
       	pcd_set_tmo(5);

        mf_com_data.mf_command = PCD_TRANSCEIVE;
        mf_com_data.mf_length  = 16;
        memcpy(&mf_com_data.mf_data[0], pwritedata, 16);
        
        status = pcd_com_transceive(pi);
        if (status != MI_NOTAGERR)
        {
            mf_com_data.mf_data[0] &= 0x0F;
            switch(mf_com_data.mf_data[0])
            {
               case 0x00:
                  status = MI_WRITEERR;
                  break;
               case 0x0A:
                  status = MI_OK;
                  break;
               default:
                  status = MI_CODEERR;
                  break;
           }
        }
        pcd_set_tmo(4);
    }
    return status;
}

int pcd_mf1_transfer(u8 addr)
{
    int status;
    
    transceive_buffer XDATA *pi;
    pi = &mf_com_data;
    
    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
	set_bit_mask(TxModeReg, BIT7); //ê1?ü·￠?ícrc
	clear_bit_mask(RxModeReg, BIT7); //2?ê1?ü?óê?crc

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = PICC_TRANSFER;
    mf_com_data.mf_data[1] = addr;

    status = pcd_com_transceive(pi);
    if (status != MI_NOTAGERR)
    {
        if(mf_com_data.mf_length != 4)
        {
			status=MI_BITCOUNTERR;
		}
        else
        {
           mf_com_data.mf_data[0] &= 0x0F;
           switch (mf_com_data.mf_data[0])
           {
              case 0x00:
                 status = MI_NOTAUTHERR;
                 break;
              case 0x0A:
                 status = MI_OK;
                 break;
              default:
                 status = MI_CODEERR;
                 break;
           }
        }
  
    }

    return status;
}

int pcd_mf1_increment(u8 addr, u8 *pVaue)
{
    int status;
    
    transceive_buffer XDATA *pi;
    pi = &mf_com_data;
    
    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
	set_bit_mask(TxModeReg, BIT7); //ê1?ü·￠?ícrc
	clear_bit_mask(RxModeReg, BIT7); //2?ê1?ü?óê?crc
	
	pcd_set_tmo(5);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = PICC_INCREMENT;
    mf_com_data.mf_data[1] = addr;

    status = pcd_com_transceive(pi);
    if (status != MI_NOTAGERR)
    {
 // lite_printf("---001---%d\r\n",mf_com_data.mf_length);
        if(mf_com_data.mf_length != 4)
        {
			status=MI_BITCOUNTERR;
		}
        else
        {
           mf_com_data.mf_data[0] &= 0x0F;
           switch (mf_com_data.mf_data[0])
           {
              case 0x00:
                 status = MI_NOTAUTHERR;
                 break;
              case 0x0A:
                 status = MI_OK;
                 break;
              default:
                 status = MI_CODEERR;
                 break;
           }
        }
    }
//lite_printf("---002---%d\r\n",mf_com_data.mf_length);

    if (status == MI_OK)
    {	
       	pcd_set_tmo(5);

        mf_com_data.mf_command = PCD_TRANSCEIVE;
        mf_com_data.mf_length  = 4;
        memcpy(&mf_com_data.mf_data[0], pVaue, 4);
        
        status = pcd_com_transceive(pi);
        
        if(status == MI_NOTAGERR)
        {
            return 0;
        }
        else if(status == MI_OK)
        {
            return MI_CODEERR;
        }
    }
    return status;
}

int pcd_mf1_decrement(u8 addr, u8 *pVaue)
{
    int status;
    
    transceive_buffer XDATA *pi;
    pi = &mf_com_data;
    
    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
	set_bit_mask(TxModeReg, BIT7); //ê1?ü·￠?ícrc
	clear_bit_mask(RxModeReg, BIT7); //2?ê1?ü?óê?crc
	
	pcd_set_tmo(5);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = PICC_DECREMENT;
    mf_com_data.mf_data[1] = addr;

    status = pcd_com_transceive(pi);
    if (status != MI_NOTAGERR)
    {
        if(mf_com_data.mf_length != 4)
        {
			status=MI_BITCOUNTERR;
		}
        else
        {
           mf_com_data.mf_data[0] &= 0x0F;
           switch (mf_com_data.mf_data[0])
           {
              case 0x00:
                 status = MI_NOTAUTHERR;
                 break;
              case 0x0A:
                 status = MI_OK;
                 break;
              default:
                 status = MI_CODEERR;
                 break;
           }
        }
    }
    if (status == MI_OK)
    {	
       	pcd_set_tmo(5);

        mf_com_data.mf_command = PCD_TRANSCEIVE;
        mf_com_data.mf_length  = 4;
        memcpy(&mf_com_data.mf_data[0], pVaue, 4);
        
        status = pcd_com_transceive(pi);
       
        if(status == MI_NOTAGERR)
        {
            return 0;
        }
        else if(status == MI_OK)
        {
            return MI_CODEERR;
        }
    }
    return status;
}

int pcd_mf1_restore(u8 addr)
{
    int status;
    
    transceive_buffer XDATA *pi;
    pi = &mf_com_data;
    
    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
	set_bit_mask(TxModeReg, BIT7); //ê1?ü·￠?ícrc
	clear_bit_mask(RxModeReg, BIT7); //2?ê1?ü?óê?crc
	
	pcd_set_tmo(5);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = PICC_RESTORE;
    mf_com_data.mf_data[1] = addr;

    status = pcd_com_transceive(pi);
    if (status != MI_NOTAGERR)
    {
        if(mf_com_data.mf_length != 4)
        {
			status=MI_BITCOUNTERR;
		}
        else
        {
           mf_com_data.mf_data[0] &= 0x0F;
           switch (mf_com_data.mf_data[0])
           {
              case 0x00:
                 status = MI_NOTAUTHERR;
                 break;
              case 0x0A:
                 status = MI_OK;
                 break;
              default:
                 status = MI_CODEERR;
                 break;
           }
        }
    }
    if (status == MI_OK)
    {	
       	pcd_set_tmo(5);

        mf_com_data.mf_command = PCD_TRANSCEIVE;
        mf_com_data.mf_length  = 4;
        memcpy(&mf_com_data.mf_data[0], "\x00\x00\x00\x00", 4);
        
        status = pcd_com_transceive(pi);
       
        if(status == MI_NOTAGERR)
        {
            return 0;
        }
        else if(status == MI_OK)
        {
            return MI_CODEERR;
        }
       
    }
    return status;
}


/**
 ****************************************************************
 * @brief pcd_write_ultralight() 
 *
 * 功能：写数据到卡上的一块
 *
 * @param: addr = 要写的绝对块号
 * @param: pwritedata = 存放写的数据缓存区的首地址
 * @return: status 值为MI_OK:成功
 *
 ****************************************************************
 */
int pcd_write_ultralight(u8 addr,u8 *pwritedata)
{
    int status;
    
    transceive_buffer XDATA *pi;
    pi = &mf_com_data;
	
#if (NFC_DEBUG)
	printf("WRITE_UL:\n");
#endif

    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
	set_bit_mask(TxModeReg, BIT7); //使能发送crc
	clear_bit_mask(RxModeReg, BIT7); //不使能接收crc
	pcd_set_tmo(5);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 6; //a2h ADR D0 D1 D2 D3
    mf_com_data.mf_data[0] = PICC_WRITE_ULTRALIGHT;
    mf_com_data.mf_data[1] = addr;
	memcpy(&mf_com_data.mf_data[2], pwritedata, 4); // 4 字节数据

    status = pcd_com_transceive(pi);
    
    if (status != MI_NOTAGERR)
    {
        mf_com_data.mf_data[0] &= 0x0F;
        switch(mf_com_data.mf_data[0])
        {
           case 0x00:
              status = MI_WRITEERR;
              break;
           case 0x0A:
              status = MI_OK;
              break;
           default:
              status = MI_CODEERR;
              break;
       }
    }
	pcd_set_tmo(4);

    return status;
}

