/*
 * Host-side Phase2 checks (no Keil / no RF).
 * gcc -I1608c -IUser -o tools/ntag_phase2_selftest \
 *     tools/ntag_phase2_selftest.c 1608c/ntag_secrets.c
 */

#include <stdio.h>
#include <string.h>

#include "ntag.h"
#include "ntag_secrets.h"

static int g_fail;

static void expect(int cond, const char *name)
{
    if (cond)
    {
        printf("PASS  %s\n", name);
    }
    else
    {
        printf("FAIL  %s\n", name);
        g_fail = 1;
    }
}

static int pack_matches(const unsigned char *got, const unsigned char *exp)
{
    unsigned char diff;

    diff = (unsigned char)(got[0] ^ exp[0]);
    diff = (unsigned char)(diff | (got[1] ^ exp[1]));
    return (diff == 0U) ? 1 : 0;
}

int main(void)
{
    unsigned char pwd[NTAG_PWD_LEN];
    unsigned char pack[NTAG_PACK_LEN];
    unsigned char zero[NTAG_PWD_LEN];
    unsigned char ff[NTAG_PACK_LEN];
    unsigned char got[NTAG_PACK_LEN];

    printf("NTAG213 Phase2 host self-test\n");

    /* Datasheet pages, not the frozen 0x28/0x29/0x2A/0x2B sketch. */
    expect(NTAG213_PAGE_DYN_LOCK == 0x28U, "page 0x28 is dynamic lock");
    expect(NTAG213_PAGE_CFG0 == 0x29U, "AUTH0 lives in CFG0 page 0x29");
    expect(NTAG213_PAGE_CFG1 == 0x2AU, "ACCESS lives in CFG1 page 0x2A");
    expect(NTAG213_PAGE_PWD == 0x2BU, "PWD page 0x2B");
    expect(NTAG213_PAGE_PACK == 0x2CU, "PACK page 0x2C");
    expect(NTAG213_AUTH0_SCHEME_B == 0x04U, "AUTH0 Scheme B = 0x04");
    expect(NTAG213_ACCESS_SCHEME_B == 0x80U, "ACCESS PROT=1 (read+write)");
    expect((NTAG213_ACCESS_SCHEME_B & 0x80U) != 0U, "PROT bit set");
    expect((NTAG213_ACCESS_SCHEME_B & 0x07U) == 0U, "AUTHLIM disabled");

    expect(NTAG213_PROV_ORDER_0 == NTAG213_PAGE_PWD, "order 0 = PWD");
    expect(NTAG213_PROV_ORDER_1 == NTAG213_PAGE_PACK, "order 1 = PACK");
    expect(NTAG213_PROV_ORDER_2 == NTAG213_PAGE_CFG1, "order 2 = ACCESS");
    expect(NTAG213_PROV_ORDER_3 == NTAG213_PAGE_CFG0, "order 3 = AUTH0 last");
    expect(NTAG213_PROV_ORDER_3 != NTAG213_PAGE_DYN_LOCK, "AUTH0 is not page 0x28");

    expect(NTAG_OK == 0, "NTAG_OK");
    expect(NTAG_ERR_AUTH_FAIL == -200, "auth-fail code");
    expect(NTAG_ERR_UNRECOGNIZED == -201, "unrecognized code");
    expect(NTAG_ERR_PROVISION_FAIL == -202, "provision-fail code");
    expect(NTAG_ERR_AUTH_FAIL != NTAG_ERR_UNRECOGNIZED, "auth != unrecognized");
    expect(NTAG_ERR_AUTH_FAIL != NTAG_ERR_PROVISION_FAIL, "auth != provision");
    expect(PICC_NTAG_PWD_AUTH == 0x1BU, "PWD_AUTH cmd 0x1B");
    expect(PICC_NTAG_PWD_AUTH != 0x60U, "not Classic AUTH A");
    expect(PICC_NTAG_PWD_AUTH != 0x61U, "not Classic AUTH B");

    memset(zero, 0, sizeof(zero));
    ntag_secrets_get_pwd(pwd);
    ntag_secrets_get_pack(pack);
    expect(memcmp(pwd, zero, NTAG_PWD_LEN) != 0, "placeholder PWD is not all-zero");
    memset(zero, 0, sizeof(zero));
    expect(memcmp(pack, zero, NTAG_PACK_LEN) != 0, "placeholder PACK is not all-zero");

    got[0] = pack[0];
    got[1] = pack[1];
    expect(pack_matches(got, pack) == 1, "PACK match");
    ff[0] = (unsigned char)(pack[0] ^ 0xFFU);
    ff[1] = (unsigned char)(pack[1] ^ 0xFFU);
    expect(pack_matches(ff, pack) == 0, "PACK mismatch (wrong PWD analogue)");

    ntag_secrets_wipe(pwd, NTAG_PWD_LEN);
    ntag_secrets_wipe(pack, NTAG_PACK_LEN);
    expect(pwd[0] == 0U && pwd[1] == 0U && pwd[2] == 0U && pwd[3] == 0U,
           "PWD wipe");
    expect(pack[0] == 0U && pack[1] == 0U, "PACK wipe");

    expect(NTAG_PROVISION_ENABLE == 0, "field default: provision compiled out");

    if (g_fail)
    {
        printf("RESULT FAIL\n");
        return 1;
    }
    printf("RESULT PASS\n");
    return 0;
}
