/**
 ****************************************************************
 * @file main.c
 *
 * @brief  main entry of test.
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
#include "define.h"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>



#include "mh523.h"
#include "mifare.h"

#include "rfid.h"
#include "iso14443_4.h"
#include "iso14443a.h"
#include "user.h"

#define COM_BUF_SIZE 50
#define HEAD 0x68

extern tag_info XDATA g_tag_info;
extern u8 gucATQB[13];

static int g_picc_open_flag = 0;

typedef struct r_w_reg_s
{
	u8 addr;//дļĴַ
	u8 rw_dat;//дļĴֵ
}r_w_reg;


#if 0
typedef struct pc_tx_pkt_s
{
	u8 head;//ͷ
	u8 len; //ӰͷУ
	u8 cmd;	//

	union datu
	{
		r_w_reg reg;
		u8 dat[1];//ͻյ
	}datu;
}pc_tx_pkt;

typedef struct lpcd_config_s
{
	u8 delta;
	u32 t_inactivity_ms;
	u8 skip_times;
	u8 t_detect_us;
}lpcd_config_t;

lpcd_config_t XDATA g_lpcd_config;
static u8 XDATA com_tx_buf[ COM_BUF_SIZE ];//pc to stc
static u16 tx_buf_index = 0;
static U8 xdata Snr_RC500[4];


#if (POWERON_POLLING)
	bit g_query_mode = 1; //ֶоƬģʽ
	bit g_typa_poll = TRUE;
	bit g_typb_poll = TRUE;
	bit g_need_reconfig = TRUE;//Ҫ³ʼPCDЭ
#else
	bit g_query_mode = 0; //ֶоƬģʽ
	bit g_typa_poll = FALSE;
	bit g_typb_poll = FALSE;
	bit g_need_reconfig = FALSE;//Ҫ³ʼPCDЭ
#endif

u32 XDATA g_polling_cnt = 0;
bit g_lpcd_started = FALSE; //LPCDԶ̽⿨�?
bit g_lpcd_config_test_start = FALSE;//LPCDԶ̽⿨�?
pc_tx_pkt XDATA *recv_packet(void);
char prase_packet(pc_tx_pkt XDATA *ppkt);
void discard_pc_pkt(pc_tx_pkt XDATA *ppkt);




#endif

static int Dll_PiccCommand(APDU_SEND *ApduSend, APDU_RESP *ApduResp) 
{
	u8 XDATA loop_buf[512];
	char status;
	int i;
	//int tx_len;
	unsigned int rx_len;
	uint32_t lIndice		= 0;

       if(g_picc_open_flag == 0)
       {
       
#ifdef PICC_STANDARD_RETURN_VALUE_35XX
             return (-3502);
 #endif
       }
	
	if(0x00 != ApduSend->Lc)
	{
	//	tx_len = 4 /*Command*/ + 1 /*Lc*/+ApduSend->Lc  /*DataIn*/ + 1 /*Le*/;
	}
	else
	{
	//	tx_len = 4 /*Command*/ + 1 /*Le*/;
	}
		
	/*Command*/
	loop_buf[lIndice++] = ApduSend->Command[0];
	loop_buf[lIndice++] = ApduSend->Command[1];
	loop_buf[lIndice++] = ApduSend->Command[2];
	loop_buf[lIndice++] = ApduSend->Command[3];

	/*Lc*/
	if(0x00 != ApduSend->Lc)
	{
		loop_buf[lIndice++] = ApduSend->Lc;
		/*DataIn*/
		for(i = 0; i < ApduSend->Lc; i++)
		{
			loop_buf[lIndice++] = ApduSend->DataIn[i];
		}
	}
	/*Le*/
	//loop_buf[lIndice++] = ApduSend->Le;
	if(ApduSend->Le != 0)
    {
       
       if(ApduSend->Le == 256)
       {
            loop_buf[lIndice++] = 0;
       }
       else
       {
            loop_buf[lIndice++] = ApduSend->Le;
       }
    }
	
	status = ISO14443_4_HalfDuplexExchange(&g_pcd_module_info, loop_buf, lIndice, loop_buf, &rx_len);

	//printf("status:%d\r\n",status);
	
	if (status == MI_OK && rx_len < sizeof(loop_buf))
	{
		//for(i=0;i<rx_len;i++)
		//	printf("%02x ",loop_buf[i]);
		if(rx_len < 2)
		{
#ifdef PICC_STANDARD_RETURN_VALUE_35XX
                return (-3524);
#else
                return  (-1);
#endif
		}
		memcpy(ApduResp->DataOut,loop_buf,rx_len-2);
		ApduResp->LenOut = rx_len-2;
		ApduResp->SWA = loop_buf[rx_len-2];
		ApduResp->SWB = loop_buf[rx_len-1];

	}
	if (status != MI_OK)
	{
	
#ifdef PICC_STANDARD_RETURN_VALUE_35XX
	    return (-3524);
#endif
	}
	
	
	return status;//MI_OK;
}

int MH1608_Picc_Command(APDU_SEND *ApduSend, APDU_RESP *ApduResp) 
{
    return Dll_PiccCommand(ApduSend, ApduResp);
}

static int Dll_PiccApdu(unsigned char *tx_buf, unsigned int tx_len, unsigned char *rx_buf, unsigned int *rx_len)
{
    int status;
      if(g_picc_open_flag == 0)
       {
#ifdef PICC_STANDARD_RETURN_VALUE_35XX
             return (-3502);
#endif
       }
    status = ISO14443_4_HalfDuplexExchange(&g_pcd_module_info, tx_buf, tx_len, rx_buf, rx_len);
    if(status)
    {
    
#ifdef PICC_STANDARD_RETURN_VALUE_35XX
        return (-3524);
#endif
    }
    return status;
}

int MH1608_Picc_Apdu(unsigned char *tx_buf, unsigned int tx_len, unsigned char *rx_buf, unsigned int *rx_len)
{
    return Dll_PiccApdu(tx_buf, tx_len, rx_buf, rx_len);
}

void picc_reset()
{
	pcd_antenna_off();
	delay_ms(10);
	pcd_antenna_on();
	delay_ms(10);
}

static void Dll_PiccReset(void)
{
    picc_reset();
}

void MH1608_Picc_Reset(void)
{
    Dll_PiccReset();
}

static unsigned char ATS_Buf[20];

static int Dll_TypeA_GetATS(unsigned char *ATS)
{
    if(ATS_Buf[0] <= 20)
    {
        memcpy(ATS, ATS_Buf, ATS_Buf[0]);
    }
    else
    {
        return -2;
    }
    return 0;
}

int MH1608_TypeA_GetATS(unsigned char *ATS)
{
    return Dll_TypeA_GetATS(ATS);
}

static int Dll_PiccCheck(unsigned char  mode,unsigned char  *cardtype,unsigned char  *serialno)
{
	u8 cmd[2],tag_type[2],ats[20];
	int status = 1;
//	unsigned int nCount;

	int count=3;

       if(g_picc_open_flag == 0)
       {
#ifdef PICC_STANDARD_RETURN_VALUE_35XX
             return (-3502);
#endif
       }

	if(serialno)
	{
         memset(serialno, 0x00, sizeof(serialno));      
	}

        if( !((mode==0)||(mode=='A')||(mode=='a')||(mode=='M')||(mode=='m') ||(mode=='B')||(mode=='b')))
        {
#ifdef PICC_STANDARD_RETURN_VALUE_35XX
             return (-3503);
#endif
        }
    
	while(count--)
	{
		if(mode==0||mode=='A'||mode=='a'||mode=='M'||mode=='m')
		{
			cmd[1] = 0x52;
            
			pcd_config('A');
		//	printf("26=%02x 28=%02x 29=%02x 35=%02x 3b=%02x 31=%02x 17=%02x\r\n",read_reg(0x26), read_reg(0x28), read_reg(0x29),read_reg(0x35), read_reg(0x3b), read_reg(0x31), read_reg(0x17));
            //printf("11111\r\n");
			status = com_reqa(cmd);
            //printf("22222\r\n");
			if(status==MI_OK)
			{
				//HALT
				//status_a = pcd_hlta();
				
				serialno[0] = g_tag_info.uid_length;
				memcpy(serialno+1,g_tag_info.serial_num,g_tag_info.uid_length);
			//	nCount = g_tag_info.uid_length + 1;
			//	serialno[nCount] = 0;
				if(status==MI_OK)
				{
					//status_a = pcd_request(0x26, tag_type);
					//printf("pcd_request:%d\r\n",status_a);
					if(status==MI_OK){
						//RATS
						if((g_tag_info.sak&0x20)&&(mode!='M'&&mode!='m'))
						{
							status = pcd_rats_a(0,ats);
							//printf("pcd_rats_a:%d\r\n",status);
							if(status == 0) 
							{
								cardtype[0] = 'A';
								cardtype[1] = 'C';
                                
                                                          memset(ATS_Buf, 0, sizeof(ATS_Buf));
                                                          memcpy(ATS_Buf, ats, ats[0]);
							//	serialno[nCount++] = ats[0] + 1;
							//	memcpy(&serialno[nCount], ats, ats[0]);
							//	nCount += ats[0]; 
								return 0;
							}
						}
						cardtype[0] = 'A';
						cardtype[1] = 'M';

                                            return status;
					}
					//else pcd_hlta();
				}
				//else pcd_hlta();
			}
			else 
			{
				//printf("picc pcd reset\r\n");
				//pcd_reset();
				//delay_ms(15);
				picc_reset();
			}
		}
		if(mode==0||mode=='B'||mode=='b')
		{
//		    nCount = 0;
			pcd_config('B');
			cmd[1] = 0x08;
			status = com_reqb(cmd);
			
			if(status==MI_OK)
			{
				serialno[0] = g_tag_info.uid_length;
				memcpy(serialno+1,g_tag_info.serial_num,g_tag_info.uid_length);
			//	nCount = g_tag_info.uid_length + 1;
				cardtype[0] = 'B';
				cardtype[1] = 'C';
             //   serialno[nCount++] = 13;
			//	memcpy(&serialno[nCount], gucATQB, 13);
				return 0;
			}
			else 
			{
				iso14443_4_deselect(0);
			}
		}
        
	}

       if(status == MI_NOTAGERR)
       {
#ifdef PICC_STANDARD_RETURN_VALUE_35XX
              status = -3505;
#endif
       }
       else if(status == MI_COLLERR)
       {
#ifdef PICC_STANDARD_RETURN_VALUE_35XX
              status = -3526;
#endif
       }
	return status;
}

int MH1608_Picc_Check(unsigned char  mode,unsigned char  *cardtype,unsigned char  *serialno)
{
    return Dll_PiccCheck(mode, cardtype, serialno);
}

static int Dll_PiccHaltA(void)
{
     com_halt_a();
     return 0;
}

int MH1608_Picc_HaltA(void)
{
    return Dll_PiccHaltA();
}

static int Dll_PiccOpen()
{
  //  GPIO_SetBits(GPIOC, GPIO_Pin_3);//RF_EN--------HIGH
//	delay_ms(200);
	
//	pcd_init();//ʼpcdĴ
//	rfid_init();//ʼrfidر

       //s_PiccInit();

	pcd_antenna_off();
	delay_ms(6);//10
	
	pcd_antenna_on();
	delay_ms(6);//10

       g_picc_open_flag = 1;
	return 0;
}

int MH1608_Picc_Open()
{
     return Dll_PiccOpen();
}


//#define RF_EN_HIGH    GPIO_SetBits(GPIOC, GPIO_Pin_3)
//#define RF_EN_LOW     GPIO_ResetBits(GPIOC, GPIO_Pin_3)

static int Dll_PiccClose()
{
	pcd_antenna_off();
//	RF_EN_LOW;
	delay_ms(6);//10
       g_picc_open_flag = 0;
	return 0;
}

int MH1608_Picc_Close()
{
    return Dll_PiccClose();
}

static int Dll_PiccRemove(u8 isLOOP)
{
    if(isLOOP)
    {
        while(1)
        {
            picc_reset();
            pcd_config('A');
            if(PiccWupa(0) != MI_NOTAGERR)
               continue;            
            if(PiccWupa(0) != MI_NOTAGERR)
                continue;
            if(PiccWupa(0) != MI_NOTAGERR)
                continue;
           
            
            delay_ms(10);
            pcd_config('B');
            if(PiccWupb(0) != MI_NOTAGERR)
                continue;
            if(PiccWupb(0) != MI_NOTAGERR)
                continue;
            if(PiccWupb(0) != MI_NOTAGERR)
                continue;

            return 0;
        }         
     }
     else
     {
            while(1)
            {
                picc_reset();
                pcd_config('A');
                if(PiccWupa(0) != MI_NOTAGERR)
                  return 1;            
                if(PiccWupa(0) != MI_NOTAGERR)
                  return 1;
                if(PiccWupa(0) != MI_NOTAGERR)
                  return 1;


                delay_ms(10);
                pcd_config('B');
                if(PiccWupb(0) != MI_NOTAGERR)
                   return 1;
                if(PiccWupb(0) != MI_NOTAGERR)
                   return 1;
                if(PiccWupb(0) != MI_NOTAGERR)
                   return 1;

                return 0;
            } 
     }
    
	return 0;
}

int MH1608_Picc_Remove(u8 isLOOP)
{
     return Dll_PiccRemove(isLOOP);
}


static int Dll_MfcAuthenticate(uint8_t cType, uint16_t BlkNo, uint8_t *pKey, uint8_t *pSerialNo)
{
    uint8_t auth_mode;
    uint8_t Type;
    int  ret;
    
    auth_mode = 0;
    Type = cType;

    switch(Type)
    {
         case 'A':
         case 'a':
         case 0x0A:
         {
              auth_mode = 0x60;
         }
         break;
         case 'B':
         case 'b':
         case 0x0B:
         {
              auth_mode = 0x61;
         }
         break;
         default:
            break;
    }
    
    if(BlkNo > 63)
    {
         return USER_ERROR;
    }
    
    if((auth_mode != 0x60) && (auth_mode != 0x61))
    {
         return USER_ERROR;
    }

    ret = pcd_auth_state(0x60, BlkNo, pSerialNo, pKey);
    return ret;
    
}

int MH1608_Mfc_Authenticate(uint8_t cType, uint16_t BlkNo, uint8_t *pKey, uint8_t *pSerialNo)
{
    return Dll_MfcAuthenticate(cType,  BlkNo, pKey, pSerialNo);
}


static int Dll_MfcReadBlock(uint16_t BlkNo, uint8_t *BlkValue)
{
    int  status;
    int  ret;
    
    if(BlkNo > 63)
    {
         return USER_ERROR;
    }

    if(BlkValue == NULL)
    {
         return USER_ERROR;
    }

    ret = pcd_read(BlkNo, BlkValue);
    return ret;
}

int MH1608_Mfc_ReadBlock(uint16_t BlkNo, uint8_t *BlkValue)
{
    return Dll_MfcReadBlock(BlkNo, BlkValue);
}


static int Dll_MfcWriteBlock(uint16_t BlkNo, uint8_t *BlkValue)
{
    int    ret;

    if(BlkNo > 63)
    {
         return USER_ERROR;
    }
    if(BlkValue == NULL)
    {
         return USER_ERROR;
    }

    ret = pcd_write(BlkNo, BlkValue);
   
    return ret;
}

int MH1608_Mfc_WriteBlock(uint16_t BlkNo, uint8_t *BlkValue)
{
    return Dll_MfcWriteBlock(BlkNo, BlkValue);
}


static int Dll_MfcIncrement(uint8_t BlkNo,uint8_t *Value)
{
    int ret;

    if(BlkNo > 63)
    {
         return USER_ERROR;
    }

    if(Value == NULL)
    {
         return USER_ERROR;
    }
    ret = pcd_mf1_increment(BlkNo, Value);
    return ret;
}

int MH1608_Mfc_Increment(uint8_t BlkNo,uint8_t *Value)
{
    return Dll_MfcIncrement(BlkNo, Value);
}

static int Dll_MfcDecrement(uint8_t BlkNo,uint8_t *Value)
{
    int ret;
    if(BlkNo > 63)
    {
         return USER_ERROR;
    }

    if(Value == NULL)
    {
         return USER_ERROR;
    }

    ret = pcd_mf1_decrement(BlkNo, Value);
    return ret;
}

int MH1608_Mfc_Decrement(uint8_t BlkNo,uint8_t *Value)
{
    return Dll_MfcDecrement(BlkNo, Value);
}

static int Dll_MfcRestore(uint8_t BlkNo)
{
    int ret;
    if(BlkNo > 63)
    {
         return USER_ERROR;
    }

    ret = pcd_mf1_restore(BlkNo);
    return ret;
}

int MH1608_Mfc_Restore(uint8_t BlkNo)
{
    return Dll_MfcRestore(BlkNo);
}

static int Dll_MfcTransfer(uint8_t BlkNo)
{
    int ret;
    if(BlkNo > 63)
    {
         return USER_ERROR;
    }

    ret = pcd_mf1_transfer(BlkNo);
    return ret;
}

int MH1608_Mfc_Transfer(uint8_t BlkNo)
{
    return Dll_MfcTransfer(BlkNo);
}

static int Dll_MfcOperate(uint8_t mode,uint16_t BlkNo,uint8_t *Value,uint8_t UpdateBlkNo)
{
    int  ret;
    uint8_t bSrcBlockNo;
    uint8_t bDstBlockNo;
    uint8_t *pValue;

    if(BlkNo > 63)
    {
         return USER_ERROR;
    }

    bSrcBlockNo = BlkNo;
    bDstBlockNo = UpdateBlkNo;
    pValue = Value;
    
    if(mode==1 || mode=='+')
    {
        if(pValue == NULL)
        {
             return USER_ERROR;
        }
        ret = pcd_mf1_increment(bSrcBlockNo,pValue);
        if(ret == 0)
        {
           ret = pcd_mf1_transfer(bSrcBlockNo);
        }
    }
	else if(mode==2 || mode=='-')
    {
        if(pValue == NULL)
        {
             return USER_ERROR;
        }
        ret = pcd_mf1_decrement(bSrcBlockNo,pValue);
        if(ret == 0)
        {
           ret = pcd_mf1_transfer(bSrcBlockNo);
        }
    }   
	else if(mode == '=')
    {
        ret = pcd_mf1_restore(bSrcBlockNo);
        if(ret == 0)
        {
           ret = pcd_mf1_transfer(UpdateBlkNo);
        }
    } 
    else
    {
        return USER_ERROR;
    }

    return ret;
}

int MH1608_Mfc_Operate(uint8_t mode,uint16_t BlkNo,uint8_t *Value,uint8_t UpdateBlkNo)
{
      return Dll_MfcOperate(mode, BlkNo, Value, UpdateBlkNo);
}

unsigned int nCnt = 0;
void TestPicc()
{
	int check_ret;
	unsigned char cardtype[2],serial[50];
    unsigned char block;
    unsigned char AddrData;
    unsigned char buff[100];
    unsigned char Value[4];
    unsigned char Block_Value_Buff[16];
	APDU_SEND apdusend;
	APDU_RESP apduresp;
	int iRet,i;
    
	//s_PiccInit();

    AddrData = 56;
    Value[0] = 0x64;
    Value[1] = 0x00;
    Value[2] = 0x00;
    Value[3] = 0x00;
    
    Block_Value_Buff[0]  = (uint8_t)Value[0];
    Block_Value_Buff[1]  = (uint8_t)Value[1];
    Block_Value_Buff[2]  = (uint8_t)Value[2];
    Block_Value_Buff[3]  = (uint8_t)Value[3];
    Block_Value_Buff[4]  = (uint8_t)~(uint8_t)Value[0];
    Block_Value_Buff[5]  = (uint8_t)~(uint8_t)Value[1];
    Block_Value_Buff[6]  = (uint8_t)~(uint8_t)Value[2];
    Block_Value_Buff[7]  = (uint8_t)~(uint8_t)Value[3];
    Block_Value_Buff[8]  = (uint8_t)Value[0];
    Block_Value_Buff[9]  = (uint8_t)Value[1];
    Block_Value_Buff[10] = (uint8_t)Value[2];
    Block_Value_Buff[11] = (uint8_t)Value[3];
    Block_Value_Buff[12] = (uint8_t)AddrData;
    Block_Value_Buff[13] = (uint8_t)~(uint8_t)AddrData;
    Block_Value_Buff[14] = (uint8_t)AddrData;
    Block_Value_Buff[15] = (uint8_t)~(uint8_t)AddrData;

	apdusend.Command[0]=0x00;
	apdusend.Command[1]=0xa4;
	apdusend.Command[2]=0x04;
	apdusend.Command[3]=0x00;
	apdusend.Lc=0x0e;
	apdusend.Le=256;
	//memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);
	memcpy(apdusend.DataIn,"2PAY.SYS.DDF01",14);
	printf("Dll_PiccOpen\r\n"); 

	Dll_PiccOpen();
	
	printf("please put card\r\n");
	
	/* ===== (CRC test) ===== */
	/*
	write_reg(0x01, 0x00);              // IDLE,??????
	write_reg(0x0A, 0x80);              // ? FIFO
	write_reg(0x09, 0x12);              // ? FIFO ? 1 ??
	write_reg(0x01, 0x03);              // CalcCRC ??

	{
		u32 t = 0;
		while (!(read_reg(0x05) & 0x04))    // ? DivIrqReg bit2 (CRCIRq)
		{
			if (++t > 2000000) break;       // ????
		}
		printf("CRC test: t=%lu DivIrq=%02x\r\n", t, read_reg(0x05));
	}
	*/
	/* ================================== */
	
	while(1)
	{
		printf("Dll_PiccCheck...\r\n");
		memset(cardtype,0,sizeof(cardtype));
		memset(serial,0,sizeof(serial));
		check_ret = Dll_PiccCheck(0,cardtype,serial);
		if(check_ret == 0)
		{
			printf("dected %c %c card\r\n",cardtype[0], cardtype[1]);
			printf("sn:");
			for(i = 0; i < serial[0]; i++)
			{
			     printf("%02x ", serial[1+i]);
			}
			printf("\r\n");

			if(cardtype[0]=='B')
			{
			//	select_idcard();
			//	com_get_idcard_num();
			    if(nCnt % 2)
                {         
             //      iRet = iso14443_4_deselect(0);
             //      printf("iso14443_4_deselect iret:%d\r\n",iRet);
                 
                }
                nCnt++;
                memset(&apduresp,0,sizeof(apduresp));
				iRet = Dll_PiccCommand(&apdusend, &apduresp);
				printf("command iret:%d\r\n",iRet);
				if(iRet==0)
				{
					for(i=0;i<apduresp.LenOut;i++)
						printf("%02x ",apduresp.DataOut[i]);
					printf("\r\nSWA:%02x SWB:%02x\r\n",apduresp.SWA,apduresp.SWB);
				}
			}
			else if((cardtype[0]=='A') && (cardtype[1]=='C'))
			{
			    if(nCnt % 2)
                {         
             //      iRet = iso14443_4_deselect(0);
            //       printf("iso14443_4_deselect iret:%d\r\n",iRet);
                   
                }
                nCnt++;
                
				memset(&apduresp,0,sizeof(apduresp));
				iRet = Dll_PiccCommand(&apdusend, &apduresp);
				printf("command iret:%d\r\n",iRet);
				if(iRet==0)
				{
					for(i=0;i<apduresp.LenOut;i++)
						printf("%02x ",apduresp.DataOut[i]);
					printf("\r\nSWA:%02x SWB:%02x\r\n",apduresp.SWA,apduresp.SWB);
				}
			}
            else if((cardtype[0]=='A') && (cardtype[1]=='M'))
            {
                block = 56;
                iRet = Dll_MfcAuthenticate('A',block, "\xff\xff\xff\xff\xff\xff", &serial[1]);
                if(iRet == 0)
                {
                     printf("pcd_auth_state ok\r\n");
                     memset(buff,0,sizeof(buff));
                     iRet = Dll_MfcReadBlock(block, buff);
                     if(iRet == 0)
                     {
                         printf("----001---pcd_read block %d ok\r\n",block);
                         for(i = 0; i < 16; i++)
                         {
                               printf("%02x ", buff[i]);
                         }
                         printf("\r\n");

                         
                         for(i = 0; i < 16; i++)
                         {
                               buff[i] = ~buff[i];
                         }
                        // iRet = Dll_MfcWriteBlock(block, Block_Value_Buff);
                        Value[0] = 0x01;
                        Value[1] = 0x00;
                        Value[2] = 0x00;
                        Value[3] = 0x00;
                        iRet = Dll_MfcOperate('-', block, Value, block);
                         if(iRet == 0)
                         {
                                printf("Dll_MfcOperateblock %d success\r\n",block);
                                memset(buff,0,sizeof(buff));
                                iRet = Dll_MfcReadBlock(block, buff);
                                if(iRet == 0)
                                {
                                    printf("----002---pcd_read block %d ok\r\n",block);
                                    for(i = 0; i < 16; i++)
                                    {
                                        printf("%02x ", buff[i]);
                                    }
                                    printf("\r\n");
                                }
                         }
                     }
                }
            }
			break;
		}
		printf("check_ret is %d\r\n",check_ret);
	//	Lib_DelayMs(500);
	}

//	Dll_PiccRemove(1);
    
	
	Dll_PiccClose();
	Lib_DelayMs(1000);

}

void TestPicc1()
{

	tick XDATA g_statistic_last_t = 0;
	u8 XDATA status;
	u8 status_a = 1, status_b = 1;
	u8 cmd[2],tag_type[2];
	u8 cardtype[2],seral[10];
	int iret;
   	APDU_SEND apdusend;
	APDU_RESP apduresp;
	printf("111111...\r\n");
	apdusend.Command[0]=0x00;
	apdusend.Command[1]=0xa4;
	apdusend.Command[2]=0x04;
	apdusend.Command[3]=0x00;
	apdusend.Lc=0x0e;
	apdusend.Le=256;
	memcpy(apdusend.DataIn,"2PAY.SYS.DDF01",14);
    
    printf("s_PiccInit...\r\n");
	s_PiccInit();
    printf("s_PiccInit OK\r\n");
    
	Dll_PiccOpen();
    printf("Dll_PiccOpen \r\n");
    
	while(1)
	{

		iret = Dll_PiccCheck('A',cardtype,seral);
		printf("Lib_PiccCheck:%d\r\n",iret);
		if(iret == 0)
		{
			iret = Dll_PiccCommand(&apdusend, &apduresp);
			printf("command iret:%d\r\n",iret);
		}
	}

#if 0
	while(1)
	{
		
		ppkt = recv_packet();
		if (ppkt)
		{
			//Ƿǵ
			if (FALSE == prase_packet(ppkt))
			{
				//RFID
				rfid_operation(&ppkt->cmd);
			}
			discard_pc_pkt(ppkt);
		}

		//ԶѯѰģʽ
		if (g_query_mode && (g_typa_poll || g_typb_poll))
		{
			u8 cmd[2];
			u8 status_a = 1, status_b = 1;

			pcd_antenna_off();
			delay_ms(10);
			pcd_antenna_on();
			delay_ms(10);
			
			if (g_typa_poll == TRUE)
			{
				if (g_typa_poll & g_typb_poll)
				{
					pcd_config('A');
				}
				cmd[1] = 0x52;

				//pcd_default_info();
					
				status_a = com_reqa(&cmd);//ѯA				
			}
		
			if (g_typb_poll == TRUE)
			{	
				if (g_typa_poll & g_typb_poll)
				{
					pcd_config('B');
				}			
				cmd[1] = 0x08;
				status_b = com_reqb(&cmd);//ѯB
			}
		
			//Ӧ
			if (status_a == MI_OK || status_b == MI_OK)
			{
				//led_success_on();//ɹ
			}
			else
			{
				//led_fail_on();//ʧ
			}
		
		}

		//cosָѯ
		if (g_cos_loop == TRUE)
		{
			if (g_cos_loop_times > 0)
			{
				com_exchange(g_loop_buf);
				g_cos_loop_times--;
				delay_ms(20);
			}
			else
			{
				delay_ms(200);
				g_statistic_refreshed = TRUE;
				statistic_print();
				make_packet(COM_PKT_CMD_TEST_STOP, NULL, NULL);
			}
		}
		
		//Զ
		if (g_lpcd_started == TRUE)
		{
			if (TRUE == pcd_lpcd_check())
			{
				g_lpcd_started = FALSE;
				make_packet(COM_PKT_CMD_LPCD, NULL, NULL);
			}
		}
		//Զ
		if (g_lpcd_config_test_start == TRUE)
		{
			if (TRUE == pcd_lpcd_check())
			{
				u8 XDATA tag_type[2];
					
				g_statistics.lpcd_cnt++;
				g_statistic_refreshed = TRUE;
				statistic_print();
				
				//֤Ƿп�?
				pcd_config('A');
				status = pcd_request(0x52, tag_type);
				//ռ⵽ʱܴھԵпܵһѰʧܣӵڶѰ�?
				if (status != MI_OK)
				{					
					status = pcd_request(0x52, tag_type);
				}
				///ռ⵽ʱܴھԵǰѰʧܣѰ�?
				if (status != MI_OK)
				{					
					status = pcd_request(0x52, tag_type);
				}
				
				if (status == MI_OK)
				{//пƬ�?
					//ӾӦùܴ
					/*



					*/
					//ȴ�?,Ӧôпɲصȴ�?
					while(1)
					{
						pcd_antenna_off();
						delay_ms(10);
						pcd_antenna_on();
						delay_ms(10);	
						status = pcd_request(0x52, tag_type);
						if(status != MI_OK)
						{//һ֤�?
							delay_ms(100);
							status = pcd_request(0x52, tag_type);
							if(status != MI_OK)
							{//֤�?
								delay_ms(100);
								status = pcd_request(0x52, tag_type);
								if(status != MI_OK)
								{//֤�?
									delay_ms(100);
									pcd_lpcd_config_start(g_lpcd_config.delta, g_lpcd_config.t_inactivity_ms, g_lpcd_config.skip_times, g_lpcd_config.t_detect_us);	
									break;	
								}
							}
	
						}
					}
				}
				else
				{
					g_statistics.lpcd_fail++;
					g_statistic_refreshed = TRUE;
					pcd_lpcd_config_start(g_lpcd_config.delta, g_lpcd_config.t_inactivity_ms, g_lpcd_config.skip_times, g_lpcd_config.t_detect_us);	
				}
			}
		}
		//ͳϢ
		if (is_timeout(g_statistic_last_t, 100))
		{
			g_statistic_last_t = get_tick();
			statistic_print();
		}
	}	
#endif
}


/* NFC Type2 Tag NDEF read (static layout), aligned with PN5190 Dll_NfcReadTag */
#define NFC_T2T_NDEF_MAGIC          0xE1U
#define NFC_T2T_NDEF_SUPPORTED_VNO  0x10U
#define NFC_T2T_CC_PAGE             3U
#define NFC_T2T_DATA_START_PAGE     4U
#define NFC_T2T_BYTES_PER_PAGE      4U
#define NFC_T2T_NULL_TLV            0x00U
#define NFC_T2T_LOCK_CTRL_TLV       0x01U
#define NFC_T2T_MEM_CTRL_TLV        0x02U
#define NFC_T2T_NDEF_TLV            0x03U
#define NFC_T2T_PROPRIETARY_TLV     0xFDU
#define NFC_T2T_TERMINATOR_TLV      0xFEU
#define NFC_NDEF_MAX_LEN            1024U

/* Read one absolute byte from Type2 tag memory using pcd_read cache */
static int t2t_read_byte(unsigned int abs_off, unsigned char *out,
                        unsigned char *cache_page, unsigned char *cache_buf, int *cache_valid)
{
    unsigned char page;
    unsigned char page_base;
    int status;

    page = (unsigned char)(abs_off / NFC_T2T_BYTES_PER_PAGE);
    page_base = (unsigned char)(page & 0xFCU); /* align to 4-page READ window */

    if(!(*cache_valid) || (*cache_page != page_base))
    {
        status = pcd_read(page_base, cache_buf);
        if(status != MI_OK)
        {
            *cache_valid = 0;
            return status;
        }
        *cache_page = page_base;
        *cache_valid = 1;
    }

    *out = cache_buf[abs_off - ((unsigned int)page_base * NFC_T2T_BYTES_PER_PAGE)];
    return MI_OK;
}

/*
 * Dll_NfcReadTag
 * Read NDEF message from currently activated NFC Type2 Tag (static layout).
 * Return: 0 success, -100 no NDEF, -101 empty NDEF, other negative on I/O error.
 */
int Dll_NfcReadTag(unsigned char *buf, uint16_t *rlen)
{
    unsigned char cc[16];
    unsigned char cache_buf[16];
    unsigned char cache_page = 0;
    int cache_valid = 0;
    unsigned char tms;
    unsigned char rwa;
    unsigned int mem_bytes;
    unsigned int abs_off;
    unsigned int end_off;
    unsigned char tlv;
    unsigned int tlv_len;
    unsigned int i;
    unsigned char b;
    int status;

    if((buf == 0) || (rlen == 0))
    {
        return -1;
    }
    *rlen = 0;

    printf("Dll_NfcReadTag (Type2 static)...\r\n");

    /* Read CC at page 3 (also returns pages 3..6) */
    status = pcd_read(NFC_T2T_CC_PAGE, cc);
    if(status != MI_OK)
    {
        printf("pcd_read CC failed: %d\r\n", status);
        return status;
    }

    if(cc[0] != NFC_T2T_NDEF_MAGIC)
    {
        printf("No NDEF magic (CC0=%02x)\r\n", cc[0]);
        return (-100);
    }

    if((unsigned char)(cc[1] & 0xF0U) > (unsigned char)(NFC_T2T_NDEF_SUPPORTED_VNO & 0xF0U))
    {
        printf("Unsupported CC version %02x\r\n", cc[1]);
        return (-100);
    }

    tms = cc[2];
    rwa = cc[3];
    mem_bytes = ((unsigned int)tms) * 8U; /* user memory size in bytes */
    if(mem_bytes < 48U)
    {
        printf("Misconfigured TMS=%02x\r\n", tms);
        return (-100);
    }
    /* Static layout: only use first-sector style scan; cap by CC size */
    end_off = 16U + mem_bytes; /* byte offset after UID/CC area */
    if(end_off > (16U + 48U))
    {
        /* Still allow larger TMS for NTAG-like cards with contiguous TLV from page 4,
         * but do not implement dynamic lock skipping. */
    }
    if((rwa != 0x00U) && (rwa != 0x0FU))
    {
        printf("Unsupported RWA=%02x\r\n", rwa);
        return (-100);
    }

    /* Seed cache with the CC read (pages 3..6) */
    memcpy(cache_buf, cc, 16);
    cache_page = NFC_T2T_CC_PAGE;
    cache_valid = 1;

    abs_off = (unsigned int)NFC_T2T_DATA_START_PAGE * NFC_T2T_BYTES_PER_PAGE;

    while(abs_off < end_off)
    {
        status = t2t_read_byte(abs_off, &tlv, &cache_page, cache_buf, &cache_valid);
        if(status != MI_OK)
        {
            return status;
        }

        if(tlv == NFC_T2T_NULL_TLV)
        {
            abs_off++;
            continue;
        }
        if(tlv == NFC_T2T_TERMINATOR_TLV)
        {
            printf("Terminator TLV before NDEF\r\n");
            return (-100);
        }

        /* Length field */
        status = t2t_read_byte(abs_off + 1U, &b, &cache_page, cache_buf, &cache_valid);
        if(status != MI_OK)
        {
            return status;
        }

        if(b == 0xFFU)
        {
            unsigned char b1, b2;
            status = t2t_read_byte(abs_off + 2U, &b1, &cache_page, cache_buf, &cache_valid);
            if(status != MI_OK) return status;
            status = t2t_read_byte(abs_off + 3U, &b2, &cache_page, cache_buf, &cache_valid);
            if(status != MI_OK) return status;
            tlv_len = (((unsigned int)b1) << 8) | b2;
            abs_off += 4U; /* T + 0xFF + L1 + L2 */
        }
        else
        {
            tlv_len = b;
            abs_off += 2U; /* T + L */
        }

        if(tlv == NFC_T2T_NDEF_TLV)
        {
            if(tlv_len == 0U)
            {
                printf("NDEF content is NULL\r\n");
                return (-101);
            }
            if(tlv_len > NFC_NDEF_MAX_LEN)
            {
                printf("NDEF too large: %u\r\n", tlv_len);
                return (-1);
            }
            if((abs_off + tlv_len) > end_off)
            {
                printf("NDEF exceeds tag memory\r\n");
                return (-100);
            }

            for(i = 0; i < tlv_len; i++)
            {
                status = t2t_read_byte(abs_off + i, &buf[i], &cache_page, cache_buf, &cache_valid);
                if(status != MI_OK)
                {
                    return status;
                }
            }
            *rlen = (uint16_t)tlv_len;
            printf("NDEF len=%u\r\n", tlv_len);
            return 0;
        }

        /* Skip Lock/Memory/Proprietary TLV value */
        abs_off += tlv_len;
    }

    printf("No NDEF content detected\r\n");
    return (-100);
}

/* Write contiguous bytes to Type2 tag memory (4-byte page RMW via ultralight write) */
static int t2t_write_bytes(unsigned int abs_off, const unsigned char *data, unsigned int len)
{
    unsigned int done = 0;
    unsigned char page_buf[16];
    unsigned char wr[4];
    unsigned char page;
    unsigned char page_base;
    unsigned char off_in_page;
    unsigned int k;
    int status;

    if((len == 0U) || (data == 0))
    {
        return MI_OK;
    }

    while(done < len)
    {
        page = (unsigned char)((abs_off + done) / NFC_T2T_BYTES_PER_PAGE);
        page_base = (unsigned char)(page & 0xFCU);
        off_in_page = (unsigned char)((abs_off + done) % NFC_T2T_BYTES_PER_PAGE);

        status = pcd_read(page_base, page_buf);
        if(status != MI_OK)
        {
            return status;
        }

        memcpy(wr, &page_buf[(unsigned int)(page - page_base) * NFC_T2T_BYTES_PER_PAGE],
               NFC_T2T_BYTES_PER_PAGE);

        for(k = (unsigned int)off_in_page; (k < NFC_T2T_BYTES_PER_PAGE) && (done < len); k++)
        {
            wr[k] = data[done];
            done++;
        }

        status = pcd_write_ultralight(page, wr);
        if(status != MI_OK)
        {
            return status;
        }
    }

    return MI_OK;
}

/*
 * Dll_NfcWriteTag
 * Write raw NDEF message bytes into an existing Type2 NDEF TLV (overwrite only).
 * Does not format a blank tag / create CC or NDEF area.
 * text     : raw NDEF message bytes (not plain text wrapper)
 * text_len : NDEF message length in bytes (0 clears to empty NDEF TLV)
 * Return: 0 success, -100 no writable NDEF area / too large, other negative on I/O error.
 */
int Dll_NfcWriteTag(const unsigned char *text, uint16_t text_len)
{
    unsigned char cc[16];
    unsigned char cache_buf[16];
    unsigned char cache_page = 0;
    int cache_valid = 0;
    unsigned char tms;
    unsigned char rwa;
    unsigned int mem_bytes;
    unsigned int abs_off;
    unsigned int end_off;
    unsigned char tlv;
    unsigned int tlv_len;
    unsigned char b;
    unsigned int ndef_hdr_off;
    unsigned int hdr_size;
    unsigned int val_off;
    unsigned int max_ndef;
    unsigned char hdr[4];
    unsigned char term;
    int status;
    int found;

    if((text_len > 0U) && (text == 0))
    {
        return -1;
    }
    if(text_len > NFC_NDEF_MAX_LEN)
    {
        return -1;
    }

    printf("Dll_NfcWriteTag (Type2 static), len=%u\r\n", (unsigned int)text_len);

    status = pcd_read(NFC_T2T_CC_PAGE, cc);
    if(status != MI_OK)
    {
        printf("pcd_read CC failed: %d\r\n", status);
        return status;
    }

    if(cc[0] != NFC_T2T_NDEF_MAGIC)
    {
        printf("No NDEF magic (CC0=%02x)\r\n", cc[0]);
        return (-100);
    }

    if((unsigned char)(cc[1] & 0xF0U) > (unsigned char)(NFC_T2T_NDEF_SUPPORTED_VNO & 0xF0U))
    {
        printf("Unsupported CC version %02x\r\n", cc[1]);
        return (-100);
    }

    tms = cc[2];
    rwa = cc[3];
    mem_bytes = ((unsigned int)tms) * 8U;
    if(mem_bytes < 48U)
    {
        printf("Misconfigured TMS=%02x\r\n", tms);
        return (-100);
    }
    end_off = 16U + mem_bytes;

    /* Read/write access only; read-only CC is not supported for write */
    if(rwa != 0x00U)
    {
        printf("Tag not writable RWA=%02x\r\n", rwa);
        return (-100);
    }

    memcpy(cache_buf, cc, 16);
    cache_page = NFC_T2T_CC_PAGE;
    cache_valid = 1;

    abs_off = (unsigned int)NFC_T2T_DATA_START_PAGE * NFC_T2T_BYTES_PER_PAGE;
    found = 0;
    ndef_hdr_off = 0;

    while(abs_off < end_off)
    {
        status = t2t_read_byte(abs_off, &tlv, &cache_page, cache_buf, &cache_valid);
        if(status != MI_OK)
        {
            return status;
        }

        if(tlv == NFC_T2T_NULL_TLV)
        {
            abs_off++;
            continue;
        }
        if(tlv == NFC_T2T_TERMINATOR_TLV)
        {
            printf("Terminator TLV before NDEF\r\n");
            return (-100);
        }

        status = t2t_read_byte(abs_off + 1U, &b, &cache_page, cache_buf, &cache_valid);
        if(status != MI_OK)
        {
            return status;
        }

        if(b == 0xFFU)
        {
            unsigned char b1, b2;
            status = t2t_read_byte(abs_off + 2U, &b1, &cache_page, cache_buf, &cache_valid);
            if(status != MI_OK) return status;
            status = t2t_read_byte(abs_off + 3U, &b2, &cache_page, cache_buf, &cache_valid);
            if(status != MI_OK) return status;
            tlv_len = (((unsigned int)b1) << 8) | b2;
            if(tlv == NFC_T2T_NDEF_TLV)
            {
                ndef_hdr_off = abs_off;
                found = 1;
                break;
            }
            abs_off += 4U + tlv_len;
        }
        else
        {
            tlv_len = b;
            if(tlv == NFC_T2T_NDEF_TLV)
            {
                ndef_hdr_off = abs_off;
                found = 1;
                break;
            }
            abs_off += 2U + tlv_len;
        }
    }

    if(!found)
    {
        printf("No existing NDEF TLV to overwrite\r\n");
        return (-100);
    }

    /* Choose TLV length encoding from new payload size */
    if(text_len > 0xFEU)
    {
        hdr_size = 4U;
        hdr[0] = NFC_T2T_NDEF_TLV;
        hdr[1] = 0xFFU;
        hdr[2] = (unsigned char)(text_len >> 8);
        hdr[3] = (unsigned char)(text_len & 0xFFU);
    }
    else
    {
        hdr_size = 2U;
        hdr[0] = NFC_T2T_NDEF_TLV;
        hdr[1] = (unsigned char)text_len;
    }

    val_off = ndef_hdr_off + hdr_size;
    if(val_off >= end_off)
    {
        printf("NDEF header exceeds tag memory\r\n");
        return (-100);
    }
    max_ndef = end_off - val_off;
    if((unsigned int)text_len > max_ndef)
    {
        printf("NDEF too large for tag: %u > %u\r\n", (unsigned int)text_len, max_ndef);
        return (-100);
    }

    /* Step1: set length to 0 (same header size) so partial write is not readable as NDEF */
    if(hdr_size == 4U)
    {
        hdr[2] = 0;
        hdr[3] = 0;
    }
    else
    {
        hdr[1] = 0;
    }
    status = t2t_write_bytes(ndef_hdr_off, hdr, hdr_size);
    if(status != MI_OK)
    {
        printf("write NDEF hdr(len0) failed: %d\r\n", status);
        return status;
    }

    /* Step2: write NDEF payload */
    if(text_len > 0U)
    {
        status = t2t_write_bytes(val_off, text, (unsigned int)text_len);
        if(status != MI_OK)
        {
            printf("write NDEF payload failed: %d\r\n", status);
            return status;
        }
    }

    /* Step3: terminator TLV if at least one free byte remains */
    if(((unsigned int)text_len + 1U) <= max_ndef)
    {
        term = NFC_T2T_TERMINATOR_TLV;
        status = t2t_write_bytes(val_off + (unsigned int)text_len, &term, 1U);
        if(status != MI_OK)
        {
            printf("write terminator failed: %d\r\n", status);
            return status;
        }
    }

    /* Step4: commit real NDEF length */
    if(hdr_size == 4U)
    {
        hdr[0] = NFC_T2T_NDEF_TLV;
        hdr[1] = 0xFFU;
        hdr[2] = (unsigned char)(text_len >> 8);
        hdr[3] = (unsigned char)(text_len & 0xFFU);
    }
    else
    {
        hdr[0] = NFC_T2T_NDEF_TLV;
        hdr[1] = (unsigned char)text_len;
    }
    status = t2t_write_bytes(ndef_hdr_off, hdr, hdr_size);
    if(status != MI_OK)
    {
        printf("write NDEF hdr(final) failed: %d\r\n", status);
        return status;
    }

    printf("Dll_NfcWriteTag OK, NDEF len=%u\r\n", (unsigned int)text_len);
    return 0;
}

/* URI identifier codes (NFC Forum RTD-URI) */
static const char * const g_ndef_uri_prefix[] = {
    "",
    "http://www.",
    "https://www.",
    "http://",
    "https://",
    "tel:",
    "mailto:",
    "ftp://anonymous:anonymous@",
    "ftp://ftp.",
    "ftps://",
    "sftp://",
    "smb://",
    "nfs://",
    "ftp://",
    "dav://",
    "news:",
    "telnet://",
    "imap:",
    "rtsp://",
    "urn:",
    "pop:",
    "sip:",
    "sips:",
    "tftp:",
    "btspp://",
    "btl2cap://",
    "btgoep://",
    "tcpobex://",
    "irdaobex://",
    "file://",
    "urn:epc:id:",
    "urn:epc:tag:",
    "urn:epc:pat:",
    "urn:epc:raw:",
    "urn:epc:",
    "urn:nfc:"
};
#define NFC_NDEF_URI_PREFIX_COUNT  (sizeof(g_ndef_uri_prefix)/sizeof(g_ndef_uri_prefix[0]))

static int ndef_append_str(char *out, uint16_t out_max, uint16_t *used, const char *s)
{
    unsigned int i;
    if(s == 0) return 0;
    for(i = 0; s[i] != 0; i++)
    {
        if((*used + 1U) >= out_max)
        {
            return -1;
        }
        out[*used] = s[i];
        (*used)++;
    }
    out[*used] = 0;
    return 0;
}

static int ndef_append_bytes(char *out, uint16_t out_max, uint16_t *used,
                           const unsigned char *p, unsigned int len)
{
    unsigned int i;
    for(i = 0; i < len; i++)
    {
        if((*used + 1U) >= out_max)
        {
            return -1;
        }
        out[*used] = (char)p[i];
        (*used)++;
    }
    out[*used] = 0;
    return 0;
}

/*
 * Dll_NfcParseNdefToText
 * Parse NDEF message into Text / URL string (Well-Known Type "T" / "U").
 * out receives a NUL-terminated string; multiple records are separated by "; ".
 * Return: 0 if at least one Text/URI parsed, -1 invalid args, -2 no Text/URI found.
 */
int Dll_NfcParseNdefToText(unsigned char *ndef, uint16_t ndef_len,
                          char *out, uint16_t out_max, uint16_t *out_len)
{
    unsigned int off;
    unsigned int parsed;
    uint16_t used;

    if((ndef == 0) || (out == 0) || (out_max < 2U))
    {
        return -1;
    }
    out[0] = 0;
    used = 0;
    if(out_len) *out_len = 0;
    if(ndef_len == 0)
    {
        return -2;
    }

    parsed = 0;
    off = 0;
    while(off < ndef_len)
    {
        unsigned char hdr;
        unsigned char tnf;
        unsigned char sr;
        unsigned char il;
        unsigned char type_len;
        unsigned int payload_len;
        unsigned char id_len;
        unsigned int type_off;
        unsigned int payload_off;
        unsigned int rec_end;

        hdr = ndef[off++];
        tnf = (unsigned char)(hdr & 0x07U);
        sr  = (unsigned char)((hdr >> 4) & 0x01U);
        il  = (unsigned char)((hdr >> 3) & 0x01U);

        if(off >= ndef_len) return -2;
        type_len = ndef[off++];

        if(sr)
        {
            if(off >= ndef_len) return -2;
            payload_len = ndef[off++];
        }
        else
        {
            if((off + 4U) > ndef_len) return -2;
            payload_len = ((unsigned int)ndef[off] << 24) |
                          ((unsigned int)ndef[off+1] << 16) |
                          ((unsigned int)ndef[off+2] << 8) |
                          ((unsigned int)ndef[off+3]);
            off += 4U;
        }

        id_len = 0;
        if(il)
        {
            if(off >= ndef_len) return -2;
            id_len = ndef[off++];
        }

        if((off + type_len) > ndef_len) return -2;
        type_off = off;
        off += type_len;

        if((off + id_len) > ndef_len) return -2;
        off += id_len;

        if((off + payload_len) > ndef_len) return -2;
        payload_off = off;
        rec_end = off + payload_len;

        /* Well-known type Text: TNF=0x01, Type='T' */
        if((tnf == 0x01U) && (type_len == 1U) && (ndef[type_off] == 'T') && (payload_len >= 1U))
        {
            unsigned char status_b;
            unsigned char lang_len;
            unsigned int text_off;
            unsigned int text_len;

            status_b = ndef[payload_off];
            lang_len = (unsigned char)(status_b & 0x3FU);
            /* UTF-16 not converted here; only copy UTF-8 / raw bytes */
            if((1U + lang_len) > payload_len)
            {
                off = rec_end;
                if(hdr & 0x40U) break; /* ME */
                continue;
            }
            text_off = payload_off + 1U + lang_len;
            text_len = payload_len - 1U - lang_len;

            if(parsed > 0)
            {
                if(ndef_append_str(out, out_max, &used, "; ") != 0) break;
            }
            if(ndef_append_bytes(out, out_max, &used, &ndef[text_off], text_len) != 0) break;
            parsed++;
        }
        /* Well-known type URI: TNF=0x01, Type='U' */
        else if((tnf == 0x01U) && (type_len == 1U) && (ndef[type_off] == 'U') && (payload_len >= 1U))
        {
            unsigned char id_code;
            const char *prefix;

            id_code = ndef[payload_off];
            prefix = "";
            if(id_code < NFC_NDEF_URI_PREFIX_COUNT)
            {
                prefix = g_ndef_uri_prefix[id_code];
            }

            if(parsed > 0)
            {
                if(ndef_append_str(out, out_max, &used, "; ") != 0) break;
            }
            if(ndef_append_str(out, out_max, &used, prefix) != 0) break;
            if(ndef_append_bytes(out, out_max, &used, &ndef[payload_off + 1U], payload_len - 1U) != 0) break;
            parsed++;
        }

        off = rec_end;
        if(hdr & 0x40U) /* ME */
        {
            break;
        }
    }

    if(out_len) *out_len = used;
    if(parsed == 0)
    {
        return -2;
    }
    return 0;
}

int Dll_NfcWriteTextToTag(const unsigned char *text, uint16_t text_len)
{
    unsigned char ndef_buf[NFC_NDEF_MAX_LEN];
    unsigned int payload_len;
    unsigned int ndef_len;
    unsigned int i;

    if((text_len > 0U) && (text == 0))
    {
        return -1;
    }

    /* NDEF Text payload: status(1) + lang("en",2) + text */
    payload_len = 1U + 2U + (unsigned int)text_len;

    printf("Dll_NfcWriteTextToTag len=%u\r\n", (unsigned int)text_len);

    if(payload_len <= 0xFFU)
    {
        /* Short record: MB|ME|SR|TNF=Well-Known */
        ndef_len = 4U + payload_len;
        if(ndef_len > NFC_NDEF_MAX_LEN)
        {
            return -1;
        }
        ndef_buf[0] = 0xD1U;
        ndef_buf[1] = 0x01U;
        ndef_buf[2] = (unsigned char)payload_len;
        ndef_buf[3] = (unsigned char)'T';
        ndef_buf[4] = 0x02U; /* UTF-8, lang length = 2 */
        ndef_buf[5] = (unsigned char)'e';
        ndef_buf[6] = (unsigned char)'n';
        for(i = 0; i < (unsigned int)text_len; i++)
        {
            ndef_buf[7U + i] = text[i];
        }
    }
    else
    {
        /* Long record: MB|ME|TNF=Well-Known (no SR) */
        ndef_len = 7U + payload_len;
        if(ndef_len > NFC_NDEF_MAX_LEN)
        {
            return -1;
        }
        ndef_buf[0] = 0xC1U;
        ndef_buf[1] = 0x01U;
        ndef_buf[2] = (unsigned char)((payload_len >> 24) & 0xFFU);
        ndef_buf[3] = (unsigned char)((payload_len >> 16) & 0xFFU);
        ndef_buf[4] = (unsigned char)((payload_len >> 8) & 0xFFU);
        ndef_buf[5] = (unsigned char)(payload_len & 0xFFU);
        ndef_buf[6] = (unsigned char)'T';
        ndef_buf[7] = 0x02U;
        ndef_buf[8] = (unsigned char)'e';
        ndef_buf[9] = (unsigned char)'n';
        for(i = 0; i < (unsigned int)text_len; i++)
        {
            ndef_buf[10U + i] = text[i];
        }
    }

    return Dll_NfcWriteTag(ndef_buf, (uint16_t)ndef_len);
}

/*
 * Dll_NfcReadTextFromTag
 * Read tag NDEF then parse to Text/URL string via Dll_NfcParseNdefToText.
 * buf      : output NUL-terminated string
 * buf_max  : buffer size
 * out_len  : optional text length (excluding NUL)
 * Return: 0 success, same error codes as ReadTag / ParseNdefToText.
 */
int Dll_NfcReadTextFromTag(char *buf, uint16_t buf_max, uint16_t *out_len)
{
    unsigned char ndef_buf[NFC_NDEF_MAX_LEN];
    uint16_t ndef_len;
    int ret;

    if((buf == 0) || (buf_max < 2U))
    {
        return -1;
    }

    printf("Dll_NfcReadTextFromTag...\r\n");
    ndef_len = 0;
    ret = Dll_NfcReadTag(ndef_buf, &ndef_len);
    if(ret != 0)
    {
        if(out_len) *out_len = 0;
        buf[0] = 0;
        return ret;
    }

    return Dll_NfcParseNdefToText(ndef_buf, ndef_len, buf, buf_max, out_len);
}

void TestNfcType2TagRead(void)
{
	int check_ret;
	unsigned char cardtype[2],serial[50];
	unsigned char buff[NFC_NDEF_MAX_LEN];
	uint16_t ndef_len;
	int iRet,i;
	unsigned char ch;

	

	Dll_PiccOpen();

	printf("please put card\r\n");

	while(1)
	{
		printf("Dll_PiccCheck...\r\n");
		memset(cardtype,0,sizeof(cardtype));
		memset(serial,0,sizeof(serial));
		check_ret = Dll_PiccCheck(0,cardtype,serial);
		if(check_ret == 0)
		{
			printf("dected %c card\r\n",cardtype[0]);
			printf("sn:");
			for(i = 0; i < serial[0]; i++)
			{
			     printf("%02x ", serial[1+i]);
			}
			printf("\r\n");
			printf("ATQA: %02x %02x\r\n", g_tag_info.tag_type_bytes[0], g_tag_info.tag_type_bytes[1]);
			printf("SAK: %02x\r\n", g_tag_info.sak);

			ndef_len = 0;
			memset(buff, 0, sizeof(buff));
			iRet = Dll_NfcReadTag(buff, &ndef_len);
			printf("Dll_NfcReadTag iRet=%d, len=%u\r\n", iRet, ndef_len);
			if(iRet == 0)
			{
				char text_buf[NFC_NDEF_MAX_LEN];
				uint16_t text_len;
				int parse_ret;

				printf("NDEF content:\r\n");
				for(i = 0; i < ndef_len; i++)
				{
					printf("%02x ", buff[i]);
					if(((i + 1) % 16) == 0)
					{
						printf("\r\n");
					}
				}
				if((ndef_len % 16) != 0)
				{
					printf("\r\n");
				}

				text_len = 0;
				memset(text_buf, 0, sizeof(text_buf));
				parse_ret = Dll_NfcParseNdefToText(buff, ndef_len, text_buf, (uint16_t)sizeof(text_buf), &text_len);
				printf("Dll_NfcParseNdefToText ret=%d, len=%u\r\n", parse_ret, text_len);
				if(parse_ret == 0)
				{
					printf("NDEF text/URL: %s\r\n", text_buf);
				}
				else
				{
					printf("NDEF text/URL parse failed\r\n");
				}
			}

			break;
		}
		printf("check_ret is %d\r\n",check_ret);
	//	Lib_DelayMs(500);
	}

//	Dll_PiccRemove(1);

	Dll_PiccClose();
	Lib_DelayMs(1000);
}

void TestNfcType2TagWrite(void)
{
	int check_ret;
	unsigned char cardtype[2],serial[50];
	unsigned char buff[NFC_NDEF_MAX_LEN];
	uint16_t ndef_len;
	int iRet,i;
	unsigned char ch;

	Dll_PiccOpen();

	printf("please put card\r\n");

	while(1)
	{
		printf("Dll_PiccCheck...\r\n");
		memset(cardtype,0,sizeof(cardtype));
		memset(serial,0,sizeof(serial));
		check_ret = Dll_PiccCheck(0,cardtype,serial);
		if(check_ret == 0)
		{
			printf("dected %c card\r\n",cardtype[0]);
			printf("sn:");
			for(i = 0; i < serial[0]; i++)
			{
			     printf("%02x ", serial[1+i]);
			}
			printf("\r\n");
			printf("ATQA: %02x %02x\r\n", g_tag_info.tag_type_bytes[0], g_tag_info.tag_type_bytes[1]);
			printf("SAK: %02x\r\n", g_tag_info.sak);

			iRet = Dll_NfcWriteTextToTag("MN:123456789\r\nID:K002\r\n", 23);
			printf("Dll_NfcWriteTextToTag: ret=%d\r\n", iRet);

			break;
		}
		else
		{
			printf("check_ret is %d\r\n",check_ret);
			Lib_DelayMs(500);
		}
	}

//	Dll_PiccRemove(1);

	Dll_PiccClose();
	Lib_DelayMs(1000);
}

void NFC_Tag2_Test(void)
{
	unsigned char ch;
	int ret;

	printf("\r\n=======================\r\n");
	printf("1.Read  NDEF data\r\n");
	printf("2.Write NDEF data\r\n");
	printf("\r\n=======================\r\n");
	ch = s_get_key();
	switch(ch)
	{
		case '1':
		{
			TestNfcType2TagRead();
		}
		break;
		case '2':
		{
			TestNfcType2TagWrite();
		}
		break;
		
	}

}

