/**
 ****************************************************************
 * @file nfc_type2.h
 *
 * @brief Existing Type2 NDEF helpers implemented in mhtest.c.
 *
 ****************************************************************
 */
#ifndef NFC_TYPE2_H
#define NFC_TYPE2_H

#include "user.h"

int Dll_NfcReadTag(unsigned char *buf, uint16_t *rlen);
int Dll_NfcWriteTag(const unsigned char *text, uint16_t text_len);
int Dll_NfcParseNdefToText(unsigned char *ndef, uint16_t ndef_len,
                           char *out, uint16_t out_max, uint16_t *out_len);
int Dll_NfcWriteTextToTag(const unsigned char *text, uint16_t text_len);
int Dll_NfcReadTextFromTag(char *buf, uint16_t buf_max, uint16_t *out_len);

void NFC_Tag2_Test(void);

#endif
