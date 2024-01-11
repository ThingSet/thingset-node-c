/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include <math.h>

#include <thingset.h>

#include "data.h"
#include "test_utils.h"

static struct thingset_context ts;

ZTEST(thingset_bin, test_get_root_ids)
{
    const char req_hex[] = "01 00";
    const char rsp_exp_hex[] =
        "85 F6 AA "
        "10 19 03 E8 "                      /* t_s */
        "18 1D 68 41 42 43 44 31 32 33 34 " /* cNodeID */
        "19 02 00 F6 "                      /* Types */
        "19 03 00 F6 "                      /* Arrays */
        "19 04 00 F6 "                      /* Exec */
        "19 05 00 F6 "                      /* Access */
        "19 06 00 02 "                      /* Records */
        "19 06 80 0A "                      /* DynRecords */
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
        "85 F6 AA "
        "63 745F73 19 03E8 "                           /* t_s */
        "67 634E6F64654944 "                           /* cNodeID */
        "68 41 42 43 44 31 32 33 34 "                  /* ABCD1234 */
        "65 5479706573 F6 "                            /* Types */
        "66 417272617973 F6 "                          /* Arrays */
        "64 45786563 F6 "                              /* Exec */
        "66 416363657373 F6 "                          /* Access */
        "67 5265636F726473 02 "                        /* Records */
        "6A 44796E5265636F726473 0A "                  /* DynRecords */
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

ZTEST(thingset_bin, test_fetch_root_ids)
{
    const char req_hex[] = "05 00 F6";
    const char rsp_exp_hex[] =
        "85 f6 8A 10 18 1d 19 02 00 19 03 00 19 04 00 19 05 00 19 06 00 19 06 80 19 07 00 19 08 00";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_root_names)
{
    const char req_hex[] = "05 60 F6";
    const char rsp_exp_hex[] =
        "85 f6 8A "
        "63 74 5f 73 "             /* t_s */
        "67 63 4e 6f 64 65 49 44"  /* cNodeID */
        "65 54 79 70 65 73 "       /* Types */
        "66 41 72 72 61 79 73 "    /* Arrays */
        "64 45 78 65 63 "          /* Exec */
        "66 41 63 63 65 73 73 "    /* Access */
        "67 52 65 63 6f 72 64 73 " /* Records */
        "6A 44796E5265636F726473 " /* DynRecords */
        "66 4e 65 73 74 65 64 "    /* Nested */
        "65 6d 4c 69 76 65";       /* mLive */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_nested_ids)
{
    const char req_hex[] = "05 19 0700 F6";
    const char rsp_exp_hex[] = "85 F6 85 19 07 01 19 07 02 19 07 05 19 07 06 19 07 09";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_nested_names)
{
    const char req_hex[] = "05 66 4E6573746564 F6";
    const char rsp_exp_hex[] =
        "85 F6 85 "
        "6A 72426567696E6E696E67" /* rBeginning */
        "64 4F626A31"             /* Obj1 */
        "68 724265747765656E"     /* rBetween */
        "64 4F626A32"             /* Obj2 */
        "64 72456E64";            /* rEnd */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_node_id_id)
{
    const char req_hex[] = "05 00 81 18 1D";                   /* ? ["cNodeID"] */
    const char rsp_exp_hex[] = "85 F6 81 68 4142434431323334"; /* ["ABCD1234"] */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_node_id_name)
{
    const char req_hex[] = "05 00 81 67 634E6F64654944";       /* ? ["cNodeID"] */
    const char rsp_exp_hex[] = "85 F6 81 68 4142434431323334"; /* ["ABCD1234"] */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_bad_elem)
{
    const char req_hex[] = "05 00 81 F5"; /* ? [true] */
    const char rsp_exp_hex[] = "A0 F6 F6";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_not_found)
{
    const char req_hex[] = "05 00 81 63 666F6F"; /* ? ["foo"] */
    const char rsp_exp_hex[] = "A4 F6 F6";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_group)
{
    const char req_hex[] = "05 00 81 66 4E6573746564"; /* ? ["foo"] */
    const char rsp_exp_hex[] =
        "A0 F6 71 4e 65 73 74 65 64 20 69 73 20 61 20 67 72 6f 75 70"; /* Nested is a group */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_multiple)
{
    const char req_hex[] =
        "05 65 5479706573 83 " /* ?Types [*/
        "64 77463332"          /* "wF32", */
        "65 77426F6F6C"        /* "wBool", */
        "64 77553332";         /* "wU32"] */

    const char rsp_exp_hex[] = "85 F6 83 FA C04CCCCD F5 18 20"; /* [-3.2,true,32] */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_rounded)
{
    float bak = f32;
    f32 = 3.15;

    /* the binary float values are not rounded */
    THINGSET_ASSERT_REQUEST_HEX("05 19 0200 81 19 020A", "85 F6 81 FA 4049999A"); /* 3.15 */

    f32 = bak;
}

ZTEST(thingset_bin, test_fetch_nan)
{
    float bak = f32;
    uint32_t nan = 0x7F800001;
    f32 = *(float *)&nan;

    zassert_true(isnan(f32));

    /* for some reason the below fetch request may result in 0x7C800001, which is also NaN. The
     * assert cannot handle two or more possible values, so this test is disabled */
    /* THINGSET_ASSERT_REQUEST_HEX("05 19 0200 81 19 020A", "85 f6 81 fa 7f800001"); */

    f32 = bak;
}

ZTEST(thingset_bin, test_fetch_inf)
{
    float bak = f32;
    uint32_t inf = 0x7F800000;
    f32 = *(float *)&inf;

    THINGSET_ASSERT_REQUEST_HEX("05 19 0200 81 19 020A", "85 f6 81 fa 7f800000");

    f32 = bak;
}

ZTEST(thingset_bin, test_fetch_int32_array)
{
    const char req_hex[] = "05 19 0300 81 19 0307";    /* ?Arrays ["wI32"] */
    const char rsp_exp_hex[] = "85 F6 81 83 20 21 22"; /* [[-1,-2,-3]] */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_float_array)
{
    const char req_hex[] = "05 19 0300 81 19 030A"; /* ?Arrays ["wF32"] */
    const char rsp_exp_hex[] =
        "85 F6 81 83 fa bf 8c cc cd fa c0 0c cc cd fa c0 53 33 33"; /* [[-1.1,-2.2,-3.3]] */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_paths)
{
    const char req_hex[] =
        "05 "       /* FETCH */
        "17 "       /* _Paths */
        "84 "       /* array with 4 elements */
        "19 0207 "  /* uint 0x207 */
        "19 0601 "  /* uint 0x601 */
        "10 "       /* uint 0x10 */
        "19 0702 "; /* uint 0x702 */

    const char rsp_exp_hex[] =
        "85 F6 "
        "84 "                         /* array with 4 elements */
        "6A 54797065732F77493332 "    /* string "Types/wI32" */
        "6B 5265636F7264732F745F73 "  /* string "Records/t_s" */
        "63 745F73 "                  /* string "t_s" */
        "6B 4E65737465642F4F626A31 "; /* string "Nested/Obj1" */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_metadata)
{
    const char req_hex[] =
        "05 "       /* FETCH */
        "18 19 "    /* _Metadata */
        "84 "       /* array with 4 elements */
        "19 0207 "  /* uint 0x207 */
        "19 0601 "  /* uint 0x601 */
        "10 "       /* uint 0x10 */
        "19 0702 "; /* uint 0x702 */

    const char rsp_exp_hex[] =
        "85 F6 "
        "84 "                                                          /* array with 4 elements */
        "a2 64 6e 61 6d 65 64 77 49 33 32 64 74 79 70 65 63 69 33 32 " /* name: wI32, type: i32
                                                                        */
        "a2 64 6e 61 6d 65 63 74 5f 73 64 74 79 70 65 63 75 33 32 "    /* name: t_s, type: u32 */
        "a2 64 6e 61 6d 65 63 74 5f 73 64 74 79 70 65 63 75 33 32 "    /* name: t_s, type: u32 */
        "a2 64 6e 61 6d 65 64 4f 62 6a 31 64 74 79 70 65 65 67 72 6f 75 70"; /* name: Obj1,
                                                                               type: group */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_metadata_array)
{
    const char req_hex[] =
        "05 "       /* FETCH */
        "18 19 "    /* _Metadata */
        "81 "       /* array with 1 element */
        "19 0304 "; /* uint 0x304 */

    const char rsp_exp_hex[] =
        "85 F6 "
        "81 " /* array with 1 element */
        /* name: wU16, type: u16[] */
        "a2 64 6e 61 6d 65 64 77 55 31 36 64 74 79 70 65 65 75 31 36 5b 5d";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_metadata_void_func)
{
    const char req_hex[] =
        "05 "       /* FETCH */
        "18 19 "    /* _Metadata */
        "81 "       /* array with 1 element */
        "19 0401 "; /* uint 0x401 */

    const char rsp_exp_hex[] =
        "85 F6 "
        "81 " /* array with 1 element */
        /* name: xVoid, type: ()->() */
        "a2 64 6e 61 6d 65 65 78 56 6f 69 64 64 74 79 70 65 66 28 29 2d 3e 28 29 ";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_metadata_parameterised_func)
{
    const char req_hex[] =
        "05 "       /* FETCH */
        "18 19 "    /* _Metadata */
        "81 "       /* array with 1 element */
        "19 0404 "; /* uint 0x404 */

    const char rsp_exp_hex[] =
        "85 F6 "
        "81 " /* array with 1 element */
        /* name: xVoid, type: ()->() */
        "a2 64 6e 61 6d 65 6a 78 49 33 32 50 61 72 61 6d 73 64 74 79 70 65 73 28 73 74 72 69 6e 67 "
        "2c 69 33 32 29 2d 3e 28 69 33 32 29"; /* name: xI32Params, type: (string,i32)->(i32) */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_update_timestamp_zero_id)
{
    const char req_hex[] = "07 00 A1 10 00";
    const char rsp_exp_hex[] = "84 F6 F6";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
    zassert_equal(timestamp, 0);

    timestamp = 1000;
}

ZTEST(thingset_bin, test_update_timestamp_zero_name)
{
    const char req_hex[] = "07 00 A1 63 745F73 00";
    const char rsp_exp_hex[] = "84 F6 F6";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
    zassert_equal(timestamp, 0);

    timestamp = 1000;
}

ZTEST(thingset_bin, test_update_wrong_endpoint_id)
{
    const char req_hex[] = "07 00 A1 19 0206 00";
    const char rsp_exp_hex[] = "A0 F6 F6";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT

ZTEST(thingset_bin, test_update_bytes_buffer)
{
    const uint8_t bytes_exp[] = { 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 };

    /* =Types {"wBytes":"QUJDREVGRw=="} */
    THINGSET_ASSERT_REQUEST_HEX("07 19 0200 A1 19 020D 47 41424344454647", "84 F6 F6");

    zassert_equal(7, bytes_item.num_bytes);
    zassert_mem_equal(bytes_item.bytes, bytes_exp, sizeof(bytes_exp));
}

#else

ZTEST(thingset_bin, test_update_bytes_buffer)
{
    THINGSET_ASSERT_REQUEST_HEX("07 19 0200 A1 19 020D 47 41424344454647", "AF F6 F6");
}

#endif

ZTEST(thingset_bin, test_update_readonly)
{
    /* =Access {"rItem":52} */
    THINGSET_ASSERT_REQUEST_HEX("07 19 0500 A1 19 0501 18 34",
                                "A3 F6 77 49 74 65 6D 20 72 49 74 65 6D 20 69 73 20 72 65 61 64 2D "
                                "6F 6E 6C 79"); /* Item rItem is read-only */
}

ZTEST(thingset_bin, test_update_wrong_path)
{
    /* =Type {"wI32":52} */
    THINGSET_ASSERT_REQUEST_HEX(
        "07 64 54797065 A1 64 77493332 18 34",
        "A4 F6 70 49 6E 76 61 6C 69 64 20 65 6E 64 70 6F 69 6E 74"); /* Invalid endpoint */
}

ZTEST(thingset_bin, test_update_unknown_object)
{
    /* =Types {"wI3":52} */
    THINGSET_ASSERT_REQUEST_HEX("07 65 5479706573 A1 63 774933 18 34", "A4 F6 F6");
}

ZTEST(thingset_bin, test_update_group_callback)
{
    group_callback_pre_read_count = 0;
    group_callback_post_read_count = 0;
    group_callback_pre_write_count = 0;
    group_callback_post_write_count = 0;

    /* =Access {"wItem":1} */
    THINGSET_ASSERT_REQUEST_HEX("07 19 0500 A1 19 0502 01", "84 F6 F6");

    zassert_equal(group_callback_pre_read_count, 0);
    zassert_equal(group_callback_post_read_count, 0);
    zassert_equal(group_callback_pre_write_count, 1);
    zassert_equal(group_callback_post_write_count, 1);

    /* ?Access */
    THINGSET_ASSERT_REQUEST_HEX(
        "01 19 0500",
        "85 f6 a3 19 05 01 fa 3f 80 00 00 19 05 02 fa 3f 80 00 00 19 05 03 fa 3f 80 00 00");

    zassert_equal(group_callback_pre_read_count, 1);
    zassert_equal(group_callback_post_read_count, 1);
    zassert_equal(group_callback_pre_write_count, 1);
    zassert_equal(group_callback_post_write_count, 1);

    /* ?Access ["wItem"] */
    THINGSET_ASSERT_REQUEST_HEX("05 19 0500 81 19 0502", "85 F6 81 fa 3f 80 00 00");

    zassert_equal(group_callback_pre_read_count, 2);
    zassert_equal(group_callback_post_read_count, 2);
    zassert_equal(group_callback_pre_write_count, 1);
    zassert_equal(group_callback_post_write_count, 1);
}

ZTEST(thingset_bin, test_update_int32_array)
{
    const char req_hex[] = "07 19 0300 A1 19 0307 83 01 02 03"; /* =Arrays {"wI32":[1,2,3]} */
    const char rsp_exp_hex[] = "84 F6 F6";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);

    zassert_equal(i32_arr[0], 1);
    zassert_equal(i32_arr[1], 2);
    zassert_equal(i32_arr[2], 3);

    i32_arr[0] = -1;
    i32_arr[1] = -2;
    i32_arr[2] = -3;
}

ZTEST(thingset_bin, test_update_float_array)
{
    /* =Arrays {"wF32":[1.1,2.2,3.3]} */
    const char req_hex[] = "07 19 0300 A1 19 030A 83 FA 3f8ccccd FA 400ccccd FA 40533333";
    const char rsp_exp_hex[] = "84 F6 F6";

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);

    zassert_equal(f32_arr[0], (float)1.1);
    zassert_equal(f32_arr[1], (float)2.2);
    zassert_equal(f32_arr[2], (float)3.3);

    f32_arr[0] = -1.1;
    f32_arr[1] = -2.2;
    f32_arr[2] = -3.3;
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
    THINGSET_ASSERT_REQUEST_HEX("02 19 0407",
                                "A1 F6 77 41 75 74 68 65 6E 74 69 63 61 74 69 6F 6E 20 72 65 71 75 "
                                "69 72 65 64"); /* Authentication required */
}

ZTEST(thingset_bin, test_exec_fn_void_mfr_only_name)
{
    /* !Exec/xVoidMfrOnly */
    THINGSET_ASSERT_REQUEST_HEX("02 71 457865632F78566F69644D66724F6E6C79 80",
                                "A1 F6 77 41 75 74 68 65 6E 74 69 63 61 74 69 6F 6E 20 72 65 71 75 "
                                "69 72 65 64"); /* Authentication required */
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
    THINGSET_ASSERT_REQUEST_HEX(
        "02 19 0402 F5",
        "A0 F6 72 49 6E 76 61 6C 69 64 20 70 61 72 61 6D 65 74 65 72 73"); /* Invalid parameters */
}

ZTEST(thingset_bin, test_exec_fn_void_too_many_params)
{
    THINGSET_ASSERT_REQUEST_HEX(
        "02 19 0402 82 F5 18 7B",
        "A0 F6 73 54 6F 6F 20 6D 61 6E 79 20 70 61 72 61 6D 65 74 65 72 73"); /* Too many parameters
                                                                               */
}

ZTEST(thingset_bin, test_exec_fn_void_not_enough_params)
{
    THINGSET_ASSERT_REQUEST_HEX(
        "02 19 0402 80",
        "A0 F6 75 4E 6f 74 20 65 6E 6F 75 67 68 20 70 61 72 61 6D 65 74 65 72 73"); /* Not enough
                                                                                       parameters */
}

ZTEST(thingset_bin, test_exec_fn_void_wrong_params)
{
    THINGSET_ASSERT_REQUEST_HEX("02 19 0402 81 65 77726F6E67", "AF F6 F6");
}

ZTEST(thingset_bin, test_exec_fn_not_executable)
{
    /* !Access/rItem */
    THINGSET_ASSERT_REQUEST_HEX("02 6C 4163636573732F724974656D 80",
                                "A3 F6 77 72 49 74 65 6D 20 69 73 20 6E 6F 74 20 65 78 65 63 75 74 "
                                "61 62 6C 65"); /* rItem is not executable */
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

ZTEST(thingset_bin, test_export_item)
{
    struct thingset_endpoint endpoint;
    uint8_t buf_act[10];
    uint8_t buf_exp[10];
    int ret;

    /*
     * This value caused issues with previous implementations because it is serialized to 2 bytes
     * and serialize_finish erroneously added 0xF6.
     */
    ret = thingset_endpoint_by_path(&ts, &endpoint, "Types/wU32", strlen("Types/wU32"));
    zassert_equal(ret, 0);

    ret = thingset_export_item(&ts, buf_act, sizeof(buf_act), endpoint.object,
                               THINGSET_BIN_VALUES_ONLY);
    zassert_equal(ret, 2);
    hex2bin_spaced("18 20", buf_exp, sizeof(buf_exp));
    zassert_mem_equal(buf_exp, buf_act, 2);
}

ZTEST(thingset_bin, test_iterate_subsets)
{
    struct thingset_data_object *obj = NULL;
    uint8_t buf_act[10];
    uint8_t buf_exp[10];
    int err;

    /* t_s */
    obj = thingset_iterate_subsets(&ts, SUBSET_LIVE, obj);
    zassert_not_equal(obj, NULL);
    zassert_equal(obj->id, 0x10);

    err = thingset_export_item(&ts, buf_act, sizeof(buf_act), obj, THINGSET_BIN_VALUES_ONLY);
    hex2bin_spaced("19 03E8", buf_exp, sizeof(buf_exp));
    zassert_mem_equal(buf_exp, buf_act, 3);

    /* Types/wBool */
    obj = thingset_iterate_subsets(&ts, SUBSET_LIVE, ++obj);
    zassert_not_equal(obj, NULL);
    zassert_equal(obj->id, 0x201);

    /* Nested/rBeginning */
    obj = thingset_iterate_subsets(&ts, SUBSET_LIVE, ++obj);
    zassert_not_equal(obj, NULL);
    zassert_equal(obj->id, 0x701);

    /* Nested/Obj2/rItem2_V */
    obj = thingset_iterate_subsets(&ts, SUBSET_LIVE, ++obj);
    zassert_not_equal(obj, NULL);
    zassert_equal(obj->id, 0x708);

    obj = thingset_iterate_subsets(&ts, SUBSET_LIVE, ++obj);
    zassert_equal(obj, NULL);
}

ZTEST(thingset_bin, test_import_data)
{
    const char data_hex[] =
        "A2 "
        "10 19 03E9 "  /* t_s */
        "19 02 01 F4"; /* Types/wBool */

    THINGSET_ASSERT_IMPORT_HEX_IDS(data_hex, 0, THINGSET_WRITE_MASK);

    zassert_equal(timestamp, 1001);
    zassert_equal(b, false);

    /* reset to default values */
    timestamp = 1000;
    b = true;
}

ZTEST(thingset_bin, test_import_record)
{
    struct thingset_endpoint endpoint;
    uint8_t data[THINGSET_TEST_BUF_SIZE];
    int err;

    const char data_hex[] =
        "A2 "
        "19 06 02 F4"           /* Records/wBool */
        "19 06 0F 83 01 02 03"; /* Records/wF32Array */
    int data_len = hex2bin_spaced(data_hex, data, sizeof(data));

    err = thingset_endpoint_by_path(&ts, &endpoint, "Records/1", strlen("Records/1"));
    zassert_equal(err, 0);

    zassert_equal(records[1].b, true);
    zassert_equal(records[1].f32_arr[0], (float)1.23F);
    zassert_equal(records[1].f32_arr[1], (float)4.56F);
    zassert_equal(records[1].f32_arr[2], (float)7.89F);

    err = thingset_import_record(&ts, data, data_len, &endpoint, THINGSET_BIN_IDS_VALUES);
    zassert_equal(err, 0, "act: 0x%X", -err);

    zassert_equal(records[1].b, false);
    zassert_equal(records[1].f32_arr[0], (float)1.0F);
    zassert_equal(records[1].f32_arr[1], (float)2.0F);
    zassert_equal(records[1].f32_arr[2], (float)3.0F);

    records[1].b = true;
    records[1].f32_arr[0] = 1.23F;
    records[1].f32_arr[1] = 4.56F;
    records[1].f32_arr[2] = 7.89F;
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_bin, NULL, thingset_setup, NULL, NULL, NULL);
