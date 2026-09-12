/**
 ****************************************************************
 * @file ntag_secrets.c
 *
 * @brief Fixed NTAG213 PWD/PACK constants for Phase2 anti-copy.
 *
 * PLACEHOLDER_NOT_FOR_PRODUCTION
 * Replace NTAG_PLACEHOLDER_PWD / NTAG_PLACEHOLDER_PACK before factory flash.
 * Do not derive per device. Do not fetch from a backend in this phase.
 *
 * Production hardening (not wired here):
 *  - Move these bytes into MH2020C OTP and lock the OTP page.
 *  - Disable SWD / debug after programming.
 *  - Do not leave a debug printf build in the field.
 *
 ****************************************************************
 */

#include "ntag_secrets.h"

/*
 * PLACEHOLDER_NOT_FOR_PRODUCTION ¡ª conspicuous test values.
 * Production firmware MUST replace both arrays before any field deployment.
 * These values are intentionally obvious so a leftover placeholder is visible
 * in a hex dump of the image (search for the label above).
 */
static const unsigned char NTAG_PLACEHOLDER_PWD[NTAG_PWD_LEN] = {
    0xA5U, 0x5AU, 0xC3U, 0x3CU
};

static const unsigned char NTAG_PLACEHOLDER_PACK[NTAG_PACK_LEN] = {
    0xB1U, 0x2CU
};

void ntag_secrets_get_pwd(unsigned char *out4)
{
    unsigned int i;

    if (out4 == 0)
    {
        return;
    }
    for (i = 0U; i < NTAG_PWD_LEN; i++)
    {
        out4[i] = NTAG_PLACEHOLDER_PWD[i];
    }
}

void ntag_secrets_get_pack(unsigned char *out2)
{
    unsigned int i;

    if (out2 == 0)
    {
        return;
    }
    for (i = 0U; i < NTAG_PACK_LEN; i++)
    {
        out2[i] = NTAG_PLACEHOLDER_PACK[i];
    }
}

void ntag_secrets_wipe(unsigned char *p, unsigned int n)
{
    unsigned int i;

    if (p == 0)
    {
        return;
    }
    for (i = 0U; i < n; i++)
    {
        p[i] = 0U;
    }
}
