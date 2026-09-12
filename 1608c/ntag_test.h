/**
 ****************************************************************
 * @file ntag_test.h
 *
 * @brief Phase2 hardware test hooks (UART menu / host commands).
 *
 ****************************************************************
 */
#ifndef NTAG_TEST_H
#define NTAG_TEST_H

#include "ntag.h"

/* Field read path: select -> PWD_AUTH -> Type2 NDEF read. */
void TestNtagPhase2FieldRead(void);

#if NTAG_PROVISION_ENABLE
/* Factory path only. Writes config/keys; never used on the daily field path. */
void TestNtagPhase2Provision(void);
#endif

/* Probe READ of user page 4 without PWD_AUTH. Must fail on a provisioned tag. */
int ntag_test_unauth_user_read(void);

/* PWD_AUTH with inverted firmware PWD. Must fail; does not print secrets. */
int ntag_test_wrong_pwd(void);

void TestNtagPhase2UnauthRead(void);
void TestNtagPhase2WrongPwd(void);

#endif
