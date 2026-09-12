/**
 ****************************************************************
 * @file ntag.c
 *
 * @brief NTAG213 PWD_AUTH primitive, field gate, and factory provision.
 *
 * MIFARE Classic Crypto1 (pcd_auth_state) is intentionally unused here.
 *
 ****************************************************************
 */

#include <stdio.h>

#include "mh523.h"
#include "mifare.h"
#include "rfid.h"
#include "ntag.h"
#include "ntag_secrets.h"
#if NTAG_PROVISION_ENABLE
#include "nfc_type2.h"
#endif

extern tag_info XDATA g_tag_info;

static int s_provision_active = 0;

unsigned char ntag_access_byte_scheme_b(void)
{
    return NTAG213_ACCESS_SCHEME_B;
}

unsigned char ntag_auth0_scheme_b(void)
{
    return NTAG213_AUTH0_SCHEME_B;
}

int ntag_provision_is_active(void)
{
    return s_provision_active;
}

int ntag_pack_matches(const unsigned char *got, const unsigned char *exp)
{
    unsigned char diff;

    if ((got == 0) || (exp == 0))
    {
        return 0;
    }
    diff = (unsigned char)(got[0] ^ exp[0]);
    diff = (unsigned char)(diff | (got[1] ^ exp[1]));
    return (diff == 0U) ? 1 : 0;
}

const char *ntag_err_str(int err)
{
    if (err == NTAG_OK)
    {
        return "NTAG OK";
    }
    if (err == NTAG_ERR_AUTH_FAIL)
    {
        return "NTAG auth failed";
    }
    if (err == NTAG_ERR_UNRECOGNIZED)
    {
        return "NTAG unrecognized (unprotected or unsupported)";
    }
    if (err == NTAG_ERR_PROVISION_FAIL)
    {
        return "NTAG provision failed";
    }
    return "NTAG error";
}

/**
 * pcd_ntag_pwd_auth
 * Transceive 0x1B + PWD[4] with CRC. Success response is PACK (16 bits).
 *
 * Keep NFC_DEBUG=0 on field images: pcd_com_transceive hex-dumps TX bytes
 * and would otherwise print the PWD.
 */
int pcd_ntag_pwd_auth(const unsigned char *pwd, unsigned char *pack_out)
{
    int status;
    transceive_buffer XDATA *pi;

    if ((pwd == 0) || (pack_out == 0))
    {
        return MI_WRONG_PARAMETER_VALUE;
    }

    pi = &mf_com_data;

    write_reg(BitFramingReg, 0x00);
    set_bit_mask(TxModeReg, BIT7);  /* Tx CRC on */
    set_bit_mask(RxModeReg, BIT7);  /* Rx CRC on (PACK + CRC) */
    pcd_set_tmo(5);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length = 5;
    mf_com_data.mf_data[0] = PICC_NTAG_PWD_AUTH;
    mf_com_data.mf_data[1] = pwd[0];
    mf_com_data.mf_data[2] = pwd[1];
    mf_com_data.mf_data[3] = pwd[2];
    mf_com_data.mf_data[4] = pwd[3];

    status = pcd_com_transceive(pi);
    if (status == MI_OK)
    {
        /* PACK is 2 bytes = 16 bits after CRC strip */
        if (mf_com_data.mf_length != 0x10)
        {
            status = MI_BITCOUNTERR;
        }
        else
        {
            pack_out[0] = mf_com_data.mf_data[0];
            pack_out[1] = mf_com_data.mf_data[1];
        }
    }

    return status;
}

/* Classic / ISO-DEP are not NTAG Type2. */
static int ntag_picc_is_type2(void)
{
    if (g_tag_info.sak & 0x08U)
    {
        return 0;
    }
    if (g_tag_info.sak & 0x20U)
    {
        return 0;
    }
    return 1;
}

/*
 * After a failed PWD_AUTH, an unprotected tag still allows READ of page 4.
 * A protected tag with the wrong PWD NAKs that READ.
 */
static int ntag_user_page_readable_unauth(void)
{
    unsigned char buf[16];
    int status;

    status = pcd_read(NTAG213_USER_START_PAGE, buf);
    ntag_secrets_wipe(buf, (unsigned int)sizeof(buf));
    return (status == MI_OK) ? 1 : 0;
}

int ntag_field_authenticate(void)
{
    unsigned char pwd[NTAG_PWD_LEN];
    unsigned char pack_got[NTAG_PACK_LEN];
    unsigned char pack_exp[NTAG_PACK_LEN];
    int status;
    int match;

    pack_got[0] = 0U;
    pack_got[1] = 0U;

    if (!ntag_picc_is_type2())
    {
        printf("%s\r\n", ntag_err_str(NTAG_ERR_UNRECOGNIZED));
        return NTAG_ERR_UNRECOGNIZED;
    }

    ntag_secrets_get_pwd(pwd);
    status = pcd_ntag_pwd_auth(pwd, pack_got);
    ntag_secrets_wipe(pwd, NTAG_PWD_LEN);

    if (status != MI_OK)
    {
        if (ntag_user_page_readable_unauth())
        {
            printf("%s\r\n", ntag_err_str(NTAG_ERR_UNRECOGNIZED));
            return NTAG_ERR_UNRECOGNIZED;
        }
        printf("%s\r\n", ntag_err_str(NTAG_ERR_AUTH_FAIL));
        return NTAG_ERR_AUTH_FAIL;
    }

    ntag_secrets_get_pack(pack_exp);
    match = ntag_pack_matches(pack_got, pack_exp);
    ntag_secrets_wipe(pack_exp, NTAG_PACK_LEN);
    ntag_secrets_wipe(pack_got, NTAG_PACK_LEN);

    if (!match)
    {
        printf("%s\r\n", ntag_err_str(NTAG_ERR_AUTH_FAIL));
        return NTAG_ERR_AUTH_FAIL;
    }

    return NTAG_OK;
}

#if NTAG_PROVISION_ENABLE

static int ntag_write_cfg_page(unsigned char page, const unsigned char *data4)
{
    return pcd_write_ultralight(page, (u8 *)data4);
}

static int ntag_provision_selfcheck(void)
{
    unsigned char pwd[NTAG_PWD_LEN];
    unsigned char pack_got[NTAG_PACK_LEN];
    unsigned char pack_exp[NTAG_PACK_LEN];
    unsigned char user[16];
    int status;
    int match;

    ntag_secrets_get_pwd(pwd);
    status = pcd_ntag_pwd_auth(pwd, pack_got);
    ntag_secrets_wipe(pwd, NTAG_PWD_LEN);
    if (status != MI_OK)
    {
        return NTAG_ERR_PROVISION_FAIL;
    }

    ntag_secrets_get_pack(pack_exp);
    match = ntag_pack_matches(pack_got, pack_exp);
    ntag_secrets_wipe(pack_exp, NTAG_PACK_LEN);
    ntag_secrets_wipe(pack_got, NTAG_PACK_LEN);
    if (!match)
    {
        return NTAG_ERR_PROVISION_FAIL;
    }

    status = pcd_read(NTAG213_USER_START_PAGE, user);
    ntag_secrets_wipe(user, (unsigned int)sizeof(user));
    if (status != MI_OK)
    {
        return NTAG_ERR_PROVISION_FAIL;
    }

    return NTAG_OK;
}

int ntag_provision_tag(const unsigned char *text, unsigned short text_len)
{
    unsigned char pwd[NTAG_PWD_LEN];
    unsigned char pack[4];
    unsigned char cfg0[16];
    unsigned char cfg1[4];
    unsigned char wr[4];
    int status;
    int ndef_ret;

    if (!ntag_picc_is_type2())
    {
        printf("%s\r\n", ntag_err_str(NTAG_ERR_UNRECOGNIZED));
        return NTAG_ERR_UNRECOGNIZED;
    }

    /*
     * Already-protected cards must authenticate before config writes.
     * Blank / old unprotected cards stay writable without PWD_AUTH.
     * Do not treat that unprotected state as a field-path "unrecognized" reject.
     */
    if (!ntag_user_page_readable_unauth())
    {
        status = ntag_field_authenticate();
        if (status != NTAG_OK)
        {
            printf("%s\r\n", ntag_err_str(NTAG_ERR_PROVISION_FAIL));
            return NTAG_ERR_PROVISION_FAIL;
        }
    }

    s_provision_active = 1;

    /* Optional NDEF (existing Type2 overwrite; tag must already have NDEF TLV). */
    if ((text != 0) && (text_len > 0U))
    {
        ndef_ret = Dll_NfcWriteTextToTag(text, text_len);
        if (ndef_ret != 0)
        {
            s_provision_active = 0;
            printf("%s\r\n", ntag_err_str(NTAG_ERR_PROVISION_FAIL));
            return NTAG_ERR_PROVISION_FAIL;
        }
    }

    /* 1) PWD at page 0x2B */
    ntag_secrets_get_pwd(pwd);
    status = ntag_write_cfg_page(NTAG213_PAGE_PWD, pwd);
    ntag_secrets_wipe(pwd, NTAG_PWD_LEN);
    if (status != MI_OK)
    {
        s_provision_active = 0;
        printf("%s\r\n", ntag_err_str(NTAG_ERR_PROVISION_FAIL));
        return NTAG_ERR_PROVISION_FAIL;
    }

    /* 2) PACK at page 0x2C (RFUI bytes written 0 per datasheet) */
    pack[0] = 0U;
    pack[1] = 0U;
    pack[2] = 0U;
    pack[3] = 0U;
    ntag_secrets_get_pack(pack);
    status = ntag_write_cfg_page(NTAG213_PAGE_PACK, pack);
    ntag_secrets_wipe(pack, (unsigned int)sizeof(pack));
    if (status != MI_OK)
    {
        s_provision_active = 0;
        printf("%s\r\n", ntag_err_str(NTAG_ERR_PROVISION_FAIL));
        return NTAG_ERR_PROVISION_FAIL;
    }

    /* 3) ACCESS at page 0x2A: PROT=1, RFUI=0, AUTHLIM=0 */
    cfg1[0] = ntag_access_byte_scheme_b();
    cfg1[1] = 0U;
    cfg1[2] = 0U;
    cfg1[3] = 0U;
    status = ntag_write_cfg_page(NTAG213_PAGE_CFG1, cfg1);
    if (status != MI_OK)
    {
        s_provision_active = 0;
        printf("%s\r\n", ntag_err_str(NTAG_ERR_PROVISION_FAIL));
        return NTAG_ERR_PROVISION_FAIL;
    }

    /* 4) LAST: AUTH0=0x04 in CFG0 page 0x29 (RMW, keep MIRROR fields) */
    status = pcd_read(NTAG213_PAGE_CFG0, cfg0);
    if (status != MI_OK)
    {
        s_provision_active = 0;
        printf("%s\r\n", ntag_err_str(NTAG_ERR_PROVISION_FAIL));
        return NTAG_ERR_PROVISION_FAIL;
    }
    wr[0] = cfg0[0];
    wr[1] = cfg0[1];
    wr[2] = cfg0[2];
    wr[3] = ntag_auth0_scheme_b();
    status = ntag_write_cfg_page(NTAG213_PAGE_CFG0, wr);
    if (status != MI_OK)
    {
        s_provision_active = 0;
        printf("%s\r\n", ntag_err_str(NTAG_ERR_PROVISION_FAIL));
        return NTAG_ERR_PROVISION_FAIL;
    }

    s_provision_active = 0;

    /* PWD_AUTH self-check is required for provision success. */
    if (ntag_provision_selfcheck() != NTAG_OK)
    {
        printf("%s\r\n", ntag_err_str(NTAG_ERR_PROVISION_FAIL));
        return NTAG_ERR_PROVISION_FAIL;
    }

    printf("NTAG provision OK\r\n");
    return NTAG_OK;
}

#else /* !NTAG_PROVISION_ENABLE */

int ntag_provision_tag(const unsigned char *text, unsigned short text_len)
{
    (void)text;
    (void)text_len;
    printf("%s\r\n", ntag_err_str(NTAG_ERR_PROVISION_FAIL));
    return NTAG_ERR_PROVISION_FAIL;
}

#endif /* NTAG_PROVISION_ENABLE */
