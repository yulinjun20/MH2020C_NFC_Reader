#include <stdio.h>
#include <string.h>
#include "mhscpu.h"

#include "commandpro.h"

#include "user.h"
#include "ntag.h"
#include "ntag_test.h"
#include "nfc_type2.h"

unsigned char cmdBuf[CMD_MAX_BUFLEN];


volatile unsigned char g_Recv_Status = 0;
volatile int g_RecvFrame_OK;
int g_CommunicationPort_Mode = 1;   // 1:uart

//int ICARD_FIRST = 0;


int g_TestMode_Enable = 0;

static uint8_t g_AutoPiccCheck = 0;

extern unsigned char gucTypeB_ModGsp;

enum{
RECV_IDLE,
RECV_CMD,
RECV_SUBCMD,
RECV_LEN1,
RECV_LEN2,
RECV_DATA,
RECV_CRC1,
RECV_CRC2,

};

#define PICC_CHECK_IDLE_MS         1000 // 5000

static unsigned int g_Port_Idle_Ms = 0;

void SendingCommand()
{
	unsigned char ch;
	BYTE cRet,crc[3];
	int i,cmdLen;
//	BYTE buf[4096+20] = {0};
        	cmdLen = ((unsigned int)cmdBuf[2]<<8)+(unsigned int)cmdBuf[3];
        	Crc16CCITT((BYTE*)cmdBuf, cmdLen+4, crc);
        	memcpy(cmdBuf+cmdLen+4,crc,2);
        //	buf[0] = 0x02;
        //	memcpy(buf+1,cmdBuf,cmdLen+4+2);
        	
        	PortSend(PCI_COM,0x02);
        	for(i=0;i<cmdLen+4+2;i++){
        		PortSend(PCI_COM,cmdBuf[i]);
        	}
				g_Port_Idle_Ms = 0;
}

void SendingCommand_notify()
{
	unsigned char ch;
	BYTE cRet,crc[3];
	int i,cmdLen;
//	BYTE buf[4096+20] = {0};
        	cmdLen = ((unsigned int)cmdBuf[2]<<8)+(unsigned int)cmdBuf[3];
        	Crc16CCITT((BYTE*)cmdBuf, cmdLen+4, crc);
        	memcpy(cmdBuf+cmdLen+4,crc,2);
        //	buf[0] = 0x02;
        //	memcpy(buf+1,cmdBuf,cmdLen+4+2);
        	
        	PortSend(PCI_COM,0x02);
        	for(i=0;i<cmdLen+4+2;i++){
        		PortSend(PCI_COM,cmdBuf[i]);
        	}
}

unsigned char CardDetected_OK;
unsigned char CardSN_BackupBuff[30];
unsigned char CardSN_AltBuff[30];
unsigned char CardSN_AltSeen;
unsigned int CardSN_NoBackupCnt;

#define CARD_ALT_DETECT_THRESHOLD  3

static int Picc_SerialNo_IsSame(unsigned char *sn1, unsigned char *sn2)
{
    if(sn1[0] != sn2[0])
    {
        return 0;
    }
    if(sn1[0] == 0)
    {
        return 1;
    }
    return (memcmp(sn1, sn2, sn1[0] + 1) == 0);
}

static void Picc_SendCardDetectedNotify(unsigned char *cardtype, unsigned char *serialno)
{
    cmdBuf[0] = 0xc3;
    cmdBuf[1] = 0x06;
    cmdBuf[2] = 0;
    cmdBuf[3] = 2 + serialno[0] + 1 + 2;

    cmdBuf[4] = HI_BYTE(ABS(0));
    cmdBuf[5] = LOW_BYTE(ABS(0));

    memcpy(&cmdBuf[6], cardtype, 2);
    memcpy(&cmdBuf[8], serialno, serialno[0] + 1);
    SendingCommand_notify();
}

static void Picc_ResetCardDetectState(void)
{
    CardDetected_OK = 0;
    memset(CardSN_BackupBuff, 0, sizeof(CardSN_BackupBuff));
    memset(CardSN_AltBuff, 0, sizeof(CardSN_AltBuff));
    CardSN_AltSeen = 0;
    CardSN_NoBackupCnt = 0;
}

int WaitingCommand()
{
    uint8_t cRet, crc[3];
    unsigned char ch;
    unsigned int nRecvCount, cmdLen;
    unsigned char cLrc=0;
    unsigned char cTypeB_ModGsp_back;
    unsigned int nDebugCount;
    unsigned int nRecv_Idle_Cnt;
    int ret;
	  unsigned char cardtype[2],serialno[30];
	  

    nRecv_Idle_Cnt = 0;
    nDebugCount = 0;
    g_RecvFrame_OK = 0;
    g_Port_Idle_Ms = 0;
	Picc_ResetCardDetectState();
	
	if(g_AutoPiccCheck == 1)
	{
	  Dll_PiccOpen();
	}
   
    while(1)
    {             
        if(PortCheck(PCI_COM) == 0)
        {
            cRet = PortRecv(PCI_COM, &ch, 1000);
            nRecv_Idle_Cnt = 0;
            g_Port_Idle_Ms = 0;
             if(g_Recv_Status == RECV_IDLE)
            {
                g_CommunicationPort_Mode = 1;
            }
        }
        else
        {
             if(g_Recv_Status == RECV_IDLE)
             {
                 g_Port_Idle_Ms++;
             }
             else
             {
                 g_Port_Idle_Ms = 0;
             }
             nRecv_Idle_Cnt++;
             if(nRecv_Idle_Cnt > 2000)
             {
                   g_Recv_Status = RECV_IDLE;
                   nRecv_Idle_Cnt = 0;
             }

			if(g_AutoPiccCheck == 1)
			{
             /* Scenario F: card check only after port idle for 5 seconds */
             if(g_Recv_Status == RECV_IDLE && g_Port_Idle_Ms >= PICC_CHECK_IDLE_MS)
             {
                 ret = Dll_PiccCheck(0, cardtype, serialno);
                 if(ret == 0)
                 {
                     if(CardDetected_OK == 0)
                     {
                         /* Scenario A/C: first detection or card replaced after removal */
                         Picc_SendCardDetectedNotify(cardtype, serialno);
                         CardDetected_OK = 1;
                         memcpy(CardSN_BackupBuff, serialno, serialno[0] + 1);
                         CardSN_AltSeen = 0;
                         CardSN_NoBackupCnt = 0;
                     }
                     else if(Picc_SerialNo_IsSame(CardSN_BackupBuff, serialno))
                     {
                         /* Scenario B: same card still on antenna, do not notify again */
                         CardSN_NoBackupCnt = 0;
                     }
                     else
                     {
                         /* Scenario B/D: different card detected while notified card may still be present */
                         if(CardSN_AltSeen && Picc_SerialNo_IsSame(CardSN_AltBuff, serialno))
                         {
                             CardSN_NoBackupCnt++;
                             if(CardSN_NoBackupCnt >= CARD_ALT_DETECT_THRESHOLD)
                             {
                                 /* Scenario D: previous card removed, alternate card remains */
                                 Picc_SendCardDetectedNotify(cardtype, serialno);
                                 memcpy(CardSN_BackupBuff, serialno, serialno[0] + 1);
                                 CardSN_AltSeen = 0;
                                 CardSN_NoBackupCnt = 0;
                             }
                         }
                         else
                         {
                             memcpy(CardSN_AltBuff, serialno, serialno[0] + 1);
                             CardSN_AltSeen = 1;
                             CardSN_NoBackupCnt = 1;
                         }
                     }
                 }
                 else
                 {
                     /* Card removed from antenna, ready for next placement */
                     Picc_ResetCardDetectState();
                 }
             }
			}
			else
			{
				Picc_ResetCardDetectState();
			}

             delay_ms(1);
             continue;
        }
                
        if(cRet == 0)
        {
            
//        printf("[%02x]",ch);
            switch(g_Recv_Status)
            {
                case RECV_IDLE:
                {
            //        printf("%02x",ch);
                    if(ch == 0x02)
                    {
                        g_Recv_Status = RECV_CMD;
                        nRecvCount = 0;
                        nDebugCount = 0;
                    }
                }
                break;
                case RECV_CMD:
                {
             //       printf("%02x",ch);
                    cmdBuf[nRecvCount++] = ch;
                    g_Recv_Status = RECV_SUBCMD;
                }
                break;
                case RECV_SUBCMD:
                {
              //      printf("%02x",ch);
                    cmdBuf[nRecvCount++] = ch;
                    g_Recv_Status = RECV_LEN1;
                }
                break;
                case RECV_LEN1:
                {
               //     printf("%02x",ch);
                    cmdBuf[nRecvCount++] = ch;
                    g_Recv_Status = RECV_LEN2;
                }
                break;
                case RECV_LEN2:
                {
                 //   printf("%02x",ch);
                    cmdBuf[nRecvCount++] = ch;

                    cmdLen = ((unsigned int)cmdBuf[2]) << 8;
                    cmdLen += (unsigned int)cmdBuf[3];
                    if(cmdLen>CMD_MAX_BUFLEN) 
                    {
                        g_Recv_Status = RECV_IDLE;
                        return CMD_ABORT+1;
                    }
                    if(cmdLen == 0)
                    {
                         g_Recv_Status = RECV_CRC1;
                    }
                    else
                    {
                        g_Recv_Status = RECV_DATA;
                    }
                }
                break;
                case RECV_DATA:
                {
               //     printf("%02x",ch);
                    cmdBuf[nRecvCount++] = ch;
                    if(nRecvCount >= (cmdLen+4)) 
                    {
                        g_Recv_Status = RECV_CRC1;
                    }
                    else
                    {
                        g_Recv_Status = RECV_DATA;
                    }
                }
                break;
                case RECV_CRC1:
                {
           //         printf("%02x",ch);
                    cmdBuf[nRecvCount++] = ch;
                    g_Recv_Status = RECV_CRC2;
             //       printf("crc1=%02x\r\n", ch);
                }
                break;
                case RECV_CRC2:
                {
            //        printf("%02x",ch);
               //     printf("recv crc2 ok\r\n");
            //        printf("len=%d,",cmdLen);
                    cmdBuf[nRecvCount++] = ch;
          //          printf("crc2=%02x\r\n", ch);
                    g_Recv_Status = RECV_IDLE;
                    g_RecvFrame_OK = 1;
       //             Crc16CCITT((BYTE*)cmdBuf, nRecvCount-2, crc); 
       //             if(!memcmp((cmdBuf+nRecvCount-2),crc,2))
                    {
             //       printf(" crc verify ok\r\n");
                        return CMD_OK;
                    }
          //         printf(" crc verify failed\r\n");
                }
                break;
//---------------------------------------------------------------

                default:
                    g_Recv_Status = RECV_IDLE;
                    break;
//---------------------------------------------------------------
            }
        }
    }
}

int command_handler()
{
    uint8_t cRet;
    while(1)
    {
        cRet = WaitingCommand(); 

        if(cRet != CMD_OK) continue;

        switch(cmdBuf[0])
        {
            case PICC_CODE:
            case PICC_CODE2:
                Picc_Pro();
                break;
            case SYS_CODE:
                System_Pro();
                break;
            default:
                break;
        }
    }
      
}

int Picc_Pro(void)
{
    char subCmd;
    int len,i,j,iRet,flag;
    uint8_t byRet,mode;
    BYTE cardtype[2],serialno[30];
    APDU_SEND ApduSend;
    APDU_RESP ApduResp;
    BYTE buf[50];
    BYTE type,blkno,pwd[7]={0},UpdataBlkNo;
    uint16_t option,ms,TxLen,RxLen;
    uint16_t nfc_text_len;
    BYTE txbuf[256],rxbuf[256],uid[20],sak[2];
    BYTE uidlen = 0,atslen = 0;
    BYTE buf_temp[256];
    BYTE cAts[40];
    BYTE reg_addr, reg_value;
	
    len = (((unsigned int)cmdBuf[2])<<8)+(unsigned int)cmdBuf[3];
    subCmd = cmdBuf[1];
    
    switch(subCmd)
    {
    case 0x01:     //picc open

        iRet = Dll_PiccOpen();
        cmdBuf[1] = 0x02;
        cmdBuf[2] = 0;
        cmdBuf[3] = 0x02;
        cmdBuf[4] = HI_BYTE(ABS(iRet));
        cmdBuf[5] = LOW_BYTE(ABS(iRet));
        SendingCommand();
        break;
    case 0x03: //picc close

        iRet = Dll_PiccClose();
        cmdBuf[1] = 0x04;
        cmdBuf[2] = 0;
        cmdBuf[3] = 0x02;
        cmdBuf[4] = HI_BYTE(ABS(iRet));
        cmdBuf[5] = LOW_BYTE(ABS(iRet));

        SendingCommand();
        break;
    case 0x05: //picc check
        //Lib_LcdCheckCard();
        cmdBuf[1] = 0x06;
        mode = cmdBuf[4];

        iRet = Dll_PiccCheck(mode,cardtype,serialno); 
        cmdBuf[2] = 0;
        if(iRet == 0) 
        {
#ifdef PICC_CHECK_RETURN_OTHERS_INFO
           memset(buf_temp, 0, sizeof(buf_temp));
           iRet = Dll_PiccGetInformation(buf_temp);
           if(iRet == 0)
           {
               cmdBuf[3] = 2+serialno[0]+1+2 + buf_temp[0];
           }
           else
           {
               cmdBuf[3] = 2;
           }
#else
            cmdBuf[3] = 2+serialno[0]+1+2;
#endif
        }
        else 
        {
            cmdBuf[3] = 2;
        }
  
        cmdBuf[4] = HI_BYTE(ABS(iRet));
        cmdBuf[5] = LOW_BYTE(ABS(iRet));
	 if(!iRet)
	 {
#ifdef PICC_CHECK_RETURN_OTHERS_INFO
             memcpy(&cmdBuf[6],cardtype,2);
	      memcpy(&cmdBuf[8],serialno,serialno[0]+1);
             memcpy(&cmdBuf[8+serialno[0]+1], &buf_temp[0], buf_temp[0] + 1);
#else
	      memcpy(&cmdBuf[6],cardtype,2);
	      memcpy(&cmdBuf[8],serialno,serialno[0]+1);
#endif
	 }
        SendingCommand();                           
        break;                        
    case 0x07:   //cmd change           
         cmdBuf[1] = 0x08;
        //Picc_Open();
        //Picc_Check2(mode,cardtype,serialno,3);
        memcpy(ApduSend.Command,&cmdBuf[4],4);
        ApduSend.Lc = MAKEWORD(cmdBuf[9],cmdBuf[8]);
        memcpy(ApduSend.DataIn,&cmdBuf[10],ApduSend.Lc);
        ApduSend.Le = MAKEWORD(cmdBuf[10+ApduSend.Lc+1],cmdBuf[10+ApduSend.Lc]);
           
        iRet = Dll_PiccCommand(&ApduSend,&ApduResp);

	 if(!iRet)
	 {
        	memcpy(&cmdBuf[6],ApduResp.DataOut,ApduResp.LenOut);
        	cmdBuf[6+ApduResp.LenOut] = ApduResp.SWA;
        	cmdBuf[6+ApduResp.LenOut+1] = ApduResp.SWB;
	 }
        if(!iRet)
        {
	        cmdBuf[2] = HI_BYTE(ApduResp.LenOut+4);
	        cmdBuf[3] = LOW_BYTE(ApduResp.LenOut+4);
        }
	 else 
	 {
	 	cmdBuf[2] = 0;
		cmdBuf[3] = 2;
	 }
        cmdBuf[4] = HI_BYTE(ABS(iRet));
        cmdBuf[5] = LOW_BYTE(ABS(iRet));
        SendingCommand();                              
        break;
    case 0x09:    //remove card

        iRet = Dll_PiccRemove(0);
        cmdBuf[1] = 0x0a;
        cmdBuf[2] = 0;
        cmdBuf[3] = 0x02;
        cmdBuf[4] = HI_BYTE(ABS(iRet));
        cmdBuf[5] = LOW_BYTE(ABS(iRet));
        SendingCommand();
        break;
    case 0x0b:    // halt
        iRet = Dll_PiccHaltA();//Lib_PiccHalt();
        cmdBuf[1] = 0x0c;
        cmdBuf[2] = 0;
        cmdBuf[3] = 2;       
        cmdBuf[4] = HI_BYTE(ABS(iRet));
        cmdBuf[5] = LOW_BYTE(ABS(iRet));
        SendingCommand();
        break;
    case 0x0d:
        Dll_PiccReset();//picc_reset();
        iRet = 0;//Lib_PiccReset();
        cmdBuf[1] = 0x0e;
        cmdBuf[2] = 0;
        cmdBuf[3] = 2;       
        cmdBuf[4] = HI_BYTE(ABS(iRet));
        cmdBuf[5] = LOW_BYTE(ABS(iRet));
        SendingCommand();
        break;
    case 0x11:
		cmdBuf[1] = 0x12;
		type = cmdBuf[4];
		blkno = cmdBuf[5];
		memset(pwd,0,sizeof(pwd));
		memset(serialno,0,sizeof(serialno));
		memcpy(pwd,cmdBuf+6,6);
		memcpy(serialno,cmdBuf+12,4);

		iRet = Dll_MfcAuthenticate(type,blkno,pwd,serialno);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
       break;
    case 0x13:
		cmdBuf[1] = 0x14;
		blkno = cmdBuf[4];

		iRet = Dll_MfcReadBlock(blkno,buf);
		if(iRet != 0)
		{
			cmdBuf[2] = 0;
			cmdBuf[3] = 2;       
			cmdBuf[4] = HI_BYTE(ABS(iRet));
			cmdBuf[5] = LOW_BYTE(ABS(iRet));
			SendingCommand();
			return 1;
		}
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+16;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		memcpy(cmdBuf+6,buf,16);
		SendingCommand();
       break;
    case 0x15:
		cmdBuf[1] = 0x16;
		blkno = cmdBuf[4];
		memcpy(buf,cmdBuf+5,16);

		iRet = Dll_MfcWriteBlock(blkno,buf);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
		break;
    case 0x17:
		cmdBuf[1] = 0x18;
		type = cmdBuf[4];
		blkno = cmdBuf[5];
		memcpy(buf,cmdBuf+6,4);
		UpdataBlkNo = cmdBuf[10];

		iRet = Dll_MfcOperate(type,blkno,buf,UpdataBlkNo);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
		break;
     case 0x19:
            cmdBuf[1] ++;
            memset(cAts, 0, sizeof(cAts));
            iRet =Dll_TypeA_GetATS(cAts);
            if(iRet)
            {
                cmdBuf[2] = 0;
                cmdBuf[3] = 2; 
                cmdBuf[4] = HI_BYTE(ABS(iRet));
                cmdBuf[5] = LOW_BYTE(ABS(iRet));
                SendingCommand();
                break;
            }
            
            memcpy(&cmdBuf[6], cAts, cAts[0]);
            cmdBuf[2] = 0;
            cmdBuf[3] = 2 + cAts[0]; 
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            SendingCommand();
            break;
     case 0x1B://PICC_M1INCREMENT
     		cmdBuf[1] ++;
		blkno = cmdBuf[4];
		memcpy(buf,cmdBuf+5,len-1);

     		iRet = Dll_MfcIncrement(blkno,buf);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
     case 0x1D://PICC_M1DECREMENT
     		cmdBuf[1] ++;
		blkno = cmdBuf[4];
		memcpy(buf,cmdBuf+5,len-1);

     		iRet = Dll_MfcDecrement(blkno,buf);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
     case 0x1F://PICC_M1RESTORE
     		cmdBuf[1] ++;
		blkno = cmdBuf[4];

     		iRet = Dll_MfcRestore(blkno);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
     case 0x21://PICC_M1TRANSFER
     		cmdBuf[1]++;
		blkno = cmdBuf[4];

     		iRet = Dll_MfcTransfer(blkno);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
        case 0x29://PICC_REQA
     		cmdBuf[1] ++;
     		iRet = Dll_PiccReqa(&cmdBuf[6]);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+2;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
        case 0x2B://PICC_REQB
     		cmdBuf[1] ++;
     		iRet = Dll_PiccReqb(&cmdBuf[6]);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+2;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
     case 0x2D://PICC_WUPA
     		cmdBuf[1] ++;
     		iRet = Dll_PiccWupa(&cmdBuf[6]);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+2;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
        case 0x2F://PICC_WUPB
     		cmdBuf[1] ++;
     		iRet = Dll_PiccWupb(&cmdBuf[6]);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+13;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
     case 0x31://PICC_ANTICOLLA
     		cmdBuf[1] ++;
     		iRet = Dll_PiccAnticoll_A(&cmdBuf[6],&uidlen,uid,sak);
		memcpy(cmdBuf+7,uid,uidlen);
		cmdBuf[7+uidlen] = sak[0];
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+2+uidlen;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
        case 0x33://PICC_ANTICOLLB
     		cmdBuf[1] ++;
     		iRet = Dll_PiccAnticoll_B(&cmdBuf[6]);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+4;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
     case 0x35://PICC_RATS
     		cmdBuf[1] ++;
     		iRet = Dll_PiccRats(&atslen,&cmdBuf[6]);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+atslen;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
     case 0x37://PICC_ATTRIB
     		cmdBuf[1] ++;
     		iRet = Dll_PiccAttrib(&atslen,&cmdBuf[6]);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+atslen;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
        case 0x41://PICC_GETINFO
     		cmdBuf[1] ++;
     		iRet = Dll_PiccGetInformation(&cmdBuf[6]);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+cmdBuf[6]+1;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
	 	break;
        case 0x43://PICC_CMDEXCHANGE
     		cmdBuf[1] ++;
		TxLen = cmdBuf[4]*256+cmdBuf[5];
               ms = cmdBuf[6+TxLen];
		memcpy(txbuf,cmdBuf+6,TxLen);
     		iRet = Dll_PiccApdu(txbuf,TxLen,cmdBuf+6,&RxLen);
		cmdBuf[2] = 0;
		cmdBuf[3] = 2+RxLen;       
		cmdBuf[4] = HI_BYTE(ABS(iRet));
		cmdBuf[5] = LOW_BYTE(ABS(iRet));
		SendingCommand();
		break;
        case 0x51:
            cmdBuf[1]++;

            iRet = Dll_PiccAntennaOff();//pcd_antenna_off();
            cmdBuf[2] = 0;
            cmdBuf[3] = 2;       
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            SendingCommand();
            break;
        case 0x53:
            cmdBuf[1]++;

            iRet = Dll_PiccAntennaOn();//pcd_antenna_on();
            cmdBuf[2] = 0;
            cmdBuf[3] = 2;       
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            SendingCommand();
            break;
        case 0x55:
            cmdBuf[1]++;

            iRet = Dll_PiccAntennaOn();//pcd_antenna_on();
            cmdBuf[2] = 0;
            cmdBuf[3] = 2;       
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            SendingCommand();
            break;
        case 0x59:
        case 0x20:
            iRet = Dll_ID_Command(&cmdBuf[4],len,&cmdBuf[6],&len);
            cmdBuf[1]++;
            cmdBuf[2] = 0;
            cmdBuf[3] = 2+len;       
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            SendingCommand();
            break;
		case 0x5a:
			cmdBuf[1]++;
            nfc_text_len = 0;
            iRet = Dll_NfcReadTextFromTag((char *)&cmdBuf[6], 200, &nfc_text_len);
			if(iRet == 0)
			{
            	cmdBuf[2] = 0;
            	cmdBuf[3] = 2 + nfc_text_len;
			}
			else
			{
				cmdBuf[2] = 0;
            	cmdBuf[3] = 2; 
			}
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            SendingCommand();
			break;
		case 0x5c:
			cmdBuf[1]++;
            iRet = Dll_NfcWriteTextToTag(&cmdBuf[4], len);
			cmdBuf[2] = 0;
        	cmdBuf[3] = 2; 
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            SendingCommand();
			break;
		case 0x5d: /* factory provision; field builds return provision-fail */
			cmdBuf[1]++;
			iRet = ntag_provision_tag((len > 0) ? &cmdBuf[4] : 0, (unsigned short)len);
			cmdBuf[2] = 0;
			cmdBuf[3] = 2;
			cmdBuf[4] = HI_BYTE(ABS(iRet));
			cmdBuf[5] = LOW_BYTE(ABS(iRet));
			SendingCommand();
			break;
		case 0x5e: /* unauth user-area READ probe (must fail on Scheme B tag) */
			cmdBuf[1]++;
			iRet = ntag_test_unauth_user_read();
			cmdBuf[2] = 0;
			cmdBuf[3] = 2;
			cmdBuf[4] = HI_BYTE(ABS(iRet));
			cmdBuf[5] = LOW_BYTE(ABS(iRet));
			SendingCommand();
			break;
		case 0x5f: /* wrong-PWD PWD_AUTH probe */
			cmdBuf[1]++;
			iRet = ntag_test_wrong_pwd();
			cmdBuf[2] = 0;
			cmdBuf[3] = 2;
			cmdBuf[4] = HI_BYTE(ABS(iRet));
			cmdBuf[5] = LOW_BYTE(ABS(iRet));
			SendingCommand();
			break;
         case 0x61://read reg
         {
            pcd_config('B');	
            reg_addr = cmdBuf[4];
            reg_value = read_reg(reg_addr);
            iRet = 0;
            cmdBuf[1]++;
            cmdBuf[2] = 0;
            cmdBuf[3] = 3;       
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            cmdBuf[6] = reg_value;
            SendingCommand();
         }
         break;
         case 0x63://Set Type B modgsp index
         {
              cmdBuf[1]++;
              reg_value = cmdBuf[4];
              if((reg_value > 0) && (reg_value < 0x3f))
              {
                    
                    flash_read( buf, TYPEB_PARA_CONFIG_ADDR, 9);
                    *(unsigned int *)&buf[0] = TYPEB_PARA_CONFIG_FLAG;
                    buf[7] = reg_value;
                    eraseFlashPage(TYPEB_PARA_CONFIG_ADDR);
                    iRet = flash_write(TYPEB_PARA_CONFIG_ADDR, buf, 9);
                    if (iRet == 0)
                    {
                   //        s_PiccInit();
                    }
              }
              else
              {
                  iRet = -1;
              }
            cmdBuf[2] = 0;
            cmdBuf[3] = 2;       
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            SendingCommand();
         }
         break;
         case 0x65://get mh1608 version
         {
            iRet = 0;
            cmdBuf[1]++;
            cmdBuf[2] = 0;
            cmdBuf[3] = 3;       
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            cmdBuf[6] = Get_MH1608_Version();
            SendingCommand();
         }
         break;
         case 0x67:
         {
              cmdBuf[1]++;
              reg_addr = cmdBuf[4];
              reg_value = cmdBuf[5];
              flash_read( buf, TYPEB_PARA_CONFIG_ADDR, 9);
              *(unsigned int *)&buf[0] = TYPEB_PARA_CONFIG_FLAG;

              iRet = 0;
              if(reg_addr == 0x26)
              {
                    buf[4] = reg_value;
              }
              else if(reg_addr == 0x27)
              {
                    buf[5] = reg_value;
              }
              else if(reg_addr == 0x28)
              {
                    buf[6] = reg_value;
              }
              else if(reg_addr == 0x29)
              {
                    buf[7] = reg_value;
              }
              else if(reg_addr == 0x18)
              {
                    buf[8] = reg_value;
              }
              else
              {
                  iRet = -10;
              }

             if(iRet == 0)
             {
                  eraseFlashPage(TYPEB_PARA_CONFIG_ADDR);
                  iRet = flash_write(TYPEB_PARA_CONFIG_ADDR, buf, 9);
             }
                    
            cmdBuf[2] = 0;
            cmdBuf[3] = 2;       
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            SendingCommand();
         }
         break;
         case 0x69:
         {
            cmdBuf[1]++;
            flash_read( buf, TYPEB_PARA_CONFIG_ADDR, 9);
            *(unsigned int *)&buf[0] = TYPEB_PARA_CONFIG_FLAG;
            cmdBuf[2] = 0;
            cmdBuf[3] = 7;       
            memcpy(&cmdBuf[6], &buf[4], 5);
            iRet = 0;
            cmdBuf[4] = HI_BYTE(ABS(iRet));
            cmdBuf[5] = LOW_BYTE(ABS(iRet));
            SendingCommand();
         }
         break;
		case 0xbb:
		{
			cmdBuf[1]++;
			mode = cmdBuf[4];
			if(mode)
			{
				g_AutoPiccCheck = 1;
			}
			else
			{
				g_AutoPiccCheck = 0;
				Dll_PiccClose();
			}
			cmdBuf[2] = 0;
			cmdBuf[3] = 2;       
			iRet = 0;
			cmdBuf[4] = HI_BYTE(ABS(iRet));
			cmdBuf[5] = LOW_BYTE(ABS(iRet));
			SendingCommand();
		}
		break;
     default:
              break;
    }
  
}



int System_Pro(void)
{
    char subCmd;
    int len, iret;
    unsigned char name[20];
	
    len = (((unsigned int)cmdBuf[2])<<8)+(unsigned int)cmdBuf[3];
    subCmd = cmdBuf[1];
    
    switch(subCmd)
    {
    	case 0x05://get version
    	{
    		cmdBuf[1] = 0x06;
    		cmdBuf[2] = 0;
    		cmdBuf[3] = 0x08;
    		iret = Lib_GetVersion(cmdBuf+6);
    		cmdBuf[4] = HI_BYTE(ABS(iret));
    		cmdBuf[5] = LOW_BYTE(ABS(iret));
    		SendingCommand();
    	}
    	break;
        case 0x91:
        {
              g_TestMode_Enable = 1;
              cmdBuf[1]++;
    		cmdBuf[2] = 0;
    		cmdBuf[3] = 0x02;
    		iret = 0;
    		cmdBuf[4] = HI_BYTE(ABS(iret));
    		cmdBuf[5] = LOW_BYTE(ABS(iret));
    		SendingCommand();
        }
        break;
    }
}

