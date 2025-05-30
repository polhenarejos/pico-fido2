/*
 * This file is part of the Pico FIDO2 distribution (https://github.com/polhenarejos/pico-fido2).
 * Copyright (c) 2025 Pol Henarejos.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "fido/files.h"
#undef _FILES_H_
#include "openpgp/files.h"

extern const uint8_t openpgp_aid[];
extern const uint8_t openpgp_aid_full[];

#define ACL_NONE    { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
#define ACL_ALL     { 0 }
#define ACL_RO      { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 }
#define ACL_RW      { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00 }
#define ACL_R_WP    { 0xff, 0xff, 0xff, 0xff, 0x90, 0x90, 0x00 }
#define ACL_WP      { 0xff, 0xff, 0xff, 0xff, 0x90, 0x90, 0xff }

extern int parse_ch_data(const file_t *f, int mode);
extern int parse_sec_tpl(const file_t *f, int mode);
extern int parse_ch_cert(const file_t *f, int mode);
extern int parse_gfm(const file_t *f, int mode);
extern int parse_fp(const file_t *f, int mode);
extern int parse_cafp(const file_t *f, int mode);
extern int parse_ts(const file_t *f, int mode);
extern int parse_keyinfo(const file_t *f, int mode);
extern int parse_algoinfo(const file_t *f, int mode);
extern int parse_app_data(const file_t *f, int mode);
extern int parse_discrete_do(const file_t *f, int mode);
extern int parse_pw_status(const file_t *f, int mode);
extern int piv_parse_discovery(const file_t *f);

uint8_t historical_bytes[] = {
    10, 0,
    0x00,
    0x31, 0x84,         /* Full DF name, GET DATA, MF */
    0x73,
    0x80, 0x01, 0xC0,   /* Full DF name */
    /* 1-byte */
    /* Command chaining, No extended Lc and Le */
    0x05,
    0x90, 0x00          /* Status info */
};

uint8_t extended_capabilities[] = {
    10, 0,
    0x77,           /*
                     * No Secure Messaging supported
                     * GET CHALLENGE supported
                     * Key import supported
                     * PW status byte can be put
                     * No private_use_DO
                     * Algorithm attrs are changable
                     * ENC/DEC with AES
                     * KDF-DO available
                     */
    0,        /* Secure Messaging Algorithm: N/A (TDES=0, AES=1) */
    0x00, 128,      /* Max size of GET CHALLENGE */
    0x08, 0x00,   /* max. length of cardholder certificate (2KiB) */
    0x00, 0xff,
    0x00, 0x1
};

uint8_t feature_mngmnt[] = {
    3, 0,
    0x81, 0x01, 0x20,
};

uint8_t exlen_info[] = {
    8, 0,
    0x2, 0x2, 0x07, 0xff,
    0x2, 0x2, 0x08, 0x00,
};

file_t file_entries[] = {
    /*  0 */ { .fid = 0x3f00, .parent = 0xff, .name = NULL, .type = FILE_TYPE_DF, .data = NULL,
               .ef_structure = 0, .acl = ACL_NONE },                                                                                  // MF
    /*  1 */ { .fid = EF_FULL_AID, .parent = 0, .name = openpgp_aid_full,
               .type = FILE_TYPE_WORKING_EF, .data = (uint8_t *) openpgp_aid_full,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /*  2 */ { .fid = EF_CH_NAME, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /*  3 */ { .fid = EF_LOGIN_DATA, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /*  4 */ { .fid = EF_LANG_PREF, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /*  5 */ { .fid = EF_SEX, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /*  6 */ { .fid = EF_URI_URL, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /*  7 */ { .fid = EF_HIST_BYTES, .parent = 0, .name = NULL, .type = FILE_TYPE_WORKING_EF,
               .data = historical_bytes, .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /*  8 */ { .fid = EF_CH_DATA, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_ch_data,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /*  9 */ { .fid = EF_SEC_TPL, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_sec_tpl,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 10 */ { .fid = EF_CH_CERT, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_ch_cert,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 11 */ { .fid = EF_EXLEN_INFO, .parent = 0, .name = NULL, .type = FILE_TYPE_WORKING_EF,
               .data = exlen_info, .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 12 */ { .fid = EF_GFM, .parent = 0, .name = NULL, .type = FILE_TYPE_WORKING_EF,
               .data = feature_mngmnt, .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 13 */ { .fid = EF_SIG_COUNT, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 14 */ { .fid = EF_EXT_CAP, .parent = 0, .name = NULL, .type = FILE_TYPE_WORKING_EF,
               .data = extended_capabilities, .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 15 */ { .fid = EF_ALGO_SIG, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_algoinfo,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 16 */ { .fid = EF_ALGO_DEC, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_algoinfo,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 17 */ { .fid = EF_ALGO_AUT, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_algoinfo,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 18 */ { .fid = EF_PW_STATUS, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_pw_status,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 19 */ { .fid = EF_FP, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_fp,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 20 */ { .fid = EF_FP_SIG, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 21 */ { .fid = EF_FP_DEC, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 22 */ { .fid = EF_FP_AUT, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 23 */ { .fid = EF_CA_FP, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_cafp,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 24 */ { .fid = EF_FP_CA1, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 25 */ { .fid = EF_FP_CA2, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 26 */ { .fid = EF_FP_CA3, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 27 */ { .fid = EF_TS_ALL, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_ts,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 28 */ { .fid = EF_TS_SIG, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 29 */ { .fid = EF_TS_DEC, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 30 */ { .fid = EF_TS_AUT, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 31 */ { .fid = EF_RESET_CODE, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 32 */ { .fid = EF_UIF_SIG, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 33 */ { .fid = EF_UIF_DEC, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 34 */ { .fid = EF_UIF_AUT, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 35 */ { .fid = EF_KEY_INFO, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_keyinfo,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 36 */ { .fid = EF_ALGO_INFO, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_algoinfo,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 37 */ { .fid = EF_APP_DATA, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_app_data,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 38 */ { .fid = EF_DISCRETE_DO, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) parse_discrete_do,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    /* 39 */ { .fid = EF_PW1, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 40 */ { .fid = EF_RC, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 41 */ { .fid = EF_PW3, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 42 */ { .fid = EF_ALGO_PRIV1, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 43 */ { .fid = EF_ALGO_PRIV2, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 44 */ { .fid = EF_ALGO_PRIV3, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 45 */ { .fid = EF_PK_SIG, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 46 */ { .fid = EF_PK_DEC, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 47 */ { .fid = EF_PK_AUT, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 48 */ { .fid = EF_PB_SIG, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 49 */ { .fid = EF_PB_DEC, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 50 */ { .fid = EF_PB_AUT, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 51 */ { .fid = EF_PW_PRIV, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 52 */ { .fid = EF_DEK, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_NONE },
    /* 53 */ { .fid = EF_KDF, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 54 */ { .fid = EF_CH_1, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_NONE },
    /* 55 */ { .fid = EF_CH_2, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_NONE },
    /* 56 */ { .fid = EF_CH_3, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_NONE },
    // ** PIV ** //
    /* 57 */ { .fid = EF_PIV_ADMIN_DATA, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 58 */ { .fid = EF_PIV_ATTESTATION, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 59 */ { .fid = EF_PIV_MSCMAP, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 60 */ { .fid = EF_PIV_MSROOTS1, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 61 */ { .fid = EF_PIV_MSROOTS2, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 62 */ { .fid = EF_PIV_MSROOTS3, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 63 */ { .fid = EF_PIV_MSROOTS4, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 64 */ { .fid = EF_PIV_MSROOTS5, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 65 */ { .fid = EF_PIV_KEY_AUTHENTICATION, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 66 */ { .fid = EF_PIV_KEY_CARDMGM, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 67 */ { .fid = EF_PIV_KEY_SIGNATURE, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 68 */ { .fid = EF_PIV_KEY_KEYMGM, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 69 */ { .fid = EF_PIV_KEY_CARDAUTH, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 70 */ { .fid = EF_PIV_KEY_RETIRED1, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 71 */ { .fid = EF_PIV_KEY_RETIRED2, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 72 */ { .fid = EF_PIV_KEY_RETIRED3, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 73 */ { .fid = EF_PIV_KEY_RETIRED4, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 74 */ { .fid = EF_PIV_KEY_RETIRED5, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 75 */ { .fid = EF_PIV_KEY_RETIRED6, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 76 */ { .fid = EF_PIV_KEY_RETIRED7, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 77 */ { .fid = EF_PIV_KEY_RETIRED8, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 78 */ { .fid = EF_PIV_KEY_RETIRED9, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 79 */ { .fid = EF_PIV_KEY_RETIRED10, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 80 */ { .fid = EF_PIV_KEY_RETIRED11, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 81 */ { .fid = EF_PIV_KEY_RETIRED12, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 82 */ { .fid = EF_PIV_KEY_RETIRED12, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 83 */ { .fid = EF_PIV_KEY_RETIRED13, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 84 */ { .fid = EF_PIV_KEY_RETIRED14, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 85 */ { .fid = EF_PIV_KEY_RETIRED15, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 86 */ { .fid = EF_PIV_KEY_RETIRED16, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 87 */ { .fid = EF_PIV_KEY_RETIRED17, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 88 */ { .fid = EF_PIV_KEY_RETIRED18, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 89 */ { .fid = EF_PIV_KEY_RETIRED19, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 90 */ { .fid = EF_PIV_KEY_RETIRED20, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 91 */ { .fid = EF_PIV_KEY_ATTESTATION, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 92 */ { .fid = EF_PIV_CAPABILITY, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 93 */ { .fid = EF_PIV_CHUID, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 94 */ { .fid = EF_PIV_AUTHENTICATION, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 95 */ { .fid = EF_PIV_FINGERPRINTS, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 96 */ { .fid = EF_PIV_SECURITY, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 97 */ { .fid = EF_PIV_FACIAL, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 98 */ { .fid = EF_PIV_PRINTED, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 99 */ { .fid = EF_PIV_SIGNATURE, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 100 */ { .fid = EF_PIV_KEY_MANAGEMENT, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 101 */ { .fid = EF_PIV_CARD_AUTH, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 102 */ { .fid = EF_PIV_DISCOVERY, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FUNC, .data = (uint8_t *) piv_parse_discovery,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 103 */ { .fid = EF_PIV_KEY_HISTORY, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 104 */ { .fid = EF_PIV_IRIS, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 105 */ { .fid = EF_PIV_BITGT, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 106 */ { .fid = EF_PIV_SM_SIGNER, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 107 */ { .fid = EF_PIV_PC_REF_DATA, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 108 */ { .fid = EF_PIV_RETIRED1, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 109 */ { .fid = EF_PIV_RETIRED2, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 110 */ { .fid = EF_PIV_RETIRED3, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 111 */ { .fid = EF_PIV_RETIRED4, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 112 */ { .fid = EF_PIV_RETIRED5, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 113 */ { .fid = EF_PIV_RETIRED6, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 114 */ { .fid = EF_PIV_RETIRED7, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 115 */ { .fid = EF_PIV_RETIRED8, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 116 */ { .fid = EF_PIV_RETIRED9, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 117 */ { .fid = EF_PIV_RETIRED10, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 118 */ { .fid = EF_PIV_RETIRED11, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 119 */ { .fid = EF_PIV_RETIRED12, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 120 */ { .fid = EF_PIV_RETIRED13, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 121 */ { .fid = EF_PIV_RETIRED14, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 122 */ { .fid = EF_PIV_RETIRED15, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 123 */ { .fid = EF_PIV_RETIRED16, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 124 */ { .fid = EF_PIV_RETIRED17, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 125 */ { .fid = EF_PIV_RETIRED18, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 126 */ { .fid = EF_PIV_RETIRED19, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 127 */ { .fid = EF_PIV_RETIRED20, .parent = 0, .name = NULL,
               .type = FILE_TYPE_WORKING_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },
    /* 128 */ { .fid = EF_PIV_PIN, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 129 */ { .fid = EF_PIV_PUK, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_WP },
    /* 130 */ { .fid = EF_META, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_NONE },
    /* 131 */ { .fid = EF_PW_RETRIES, .parent = 0, .name = NULL,
               .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL,
               .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_R_WP },

    /* 132 */ { .fid = 0x0000, .parent = 0, .name = openpgp_aid, .type = FILE_TYPE_WORKING_EF,
        .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = ACL_RO },
    { .fid = EF_KEY_DEV, .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // Device Key
    { .fid = EF_KEY_DEV_ENC, .parent = 0, .name = NULL,.type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // Device Key Enc
    { .fid = EF_MKEK, .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // MKEK
    { .fid = EF_EE_DEV,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // End Entity Certificate Device
    { .fid = EF_EE_DEV_EA,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // End Entity Enterprise Attestation Certificate
    { .fid = EF_COUNTER,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // Global counter
    { .fid = EF_PIN,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // PIN
    { .fid = EF_AUTHTOKEN,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // AUTH TOKEN
    { .fid = EF_MINPINLEN,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // MIN PIN LENGTH
    { .fid = EF_OPTS,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // Global options
    { .fid = EF_LARGEBLOB,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // Large Blob
    { .fid = EF_OTP_PIN,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } },
    { .fid = 0x0000, .parent = 0xff, .name = NULL, .type = FILE_TYPE_NOT_KNOWN, .data = NULL,
               .ef_structure = 0, .acl = ACL_NONE }                                                                                       //end
};

const file_t *MF = &file_entries[0];
const file_t *file_openpgp = &file_entries[sizeof(file_entries) / sizeof(file_t) - 2];
const file_t *file_last = &file_entries[sizeof(file_entries) / sizeof(file_t) - 1];

file_t *ef_keydev = NULL;
file_t *ef_certdev = NULL;
file_t *ef_counter = NULL;
file_t *ef_pin = NULL;
file_t *ef_authtoken = NULL;
file_t *ef_keydev_enc = NULL;
file_t *ef_largeblob = NULL;
file_t *ef_mkek = NULL;
