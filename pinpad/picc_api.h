#ifndef _PICC_API_H_
#define _PICC_API_H_

int Dll_PiccOpen(void);
int Dll_PiccClose(void);
int Dll_PiccCheck(unsigned char  mode,unsigned char  *cardtype,unsigned char  *serialno);
int Dll_PiccCommand(APDU_SEND *ApduSend, APDU_RESP *ApduResp);
int Dll_PiccApdu(unsigned char *tx_buf, unsigned int tx_len, unsigned char *rx_buf, unsigned int *rx_len);
void Dll_PiccReset(void);
int Dll_TypeA_GetATS(unsigned char *ATS);
int Dll_PiccHaltA(void);





#endif

