/**
 ****************************************************************
 * @file ntag.h
 *
 * @brief NTAG213 Type2 PWD_AUTH (Phase2). Isolated from MIFARE Classic.
 *
 * Do NOT call pcd_auth_state() / Dll_MfcAuthenticate for NTAG.
 * NTAG PWD_AUTH is ISO14443-3 command 0x1B and is implemented in ntag.c.
 *
 * NXP NTAG213/215/216 datasheet (NTAG213_215_216):
 *   page 28h : dynamic lock bytes          (NOT AUTH0)
 *   page 29h : CFG0  (MIRROR, RFUI, MIRROR_PAGE, AUTH0)
 *   page 2Ah : CFG1  (ACCESS, RFUI, RFUI, RFUI)
 *   page 2Bh : PWD
 *   page 2Ch : PACK + RFUI
 *
 * Frozen product plan used 0x28/0x29/0x2A/0x2B. That map does not match
 * the NTAG213 datasheet, so this firmware follows the datasheet addresses
 * and keeps the frozen write ORDER (PWD -> PACK -> ACCESS -> AUTH0 last).
 *
 ****************************************************************
 */
#ifndef NTAG_H
#define NTAG_H

#include "define.h"

/* NTAG213 Type2 PWD_AUTH command (not Classic 0x60/0x61). */
#define PICC_NTAG_PWD_AUTH          0x1BU

/* Datasheet configuration pages (NTAG213). */
#define NTAG213_PAGE_DYN_LOCK       0x28U
#define NTAG213_PAGE_CFG0           0x29U
#define NTAG213_PAGE_CFG1           0x2AU
#define NTAG213_PAGE_PWD            0x2BU
#define NTAG213_PAGE_PACK           0x2CU

#define NTAG213_CFG0_OFF_AUTH0      3U
#define NTAG213_CFG1_OFF_ACCESS     0U

/* Scheme B: protect user memory + config from page 4. */
#define NTAG213_AUTH0_SCHEME_B      0x04U

/*
 * ACCESS byte (CFG1 byte 0), NXP Table "ACCESS configuration byte":
 *   bit7 PROT     1 = read AND write of protected area need PWD_AUTH
 *   bit6 CFGLCK   0 = config still writable (re-provision possible)
 *   bit5 RFUI     write 0
 *   bit4 NFC_CNT_EN
 *   bit3 NFC_CNT_PWD_PROT
 *   bit2-0 AUTHLIM 000b = attempt limit disabled
 */
#define NTAG213_ACCESS_PROT         0x80U
#define NTAG213_ACCESS_SCHEME_B     (NTAG213_ACCESS_PROT)

/* First user-memory page (also first protected page under Scheme B). */
#define NTAG213_USER_START_PAGE     0x04U

/* Strict provision page order after optional NDEF: PWD, PACK, ACCESS, AUTH0 last. */
#define NTAG213_PROV_ORDER_0        NTAG213_PAGE_PWD
#define NTAG213_PROV_ORDER_1        NTAG213_PAGE_PACK
#define NTAG213_PROV_ORDER_2        NTAG213_PAGE_CFG1
#define NTAG213_PROV_ORDER_3        NTAG213_PAGE_CFG0

/* Distinct Phase2 result codes (do not reuse Classic MI_AUTHERR for NTAG). */
#define NTAG_OK                     0
#define NTAG_ERR_AUTH_FAIL          (-200) /* PWD_AUTH NAK or PACK mismatch */
#define NTAG_ERR_UNRECOGNIZED       (-201) /* blank / unprotected / not Type2 */
#define NTAG_ERR_PROVISION_FAIL     (-202)

#ifndef NTAG_PROVISION_ENABLE
#define NTAG_PROVISION_ENABLE       0
#endif

/*
 * pcd_ntag_pwd_auth
 * Send PWD_AUTH (0x1B + 4-byte PWD). On success write 2-byte PACK to pack_out.
 * Isolated from pcd_auth_state (Classic Crypto1).
 *
 * @return MI_OK on a 2-byte PACK response; other MI_* on RF/NAK/timeout.
 */
int pcd_ntag_pwd_auth(const unsigned char *pwd, unsigned char *pack_out);

/* Constant-time PACK compare. Returns 1 if equal, 0 otherwise. */
int ntag_pack_matches(const unsigned char *got, const unsigned char *exp);

/* ACCESS / AUTH0 helpers used by provision and the host self-test. */
unsigned char ntag_access_byte_scheme_b(void);
unsigned char ntag_auth0_scheme_b(void);

/*
 * Field path: select is already done. PWD_AUTH + PACK verify.
 * No config/key writes.
 * Unprotected / blank / UID-only cards are rejected (no Phase1 fallback).
 */
int ntag_field_authenticate(void);

/* Non-zero while ntag_provision_tag() is writing NDEF (skip field auth). */
int ntag_provision_is_active(void);

/*
 * Factory / lab provision. Compiled when NTAG_PROVISION_ENABLE==1.
 * Write order (strict):
 *   select+UID (caller) -> optional NDEF -> PWD -> PACK -> ACCESS -> AUTH0 last
 *   -> PWD_AUTH self-check must pass.
 * Daily field path must never call this.
 */
int ntag_provision_tag(const unsigned char *text, unsigned short text_len);

/* Printable status; never includes PWD/PACK bytes. */
const char *ntag_err_str(int err);

#endif
