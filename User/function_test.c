#include <string.h>
#include <stdio.h>
#include "mhscpu.h"
#include "uart.h"
#include "rfid.h"
#include "user.h"
#define lite_printf  printf
#define PHHAL_HW_MFC_KEYA               0x0AU   /**< MIFARE Classic key type A. */
#define PHHAL_HW_MFC_KEYB               0x0BU   /**< MIFARE Classic key type B. */
extern unsigned char gucTypeB_ModGsp;
extern unsigned char gucATQB[13];
unsigned int g_startTick_ms;
extern unsigned int g_RF_Module;
void ID_Card_Successrate_Test(void);
int s_check_key(void)
{
    return PortCheck(0);
}
unsigned char s_get_key(void)
{
       unsigned char ch;
	int ret;
	while(1)
	{
		if(PortCheck(0) == 0)
		{
	            PortRecv(0,&ch, 1000);
			return ch;
		}
              if(get_tick() -g_startTick_ms > 60000)
              {
                     return 0x1b;
              }
	}
}
void test_piccFun(void)
{
    unsigned char atqa[2],atqb[13];
    unsigned char uid[11];
    unsigned char sak;
    unsigned char buff[50];
//	unsigned char cUidSize,cUidLen;
	unsigned char cAts_len;
    unsigned char cAts_buf[20];
    unsigned char cBlock;
	unsigned char cKEYA[6],cKEYB[6];
	unsigned char ch;
    int i;
    int ret;
	APDU_SEND apdusend;
	APDU_RESP apduresp;
    unsigned char SN[20];
    
//static unsigned char TxLevel = 4;
	cBlock = 56;
       SYSCTRL_APBPeriphClockCmd( SYSCTRL_APBPeriph_TIMM0, ENABLE);
       SYSCTRL_APBPeriphResetCmd( SYSCTRL_APBPeriph_TIMM0, ENABLE);
                                       
       init_timer0();
    
    while(1)
    {
		memcpy(cKEYA, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
		memcpy(cKEYB, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
	//	memcpy(cKEYA, "\x11\x11\x11\x11\x11\x11", 6);
	//	memcpy(cKEYB, "\x11\x11\x11\x11\x11\x11", 6);
	//	memcpy(cKEYA, "\x22\x22\x22\x22\x22\x22", 6);
	//	memcpy(cKEYB, "\x22\x22\x22\x22\x22\x22", 6);
		
              lite_printf("0------picc close \r\n");lite_printf("\r\n");
		lite_printf("1------picc open \r\n");lite_printf("\r\n");
		lite_printf("2------Find TypeA \r\n");lite_printf("\r\n");
		lite_printf("3------HALTA \r\n");lite_printf("\r\n");
		lite_printf("4------Find M1 \r\n");lite_printf("\r\n");
		lite_printf("5------Find TypeB \r\n");lite_printf("\r\n");
		lite_printf("6------APDU \r\n");lite_printf("\r\n");
		lite_printf("7------ID Card (removed)\r\n");lite_printf("\r\n");
		lite_printf("8------Dll_MfcAuthenticate  KEYA \r\n");lite_printf("\r\n");
		lite_printf("9------Dll_MfcAuthenticate  KEYB \r\n");lite_printf("\r\n");
		lite_printf("A------READ BLOCK   \r\n");lite_printf("\r\n");
		lite_printf("B------WRITE BLOCK  \r\n");lite_printf("\r\n");
	       lite_printf("C------PICC_CHECK \r\n");lite_printf("\r\n");
		lite_printf("D------PICC_CARIEER RESET \r\n");lite_printf("\r\n");
              lite_printf("E------typeB()\r\n");lite_printf("\r\n");
              lite_printf("'+'-----Type B modulation index++=0x%02x\r\n",gucTypeB_ModGsp);lite_printf("\r\n");
              lite_printf("'-'------Type B modulation index--=0x%02x\r\n",gucTypeB_ModGsp);lite_printf("\r\n");
              memset(SN, 0, sizeof(SN));
              GetUSN(SN);
              lite_printf("CPU:%s\r\n", SN);
              if(g_RF_Module == RF_MH1608)
              {
                lite_printf("MH1608 Version: %02x\r\n", Get_MH1608_Version());
              }
              Lib_GetVersion(buff);
              lite_printf("App Version: %02x.%02x.%02x\r\n", buff[0], buff[1], buff[2]);
              lite_printf("Boot Version: %02x.%02x.%02x\r\n", buff[3], buff[4], buff[5]);
              
              lite_printf("ESC KEY to Exit\r\n");
              
              
           g_startTick_ms = get_tick();
	    ch = s_get_key();
        lite_printf("ch = %c, %02x\r\n", ch, ch);
        
	    lite_printf("\r\n");
	    lite_printf("\r\n");
	    switch(ch)
	    {
            case 0x1b:
                    {
                         //   Timer0_1_disable();
                                SYSCTRL_APBPeriphClockCmd( SYSCTRL_APBPeriph_TIMM0, DISABLE);
                                SYSCTRL_APBPeriphResetCmd( SYSCTRL_APBPeriph_TIMM0, DISABLE);
                             printf("Debug mode exited\r\n");
                            return;
                    }
                    break;
	     case '+':
                     if(gucTypeB_ModGsp < 0x3f)
                        gucTypeB_ModGsp++;
                     lite_printf("'+'------Type B modulation index++=0x%02x \r\n",gucTypeB_ModGsp);
                //     Dll_PiccAdjustAntenna(TxLevel);
                     break;
            case '-':
                     if(gucTypeB_ModGsp > 0)
                        gucTypeB_ModGsp--;
                     lite_printf("'-'------Type B modulation index--=0x%02x \r\n",gucTypeB_ModGsp);
         //            Dll_PiccAdjustAntenna(TxLevel);
                     break;
            case '0':
			{
				 lite_printf("0------picc close \r\n");
                                Dll_PiccClose();
			}
			break;
            case '1':
			{
                 lite_printf("1------picc open \r\n");
				 Dll_PiccOpen();
			}
			break;
			case '2':
			{
                            {
					unsigned char cardtype[2];
					unsigned char serial[11];
                    lite_printf("Dll_PiccCheck Find TYPE A...\r\n");
					if(Dll_PiccCheck('A',cardtype,serial)==0)
        			        {
        			            lite_printf("dected %c %c card\r\n",cardtype[0],cardtype[1]);
                                     //    lite_printf("ATQA: %02x %02x\r\n", g_tag_info.tag_type_bytes[0], g_tag_info.tag_type_bytes[1]);
        			            lite_printf("sln:\r\n");
        			            for(i = 0; i < serial[0]; i++)
        			            {
        			                 lite_printf("%x ", serial[1 + i]);
        			            }
        			            lite_printf("\r\n");
                                     //   lite_printf("sak=%02x\r\n", g_tag_info.sak);
                                        memset(cAts_buf, 0, sizeof(cAts_buf));
                                        ret = Dll_TypeA_GetATS(&cAts_buf);
                                        if(ret == 0)
                                        {
                                            printf("ATS:");
                                            for(i = 0; i < cAts_buf[0]; i++)
                                            {
                                                lite_printf("%x ", cAts_buf[i]);
                                            }
                                            lite_printf("\r\n");
                                        }
        			        }
				}
			}
			break;
			case '3':
			{
                   lite_printf("3------HALTA \r\n");
				   Dll_PiccHaltA();
			}
			break;
			case '4':
			{
                            {
					unsigned char cardtype[2];
					unsigned char serial[11];
                    lite_printf("Dll_PiccCheck Find M1...\r\n");
					if(Dll_PiccCheck('M',cardtype,serial)==0)
        			        {
        			            lite_printf("dected %c %c card\r\n",cardtype[0],cardtype[1]);
                                         lite_printf("ATQA: %02x %02x\r\n", g_tag_info.tag_type_bytes[0], g_tag_info.tag_type_bytes[1]);
        			            lite_printf("sln:\r\n");
        			            for(i = 0; i < serial[0]; i++)
        			            {
        			                 lite_printf("%x ", serial[1 + i]);
        			            }
        			            lite_printf("\r\n");
        			           lite_printf("sak=%02x\r\n", g_tag_info.sak);
        			        }
				}
			}
			break;
			case '5':
			{
                            {
					unsigned char cardtype[2];
					unsigned char serial[11];
                    lite_printf("Dll_PiccCheck  Find TypeB...\r\n");
					if(Dll_PiccCheck('B',cardtype,serial)==0)
        			        {
        			            lite_printf("dected %c %c card\r\n",cardtype[0],cardtype[1]);
        			            lite_printf("ATQB:\r\n");
        			            for(i = 0; i < 13; i++)
        			            {
        			                 lite_printf("%x ", gucATQB[i]);
        			            }
        			            lite_printf("\r\n");
        			        }
				}
			}
			break;
			case '6':
			{
					lite_printf("6------APDU \r\n");
                         /*            //00 a4 04 00 0f 73 78 31 2e 73 68 2e c9 e7 bb e1 b1 a3 d5 cf
                                    apdusend.Command[0]=0x00;
                                    apdusend.Command[1]=0xa4;
                                    apdusend.Command[2]=0x04;
                                    apdusend.Command[3]=0x00;
                                    apdusend.Lc=0x0f;
                                    apdusend.Le=0;
                                    //memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);
                                    memcpy(apdusend.DataIn,"\x73\x78\x31\x2e\x73\x68\x2e\xc9\xe7\xbb\xe1\xb1\xa3\xd5\xcf",apdusend.Lc);
                                    ret = Dll_PiccCommand(&apdusend, &apduresp);
                                    lite_printf("command iret:%d\r\n",ret);
                                    if(ret==0)
                                    {
                                           for(i=0;i<apduresp.LenOut;i++)
                                               lite_printf("%02x ",apduresp.DataOut[i]);
                                           lite_printf("\r\nSWA:%02x SWB:%02x\r\n",apduresp.SWA,apduresp.SWB);
                                           //0084000008
                                            apdusend.Command[0]=0x00;
                                            apdusend.Command[1]=0x84;
                                            apdusend.Command[2]=0x00;
                                            apdusend.Command[3]=0x00;
                                            apdusend.Lc=0x00;
                                            apdusend.Le=8;
                                            ret = Dll_PiccCommand(&apdusend, &apduresp);
                                            lite_printf("command iret:%d\r\n",ret);
                                            if(ret==0)
                                            {
                                                   for(i=0;i<apduresp.LenOut;i++)
                                                       lite_printf("%02x ",apduresp.DataOut[i]);
                                                   lite_printf("\r\nSWA:%02x SWB:%02x\r\n",apduresp.SWA,apduresp.SWB);
                                            }
                                    }
                                    */
                                            
             /*       apdusend.Command[0]=0x00;
                                apdusend.Command[1]=0xa4;
                                apdusend.Command[2]=0x00;
                                apdusend.Command[3]=0x00;
                                apdusend.Lc=2;
                                apdusend.Le=256;
                                memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);*/
                    
					apdusend.Command[0]=0x00;
					apdusend.Command[1]=0xa4;
					apdusend.Command[2]=0x04;
					apdusend.Command[3]=0x00;
					apdusend.Lc=0x0e;
					apdusend.Le=256;
					//memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);
					memcpy(apdusend.DataIn,"2PAY.SYS.DDF01",14);
					ret = Dll_PiccCommand(&apdusend, &apduresp);
					lite_printf("command iret:%d\r\n",ret);
					ret = 0;
					if(ret==0)
					{
						for(i=0;i<apduresp.LenOut;i++)
							lite_printf("%02x ",apduresp.DataOut[i]);
						lite_printf("\r\nSWA:%02x SWB:%02x\r\n",apduresp.SWA,apduresp.SWB);
                        //-------------------------------------------------------------------------------------
                                           //00 a4 04 00 00 08 a0 00 00 03 33 01 01 01 01 00
                                            apdusend.Command[0]=0x00;
                                            apdusend.Command[1]=0xa4;
                                            apdusend.Command[2]=0x04;
                                            apdusend.Command[3]=0x00;
                                            apdusend.Lc=0x08;
                                            apdusend.Le=256;
                                            //memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);
                                            memcpy(apdusend.DataIn,"\xa0\x00\x00\x03\x33\x01\x01\x01",apdusend.Lc);
                                            ret = Dll_PiccCommand(&apdusend, &apduresp);
                                            lite_printf("command iret:%d\r\n",ret);
                                            if(ret==0)
					        {
        						for(i=0;i<apduresp.LenOut;i++)
        							lite_printf("%02x ",apduresp.DataOut[i]);
        						lite_printf("\r\nSWA:%02x SWB:%02x\r\n",apduresp.SWA,apduresp.SWB);
                                //80 a8 00 00 00 23 83 21 24 80 00 80 00 00 00 00 00 00 00 00 00 00 00 00 01 56 00 00 00 00 00 01 56 20 06 18 60 5e 11 8c 09 01 00
                                                apdusend.Command[0]=0x80;
                                                apdusend.Command[1]=0xa8;
                                                apdusend.Command[2]=0x00;
                                                apdusend.Command[3]=0x00;
                                                apdusend.Lc=0x23;
                                                apdusend.Le=256;
                                                //memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);
                                                memcpy(apdusend.DataIn,"\x83\x21\x24\x80\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x56\x00\x00\x00\x00\x00\x01\x56\x20\x06\x18\x60\x5e\x11\x8c\x09",apdusend.Lc);
                                                ret = Dll_PiccCommand(&apdusend, &apduresp);
                                                lite_printf("command iret:%d\r\n",ret);
                                                 if(ret==0)
                                                 {
                                                         for(i=0;i<apduresp.LenOut;i++)
        							lite_printf("%02x ",apduresp.DataOut[i]);
        						lite_printf("\r\nSWA:%02x SWB:%02x\r\n",apduresp.SWA,apduresp.SWB);
                                                 }
                                            }
                        //--------------------------------------------------------------------------------------
					}
			}
			break;
			case '7':
			{
                                lite_printf("7------ID Card Success rate test \r\n");
			//		Dll_Deselect(0,0);
			           ID_Card_Successrate_Test();
			}
			break;
			case '8':
			{
                    lite_printf("8------Dll_MfcAuthenticate  KEYA \r\n");
					ret = Dll_MfcAuthenticate(PHHAL_HW_MFC_KEYA,cBlock,cKEYA,&uid[0]);
					lite_printf("ret = %d\r\n",ret);
					if(ret==0)
					{
						lite_printf("authenticate key A success\r\n");
					}
					else
					{
						lite_printf("authenticate key A failed\r\n");
					}
			}
			break;
			case '9':
			{
                    lite_printf("9------Dll_MfcAuthenticate  KEYB \r\n");
					ret = Dll_MfcAuthenticate(PHHAL_HW_MFC_KEYB,cBlock,cKEYB,&uid[0]);
					lite_printf("ret = %d\r\n",ret);
					if(ret==0)
					{
						lite_printf("authenticate key B success\r\n");
					}
					else
					{
						lite_printf("authenticate key B failed\r\n");
					} 
			}
			break;
			case 'A':
			{
                   lite_printf("A------READ BLOCK   \r\n");
                    ret = Dll_MfcReadBlock(cBlock, buff);
					lite_printf("read block%d,ret = %d\r\n",cBlock,ret);
	                if(ret == 0)
	                {
	                     lite_printf("Read Block%d OK\r\n",cBlock);
	                     for(i = 0; i < 16; i++)
	                     {
	                         lite_printf("%x ",buff[i]);
	                     }
	                     lite_printf("\r\n");
						 ret = Dll_MfcReadBlock(cBlock+1, buff);
						 lite_printf("read block%d,ret = %d\r\n",cBlock+1,ret);
						 if(ret == 0)
						 {
							lite_printf("Read Block%d OK\r\n",cBlock+1);
							for(i = 0; i < 16; i++)
							{
								lite_printf("%x ",buff[i]);
							}
							lite_printf("\r\n");
							
							 ret = Dll_MfcReadBlock(cBlock+2, buff);
							 lite_printf("read block%d,ret = %d\r\n",cBlock+2,ret);
							 if(ret == 0)
							 {
								lite_printf("Read Block%d OK\r\n",cBlock+2);
								for(i = 0; i < 16; i++)
								{
									lite_printf("%x ",buff[i]);
								}
								lite_printf("\r\n");
							 }
						 }
						 
	                }
			}
			break;
			case 'B':
			{
					lite_printf("B------WRITE BLOCK  \r\n");
					memset(buff, 0x11, 16);
	                ret = Dll_MfcWriteBlock(cBlock, buff);
					lite_printf("write block%d,ret = %d\r\n",cBlock,ret);
	                if(ret == 0)
	                {
	                     memset(buff, 0x22, 16);
						 ret = Dll_MfcWriteBlock(cBlock+1, buff);
						 lite_printf("write block%d,ret = %d\r\n",cBlock+1,ret);
						 if(ret == 0)
						 {
							 memset(buff, 0x33, 16);
							 ret = Dll_MfcWriteBlock(cBlock+2, buff);
							 lite_printf("write block%d,ret = %d\r\n",cBlock+2,ret);
							 
						 }
						 
	                }
			}
			break;
			case 'C':
			{
				{
					unsigned char cardtype[2];
					unsigned char serial[11];
                    lite_printf("Dll_PiccCheck...\r\n");
					if(Dll_PiccCheck(0,cardtype,serial)==0)
        			        {
        			            lite_printf("dected %c %c card\r\n",cardtype[0],cardtype[1]);
                                  //      lite_printf("ATQA: %02x %02x\r\n", g_tag_info.tag_type_bytes[0], g_tag_info.tag_type_bytes[1]);
                                         if(cardtype[0] == 'A')
                                         {
                			            lite_printf("sln:\r\n");
                			            for(i = 0; i < serial[0]; i++)
                			            {
                			                 lite_printf("%x ", serial[1 + i]);
                			            }
                			            lite_printf("\r\n");
                                  //              lite_printf("sak=%02x\r\n", g_tag_info.sak);
                                                if(cardtype[1] == 'C')
                                                {
                                                    memset(cAts_buf, 0, sizeof(cAts_buf));
                                                    ret = Dll_TypeA_GetATS(&cAts_buf);
                                                    if(ret == 0)
                                                    {
                                                        printf("ATS:");
                                                        for(i = 0; i < cAts_buf[0]; i++)
                                                        {
                                                            lite_printf("%x ", cAts_buf[i]);
                                                        }
                                                        lite_printf("\r\n");
                                                    }
                                                }
                                         }
                                         else if(cardtype[0] == 'B')
                                         {
                                                  lite_printf("ATQB:\r\n");
                			            for(i = 0; i < 13; i++)
                			            {
                			                 lite_printf("%x ", gucATQB[i]);
                			            }
                			            lite_printf("\r\n");
                                         }
        			        }
				}
			}
			break;
            case 'D':
			{
                  lite_printf("D------PICC_CARIEER RESET \r\n");
				  Dll_PiccReset();
			}
			break;
            case 'E':
                    {
                        typeB();
                     }
		       break;
		}
        lite_printf("\r\n");
		lite_printf("\r\n");
		lite_printf("press any key to exit \r\n");
           g_startTick_ms = get_tick();
	    ch = s_get_key();
           if(ch == 0x1b)
           {
                SYSCTRL_APBPeriphClockCmd( SYSCTRL_APBPeriph_TIMM0, DISABLE);
                SYSCTRL_APBPeriphResetCmd( SYSCTRL_APBPeriph_TIMM0, DISABLE);
                printf("Debug mode exited\r\n");
                return;
           }
		
    }
}

void ID_Card_Successrate_Test(void)
{
    lite_printf("ID card feature removed\r\n");
}
