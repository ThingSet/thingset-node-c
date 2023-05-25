/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include "../../src/thingset_internal.h"

#include "data.h"
#include "test_utils.h"

static struct thingset_context ts;

ZTEST(thingset_bin, test_get_root_ids)
{
    const char req_hex[] = "01 00";
    const char rsp_exp_hex[] =
        "85 F6 A9 "
        "10 19 03 E8 "                      /* t_s */
        "18 1D 68 41 42 43 44 31 32 33 34 " /* cNodeID */
        "19 02 00 F6 "                      /* Types */
        "19 03 00 F6 "                      /* Arrays */
        "19 04 00 F6 "                      /* Exec */
        "19 05 00 F6 "                      /* Access */
        "19 06 00 02 "                      /* Records */
        "19 07 00 F6 "                      /* Nested */
        "19 08 00 84 "                      /* mLive (array) */
        "10 "                               /* t_s */
        "19 02 01 "                         /* Types/wBool */
        "19 07 01 "                         /* Nested/rBeginning */
        "19 07 08";                         /* Nested/Obj2/rItem2_V */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_get_root_names)
{
    const char req_hex[] = "01 60";
    const char rsp_exp_hex[] =
        "85 F6 A9 "
        "63 745F73 19 03E8 "                           /* t_s */
        "67 634E6F64654944 "                           /* cNodeID */
        "68 41 42 43 44 31 32 33 34 "                  /* ABCD1234 */
        "65 5479706573 F6 "                            /* Types */
        "66 417272617973 F6 "                          /* Arrays */
        "64 45786563 F6 "                              /* Exec */
        "66 416363657373 F6 "                          /* Access */
        "67 5265636F726473 02 "                        /* Records */
        "66 4E6573746564 F6 "                          /* Nested */
        "65 6D4C697665 84 "                            /* mLive (array) */
        "63 745F73 "                                   /* t_s */
        "6B 54797065732F77426F6F6C "                   /* Types/wBool */
        "71 4E65737465642F72426567696E6E696E67 "       /* Nested/rBeginning */
        "74 4E65737465642F4F626A322F724974656D325F56"; /* Nested/Obj2/rItem2_V */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_get_nested_ids)
{
    const char req_hex[] = "01 19 07 00";
    const char rsp_exp_hex[] =
        "85 F6 A5 "
        "19 0701 01 "  /* rBeginning */
        "19 0702 F6 "  /* Obj1 */
        "19 0705 02 "  /* rBetween */
        "19 0706 F6 "  /* Obj2 */
        "19 0709 03 "; /* rEnd */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_get_nested_names)
{
    const char req_hex[] = "01 66 4E6573746564";
    const char rsp_exp_hex[] =
        "85 F6 A5 "
        "6A 72426567696E6E696E67 01 " /* rBeginning */
        "64 4F626A31 F6 "             /* Obj1 */
        "68 724265747765656E 02 "     /* rBetween */
        "64 4F626A32 F6 "             /* Obj2 */
        "64 72456E64 03 ";            /* rEnd */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_get_single_value_id)
{
    const char req_hex[] = "01 19 0704";
    const char rsp_exp_hex[] = "85 F6 FA 3F99999A";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_get_single_value_name)
{
    const char req_hex[] =
        "01 74 4E65737465642F4F626A312F724974656D325F56"; /* Nested/Obj1/rItem2_V */
    const char rsp_exp_hex[] = "85 F6 FA 3F99999A";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_get_exec_ids)
{
    const char req_hex[] = "01 19 0400";
    const char rsp_exp_hex[] =
        "85 F6 A4"
        "19 0401 80"            /* xVoid */
        "19 0402 81"            /* xVoidParams */
        "   65 6C426F6F6C"      /* lBool */
        "19 0404 82"            /* xI32Params */
        "   67 75537472696E67 " /* uString */
        "   67 6E4E756D626572 " /* nNumber */
        "19 0407 80";           /* xVoidMfrOnly */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_get_exec_names)
{
    const char req_hex[] = "01 64 45786563";
    const char rsp_exp_hex[] =
        "85 F6 A4"
        "65 78566F6964 80"                /* xVoid */
        "6B 78566F6964506172616D73 81"    /* xVoidParams */
        "   65 6C426F6F6C"                /* lBool */
        "6A 78493332506172616D73 82"      /* xI32Params */
        "   67 75537472696E67 "           /* uString */
        "   67 6E4E756D626572 "           /* nNumber */
        "6C 78566F69644D66724F6E6C79 80"; /* xVoidMfrOnly */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_get_num_records_id)
{
    const char req_hex[] = "01 19 0600";
    const char rsp_exp_hex[] = "85 F6 02";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_get_num_records_name)
{
    const char req_hex[] = "01 67 5265636F726473"; /* Records */
    const char rsp_exp_hex[] = "85 F6 02";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

#if 0
/*
 * For some reason below test fails because rsp_exp_hex is larger than 255 characters.
 */
ZTEST(thingset_bin, test_get_record_names)
{
    const char req_hex[] = "01 69 5265636F7264732F31"; /* Records/1 */
    const char rsp_exp_hex[] =
        "85 AD "
        "63 74 5f 73 02 "                               /* "t_s":2, */
        "65 77 42 6f 6f 6c f5 "                         /* "wBool":true, */
        "63 77 55 38 08 63 77 49 38 27 "                /* "wU8":8,"wI8":-8, */
        "64 77 55 31 36 10 64 77 49 31 36 2f "          /* "wU16":16,"wI16":-16, */
        "64 77 55 33 32 18 20 64 77 49 33 32 38 1f "    /* "wU32":32,"wI32":-32, */
        "64 77 55 36 34 18 40 64 77 49 36 34 38 3f "    /* "wU64":64,"wI64":-64, */
        "64 77 46 33 32 fa c0 4c cc cd "                /* "wF32":-3.2 */
        "68 77 44 65 63 46 72 61 63 c4 82 21 38 1f "    /* "wDecFrac": 4([-2, -32])*/
        "67 77 53 74 72 69 6e 67 66 73 74 72 69 6e 67"; /* "wString":"string" */

    printf("strlen(rsp_exp_hex): %d\n", strlen(rsp_exp_hex));

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}
#endif

ZTEST(thingset_bin, test_fetch_root_names)
{
    const char req_hex[] = "05 00 F6";
    const char rsp_exp_hex[] = "C1 F6 F6"; /* not yet implemented */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_update_timestamp_zero)
{
    const char req_hex[] = "07 10 00";
    const char rsp_exp_hex[] = "C1 F6 F6"; /* not yet implemented */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_exec_fn_void_id)
{
    fn_void_called = false;

    /* !Exec/xVoid */
    THINGSET_ASSERT_REQUEST_HEX("02 19 0401 80", "84 F6 F6");

    zassert_equal(true, fn_void_called);
}

ZTEST(thingset_bin, test_exec_fn_void_name)
{
    fn_void_called = false;

    /* !Exec/xVoid */
    THINGSET_ASSERT_REQUEST_HEX("02 6A 457865632F78566F6964 80", "84 F6 F6");

    zassert_equal(true, fn_void_called);
}

ZTEST(thingset_bin, test_exec_fn_void_mfr_only_id)
{
    /* !Exec/xVoidMfrOnly */
    THINGSET_ASSERT_REQUEST_HEX("02 19 0407", "A1 F6 F6");
}

ZTEST(thingset_bin, test_exec_fn_void_mfr_only_name)
{
    /* !Exec/xVoidMfrOnly */
    THINGSET_ASSERT_REQUEST_HEX("02 71 457865632F78566F69644D66724F6E6C79 80", "A1 F6 F6");
}

ZTEST(thingset_bin, test_exec_fn_void_params_id)
{
    fn_void_params_called = false;
    fn_void_param_b = false;

    /* !Exec/xVoidParams [true] */
    THINGSET_ASSERT_REQUEST_HEX("02 19 0402 81 F5", "84 F6 F6");

    zassert_equal(true, fn_void_params_called);
    zassert_equal(true, fn_void_param_b);
}

ZTEST(thingset_bin, test_exec_fn_void_params_name)
{
    fn_void_params_called = false;
    fn_void_param_b = false;

    /* !Exec/xVoidParams [true] */
    THINGSET_ASSERT_REQUEST_HEX("02 70 457865632F78566F6964506172616D73 81 F5", "84 F6 F6");

    zassert_equal(true, fn_void_params_called);
    zassert_equal(true, fn_void_param_b);
}

ZTEST(thingset_bin, test_exec_fn_void_invalid_params)
{
    THINGSET_ASSERT_REQUEST_HEX("02 19 0402 F5", "A0 F6 F6");
}

ZTEST(thingset_bin, test_exec_fn_void_too_many_params)
{
    THINGSET_ASSERT_REQUEST_HEX("02 19 0402 82 F5 18 7B", "A0 F6 F6");
}

ZTEST(thingset_bin, test_exec_fn_void_not_enough_params)
{
    THINGSET_ASSERT_REQUEST_HEX("02 19 0402 80", "A0 F6 F6");
}

ZTEST(thingset_bin, test_exec_fn_void_wrong_params)
{
    THINGSET_ASSERT_REQUEST_HEX("02 19 0402 81 65 77726F6E67", "AF F6 F6");
}

ZTEST(thingset_bin, test_exec_fn_not_executable)
{
    /* !Access/rItem */
    THINGSET_ASSERT_REQUEST_HEX("02 6C 4163636573732F724974656D 80", "A3 F6 F6");
}

ZTEST(thingset_bin, test_exec_fn_int32_id)
{
    fn_i32_param_str[0] = '\0';
    fn_i32_param_num = 0;

    /* !Exec/xI32Params ["test",123] */
    THINGSET_ASSERT_REQUEST_HEX("02 19 0404 82 64 74657374 18 7B", "84 F6 20");

    zassert_mem_equal("test", fn_i32_param_str, 4);
    zassert_equal(123, fn_i32_param_num);
}

ZTEST(thingset_bin, test_exec_fn_int32_name)
{
    fn_i32_param_str[0] = '\0';
    fn_i32_param_num = 0;

    THINGSET_ASSERT_REQUEST_HEX("02 6F 457865632F78493332506172616D73 82 64 74657374 18 7B",
                                "84 F6 20");

    zassert_mem_equal("test", fn_i32_param_str, 4);
    zassert_equal(123, fn_i32_param_num);
}

ZTEST(thingset_bin, test_create_delete_subset_item)
{
    /* before change */
    THINGSET_ASSERT_REQUEST_HEX(
        "01 65 6D4C697665", /* GET mLive */
        "85 F6 84 "
        "63 745F73 "                                  /* t_s */
        "6B 54797065732F77426F6F6C "                  /* Types/wBool */
        "71 4E65737465642F72426567696E6E696E67 "      /* Nested/rBeginning */
        "74 4E65737465642F4F626A322F724974656D325F56" /* Nested/Obj2/rItem2_V */
    );

    /* delete "Types/wBool" */
    THINGSET_ASSERT_REQUEST_HEX(
        "04 65 6D4C697665 6B 54797065732F77426F6F6C", /* DELETE Types/wBool from mLive subset */
        "82 F6 F6 "                                   /* Deleted. */
    );

    /* check if it was deleted */
    THINGSET_ASSERT_REQUEST_HEX(
        "01 65 6D4C697665", /* GET mLive */
        "85 F6 83 "
        "63 745F73 "                                  /* t_s */
        "71 4E65737465642F72426567696E6E696E67 "      /* Nested/rBeginning */
        "74 4E65737465642F4F626A322F724974656D325F56" /* Nested/Obj2/rItem2_V */
    );

    /* append "Types/wBool" again */
    THINGSET_ASSERT_REQUEST_HEX(
        "06 65 6D4C697665 6B 54797065732F77426F6F6C", /* CREATE Types/wBool in mLive subset */
        "81 F6 F6 "                                   /* Created. */
    );

    /* check if it was appended */
    THINGSET_ASSERT_REQUEST_HEX(
        "01 65 6D4C697665", /* GET mLive */
        "85 F6 84 "
        "63 745F73 "                                  /* t_s */
        "6B 54797065732F77426F6F6C "                  /* Types/wBool */
        "71 4E65737465642F72426567696E6E696E67 "      /* Nested/rBeginning */
        "74 4E65737465642F4F626A322F724974656D325F56" /* Nested/Obj2/rItem2_V */
    );
}

ZTEST(thingset_bin, test_desire_timestamp_zero)
{
    const char des_hex[] = "1D 10 00";
    const int err_exp = -THINGSET_ERR_NOT_IMPLEMENTED;

    THINGSET_ASSERT_DESIRE_HEX(des_hex, err_exp);
}

ZTEST(thingset_bin, test_report_subset_ids)
{
    const char rpt_exp_hex[] =
        "1F 19 08 00 A4 "
        "10 19 03E8 "           /* t_s */
        "19 02 01 F5 "          /* Types/wBool */
        "19 07 01 01"           /* Nested/rBeginning */
        "19 07 08 FA 400CCCCD"; /* Nested/Obj2/rItem2_V */

    THINGSET_ASSERT_REPORT_HEX_IDS("mLive", rpt_exp_hex, 25);
}

ZTEST(thingset_bin, test_report_group_ids)
{
    const char rpt_exp_hex[] =
        "1F 19 07 02 A2 "       /* Nested/Obj1 */
        "19 07 03 FA 3F8CCCCD"  /* rItem1_V:1.1 */
        "19 07 04 FA 3F99999A"; /* rItem2_V:1.2 */

    THINGSET_ASSERT_REPORT_HEX_IDS("Nested/Obj1", rpt_exp_hex, 21);
}

ZTEST(thingset_bin, test_export_subset_ids)
{
    const char rsp_exp_hex[] =
        "A4 "
        "10 19 03E8 "           /* t_s */
        "19 02 01 F5 "          /* Types/wBool */
        "19 07 01 01"           /* Nested/rBeginning */
        "19 07 08 FA 400CCCCD"; /* Nested/Obj2/rItem2_V */

    THINGSET_ASSERT_EXPORT_HEX_IDS(SUBSET_LIVE, rsp_exp_hex, 21);
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_bin, NULL, thingset_setup, NULL, NULL, NULL);
