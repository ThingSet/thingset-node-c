/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <thingset.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "test_utils.h"

static struct thingset_context ts;

static void patch_json_get_cbor(const char *name, const char *json_value,
                                const char *cbor_value_hex)
{
    /* printf("%s patch json: %s, get cbor: %s\n", name, json_value, cbor_value_hex); */

    size_t name_len = strlen(name);
    zassert_true(name_len < 24, "Data item name must be smaller than 24 characters");

    uint8_t cbor_value[100];
    int cbor_value_len = hex2bin_spaced(cbor_value_hex, cbor_value, sizeof(cbor_value));

    /* patch JSON */
    uint8_t patch_req[100];
    snprintf((char *)patch_req, sizeof(patch_req), "=Types {\"%s\":%s}", name, json_value);

    THINGSET_ASSERT_REQUEST_TXT(patch_req, ":84");

    /* get CBOR */
    uint8_t fetch_req[9 + 24] = {
        THINGSET_BIN_GET, 0x78, name_len + 6, 'T', 'y', 'p', 'e', 's', '/'
    };
    memcpy(fetch_req + 9, name, name_len);
    int fetch_req_len = 9 + name_len;

    uint8_t fetch_rsp_exp[100] = { 0x85, 0xF6 };
    size_t fetch_rsp_exp_len = 2;
    memcpy(fetch_rsp_exp + fetch_rsp_exp_len, cbor_value, cbor_value_len);
    fetch_rsp_exp_len += cbor_value_len;

    THINGSET_ASSERT_REQUEST_BIN(fetch_req, fetch_req_len, fetch_rsp_exp, fetch_rsp_exp_len);
}

static void patch_cbor_get_json(const char *name, const char *json_value,
                                const char *cbor_value_hex)
{
    /* printf("%s patch cbor: %s, get json: %s\n", name, cbor_value_hex, json_value); */

    size_t name_len = strlen(name);
    zassert_true(name_len < 24, "Data item name must be smaller than 24 characters");

    uint8_t cbor_value[100];
    int cbor_value_len = hex2bin_spaced(cbor_value_hex, cbor_value, sizeof(cbor_value));

    /* patch CBOR */
    uint8_t patch_req[100] = { THINGSET_BIN_UPDATE, 0x65, 'T', 'y', 'p', 'e', 's', 0xA1,
                               0x60 + name_len };
    size_t patch_req_len = 9;
    memcpy(patch_req + patch_req_len, name, name_len);
    patch_req_len += name_len;
    memcpy(patch_req + patch_req_len, cbor_value, cbor_value_len);
    patch_req_len += cbor_value_len;

    uint8_t patch_rsp_exp[] = { 0x84, 0xf6, 0xf6 };

    THINGSET_ASSERT_REQUEST_BIN(patch_req, patch_req_len, patch_rsp_exp, sizeof(patch_rsp_exp));

    /* fetch JSON */
    char fetch_req[100];
    snprintf(fetch_req, sizeof(fetch_req), "?Types/%s", name);

    char fetch_rsp_exp[100];
    snprintf(fetch_rsp_exp, sizeof(fetch_rsp_exp), ":85 %s", json_value);

    THINGSET_ASSERT_REQUEST_TXT(fetch_req, fetch_rsp_exp);
}

/**
 * @brief Test conversion json to cbor.
 */
ZTEST(thingset_serde, test_txt_patch_bin_get)
{
    // uint8
    patch_json_get_cbor("wU8", "0", "00");
    patch_json_get_cbor("wU8", "23", "17");
    patch_json_get_cbor("wU8", "24", "18 18");
    patch_json_get_cbor("wU8", "255", "18 ff");

    // uint16
    patch_json_get_cbor("wU16", "0", "00");
    patch_json_get_cbor("wU16", "23", "17");
    patch_json_get_cbor("wU16", "24", "18 18");
    patch_json_get_cbor("wU16", "255", "18 ff");
    patch_json_get_cbor("wU16", "256", "19 01 00");
    patch_json_get_cbor("wU16", "65535", "19 FF FF");

    // uint32
    patch_json_get_cbor("wU32", "0", "00");
    patch_json_get_cbor("wU32", "23", "17");
    patch_json_get_cbor("wU32", "24", "18 18");
    patch_json_get_cbor("wU32", "255", "18 ff");
    patch_json_get_cbor("wU32", "256", "19 01 00");
    patch_json_get_cbor("wU32", "65535", "19 FF FF");
    patch_json_get_cbor("wU32", "65536", "1A 00 01 00 00");
    patch_json_get_cbor("wU32", "4294967295", "1A FF FF FF FF");

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    // uint64
    patch_json_get_cbor("wU64", "4294967295", "1A FF FF FF FF");
    patch_json_get_cbor("wU64", "4294967296", "1B 00 00 00 01 00 00 00 00");
    patch_json_get_cbor("wU64", "9223372036854775807",
                        "1B 7F FF FF FF FF FF FF FF"); // maximum value for int64
#endif

    // int8 (positive values)
    patch_json_get_cbor("wI8", "0", "00");
    patch_json_get_cbor("wI8", "23", "17");
    patch_json_get_cbor("wI8", "24", "18 18");
    patch_json_get_cbor("wI8", "127", "18 7f"); // maximum value for int8

    // int16 (positive values)
    patch_json_get_cbor("wI16", "0", "00");
    patch_json_get_cbor("wI16", "23", "17");
    patch_json_get_cbor("wI16", "24", "18 18");
    patch_json_get_cbor("wI16", "255", "18 ff");
    patch_json_get_cbor("wI16", "256", "19 01 00");
    patch_json_get_cbor("wI16", "32767",
                        "19 7F FF"); // maximum value for int16

    // int32 (positive values)
    patch_json_get_cbor("wI32", "0", "00");
    patch_json_get_cbor("wI32", "23", "17");
    patch_json_get_cbor("wI32", "24", "18 18");
    patch_json_get_cbor("wI32", "255", "18 ff");
    patch_json_get_cbor("wI32", "256", "19 01 00");
    patch_json_get_cbor("wI32", "65535", "19 FF FF");
    patch_json_get_cbor("wI32", "65536", "1A 00 01 00 00");
    patch_json_get_cbor("wI32", "2147483647",
                        "1A 7F FF FF FF"); // maximum value for int32

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    // int64 (positive values)
    patch_json_get_cbor("wI64", "4294967295", "1A FF FF FF FF");
    patch_json_get_cbor("wI64", "4294967296", "1B 00 00 00 01 00 00 00 00");
    patch_json_get_cbor("wI64", "9223372036854775807",
                        "1B 7F FF FF FF FF FF FF FF"); // maximum value for int64
#endif

    // int8 (negative values)
    patch_json_get_cbor("wI8", "-0", "00");
    patch_json_get_cbor("wI8", "-24", "37");
    patch_json_get_cbor("wI8", "-25", "38 18");
    patch_json_get_cbor("wI8", "-128", "38 7F");

    // int16 (negative values)
    patch_json_get_cbor("wI16", "-0", "00");
    patch_json_get_cbor("wI16", "-24", "37");
    patch_json_get_cbor("wI16", "-25", "38 18");
    patch_json_get_cbor("wI16", "-256", "38 ff");
    patch_json_get_cbor("wI16", "-257", "39 01 00");
    patch_json_get_cbor("wI16", "-32768", "39 7F FF");

    // int32 (negative values)
    patch_json_get_cbor("wI32", "-0", "00");
    patch_json_get_cbor("wI32", "-24", "37");
    patch_json_get_cbor("wI32", "-25", "38 18");
    patch_json_get_cbor("wI32", "-256", "38 ff");
    patch_json_get_cbor("wI32", "-257", "39 01 00");
    patch_json_get_cbor("wI32", "-65536", "39 FF FF");
    patch_json_get_cbor("wI32", "-65537", "3A 00 01 00 00");
    patch_json_get_cbor("wI32", "-2147483648",
                        "3A 7F FF FF FF"); // maximum value for int32

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    // int64 (negative values)
    patch_json_get_cbor("wI64", "-4294967296", "3A FF FF FF FF");
    patch_json_get_cbor("wI64", "-4294967297", "3B 00 00 00 01 00 00 00 00");
    patch_json_get_cbor("wI64", "-9223372036854775808",
                        "3B 7F FF FF FF FF FF FF FF"); // maximum value for int64
#endif

    // float
    patch_json_get_cbor("wF32", "12.340", "fa 41 45 70 a4");
    patch_json_get_cbor("wF32", "-12.340", "fa c1 45 70 a4");
    patch_json_get_cbor("wF32", "12.345", "fa 41 45 85 1f");

#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
    // decimal fraction
    patch_json_get_cbor("wDecFrac", "273.15", "c4 82 21 19 6a b3");
#endif

    // bool
    patch_json_get_cbor("wBool", "true", "f5");
    patch_json_get_cbor("wBool", "false", "f4");

    // string
    patch_json_get_cbor("wString", "\"Test\"", "64 54 65 73 74");
    patch_json_get_cbor("wString", "\"Hello World!\"", "6c 48 65 6c 6c 6f 20 57 6f 72 6c 64 21");

#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT
    // bytes (base64-encoded)
    patch_json_get_cbor("wBytes", "\"ABEiM0RVZneImaq7zN3u/w==\"",
                        "50 00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF");
#endif
}

/**
 * @brief Test conversion json to cbor.
 */
ZTEST(thingset_serde, test_bin_patch_txt_get)
{
    // uint8
    patch_cbor_get_json("wU8", "0", "00");
    patch_cbor_get_json("wU8", "23", "17");
    patch_cbor_get_json("wU8", "23", "18 17"); // less compact format
    patch_cbor_get_json("wU8", "24", "18 18");
    patch_cbor_get_json("wU8", "255", "18 ff");

    // uint16
    patch_cbor_get_json("wU16", "0", "00");
    patch_cbor_get_json("wU16", "23", "17");
    patch_cbor_get_json("wU16", "23", "18 17"); // less compact format
    patch_cbor_get_json("wU16", "24", "18 18");
    patch_cbor_get_json("wU16", "255", "18 ff");
    patch_cbor_get_json("wU16", "255",
                        "19 00 ff"); // less compact format
    patch_cbor_get_json("wU16", "256", "19 01 00");
    patch_cbor_get_json("wU16", "65535", "19 FF FF");

    // uint32
    patch_cbor_get_json("wU32", "0", "00");
    patch_cbor_get_json("wU32", "23", "17");
    patch_cbor_get_json("wU32", "23", "18 17"); // less compact format
    patch_cbor_get_json("wU32", "24", "18 18");
    patch_cbor_get_json("wU32", "255", "18 ff");
    patch_cbor_get_json("wU32", "255",
                        "19 00 ff"); // less compact format
    patch_cbor_get_json("wU32", "256", "19 01 00");
    patch_cbor_get_json("wU32", "65535", "19 FF FF");
    patch_cbor_get_json("wU32", "65535",
                        "1A 00 00 FF FF"); // less compact format
    patch_cbor_get_json("wU32", "65536", "1A 00 01 00 00");
    patch_cbor_get_json("wU32", "4294967295", "1A FF FF FF FF");

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    // uint64
    patch_cbor_get_json("wU64", "4294967295", "1A FF FF FF FF");
    patch_cbor_get_json("wU64", "4294967295",
                        "1B 00 00 00 00 FF FF FF FF"); // less compact format
    patch_cbor_get_json("wU64", "4294967296", "1B 00 00 00 01 00 00 00 00");
    patch_cbor_get_json("wU64", "18446744073709551615", "1B FF FF FF FF FF FF FF FF");
#endif

    // int8 (positive values)
    patch_cbor_get_json("wI8", "23", "17");
    patch_cbor_get_json("wI8", "23", "18 17"); // less compact format
    patch_cbor_get_json("wI8", "24", "18 18");
    patch_cbor_get_json("wI8", "127", "18 7F");

    // int16 (positive values)
    patch_cbor_get_json("wI16", "23", "17");
    patch_cbor_get_json("wI16", "23", "18 17"); // less compact format
    patch_cbor_get_json("wI16", "24", "18 18");
    patch_cbor_get_json("wI16", "255", "18 FF");
    patch_cbor_get_json("wI16", "255",
                        "19 00 FF"); // less compact format
    patch_cbor_get_json("wI16", "256", "19 01 00");
    patch_cbor_get_json("wI16", "32767", "19 7F FF");

    // int32 (positive values)
    patch_cbor_get_json("wI32", "23", "17");
    patch_cbor_get_json("wI32", "23", "18 17"); // less compact format
    patch_cbor_get_json("wI32", "24", "18 18");
    patch_cbor_get_json("wI32", "255", "18 ff");
    patch_cbor_get_json("wI32", "255",
                        "19 00 ff"); // less compact format
    patch_cbor_get_json("wI32", "256", "19 01 00");
    patch_cbor_get_json("wI32", "65535", "19 FF FF");
    patch_cbor_get_json("wI32", "65535",
                        "1A 00 00 FF FF"); // less compact format
    patch_cbor_get_json("wI32", "65536", "1A 00 01 00 00");
    patch_cbor_get_json("wI32", "2147483647",
                        "1A 7F FF FF FF"); // maximum value for int32

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    // int64 (positive values)
    patch_cbor_get_json("wI64", "4294967295", "1A FF FF FF FF");
    patch_cbor_get_json("wI64", "4294967296", "1B 00 00 00 01 00 00 00 00");
    patch_cbor_get_json("wI64", "9223372036854775807",
                        "1B 7F FF FF FF FF FF FF FF"); // maximum value for int64
#endif

    // int8 (negative values)
    patch_cbor_get_json("wI8", "-24", "37");
    patch_cbor_get_json("wI8", "-24", "38 17"); // less compact format
    patch_cbor_get_json("wI8", "-25", "38 18");
    patch_cbor_get_json("wI8", "-128", "38 7f");

    // int16 (negative values)
    patch_cbor_get_json("wI16", "-24", "37");
    patch_cbor_get_json("wI16", "-24", "38 17"); // less compact format
    patch_cbor_get_json("wI16", "-25", "38 18");
    patch_cbor_get_json("wI16", "-256", "38 ff");
    patch_cbor_get_json("wI16", "-257", "39 01 00");
    patch_cbor_get_json("wI16", "-32768", "39 7F FF");

    // int32 (negative values)
    patch_cbor_get_json("wI32", "-24", "37");
    patch_cbor_get_json("wI32", "-24", "38 17"); // less compact format
    patch_cbor_get_json("wI32", "-25", "38 18");
    patch_cbor_get_json("wI32", "-256", "38 ff");
    patch_cbor_get_json("wI32", "-257", "39 01 00");
    patch_cbor_get_json("wI32", "-65536", "39 FF FF");
    patch_cbor_get_json("wI32", "-65537", "3A 00 01 00 00");
    patch_cbor_get_json("wI32", "-2147483648",
                        "3A 7F FF FF FF"); // maximum value for int32

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    // int64 (negative values)
    patch_cbor_get_json("wI64", "-4294967296", "3A FF FF FF FF");
    patch_cbor_get_json("wI64", "-4294967297", "3B 00 00 00 01 00 00 00 00");
    patch_cbor_get_json("wI64", "-9223372036854775808",
                        "3B 7F FF FF FF FF FF FF FF"); // maximum value for int64
#endif

    // float
    patch_cbor_get_json("wF32", "12.34", "fa 41 45 70 a4");
    patch_cbor_get_json("wF32", "-12.34", "fa c1 45 70 a4");
    patch_cbor_get_json("wF32", "12.34", "fa 41 45 81 06"); // 12.344
    patch_cbor_get_json("wF32", "12.35",
                        "fa 41 45 85 1f"); // 12.345 (should be rounded to 12.35)

#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
    // decimal fraction
    patch_cbor_get_json("wDecFrac", "27315e-2",
                        "c4 82 21 19 6a b3"); // decfrac 27315e-2
    patch_cbor_get_json("wDecFrac", "27315e-2",
                        "c4 82 22 1a 00 04 2A FE"); // decfrac 273150e-3
    patch_cbor_get_json("wDecFrac", "27310e-2",
                        "c4 82 20 19 0a ab"); // decfrac 2731e-1
    patch_cbor_get_json("wDecFrac", "27315e-2",
                        "fa 43 88 93 33"); // float 273.15
    patch_cbor_get_json("wDecFrac", "27300e-2",
                        "19 01 11"); // decimal 273
#endif

    // bool
    patch_cbor_get_json("wBool", "true", "f5");
    patch_cbor_get_json("wBool", "false", "f4");

    // string
    patch_cbor_get_json("wString", "\"Test\"", "64 54 65 73 74");
    patch_cbor_get_json("wString", "\"Hello World!\"", "6c 48 65 6c 6c 6f 20 57 6f 72 6c 64 21");

#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT
    // bytes (base64-encoded)
    patch_cbor_get_json("wBytes", "\"ABEiM0RVZneImaq7zN3u/w==\"",
                        "50 00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF");
#endif
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_serde, NULL, thingset_setup, NULL, NULL, NULL);
