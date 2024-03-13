/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include <thingset.h>

#include "data.h"
#include "test_utils.h"

#ifdef CONFIG_THINGSET_REPORT_RECORD_SERIALIZATION
static struct thingset_context ts;

ZTEST(thingset_report, test_report_bin)
{
    uint8_t rsp_act[320];

    char rsp_exp_hex[] =
        "1f "          /* report, binary */
        "19 08 00 "    /* mLive 0x800 */
        "a5 "          /* map with 5 elements */
        "10 19 03 e8 " /* t_s */
        "19 02 01 f5 " /* Types/wBool */
        "19 06 00 82 " /* 0x600: array with 2 elements */
        "af "
        "19 06 01 01 " /* 0x601: 1 */
        "19 06 02 f4 " /* 0x602: false */
        "19 06 03 00 " /* 0x603...: 0 */
        "19 06 04 00 "
        "19 06 05 00 "
        "19 06 06 00 "
        "19 06 07 00 "
        "19 06 08 00 "
        "19 06 09 00 "
        "19 06 0a 00 "
        "19 06 0b fa 00 00 00 00 "
        "19 06 0c c4 82 21 00 "
        "19 06 0d 60 "
        "19 06 0f 83 fa 00 00 00 00 fa 00 00 00 00 fa 00 00 00 00 "
        "19 06 10 " /* 0x610 */
        "82 "       /* array of size 2 */
        "a2 "
        "19 06 11 00 "             /* 0x611: 0 */
        "19 06 12 fa 00 00 00 00 " /* 0x612: 0.0 */
        "a2 "
        "19 06 11 00 "
        "19 06 12 fa 00 00 00 00 "
        "af "
        "19 06 01 02 "                                              /* 0x601: 2 */
        "19 06 02 f5 "                                              /* 0x602: true */
        "19 06 03 08 "                                              /* 0x603: 8 */
        "19 06 04 27 "                                              /* 0x604: -8 */
        "19 06 05 10 "                                              /* 0x605: 16 */
        "19 06 06 2f "                                              /* 0x606: -16 */
        "19 06 07 18 20 "                                           /* 0x607: 32 */
        "19 06 08 38 1f "                                           /* 0x608: -32 */
        "19 06 09 18 40 "                                           /* 0x609: 64 */
        "19 06 0a 38 3f "                                           /* 0x60a: -64 */
        "19 06 0b fa c0 4c cc cd "                                  /* 0x60b: -3.2 */
        "19 06 0c c4 82 21 38 1f "                                  /* 0x60c: 4([-2, -32]) */
        "19 06 0d 66 73 74 72 69 6e 67 "                            /* 0x60d: "string" */
        "19 06 0f 83 fa 3f 9d 70 a4 fa 40 91 eb 85 fa 40 fc 7a e1 " /* 0x60f: [1.23, 4.56, 7.89] */
        "19 06 10 "                                                 /* 0x610 */
        "82 "                                                       /* array of size 2 */
        "a2 "
        "19 06 11 18 20 "          /* 0x611: 32 */
        "19 06 12 fa 3f 9d 70 a4 " /* 0x612: 1.23 */
        "a2 "
        "19 06 11 10 "             /* 0x611: 16 */
        "19 06 12 fa 40 91 eb 85 " /* 0x612: 4.56 */
        "19 07 01 01 "             /* 0x701: 1 */
        "19 07 08 fa 40 0c cc cd"; /* 0x708: 2.2 */
    uint8_t rsp_exp[sizeof(rsp_exp_hex) / 3];
    hex2bin_spaced(rsp_exp_hex, rsp_exp, sizeof(rsp_exp));

    int len = thingset_report_path(&ts, rsp_act, sizeof(rsp_act), "mLive", THINGSET_BIN_IDS_VALUES);

    char rsp_act_hex[len * 3];
    bin2hex_spaced(rsp_act, len, rsp_act_hex, sizeof(rsp_act_hex));
    zassert_mem_equal(rsp_exp, rsp_act, sizeof(rsp_exp), "rsp_act: %s\nrsp_exp: %s\n", rsp_act_hex,
                      rsp_exp_hex);
}

ZTEST(thingset_report, test_report_txt)
{
    // clang-format off
    const char rpt_exp[] =
        "#mLive {"
            "\"t_s\":1000,"
            "\"Types\":{\"wBool\":true},"
            "\"Records\":["
            "{"
                "\"t_s\":1,"
                "\"wBool\":false,"
                "\"wU8\":0,\"wI8\":0,"
                "\"wU16\":0,\"wI16\":0,"
                "\"wU32\":0,\"wI32\":0,"
                "\"wU64\":0,\"wI64\":0,"
                "\"wF32\":0.0,\"wDecFrac\":0e-2,"
                "\"wString\":\"\","
                "\"wF32Array\":[0.0,0.0,0.0],"
                "\"Nested\":["
                "{"
                    "\"wU32\":0,"
                    "\"wF32\":0.00"
                "},"
                "{"
                    "\"wU32\":0,"
                    "\"wF32\":0.00"
                "}"
                "]"
            "},{"
                "\"t_s\":2,"
                "\"wBool\":true,"
                "\"wU8\":8,\"wI8\":-8,"
                "\"wU16\":16,\"wI16\":-16,"
                "\"wU32\":32,\"wI32\":-32,"
                "\"wU64\":64,\"wI64\":-64,"
                "\"wF32\":-3.2,\"wDecFrac\":-32e-2,"
                "\"wString\":\"string\","
                "\"wF32Array\":[1.2,4.6,7.9],"
                "\"Nested\":["
                "{"
                    "\"wU32\":32,"
                    "\"wF32\":1.23"
                "},"
                "{"
                    "\"wU32\":16,"
                    "\"wF32\":4.56"
                "}"
                "]"
            "}],"
            "\"Nested\":{\"rBeginning\":1,\"Obj2\":{\"rItem2_V\":2.2}}"
        "}";
    // clang-format on

    THINGSET_ASSERT_REPORT_TXT("mLive", rpt_exp, strlen(rpt_exp));
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_report, NULL, thingset_setup, NULL, NULL, NULL);
#endif /* CONFIG_THINGSET_REPORT_RECORD_SERIALIZATION */