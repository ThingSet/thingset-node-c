/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>

#define THINGSET_TEST_BUF_SIZE 1024

#define THINGSET_ASSERT_REQUEST_BIN(req, req_len, rsp_exp, rsp_exp_len) \
    { \
        uint8_t rsp_act[THINGSET_TEST_BUF_SIZE]; \
        char req_hex[THINGSET_TEST_BUF_SIZE]; \
        char rsp_act_hex[THINGSET_TEST_BUF_SIZE]; \
        char rsp_exp_hex[THINGSET_TEST_BUF_SIZE]; \
        int rsp_act_len = thingset_process_message(&ts, req, req_len, rsp_act, sizeof(rsp_act)); \
        bin2hex_spaced(req, req_len, req_hex, sizeof(req_hex)); \
        bin2hex_spaced(rsp_act, rsp_act_len, rsp_act_hex, sizeof(rsp_act_hex)); \
        bin2hex_spaced(rsp_exp, rsp_exp_len, rsp_exp_hex, sizeof(rsp_exp_hex)); \
        zassert_true(rsp_exp_len > 0 && rsp_act_len > 0); \
        zassert_mem_equal(rsp_exp, rsp_act, rsp_exp_len, "req: %s\nrsp_act: %s\nrsp_exp: %s\n", \
                          req_hex, rsp_act_hex, rsp_exp_hex); \
        zassert_equal(rsp_act_len, rsp_exp_len, "act: %d, exp: %d", rsp_act_len, rsp_exp_len); \
    }

#define THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex) \
    { \
        uint8_t req[THINGSET_TEST_BUF_SIZE]; \
        uint8_t rsp_act[THINGSET_TEST_BUF_SIZE]; \
        uint8_t rsp_exp[THINGSET_TEST_BUF_SIZE]; \
        char rsp_act_hex[THINGSET_TEST_BUF_SIZE]; \
        char rsp_exp_hex_formatted[THINGSET_TEST_BUF_SIZE]; \
        int req_len = hex2bin_spaced(req_hex, req, sizeof(req)); \
        int rsp_exp_len = hex2bin_spaced(rsp_exp_hex, rsp_exp, sizeof(rsp_exp)); \
        bin2hex_spaced(rsp_exp, rsp_exp_len, rsp_exp_hex_formatted, \
                       sizeof(rsp_exp_hex_formatted)); \
        int rsp_act_len = thingset_process_message(&ts, req, req_len, rsp_act, sizeof(rsp_act)); \
        bin2hex_spaced(rsp_act, rsp_act_len, rsp_act_hex, sizeof(rsp_act_hex)); \
        zassert_true(rsp_exp_len > 0 && rsp_act_len > 0); \
        zassert_mem_equal(rsp_exp, rsp_act, rsp_exp_len, "act: %s\nexp: %s", rsp_act_hex, \
                          rsp_exp_hex_formatted); \
        zassert_equal(rsp_act_len, rsp_exp_len, "act: %d, exp: %d", rsp_act_len, rsp_exp_len); \
    }

#define THINGSET_ASSERT_DESIRE_HEX(des_hex, err_exp) \
    { \
        uint8_t des[THINGSET_TEST_BUF_SIZE]; \
        uint8_t rsp_act[THINGSET_TEST_BUF_SIZE]; \
        int des_len = hex2bin_spaced(des_hex, des, sizeof(des)); \
        int err_act = thingset_process_message(&ts, des, des_len, rsp_act, sizeof(rsp_act)); \
        zassert_equal(err_exp, err_act, "act: %d, exp: %d", err_act, err_exp); \
    }

#define THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp) \
    { \
        uint8_t rsp_act[THINGSET_TEST_BUF_SIZE]; \
        int req_len = strlen(req); \
        int rsp_exp_len = strlen(rsp_exp); \
        int rsp_act_len = thingset_process_message(&ts, req, req_len, rsp_act, sizeof(rsp_act)); \
        zassert_true(rsp_exp_len > 0 && rsp_act_len > 0); \
        zassert_mem_equal(rsp_exp, rsp_act, rsp_act_len, "act: %s\nexp: %s", rsp_act, rsp_exp); \
        zassert_equal(rsp_act_len, rsp_exp_len, "act: %d, exp: %d", rsp_act_len, rsp_exp_len); \
    }

#define THINGSET_ASSERT_DESIRE_TXT(des, err_exp) \
    { \
        uint8_t rsp_act[THINGSET_TEST_BUF_SIZE]; \
        int des_len = strlen(des); \
        int err_act = thingset_process_message(&ts, des, des_len, rsp_act, sizeof(rsp_act)); \
        zassert_equal(err_exp, err_act, "act: %d, exp: %d", err_act, err_exp); \
    }

#define THINGSET_ASSERT_REPORT_TXT(path, rpt_exp, err_exp) \
    { \
        uint8_t rpt_act[THINGSET_TEST_BUF_SIZE]; \
        int err_act = \
            thingset_report_path(&ts, rpt_act, sizeof(rpt_act), path, THINGSET_TXT_NAMES_VALUES); \
        if (err_exp > 0) { \
            zassert_true(err_act > 0, "err_act: 0x%02X", -err_act); \
            zassert_mem_equal(rpt_exp, rpt_act, err_exp, "act: %s\nexp: %s", rpt_act, rpt_exp); \
            zassert_equal(err_exp, err_act, "act: %d, exp: %d", err_act, err_exp); \
        } \
        else { \
            zassert_equal(err_exp, err_act, "act: %d, exp: %d", err_act, err_exp); \
        } \
    }

#define THINGSET_ASSERT_REPORT_HEX_IDS(path, rpt_exp_hex, err_exp) \
    { \
        uint8_t rpt_act[THINGSET_TEST_BUF_SIZE]; \
        uint8_t rpt_act_hex[THINGSET_TEST_BUF_SIZE]; \
        uint8_t rpt_exp[THINGSET_TEST_BUF_SIZE]; \
        int rpt_act_len = \
            thingset_report_path(&ts, rpt_act, sizeof(rpt_act), path, THINGSET_BIN_IDS_VALUES); \
        int rpt_exp_len = hex2bin_spaced(rpt_exp_hex, rpt_exp, sizeof(rpt_exp)); \
        bin2hex_spaced(rpt_act, rpt_act_len, rpt_act_hex, sizeof(rpt_act_hex)); \
        if (err_exp > 0) { \
            zassert_true(rpt_act_len > 0, "err_act: 0x%02X", -rpt_act_len); \
            zassert_mem_equal(rpt_exp, rpt_act, MAX(rpt_act_len, rpt_exp_len), "act: %s\nexp: %s", \
                              rpt_act_hex, rpt_exp_hex); \
            zassert_equal(rpt_act_len, err_exp, "act: %d, exp: %d", rpt_act_len, err_exp); \
        } \
        else { \
            zassert_equal(rpt_act_len, err_exp, "act: %d, exp: %d", rpt_act_len, err_exp); \
        } \
    }

#define THINGSET_ASSERT_REPORT_HEX_NAMES(path, rpt_exp_hex, err_exp) \
    { \
        uint8_t rpt_act[THINGSET_TEST_BUF_SIZE]; \
        uint8_t rpt_act_hex[THINGSET_TEST_BUF_SIZE]; \
        uint8_t rpt_exp[THINGSET_TEST_BUF_SIZE]; \
        int rpt_act_len = \
            thingset_report_path(&ts, rpt_act, sizeof(rpt_act), path, THINGSET_BIN_NAMES_VALUES); \
        int rpt_exp_len = hex2bin_spaced(rpt_exp_hex, rpt_exp, sizeof(rpt_exp)); \
        bin2hex_spaced(rpt_act, rpt_act_len, rpt_act_hex, sizeof(rpt_act_hex)); \
        if (err_exp > 0) { \
            zassert_true(rpt_act_len > 0, "err_act: 0x%02X", -rpt_act_len); \
            zassert_mem_equal(rpt_exp, rpt_act, MAX(rpt_act_len, rpt_exp_len), "act: %s\nexp: %s", \
                              rpt_act_hex, rpt_exp_hex); \
            zassert_equal(rpt_act_len, err_exp, "act: %d, exp: %d", rpt_act_len, err_exp); \
        } \
        else { \
            zassert_equal(rpt_act_len, err_exp, "act: %d, exp: %d", rpt_act_len, err_exp); \
        } \
    }

#define THINGSET_ASSERT_EXPORT_TXT(subsets, rsp_exp, err_exp) \
    { \
        uint8_t rsp_act[THINGSET_TEST_BUF_SIZE]; \
        int err_act = thingset_export_subsets(&ts, rsp_act, sizeof(rsp_act), subsets, \
                                              THINGSET_TXT_NAMES_VALUES); \
        if (err_exp > 0) { \
            zassert_true(err_act > 0, "err_act: 0x%02X", -err_act); \
            zassert_mem_equal(rsp_act, rsp_exp, err_exp, "act: %s\nexp: %s", rsp_act, rsp_exp); \
            zassert_equal(err_exp, err_act, "act: %d, exp: %d", err_act, err_exp); \
        } \
        else { \
            zassert_equal(err_exp, err_act, "act: %d, exp: %d", err_act, err_exp); \
        } \
    }

#define THINGSET_ASSERT_EXPORT_HEX_IDS(subsets, data_exp_hex, err_exp) \
    { \
        uint8_t data_act[THINGSET_TEST_BUF_SIZE]; \
        uint8_t data_act_hex[THINGSET_TEST_BUF_SIZE]; \
        uint8_t data_exp[THINGSET_TEST_BUF_SIZE]; \
        int data_act_len = thingset_export_subsets(&ts, data_act, sizeof(data_act), subsets, \
                                                   THINGSET_BIN_IDS_VALUES); \
        int data_exp_len = hex2bin_spaced(data_exp_hex, data_exp, sizeof(data_exp)); \
        bin2hex_spaced(data_act, data_act_len, data_act_hex, sizeof(data_act_hex)); \
        if (err_exp > 0) { \
            zassert_true(data_act_len > 0, "err_act: 0x%02X", -data_act_len); \
            zassert_mem_equal(data_exp, data_act, MAX(data_act_len, data_exp_len), \
                              "act: %s\nexp: %s", data_act_hex, data_exp_hex); \
            zassert_equal(data_act_len, err_exp, "act: %d, exp: %d", data_act_len, err_exp); \
        } \
        else { \
            zassert_equal(data_act_len, err_exp, "act: %d, exp: %d", data_act_len, err_exp); \
        } \
    }

#define THINGSET_ASSERT_IMPORT_HEX_IDS(data_hex, err_exp, auth_flags) \
    { \
        uint8_t data[THINGSET_TEST_BUF_SIZE]; \
        int data_len = hex2bin_spaced(data_hex, data, sizeof(data)); \
        int err = thingset_import_data(&ts, data, data_len, auth_flags, THINGSET_BIN_IDS_VALUES); \
        zassert_equal(err, err_exp, "act: 0x%X, exp: 0x%X", -err, -err_exp); \
    }

size_t hex2bin_spaced(const char *hex, uint8_t *bin, size_t bin_size);

size_t bin2hex_spaced(const uint8_t *bin, size_t bin_size, char *hex, size_t hex_size);
