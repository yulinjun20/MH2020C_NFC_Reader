# NTAG213 Phase2 anti-copy (PWD protection)

Firmware on MegaHunt MH2020C + 1608c. This is a Keil ¦ÌVision / ARMCC project.
Do not expect a full firmware image from cloud CI.

## What this phase does

Scheme B password protection for NTAG213 Type2 tags:

- One **fixed firmware PWD (4 bytes) + PACK (2 bytes)** for every reader and tag.
- Not per-device derived. Not delivered by a backend.
- Field read: `select ¡ú PWD_AUTH (0x1B) ¡ú verify PACK ¡ú Type2 NDEF read`.
- Unprotected / blank / old UID-only cards are **rejected**. No Phase1 dual-path whitelist.
- Daily field path never writes PWD / PACK / ACCESS / AUTH0.

Phase3 Originality Signature, per-device keys, and backend key delivery are out of scope.

## Placeholder secrets (must replace)

`1608c/ntag_secrets.c` holds **PLACEHOLDER_NOT_FOR_PRODUCTION** constants.

- Search the image / source for `PLACEHOLDER_NOT_FOR_PRODUCTION`.
- Replace both PWD and PACK before any factory or field flash.
- Never `printf` / log the plaintext bytes (the code does not).

Suggested production hardening (not wired in this PR):

- Store PWD/PACK in MH2020C OTP and lock the page (`mhscpu_otp.h`).
- Disable SWD / debug after programming.
- Ship a non-debug image (`NFC_DEBUG` stays 0). If `NFC_DEBUG=1`,
  `pcd_com_transceive` hex-dumps TX frames and would print the PWD.

## Datasheet vs frozen page map

NXP NTAG213/215/216 datasheet (`NTAG213_215_216`):

| Page | Role |
|------|------|
| `0x28` | Dynamic lock bytes (not AUTH0) |
| `0x29` | CFG0: MIRROR, RFUI, MIRROR_PAGE, **AUTH0** |
| `0x2A` | CFG1: **ACCESS**, RFUI, RFUI, RFUI |
| `0x2B` | **PWD** |
| `0x2C` | **PACK** + RFUI |

The frozen product sketch used `0x28/0x29/0x2A/0x2B`. That conflicts with the
datasheet, so this firmware **follows the datasheet** and keeps the frozen
**write order** (PWD ¡ú PACK ¡ú ACCESS ¡ú AUTH0 last).

ACCESS Scheme B: `PROT=1` (`0x80`) so both READ and WRITE of the protected
area need a prior PWD_AUTH. AUTH0 = `0x04` (protect from user page 4 through
config). RFUI bytes are written 0.

## Field behavior

1. ISO14443-A select (`Dll_PiccCheck` / `com_reqa`). Classic / ISO-DEP SAK ¡ú unrecognized.
2. `pcd_ntag_pwd_auth()` ¡ª command `0x1B`, **not** Classic `pcd_auth_state` / `Dll_MfcAuthenticate`.
3. Compare PACK. Mismatch ¡ú `NTAG_ERR_AUTH_FAIL` (-200), message `NTAG auth failed`.
4. If PWD_AUTH fails and page 4 is still readable ¡ú `NTAG_ERR_UNRECOGNIZED` (-201)
   (blank / old unprotected / UID-only). Message
   `NTAG unrecognized (unprotected or unsupported)`.
5. On success, existing Type2 NDEF read (`Dll_NfcReadTag`) / write (`Dll_NfcWriteTag`)
   runs. Write still does **not** touch config pages.

Host PICC commands (card already selected):

| Cmd | Path |
|-----|------|
| `0x5a` | Field text read (gated) |
| `0x5c` | Field text write (gated, NDEF only) |
| `0x5d` | Provision (fails unless `NTAG_PROVISION_ENABLE=1`) |
| `0x5e` | Unauth user-area READ probe |
| `0x5f` | Wrong-PWD PWD_AUTH probe |

## Factory provision

Compile with `NTAG_PROVISION_ENABLE=1` (in `1608c/define.h` or Keil C/C++ Define).
Field images must keep this **0** so config-write code is not compiled.

Strict order inside `ntag_provision_tag()`:

1. Select + UID (caller).
2. Optional NDEF text write (existing Type2 overwrite; tag needs an NDEF TLV).
3. Write PWD at page `0x2B`.
4. Write PACK at page `0x2C`.
5. Write ACCESS at page `0x2A`.
6. **Last** write AUTH0=`0x04` at page `0x29`.
7. PWD_AUTH self-check + PACK + READ page 4 must pass ¡ú `NTAG provision OK`.
   Any failure ¡ú `NTAG provision failed` (-202).

UART test menu `NFC_Tag2_Test()` item 3 is compiled only when provision is enabled.

### Factory steps

1. Replace placeholder PWD/PACK.
2. Build a **factory** image with `NTAG_PROVISION_ENABLE=1` on a local Keil PC.
3. Use pre-formatted NTAG213 (CC `E1h` + NDEF TLV). Truly blank tags need NDEF
   formatting first; this PR does not add a formatter.
4. Select the tag, run provision (menu 3 or cmd `0x5d`).
5. Confirm `NTAG provision OK`.
6. Power-cycle the tag (remove from field).
7. Field-read with a **field** image (`NTAG_PROVISION_ENABLE=0`).
8. Confirm unauth READ of page 4 fails; post-auth NDEF still works.

Key rotation = full re-provision of every tag plus a firmware rebuild with the
new fixed PWD/PACK. There is no online rotation.

## Keil build (local PC)

1. Open `Project.uvproj` in ¦ÌVision (ARMCC V5).
2. Confirm group **MH1608C** includes `ntag.c`, `ntag_secrets.c`, `ntag_test.c`.
3. Include path already contains `.\1608c`.
4. Field build: `NTAG_PROVISION_ENABLE` left 0.
5. Factory build: add `NTAG_PROVISION_ENABLE=1` to C/C++ Define, or set it in
   `define.h`.
6. Rebuild Target **Debug**. Cloud / CI is not required and cannot run ARMCC.

## Hardware test hooks (T6)

| Case | How | Expected |
|------|-----|----------|
| Blank tag | Field read (`0x5a` / menu 1) | `-201` unrecognized |
| Old unprotected / UID-only | Field read | `-201` unrecognized |
| Wrong PWD | Menu 5 / cmd `0x5f` (inverts firmware PWD, never prints it) | probe OK; PWD_AUTH rejected |
| Good provisioned card | Field read after PWD_AUTH | `0`, NDEF works |
| Unauth user-area READ | Menu 4 / cmd `0x5e` (`pcd_read` page 4, no PWD_AUTH) | must fail on Scheme B |
| Post-auth NDEF | Field read / write after successful PWD_AUTH | still works |

Host-side constant checks (no RF hardware):

```
gcc -I1608c -IUser -o tools/ntag_phase2_selftest tools/ntag_phase2_selftest.c 1608c/ntag_secrets.c
./tools/ntag_phase2_selftest
```

## Modules

| File | Role |
|------|------|
| `1608c/ntag.c` / `ntag.h` | `pcd_ntag_pwd_auth`, field gate, provision, error codes |
| `1608c/ntag_secrets.c` / `.h` | Placeholder PWD/PACK |
| `1608c/ntag_test.c` / `.h` | UART / host probes |
| `1608c/nfc_type2.h` | Existing `Dll_Nfc*` prototypes |
| `1608c/mhtest.c` | Gate on `Dll_NfcReadTag` / `Dll_NfcWriteTag` only |

Classic MF1 auth stays in `mifare.c` / `Dll_MfcAuthenticate`.
