#include <stdio.h>
#include <string.h>
#include "mhscpu.h"

#include "commandpro.h"

#include "user.h"
#include "picc_api.h"

extern unsigned int g_RF_Module;

int Dll_PiccOpen(void)
{
    return MH1608_Picc_Open();
}

int Dll_PiccClose(void)
{
    return MH1608_Picc_Close();
}


int Dll_PiccCheck(unsigned char  mode,unsigned char  *cardtype,unsigned char  *serialno)
{
    return MH1608_Picc_Check(mode, cardtype, serialno);
}

int Dll_PiccGetInformation(uint8_t *info)
{
    (void)info;
    return -1000;
}


int Dll_PiccCommand(APDU_SEND *ApduSend, APDU_RESP *ApduResp) 
{
     return MH1608_Picc_Command(ApduSend, ApduResp);
}

int Dll_PiccApdu(unsigned char *tx_buf, unsigned int tx_len, unsigned char *rx_buf, unsigned int *rx_len)
{
       return MH1608_Picc_Apdu(tx_buf,  tx_len, rx_buf,  rx_len);
}

void Dll_PiccReset(void)
{
     MH1608_Picc_Reset();
}

int Dll_TypeA_GetATS(unsigned char *ATS)
{
     return MH1608_TypeA_GetATS(ATS);
}

int Dll_PiccHaltA(void)
{
    return MH1608_Picc_HaltA();
}

int Dll_PiccAntennaOff(void)
{
    return pcd_antenna_off();
}

int Dll_PiccAntennaOn(void)
{
    return pcd_antenna_on();
}


void Pcd_ConfigTypeB_Mode(unsigned char rf_onoff)
{
     if(rf_onoff)
     {
          pcd_config('B');
     }
     else
     {
           pcd_config2('B');
     }
     pcd_set_rate('1');
}

int SendFrame(unsigned char *pid,int inlen,int *outlen)
{
       return MH1608_SendFrame(pid, inlen, outlen);
}

int SendFrame2(unsigned char *pid,int inlen,int *outlen)
{
       return MH1608_SendFrame2(pid, inlen, outlen);
}


int Dll_MfcAuthenticate(uint8_t cType, uint16_t BlkNo, uint8_t *pKey, uint8_t *pSerialNo)
{
          return MH1608_Mfc_Authenticate(cType,  BlkNo, pKey, pSerialNo);
}

int Dll_MfcReadBlock(uint16_t BlkNo, uint8_t *BlkValue)
{
          return MH1608_Mfc_ReadBlock(BlkNo, BlkValue);
}

int Dll_MfcWriteBlock(uint16_t BlkNo, uint8_t *BlkValue)
{
      return MH1608_Mfc_WriteBlock(BlkNo, BlkValue);
}

int Dll_MfcOperate(uint8_t mode,uint16_t BlkNo,uint8_t *Value,uint8_t UpdateBlkNo)
{
      return MH1608_Mfc_Operate( mode, BlkNo, Value, UpdateBlkNo);
}

int Dll_MfcIncrement(uint8_t BlkNo,uint8_t *Value)
{
      return MH1608_Mfc_Increment(BlkNo, Value);
}

int Dll_MfcDecrement(uint8_t BlkNo,uint8_t *Value)
{
      return MH1608_Mfc_Decrement(BlkNo, Value);
}

int Dll_MfcRestore(uint8_t BlkNo)
{
      return MH1608_Mfc_Restore(BlkNo);
}

int Dll_MfcTransfer(uint8_t BlkNo)
{
      return MH1608_Mfc_Transfer(BlkNo);
}

int Dll_PiccReqa(uint8_t *atqa)
{
    (void)atqa;
    return -1000;
}

int Dll_PiccReqb(uint8_t *atqb)
{
    (void)atqb;
    return -1000;
}

int Dll_PiccWupa(uint8_t *atqa)
{
    (void)atqa;
    return -1000;
}

int Dll_PiccWupb(uint8_t *atqb)
{
    (void)atqb;
    return -1000;
}

int Dll_PiccAnticoll_A(uint8_t *UidSize, uint8_t *uidlen, uint8_t *Uid, uint8_t *Sak)
{
    (void)UidSize;
    (void)uidlen;
    (void)Uid;
    (void)Sak;
    return -1000;
}

int Dll_PiccAnticoll_B(uint8_t *pupi)
{
    (void)pupi;
    return -1000;
}

int Dll_PiccRats(uint8_t *atslen,uint8_t *Ats)
{
    (void)atslen;
    (void)Ats;
    return -1000;
}

int Dll_PiccAttrib(uint8_t *atslen,uint8_t *AttribResponse)
{
    (void)atslen;
    (void)AttribResponse;
    return -1000;
}


int Dll_ID_Command(unsigned char *data_in, uint16_t dataInLen,unsigned char *data_out, uint16_t *dataOutLen)
{
     return MH1608_ID_Command(data_in, dataInLen, data_out, dataOutLen);
}

int Dll_PiccRemove(uint8_t isLOOP)
{
         return MH1608_Picc_Remove(isLOOP);
}
