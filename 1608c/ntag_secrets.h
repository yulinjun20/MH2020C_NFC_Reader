/**
 ****************************************************************
 * @file ntag_secrets.h
 *
 * @brief Fixed firmware NTAG213 PWD/PACK accessors (Phase2).
 *
 * Single firmware-wide secret pair. NOT per-device, NOT backend-derived.
 * Production must replace the PLACEHOLDER values in ntag_secrets.c.
 * Never log or printf the plaintext secret bytes.
 *
 ****************************************************************
 */
#ifndef NTAG_SECRETS_H
#define NTAG_SECRETS_H

#define NTAG_PWD_LEN   4
#define NTAG_PACK_LEN  2

/*
 * Copy the firmware PWD (4 bytes) or PACK (2 bytes) into caller buffers.
 * Callers must wipe the buffers with ntag_secrets_wipe() after use.
 */
void ntag_secrets_get_pwd(unsigned char *out4);
void ntag_secrets_get_pack(unsigned char *out2);

/* Wipe a local secret copy. Does not print. */
void ntag_secrets_wipe(unsigned char *p, unsigned int n);

#endif
