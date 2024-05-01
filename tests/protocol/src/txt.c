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

bool update_callback_called;

void update_callback(void)
{
    update_callback_called = true;
}

ZTEST(thingset_txt, test_get_root)
{
    const char req[] = "?";
    const char rsp_exp[] =
        ":85 {"
        "\"t_s\":1000,"
        "\"cNodeID\":\"ABCD1234\","
        "\"Types\":null,"
        "\"Arrays\":null,"
        "\"Exec\":null,"
        "\"Access\":null,"
        "\"Records\":2,"
        "\"DynRecords\":10,"
        "\"Nested\":null,"
        "\"mLive\":[\"t_s\",\"Types/wBool\",\"Records\",\"Nested/rBeginning\",\"Nested/Obj2/"
        "rItem2_V\"]"
        "}";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_get_nested)
{
    const char req[] = "?Nested";
    const char rsp_exp[] =
        ":85 {"
        "\"rBeginning\":1,"
        "\"Obj1\":null,"
        "\"rBetween\":2,"
        "\"Obj2\":null,"
        "\"rEnd\":3"
        "}";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_get_single_value)
{
    const char req[] = "?Nested/Obj1/rItem2_V";
    const char rsp_exp[] = ":85 1.2";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_get_exec)
{
    const char req[] = "?Exec";
    const char rsp_exp[] =
        ":85 {"
        "\"xVoid\":[],"
        "\"xVoidParams\":[\"lBool\"],"
        "\"xI32Params\":[\"uString\",\"nNumber\"],"
        "\"xVoidMfrOnly\":[]"
        "}";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_get_num_records)
{
    THINGSET_ASSERT_REQUEST_TXT("?Records", ":85 2");
}

ZTEST(thingset_txt, test_get_record)
{
    const char req[] = "?Records/1";
    const char rsp_exp[] =
        ":85 {"
        "\"t_s\":2,"
        "\"wBool\":true,"
        "\"wU8\":8,\"wI8\":-8,"
        "\"wU16\":16,\"wI16\":-16,"
        "\"wU32\":32,\"wI32\":-32,"
        "\"wU64\":64,\"wI64\":-64,"
        "\"wF32\":-3.2,\"wDecFrac\":-32e-2,"
        "\"wString\":\"string\","
        "\"wF32Array\":[1.2,4.6,7.9],"
        "\"Nested\":2"
        "}";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_get_dyn_record)
{
    const char req[] = "?DynRecords/7";
    const char rsp_exp[] = ":85 {\"rIndex\":7}";

    dyn_records_callback_pre_read_count = 0;
    dyn_records_callback_post_read_count = 0;
    dyn_records_callback_index = 0;

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);

    zassert_equal(dyn_records_callback_pre_read_count, 1);
    zassert_equal(dyn_records_callback_post_read_count, 1);
    zassert_equal(dyn_records_callback_index, 7);
}

ZTEST(thingset_txt, test_fetch_root_names)
{
    const char req[] = "? null";
    const char rsp_exp[] =
        ":85 ["
        "\"t_s\","
        "\"cNodeID\","
        "\"Types\","
        "\"Arrays\","
        "\"Exec\","
        "\"Access\","
        "\"Records\","
        "\"DynRecords\","
        "\"Nested\","
        "\"mLive\""
        "]";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_fetch_nested_names)
{
    const char req[] = "?Nested null";
    const char rsp_exp[] =
        ":85 ["
        "\"rBeginning\","
        "\"Obj1\","
        "\"rBetween\","
        "\"Obj2\","
        "\"rEnd\""
        "]";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_fetch_node_id)
{
    const char req[] = "? [\"cNodeID\"]";
    const char rsp_exp[] = ":85 [\"ABCD1234\"]";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_fetch_bad_elem)
{
    const char req[] = "? [true]";
    const char rsp_exp[] = ":A0";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_fetch_not_found)
{
    const char req[] = "? [\"foo\"]";
    const char rsp_exp[] = ":A4";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_fetch_group)
{
    const char req[] = "? [\"Nested\"]";
    const char rsp_exp[] = ":A0 \"Nested is a group\"";

    THINGSET_ASSERT_REQUEST_TXT(req, rsp_exp);
}

ZTEST(thingset_txt, test_fetch_multiple)
{
    THINGSET_ASSERT_REQUEST_TXT("?Types [\"wF32\",\"wBool\",\"wU32\"]", ":85 [-3.20,true,32]");
}

ZTEST(thingset_txt, test_fetch_rounded)
{
    float bak = f32;
    f32 = 3.115;

    THINGSET_ASSERT_REQUEST_TXT("?Types [\"wF32\"]", ":85 [3.12]");

    f32 = bak;
}

ZTEST(thingset_txt, test_fetch_nan)
{
    float bak = f32;
    uint32_t nan = 0x7F800001;
    f32 = *(float *)&nan;

    zassert_true(isnan(f32));
    THINGSET_ASSERT_REQUEST_TXT("?Types [\"wF32\"]", ":85 [null]");

    f32 = bak;
}

ZTEST(thingset_txt, test_fetch_inf)
{
    float bak = f32;
    uint32_t inf = 0x7F800000;
    f32 = *(float *)&inf;

    THINGSET_ASSERT_REQUEST_TXT("?Types [\"wF32\"]", ":85 [null]");

    f32 = bak;
}

ZTEST(thingset_txt, test_fetch_int32_array)
{
    THINGSET_ASSERT_REQUEST_TXT("?Arrays [\"wI32\"]", ":85 [[-1,-2,-3]]");
}

ZTEST(thingset_txt, test_fetch_float_array)
{
    THINGSET_ASSERT_REQUEST_TXT("?Arrays [\"wF32\"]", ":85 [[-1.1,-2.2,-3.3]]");
}

#ifdef CONFIG_THINGSET_METADATA_ENDPOINT

ZTEST(thingset_txt, test_fetch_metadata)
{
    THINGSET_ASSERT_REQUEST_TXT("?_Metadata [\"Arrays/wF32\"]",
                                ":85 [{\"name\":\"wF32\",\"type\":\"f32[]\"}]");
}

#endif /* CONFIG_THINGSET_METADATA_ENDPOINT */

#if CONFIG_THINGSET_JSON_STRING_ESCAPING

ZTEST(thingset_txt, test_fetch_escaped_string)
{
    strcpy(strbuf, "\n\"\\");

    THINGSET_ASSERT_REQUEST_TXT("?Types/wString", ":85 \"\\n\\\"\\\\\"");

    /* reset to default again */
    strcpy(strbuf, "string");
}

ZTEST(thingset_txt, test_update_escaped_string)
{
    const char str_exp[] = "\n\"\\";

    memset(strbuf, 0, 10);

    THINGSET_ASSERT_REQUEST_TXT("=Types {\"wString\":\"\\n\\\"\\\\\"}", ":84");

    zassert_mem_equal(strbuf, str_exp, sizeof(str_exp));

    /* reset to default again */
    strcpy(strbuf, "string");
}

#endif /* CONFIG_THINGSET_JSON_STRING_ESCAPING */

ZTEST(thingset_txt, test_update_timestamp_zero)
{
    THINGSET_ASSERT_REQUEST_TXT("= {\"t_s\":0}", ":84");
    zassert_equal(timestamp, 0);

    timestamp = 1000;
}

ZTEST(thingset_txt, test_update_wrong_data_structure)
{
    THINGSET_ASSERT_REQUEST_TXT("=Types [\"wF32\":54.3", ":A0 \"JSON parsing error\"");
    THINGSET_ASSERT_REQUEST_TXT("=Types{\"wF32\":54.3}", ":A4 \"Invalid endpoint\"");
}

ZTEST(thingset_txt, test_update_whitespaces)
{
    THINGSET_ASSERT_REQUEST_TXT("=Types {    \"wF32\" : 52.8,\"wI32\":50.6}", ":84");

    zassert_equal((float)52.8, f32);
    zassert_equal(50, i32);

    f32 = -3.2F;
    i32 = -32;
}

#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT

ZTEST(thingset_txt, test_update_bytes_buffer)
{
    THINGSET_ASSERT_REQUEST_TXT("=Types {\"wBytes\":\"QUJDREVGRw==\"}", ":84");

    zassert_equal(7, bytes_item.num_bytes);
}

#else

ZTEST(thingset_txt, test_update_bytes_buffer)
{
    THINGSET_ASSERT_REQUEST_TXT("=Types {\"wBytes\":\"QUJDREVGRw==\"}", ":AF");
}

#endif

ZTEST(thingset_txt, test_update_readonly)
{
    THINGSET_ASSERT_REQUEST_TXT("=Access {\"rItem\" : 52}", ":A3 \"Item rItem is read-only\"");
}

ZTEST(thingset_txt, test_update_wrong_path)
{
    THINGSET_ASSERT_REQUEST_TXT("=Type {\"wI32\" : 52}", ":A4 \"Invalid endpoint\"");
}

ZTEST(thingset_txt, test_update_unknown_object)
{
    THINGSET_ASSERT_REQUEST_TXT("=Types {\"wI3\" : 52}", ":A4");
}

ZTEST(thingset_txt, test_update_int32_array)
{
    THINGSET_ASSERT_REQUEST_TXT("=Arrays {\"wI32\":[1,2,3]}", ":84");

    zassert_equal(i32_arr[0], 1);
    zassert_equal(i32_arr[1], 2);
    zassert_equal(i32_arr[2], 3);

    i32_arr[0] = -1;
    i32_arr[1] = -2;
    i32_arr[2] = -3;
}

ZTEST(thingset_txt, test_update_float_array)
{
    THINGSET_ASSERT_REQUEST_TXT("=Arrays {\"wF32\":[1.1,2.2,3.3]}", ":84");

    zassert_equal(f32_arr[0], (float)1.1);
    zassert_equal(f32_arr[1], (float)2.2);
    zassert_equal(f32_arr[2], (float)3.3);

    f32_arr[0] = -1.1;
    f32_arr[1] = -2.2;
    f32_arr[2] = -3.3;
}

ZTEST(thingset_txt, test_group_callback)
{
    group_callback_pre_read_count = 0;
    group_callback_post_read_count = 0;
    group_callback_pre_write_count = 0;
    group_callback_post_write_count = 0;

    THINGSET_ASSERT_REQUEST_TXT("=Access {\"wItem\":1}", ":84");

    zassert_equal(group_callback_pre_read_count, 0);
    zassert_equal(group_callback_post_read_count, 0);
    zassert_equal(group_callback_pre_write_count, 1);
    zassert_equal(group_callback_post_write_count, 1);

    THINGSET_ASSERT_REQUEST_TXT("?Access", ":85 {\"rItem\":1.00,\"wItem\":1.00,\"wMfrOnly\":1.00}");

    zassert_equal(group_callback_pre_read_count, 1);
    zassert_equal(group_callback_post_read_count, 1);
    zassert_equal(group_callback_pre_write_count, 1);
    zassert_equal(group_callback_post_write_count, 1);

    THINGSET_ASSERT_REQUEST_TXT("?Access [\"wItem\"]", ":85 [1.00]");

    zassert_equal(group_callback_pre_read_count, 2);
    zassert_equal(group_callback_post_read_count, 2);
    zassert_equal(group_callback_pre_write_count, 1);
    zassert_equal(group_callback_post_write_count, 1);

    THINGSET_ASSERT_REQUEST_TXT("?Access/wItem", ":85 1.00");

    zassert_equal(group_callback_pre_read_count, 3);
    zassert_equal(group_callback_post_read_count, 3);
    zassert_equal(group_callback_pre_write_count, 1);
    zassert_equal(group_callback_post_write_count, 1);
}

ZTEST(thingset_txt, test_exec_fn_void)
{
    fn_void_called = false;

    THINGSET_ASSERT_REQUEST_TXT("!Exec/xVoid", ":84");

    zassert_equal(true, fn_void_called);
}

ZTEST(thingset_txt, test_exec_fn_void_mfr_only)
{
    THINGSET_ASSERT_REQUEST_TXT("!Exec/xVoidMfrOnly", ":A1 \"Authentication required\"");
}

ZTEST(thingset_txt, test_exec_fn_void_params)
{
    fn_void_params_called = false;
    fn_void_param_b = false;

    THINGSET_ASSERT_REQUEST_TXT("!Exec/xVoidParams [true]", ":84");

    zassert_equal(true, fn_void_params_called);
    zassert_equal(true, fn_void_param_b);
}

ZTEST(thingset_txt, test_exec_fn_void_invalid_json)
{
    THINGSET_ASSERT_REQUEST_TXT("!Exec/xVoidParams [true", ":A0 \"JSON parsing error\"");
}

ZTEST(thingset_txt, test_exec_fn_void_invalid_params)
{
    THINGSET_ASSERT_REQUEST_TXT("!Exec/xVoidParams true", ":A0 \"Invalid parameters\"");
}

ZTEST(thingset_txt, test_exec_fn_void_too_many_params)
{
    THINGSET_ASSERT_REQUEST_TXT("!Exec/xVoidParams [true, 123]", ":A0 \"Too many parameters\"");
}

ZTEST(thingset_txt, test_exec_fn_void_not_enough_params)
{
    THINGSET_ASSERT_REQUEST_TXT("!Exec/xVoidParams", ":A0 \"Not enough parameters\"");
}

ZTEST(thingset_txt, test_exec_fn_void_wrong_params)
{
    THINGSET_ASSERT_REQUEST_TXT("!Exec/xVoidParams [\"wrong\"]", ":AF");
}

ZTEST(thingset_txt, test_exec_fn_not_executable)
{
    THINGSET_ASSERT_REQUEST_TXT("!Access/rItem", ":A3 \"rItem is not executable\"");
}

ZTEST(thingset_txt, test_exec_fn_int32)
{
    fn_i32_param_str[0] = '\0';
    fn_i32_param_num = 0;

    THINGSET_ASSERT_REQUEST_TXT("!Exec/xI32Params [\"test\",123]", ":84 -1");

    zassert_mem_equal("test", fn_i32_param_str, 4);
    zassert_equal(123, fn_i32_param_num);
}

ZTEST(thingset_txt, test_create_delete_subset_item)
{
    /* before change */
    THINGSET_ASSERT_REQUEST_TXT(
        "?mLive",
        ":85 [\"t_s\",\"Types/wBool\",\"Records\",\"Nested/rBeginning\",\"Nested/Obj2/rItem2_V\"]");

    /* delete "Types/wBool" */
    THINGSET_ASSERT_REQUEST_TXT("-mLive \"Types/wBool\"", ":82");

    /* check if it was deleted */
    THINGSET_ASSERT_REQUEST_TXT(
        "?mLive", ":85 [\"t_s\",\"Records\",\"Nested/rBeginning\",\"Nested/Obj2/rItem2_V\"]");

    /* append "Types/wBool" again */
    THINGSET_ASSERT_REQUEST_TXT("+mLive \"Types/wBool\"", ":81");

    /* check if it was appended */
    THINGSET_ASSERT_REQUEST_TXT(
        "?mLive",
        ":85 [\"t_s\",\"Types/wBool\",\"Records\",\"Nested/rBeginning\",\"Nested/Obj2/rItem2_V\"]");
}

ZTEST(thingset_txt, test_create_root_item)
{
    THINGSET_ASSERT_REQUEST_TXT("+ \"Test\"", ":A0 \"Endpoint item required\"");
}

ZTEST(thingset_txt, test_create_multiple_values)
{
    THINGSET_ASSERT_REQUEST_TXT("+mLive [\"Types/wBool\",\"Types/wI32\"]", ":AF");
}

ZTEST(thingset_txt, test_create_non_existing_item)
{
    THINGSET_ASSERT_REQUEST_TXT("+mLive \"Types/wBoo\"", ":A4");
}

ZTEST(thingset_txt, test_create_wrong_type_item)
{
    THINGSET_ASSERT_REQUEST_TXT("+mLive 123", ":AF");
}

ZTEST(thingset_txt, test_create_array_item)
{
    THINGSET_ASSERT_REQUEST_TXT("+Arrays/wBool/- true", ":C1 \"Arrays not yet supported\"");
}

ZTEST(thingset_txt, test_create_single_value)
{
    THINGSET_ASSERT_REQUEST_TXT("+Types/wBool true", ":A5");
}

ZTEST(thingset_txt, test_create_non_existing_endpoint)
{
    THINGSET_ASSERT_REQUEST_TXT("+foo 1", ":A4 \"Invalid endpoint\"");
}

ZTEST(thingset_txt, test_desire_timestamp_zero)
{
    const char des[] = "@t_s 0";
    const int err_exp = -THINGSET_ERR_NOT_IMPLEMENTED;

    THINGSET_ASSERT_DESIRE_TXT(des, err_exp);
}

ZTEST(thingset_txt, test_report_subset)
{
    const char rpt_exp[] =
        "#mLive {"
        "\"t_s\":1000,"
        "\"Types\":{\"wBool\":true},"
        "\"Records\":2,"
        "\"Nested\":{\"rBeginning\":1,\"Obj2\":{\"rItem2_V\":2.2}}"
        "}";

    THINGSET_ASSERT_REPORT_TXT("mLive", rpt_exp, strlen(rpt_exp));
}

ZTEST(thingset_txt, test_report_group)
{
    const char rpt_exp[] =
        "#Nested/Obj1 {"
        "\"rItem1_V\":1.1,"
        "\"rItem2_V\":1.2"
        "}";

    THINGSET_ASSERT_REPORT_TXT("Nested/Obj1", rpt_exp, strlen(rpt_exp));
}

ZTEST(thingset_txt, test_report_record)
{
    const char rpt_exp[] =
        "#Records/1 {"
        "\"t_s\":2,"
        "\"wBool\":true,"
        "\"wU8\":8,\"wI8\":-8,"
        "\"wU16\":16,\"wI16\":-16,"
        "\"wU32\":32,\"wI32\":-32,"
        "\"wU64\":64,\"wI64\":-64,"
        "\"wF32\":-3.2,\"wDecFrac\":-32e-2,"
        "\"wString\":\"string\","
        "\"wF32Array\":[1.2,4.6,7.9],"
        "\"Nested\":2"
        "}";

    THINGSET_ASSERT_REPORT_TXT("Records/1", rpt_exp, strlen(rpt_exp));
}

ZTEST(thingset_txt, test_report_dyn_record)
{
    const char rpt_exp[] = "#DynRecords/3 {\"rIndex\":3}";

    dyn_records_callback_pre_read_count = 0;
    dyn_records_callback_post_read_count = 0;
    dyn_records_callback_index = 0;

    THINGSET_ASSERT_REPORT_TXT("DynRecords/3", rpt_exp, strlen(rpt_exp));

    zassert_equal(dyn_records_callback_pre_read_count, 1);
    zassert_equal(dyn_records_callback_post_read_count, 1);
    zassert_equal(dyn_records_callback_index, 3);
}

ZTEST(thingset_txt, test_export_subset)
{
    const char rsp_exp[] =
        "{"
        "\"t_s\":1000,"
        "\"Types\":{\"wBool\":true},"
        "\"Records\":2,"
        "\"Nested\":{\"rBeginning\":1,\"Obj2\":{\"rItem2_V\":2.2}}"
        "}";

    THINGSET_ASSERT_EXPORT_TXT(SUBSET_LIVE, rsp_exp, strlen(rsp_exp));
}

ZTEST(thingset_txt, test_update_callback)
{
    update_callback_called = false;

    /* without callback */
    thingset_set_update_callback(&ts, SUBSET_NVM, NULL);
    THINGSET_ASSERT_REQUEST_TXT("=Access {\"wItem\":1}", ":84");
    zassert_equal(false, update_callback_called);

    /* with configured callback */
    thingset_set_update_callback(&ts, SUBSET_NVM, update_callback);
    THINGSET_ASSERT_REQUEST_TXT("=Access {\"wItem\":1}", ":84");
    zassert_equal(true, update_callback_called);
}

ZTEST(thingset_txt, test_auth)
{
    /* before authentication */
    thingset_set_authentication(&ts, THINGSET_USR_MASK);
    THINGSET_ASSERT_REQUEST_TXT("=Access {\"wMfrOnly\":1}",
                                ":A1 \"Authentication required for wMfrOnly\"");
    THINGSET_ASSERT_REQUEST_TXT("!Exec/xVoidMfrOnly", ":A1 \"Authentication required\"");

    /* after authentication */
    thingset_set_authentication(&ts, THINGSET_USR_MASK | THINGSET_MFR_MASK);
    THINGSET_ASSERT_REQUEST_TXT("=Access {\"wItem\":1}", ":84");
    THINGSET_ASSERT_REQUEST_TXT("!Exec/xVoidMfrOnly", ":84");

    /* reset authentication */
    thingset_set_authentication(&ts, THINGSET_USR_MASK);
}

ZTEST(thingset_txt, test_import_record)
{
    struct thingset_endpoint endpoint;
    int err;

    char *data = "{\"wBool\":false}";

    err = thingset_endpoint_by_path(&ts, &endpoint, "Records/1", strlen("Records/1"));
    zassert_equal(err, 0);

    zassert_equal(records[1].b, true);

    err = thingset_import_record(&ts, data, strlen(data), &endpoint, THINGSET_TXT_NAMES_VALUES);
    zassert_equal(err, 0, "act: 0x%X", -err);

    zassert_equal(records[1].b, false);

    records[1].b = true;
}

static bool thingset_txt_predicate(const void *global_state)
{
    return IS_ENABLED(CONFIG_THINGSET_TEXT_MODE);
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_txt, thingset_txt_predicate, thingset_setup, NULL, NULL, NULL);
