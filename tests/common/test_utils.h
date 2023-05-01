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
        int req_len = hex2bin_spaced(req_hex, req, sizeof(req)); \
        int rsp_exp_len = hex2bin_spaced(rsp_exp_hex, rsp_exp, sizeof(rsp_exp)); \
        int rsp_act_len = thingset_process_message(&ts, req, req_len, rsp_act, sizeof(rsp_act)); \
        bin2hex_spaced(rsp_act, rsp_act_len, rsp_act_hex, sizeof(rsp_act_hex)); \
        zassert_true(rsp_exp_len > 0 && rsp_act_len > 0); \
        zassert_mem_equal(rsp_exp, rsp_act, rsp_exp_len, "act: %s\nexp: %s", rsp_act_hex, \
                          rsp_exp_hex); \
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
        zassert_mem_equal(rsp_exp, rsp_act, rsp_exp_len, "act: %s\nexp: %s", rsp_act, rsp_exp); \
        zassert_equal(rsp_act_len, rsp_exp_len, "act: %d, exp: %d", rsp_act_len, rsp_exp_len); \
    }

#define THINGSET_ASSERT_DESIRE_TXT(des, err_exp) \
    { \
        uint8_t rsp_act[THINGSET_TEST_BUF_SIZE]; \
        int des_len = strlen(des); \
        int err_act = thingset_process_message(&ts, des, des_len, rsp_act, sizeof(rsp_act)); \
        zassert_equal(err_exp, err_act, "act: %d, exp: %d", err_act, err_exp); \
    }

size_t hex2bin_spaced(const char *hex, uint8_t *bin, size_t bin_size);

size_t bin2hex_spaced(const uint8_t *bin, size_t bin_size, char *hex, size_t hex_size);
