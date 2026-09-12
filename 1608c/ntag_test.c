/**
 ****************************************************************
 * @file ntag_test.c
 *
 * @brief Phase2 hardware test hooks. No secret logging.
 *
 * Covers (see docs/NTAG213_PHASE2.md):
 *   blank / old unprotected -> NTAG_ERR_UNRECOGNIZED
 *   wrong PWD               -> NTAG_ERR_AUTH_FAIL
 *   good card               -> field read NDEF after PWD_AUTH
 *   unauth user-area READ   -> must fail after provision
 *   post-auth NDEF          -> still works
 *
 ****************************************************************
 */

#include <stdio.h>
#include <string.h>

#include "user.h"
#include "picc_api.h"
#include "rfid.h"
#include "mifare.h"
#include "mh523.h"
#include "ntag.h"
#include "ntag_secrets.h"
#include "ntag_test.h"
#include "nfc_type2.h"
#include "timer.h"

extern tag_info XDATA g_tag_info;

static int ntag_test_wait_typea(unsigned char *cardtype, unsigned char *serial)
{
    int check_ret;

    memset(cardtype, 0, 2);
    memset(serial, 0, 50);
    check_ret = Dll_PiccCheck(0, cardtype, serial);
    return check_ret;
}

static void ntag_test_print_uid(const unsigned char *serial)
{
    int i;

    printf("sn:");
    for (i = 0; i < serial[0]; i++)
    {
        printf("%02x ", serial[1 + i]);
    }
    printf("\r\n");
    printf("ATQA: %02x %02x SAK: %02x\r\n",
           g_tag_info.tag_type_bytes[0],
           g_tag_info.tag_type_bytes[1],
           g_tag_info.sak);
}

void TestNtagPhase2FieldRead(void)
{
    unsigned char cardtype[2];
    unsigned char serial[50];
    unsigned char buff[1024];
    uint16_t ndef_len;
    int iRet;

    Dll_PiccOpen();
    printf("NTAG Phase2 field read: put card\r\n");

    while (1)
    {
        if (ntag_test_wait_typea(cardtype, serial) == 0)
        {
            ntag_test_print_uid(serial);
            ndef_len = 0;
            memset(buff, 0, sizeof(buff));
            iRet = Dll_NfcReadTag(buff, &ndef_len);
            printf("field read iRet=%d (%s) len=%u\r\n",
                   iRet, ntag_err_str(iRet), (unsigned int)ndef_len);
            break;
        }
        printf("waiting card\r\n");
        Lib_DelayMs(300);
    }

    Dll_PiccClose();
}

#if NTAG_PROVISION_ENABLE
void TestNtagPhase2Provision(void)
{
    unsigned char cardtype[2];
    unsigned char serial[50];
    int iRet;
    static const unsigned char def_text[] = "NTAG-P2";

    Dll_PiccOpen();
    printf("NTAG Phase2 provision: put blank or re-provision card\r\n");

    while (1)
    {
        if (ntag_test_wait_typea(cardtype, serial) == 0)
        {
            ntag_test_print_uid(serial);
            iRet = ntag_provision_tag(def_text, (unsigned short)(sizeof(def_text) - 1U));
            printf("provision iRet=%d (%s)\r\n", iRet, ntag_err_str(iRet));
            break;
        }
        printf("waiting card\r\n");
        Lib_DelayMs(300);
    }

    Dll_PiccClose();
}
#endif

int ntag_test_unauth_user_read(void)
{
    unsigned char buf[16];
    int status;

    status = pcd_read(NTAG213_USER_START_PAGE, buf);
    ntag_secrets_wipe(buf, (unsigned int)sizeof(buf));
    if (status == MI_OK)
    {
        printf("unauth user-area READ succeeded (tag is NOT Scheme B protected)\r\n");
        return NTAG_ERR_UNRECOGNIZED;
    }
    printf("unauth user-area READ failed as required (status=%d)\r\n", status);
    return NTAG_OK;
}

int ntag_test_wrong_pwd(void)
{
    unsigned char pwd[NTAG_PWD_LEN];
    unsigned char pack[NTAG_PACK_LEN];
    unsigned int i;
    int status;

    ntag_secrets_get_pwd(pwd);
    for (i = 0U; i < NTAG_PWD_LEN; i++)
    {
        pwd[i] = (unsigned char)(pwd[i] ^ 0xFFU);
    }

    pack[0] = 0U;
    pack[1] = 0U;
    status = pcd_ntag_pwd_auth(pwd, pack);
    ntag_secrets_wipe(pwd, NTAG_PWD_LEN);
    ntag_secrets_wipe(pack, NTAG_PACK_LEN);

    if (status == MI_OK)
    {
        printf("%s\r\n", ntag_err_str(NTAG_ERR_AUTH_FAIL));
        return NTAG_ERR_AUTH_FAIL;
    }
    printf("wrong-PWD PWD_AUTH rejected as required\r\n");
    return NTAG_OK;
}

void TestNtagPhase2UnauthRead(void)
{
    unsigned char cardtype[2];
    unsigned char serial[50];
    int iRet;

    Dll_PiccOpen();
    printf("NTAG unauth user-area probe: put card\r\n");

    while (1)
    {
        if (ntag_test_wait_typea(cardtype, serial) == 0)
        {
            ntag_test_print_uid(serial);
            iRet = ntag_test_unauth_user_read();
            printf("unauth probe iRet=%d (%s)\r\n", iRet, ntag_err_str(iRet));
            break;
        }
        Lib_DelayMs(300);
    }

    Dll_PiccClose();
}

void TestNtagPhase2WrongPwd(void)
{
    unsigned char cardtype[2];
    unsigned char serial[50];
    int iRet;

    Dll_PiccOpen();
    printf("NTAG wrong-PWD probe: put card\r\n");

    while (1)
    {
        if (ntag_test_wait_typea(cardtype, serial) == 0)
        {
            ntag_test_print_uid(serial);
            iRet = ntag_test_wrong_pwd();
            printf("wrong-PWD probe iRet=%d (%s)\r\n", iRet, ntag_err_str(iRet));
            break;
        }
        Lib_DelayMs(300);
    }

    Dll_PiccClose();
}
