/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include <thingset.h>

#include "data.h"

/* Pre-defined data items */
char node_id[] = "ABCD1234";
uint32_t timestamp = 1000;

/* Types */
bool b = true;
uint8_t u8 = 8;
int8_t i8 = -8;
uint16_t u16 = 16;
int16_t i16 = -16;
uint32_t u32 = 32;
int32_t i32 = -32;
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
uint64_t u64 = 64;
int64_t i64 = -64;
#endif
float f32 = -3.2F;
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
int32_t decfrac = -32;
#endif
char strbuf[300] = "string";
uint8_t bytes_buf[18] = "bytes";
THINGSET_DEFINE_BYTES(bytes_item, bytes_buf, 0);

/*
 * Arrays
 */
bool bool_arr[100] = { true, false, true };
static THINGSET_DEFINE_BOOL_ARRAY(bool_arr_item, bool_arr, 3);

uint8_t u8_arr[100] = { 1, 2, 3 };
static THINGSET_DEFINE_UINT8_ARRAY(u8_arr_item, u8_arr, 3);

int8_t i8_arr[100] = { -1, -2, -3 };
static THINGSET_DEFINE_INT8_ARRAY(i8_arr_item, i8_arr, 3);

uint16_t u16_arr[100] = { 1, 2, 3 };
static THINGSET_DEFINE_UINT16_ARRAY(u16_arr_item, u16_arr, 3);

int16_t i16_arr[100] = { -1, -2, -3 };
static THINGSET_DEFINE_INT16_ARRAY(i16_arr_item, i16_arr, 3);

uint32_t u32_arr[100] = { 1, 2, 3 };
static THINGSET_DEFINE_UINT32_ARRAY(u32_arr_item, u32_arr, 3);

int32_t i32_arr[100] = { -1, -2, -3 };
static THINGSET_DEFINE_INT32_ARRAY(i32_arr_item, i32_arr, 3);

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
uint64_t u64_arr[100] = { 1, 2, 3 };
static THINGSET_DEFINE_UINT64_ARRAY(u64_arr_item, u64_arr, 3);

int64_t i64_arr[100] = { -1, -2, -3 };
static THINGSET_DEFINE_INT64_ARRAY(i64_arr_item, i64_arr, 3);
#endif

float f32_arr[100] = { -1.1, -2.2, -3.3 };
static THINGSET_DEFINE_FLOAT_ARRAY(f32_arr_item, 1, f32_arr, 3);

#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
int32_t decfrac_arr[100] = { -32 };
static THINGSET_DEFINE_DECFRAC_ARRAY(decfrac_arr_item, 2, decfrac_arr, 1);
#endif

/* Exec */
bool fn_void_called;
static void fn_void()
{
    fn_void_called = true;
}

bool fn_void_params_called;
bool fn_void_param_b;
static void fn_void_params()
{
    fn_void_params_called = true;
}

char fn_i32_param_str[100];
int32_t fn_i32_param_num;
static int32_t fn_i32_params()
{
    return -1;
}

/* Access */
static float access_item;

/* Callback for groups */
int group_callback_pre_read_count;
int group_callback_post_read_count;
int group_callback_pre_write_count;
int group_callback_post_write_count;
static void group_callback(enum thingset_callback_reason reason)
{
    switch (reason) {
        case THINGSET_CALLBACK_PRE_READ:
            group_callback_pre_read_count++;
            break;
        case THINGSET_CALLBACK_POST_READ:
            group_callback_post_read_count++;
            break;
        case THINGSET_CALLBACK_PRE_WRITE:
            group_callback_pre_write_count++;
            break;
        case THINGSET_CALLBACK_POST_WRITE:
            group_callback_post_write_count++;
            break;
    }
}

/* Records */
struct test_struct records[5] = {
    {
        .timestamp = 1,
        .unused_element = 0xFF,
    },
    {
        .timestamp = 2,
        .b = true,
        .u8 = 8,
        .i8 = -8,
        .u16 = 16,
        .i16 = -16,
        .u32 = 32,
        .i32 = -32,
        .u64 = 64,
        .i64 = -64,
        .f32 = -3.2F,
        .decfrac = -32,
        .strbuf = "string",
        .f32_arr = { 1.23F, 4.56F, 7.89F },
        .nested = {
            { 32, 1.23F },
            { 16, 4.56F },
        }
    },
};

THINGSET_DEFINE_RECORDS(records_obj, records, 2);

THINGSET_DEFINE_RECORD_FLOAT_ARRAY(f32_array_record, 1, struct test_struct, f32_arr);

THINGSET_DEFINE_RECORD_RECORDS(nested_records_obj, struct test_struct, nested);

/* Dynamic record */
static struct test_dyn_struct dyn_records = {
    .index = 0,
};

/* Callback for dynamic records */
int dyn_records_callback_pre_read_count;
int dyn_records_callback_post_read_count;
int dyn_records_callback_index;
static void dyn_records_callback(enum thingset_callback_reason reason, int index)
{
    switch (reason) {
        case THINGSET_CALLBACK_PRE_READ:
            /* assign the variable in the record dynamically (here just using the index itself) */
            dyn_records.index = index;

            dyn_records_callback_pre_read_count++;
            break;
        case THINGSET_CALLBACK_POST_READ:
            /* reset variable again (for testing purposes) */
            dyn_records.index = 0;

            dyn_records_callback_post_read_count++;
            break;
        default:
            return;
    }
    dyn_records_callback_index = index;
}

THINGSET_DEFINE_DYN_RECORDS(dyn_records_obj, &dyn_records, 10, dyn_records_callback);

/* Nested */
static int32_t nested_beginning = 1;
static float nested_obj1_item1 = 1.1F;
static float nested_obj1_item2 = 1.2F;
static int32_t nested_between = 2;
static float nested_obj2_item1 = 2.1F;
static float nested_obj2_item2 = 2.2F;
static int32_t nested_end = 3;

/*
 * Global data object definitions using iterable sections.
 */

/* Pre-defined data items */
THINGSET_ADD_ITEM_UINT32(THINGSET_ID_ROOT, 0x10, "t_s", &timestamp, THINGSET_ANY_RW, SUBSET_LIVE);
THINGSET_ADD_ITEM_STRING(THINGSET_ID_ROOT, 0x1D, "cNodeID", node_id, sizeof(node_id),
                         THINGSET_ANY_R | THINGSET_MFR_W, 0);

/* Types */
THINGSET_ADD_GROUP(THINGSET_ID_ROOT, 0x200, "Types", THINGSET_NO_CALLBACK);
THINGSET_ADD_ITEM_BOOL(0x200, 0x201, "wBool", &b, THINGSET_ANY_RW, SUBSET_LIVE);
THINGSET_ADD_ITEM_UINT8(0x200, 0x202, "wU8", &u8, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_INT8(0x200, 0x203, "wI8", &i8, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_UINT16(0x200, 0x204, "wU16", &u16, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_INT16(0x200, 0x205, "wI16", &i16, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_UINT32(0x200, 0x206, "wU32", &u32, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_INT32(0x200, 0x207, "wI32", &i32, THINGSET_ANY_RW, 0);
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
THINGSET_ADD_ITEM_UINT64(0x200, 0x208, "wU64", &u64, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_INT64(0x200, 0x209, "wI64", &i64, THINGSET_ANY_RW, 0);
#endif
THINGSET_ADD_ITEM_FLOAT(0x200, 0x20A, "wF32", &f32, 2, THINGSET_ANY_RW, 0);
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
THINGSET_ADD_ITEM_DECFRAC(0x200, 0x20B, "wDecFrac", &decfrac, 2, THINGSET_ANY_RW, 0);
#endif
THINGSET_ADD_ITEM_STRING(0x200, 0x20C, "wString", strbuf, sizeof(strbuf), THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_BYTES(0x200, 0x20D, "wBytes", &bytes_item, THINGSET_ANY_RW, 0);

/* Arrays */
THINGSET_ADD_GROUP(THINGSET_ID_ROOT, 0x300, "Arrays", THINGSET_NO_CALLBACK);
THINGSET_ADD_ITEM_ARRAY(0x300, 0x301, "wBool", &bool_arr_item, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_ARRAY(0x300, 0x302, "wU8", &u8_arr_item, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_ARRAY(0x300, 0x303, "wI8", &i8_arr_item, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_ARRAY(0x300, 0x304, "wU16", &u16_arr_item, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_ARRAY(0x300, 0x305, "wI16", &i16_arr_item, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_ARRAY(0x300, 0x306, "wU32", &u32_arr_item, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_ARRAY(0x300, 0x307, "wI32", &i32_arr_item, THINGSET_ANY_RW, 0);
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
THINGSET_ADD_ITEM_ARRAY(0x300, 0x308, "wU64", &u64_arr_item, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_ARRAY(0x300, 0x309, "wI64", &i64_arr_item, THINGSET_ANY_RW, 0);
#endif
THINGSET_ADD_ITEM_ARRAY(0x300, 0x30A, "wF32", &f32_arr_item, THINGSET_ANY_RW, 0);
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
THINGSET_ADD_ITEM_ARRAY(0x300, 0x30B, "wDecFrac", &decfrac_arr_item, THINGSET_ANY_RW, 0);
#endif

/* Exec */
THINGSET_ADD_GROUP(THINGSET_ID_ROOT, 0x400, "Exec", THINGSET_NO_CALLBACK);
THINGSET_ADD_FN_VOID(0x400, 0x401, "xVoid", &fn_void, THINGSET_ANY_RW);
THINGSET_ADD_FN_VOID(0x400, 0x402, "xVoidParams", &fn_void_params, THINGSET_ANY_RW);
THINGSET_ADD_ITEM_BOOL(0x402, 0x403, "lBool", &fn_void_param_b, THINGSET_ANY_RW, 0);
THINGSET_ADD_FN_INT32(0x400, 0x404, "xI32Params", &fn_i32_params, THINGSET_ANY_RW);
THINGSET_ADD_ITEM_STRING(0x404, 0x405, "uString", fn_i32_param_str, sizeof(fn_i32_param_str),
                         THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_INT32(0x404, 0x406, "nNumber", &fn_i32_param_num, THINGSET_ANY_RW, 0);
THINGSET_ADD_FN_VOID(0x400, 0x407, "xVoidMfrOnly", &fn_void, THINGSET_ANY_R | THINGSET_MFR_RW);

/* Access */
THINGSET_ADD_GROUP(THINGSET_ID_ROOT, 0x500, "Access", group_callback);
THINGSET_ADD_ITEM_FLOAT(0x500, 0x501, "rItem", &access_item, 2, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(0x500, 0x502, "wItem", &access_item, 2, THINGSET_ANY_RW, SUBSET_NVM);
THINGSET_ADD_ITEM_FLOAT(0x500, 0x503, "wMfrOnly", &access_item, 2, THINGSET_ANY_R | THINGSET_MFR_RW,
                        0);

/* Records */
THINGSET_ADD_RECORDS(THINGSET_ID_ROOT, 0x600, "Records", &records_obj, THINGSET_ANY_R, SUBSET_LIVE);
THINGSET_ADD_RECORD_ITEM_UINT32(0x600, 0x601, "t_s", struct test_struct, timestamp);
THINGSET_ADD_RECORD_ITEM_BOOL(0x600, 0x602, "wBool", struct test_struct, b);
THINGSET_ADD_RECORD_ITEM_UINT8(0x600, 0x603, "wU8", struct test_struct, u8);
THINGSET_ADD_RECORD_ITEM_INT8(0x600, 0x604, "wI8", struct test_struct, i8);
THINGSET_ADD_RECORD_ITEM_UINT16(0x600, 0x605, "wU16", struct test_struct, u16);
THINGSET_ADD_RECORD_ITEM_INT16(0x600, 0x606, "wI16", struct test_struct, i16);
THINGSET_ADD_RECORD_ITEM_UINT32(0x600, 0x607, "wU32", struct test_struct, u32);
THINGSET_ADD_RECORD_ITEM_INT32(0x600, 0x608, "wI32", struct test_struct, i32);
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
THINGSET_ADD_RECORD_ITEM_UINT64(0x600, 0x609, "wU64", struct test_struct, u64);
THINGSET_ADD_RECORD_ITEM_INT64(0x600, 0x60A, "wI64", struct test_struct, i64);
#endif
THINGSET_ADD_RECORD_ITEM_FLOAT(0x600, 0x60B, "wF32", struct test_struct, f32, 1);
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
THINGSET_ADD_RECORD_ITEM_DECFRAC(0x600, 0x60C, "wDecFrac", struct test_struct, decfrac, 2);
#endif
THINGSET_ADD_RECORD_ITEM_STRING(0x600, 0x60D, "wString", struct test_struct, strbuf,
                                sizeof(records[0].strbuf));
THINGSET_ADD_RECORD_ITEM_ARRAY(0x600, 0x60F, "wF32Array", &f32_array_record);
THINGSET_ADD_RECORD_ITEM_RECORD(0x600, 0x610, "Nested", &nested_records_obj);
THINGSET_ADD_RECORD_ITEM_UINT32(0x610, 0x611, "wU32", struct child_struct, child_u32);
THINGSET_ADD_RECORD_ITEM_FLOAT(0x610, 0x612, "wF32", struct child_struct, child_f32, 2);

/* Dynamic Records */
THINGSET_ADD_DYN_RECORDS(THINGSET_ID_ROOT, 0x680, "DynRecords", &dyn_records_obj, THINGSET_ANY_R,
                         0);
THINGSET_ADD_RECORD_ITEM_UINT32(0x680, 0x681, "rIndex", struct test_dyn_struct, index);

/* Nested */
THINGSET_ADD_GROUP(THINGSET_ID_ROOT, 0x700, "Nested", THINGSET_NO_CALLBACK);
THINGSET_ADD_ITEM_INT32(0x700, 0x701, "rBeginning", &nested_beginning, THINGSET_ANY_RW,
                        SUBSET_LIVE);
THINGSET_ADD_GROUP(0x700, 0x702, "Obj1", THINGSET_NO_CALLBACK);
THINGSET_ADD_ITEM_FLOAT(0x702, 0x703, "rItem1_V", &nested_obj1_item1, 1, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_FLOAT(0x702, 0x704, "rItem2_V", &nested_obj1_item2, 1, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_INT32(0x700, 0x705, "rBetween", &nested_between, THINGSET_ANY_RW, 0);
THINGSET_ADD_GROUP(0x700, 0x706, "Obj2", THINGSET_NO_CALLBACK);
THINGSET_ADD_ITEM_FLOAT(0x706, 0x707, "rItem1_V", &nested_obj2_item1, 1, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_FLOAT(0x706, 0x708, "rItem2_V", &nested_obj2_item2, 1, THINGSET_ANY_RW,
                        SUBSET_LIVE);
THINGSET_ADD_ITEM_INT32(0x700, 0x709, "rEnd", &nested_end, THINGSET_ANY_RW, 0);

/* Subset */
THINGSET_ADD_SUBSET(THINGSET_ID_ROOT, 0x800, "mLive", SUBSET_LIVE, THINGSET_ANY_RW);

/*
 * Same data objects defined using data object array macros.
 */

struct thingset_data_object data_objects[] = {
    /* Pre-defined data items */
    THINGSET_ITEM_UINT32(THINGSET_ID_ROOT, 0x10, "t_s", &timestamp, THINGSET_ANY_RW, SUBSET_LIVE),
    THINGSET_ITEM_STRING(THINGSET_ID_ROOT, 0x1D, "cNodeID", node_id, sizeof(node_id),
                         THINGSET_ANY_R | THINGSET_MFR_W, 0),

    /* Types */
    THINGSET_GROUP(THINGSET_ID_ROOT, 0x200, "Types", THINGSET_NO_CALLBACK),
    THINGSET_ITEM_BOOL(0x200, 0x201, "wBool", &b, THINGSET_ANY_RW, SUBSET_LIVE),
    THINGSET_ITEM_UINT8(0x200, 0x202, "wU8", &u8, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_INT8(0x200, 0x203, "wI8", &i8, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_UINT16(0x200, 0x204, "wU16", &u16, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_INT16(0x200, 0x205, "wI16", &i16, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_UINT32(0x200, 0x206, "wU32", &u32, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_INT32(0x200, 0x207, "wI32", &i32, THINGSET_ANY_RW, 0),
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    THINGSET_ITEM_UINT64(0x200, 0x208, "wU64", &u64, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_INT64(0x200, 0x209, "wI64", &i64, THINGSET_ANY_RW, 0),
#endif
    THINGSET_ITEM_FLOAT(0x200, 0x20A, "wF32", &f32, 2, THINGSET_ANY_RW, 0),
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
    THINGSET_ITEM_DECFRAC(0x200, 0x20B, "wDecFrac", &decfrac, 2, THINGSET_ANY_RW, 0),
#endif
    THINGSET_ITEM_STRING(0x200, 0x20C, "wString", strbuf, sizeof(strbuf), THINGSET_ANY_RW, 0),
    THINGSET_ITEM_BYTES(0x200, 0x20D, "wBytes", &bytes_item, THINGSET_ANY_RW, 0),

    /* Arrays */
    THINGSET_GROUP(THINGSET_ID_ROOT, 0x300, "Arrays", THINGSET_NO_CALLBACK),
    THINGSET_ITEM_ARRAY(0x300, 0x301, "wBool", &bool_arr_item, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_ARRAY(0x300, 0x302, "wU8", &u8_arr_item, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_ARRAY(0x300, 0x303, "wI8", &i8_arr_item, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_ARRAY(0x300, 0x304, "wU16", &u16_arr_item, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_ARRAY(0x300, 0x305, "wI16", &i16_arr_item, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_ARRAY(0x300, 0x306, "wU32", &u32_arr_item, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_ARRAY(0x300, 0x307, "wI32", &i32_arr_item, THINGSET_ANY_RW, 0),
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    THINGSET_ITEM_ARRAY(0x300, 0x308, "wU64", &u64_arr_item, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_ARRAY(0x300, 0x309, "wI64", &i64_arr_item, THINGSET_ANY_RW, 0),
#endif
    THINGSET_ITEM_ARRAY(0x300, 0x30A, "wF32", &f32_arr_item, THINGSET_ANY_RW, 0),
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
    THINGSET_ITEM_ARRAY(0x300, 0x30B, "wDecFrac", &decfrac_arr_item, THINGSET_ANY_RW, 0),
#endif

    /* Exec */
    THINGSET_GROUP(THINGSET_ID_ROOT, 0x400, "Exec", THINGSET_NO_CALLBACK),
    THINGSET_FN_VOID(0x400, 0x401, "xVoid", &fn_void, THINGSET_ANY_RW),
    THINGSET_FN_VOID(0x400, 0x402, "xVoidParams", &fn_void_params, THINGSET_ANY_RW),
    THINGSET_ITEM_BOOL(0x402, 0x403, "lBool", &fn_void_param_b, THINGSET_ANY_RW, 0),
    THINGSET_FN_INT32(0x400, 0x404, "xI32Params", &fn_i32_params, THINGSET_ANY_RW),
    THINGSET_ITEM_STRING(0x404, 0x405, "uString", fn_i32_param_str, sizeof(fn_i32_param_str),
                         THINGSET_ANY_RW, 0),
    THINGSET_ITEM_INT32(0x404, 0x406, "nNumber", &fn_i32_param_num, THINGSET_ANY_RW, 0),
    THINGSET_FN_VOID(0x400, 0x407, "xVoidMfrOnly", &fn_void, THINGSET_ANY_R | THINGSET_MFR_RW),

    /* Access */
    THINGSET_GROUP(THINGSET_ID_ROOT, 0x500, "Access", group_callback),
    THINGSET_ITEM_FLOAT(0x500, 0x501, "rItem", &access_item, 2, THINGSET_ANY_R, 0),
    THINGSET_ITEM_FLOAT(0x500, 0x502, "wItem", &access_item, 2, THINGSET_ANY_RW, SUBSET_NVM),
    THINGSET_ITEM_FLOAT(0x500, 0x503, "wMfrOnly", &access_item, 2, THINGSET_ANY_R | THINGSET_MFR_RW,
                        0),

    /* Records */
    THINGSET_RECORDS(THINGSET_ID_ROOT, 0x600, "Records", &records_obj, THINGSET_ANY_R, SUBSET_LIVE),
    THINGSET_RECORD_ITEM_UINT32(0x600, 0x601, "t_s", struct test_struct, timestamp),
    THINGSET_RECORD_ITEM_BOOL(0x600, 0x602, "wBool", struct test_struct, b),
    THINGSET_RECORD_ITEM_UINT8(0x600, 0x603, "wU8", struct test_struct, u8),
    THINGSET_RECORD_ITEM_INT8(0x600, 0x604, "wI8", struct test_struct, i8),
    THINGSET_RECORD_ITEM_UINT16(0x600, 0x605, "wU16", struct test_struct, u16),
    THINGSET_RECORD_ITEM_INT16(0x600, 0x606, "wI16", struct test_struct, i16),
    THINGSET_RECORD_ITEM_UINT32(0x600, 0x607, "wU32", struct test_struct, u32),
    THINGSET_RECORD_ITEM_INT32(0x600, 0x608, "wI32", struct test_struct, i32),
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    THINGSET_RECORD_ITEM_UINT64(0x600, 0x609, "wU64", struct test_struct, u64),
    THINGSET_RECORD_ITEM_INT64(0x600, 0x60A, "wI64", struct test_struct, i64),
#endif
    THINGSET_RECORD_ITEM_FLOAT(0x600, 0x60B, "wF32", struct test_struct, f32, 1),
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
    THINGSET_RECORD_ITEM_DECFRAC(0x600, 0x60C, "wDecFrac", struct test_struct, decfrac, 2),
#endif
    THINGSET_RECORD_ITEM_STRING(0x600, 0x60D, "wString", struct test_struct, strbuf,
                                sizeof(records[0].strbuf)),
    THINGSET_RECORD_ITEM_ARRAY(0x600, 0x60F, "wF32Array", &f32_array_record),
    THINGSET_RECORD_ITEM_RECORD(0x600, 0x610, "Nested", &nested_records_obj),
    THINGSET_RECORD_ITEM_UINT32(0x610, 0x611, "wU32", struct child_struct, child_u32),
    THINGSET_RECORD_ITEM_FLOAT(0x610, 0x612, "wF32", struct child_struct, child_f32, 2),

    /* Dynamic Records */
    THINGSET_DYN_RECORDS(THINGSET_ID_ROOT, 0x680, "DynRecords", &dyn_records_obj, THINGSET_ANY_R,
                         0),
    THINGSET_RECORD_ITEM_UINT32(0x680, 0x681, "rIndex", struct test_dyn_struct, index),

    /* Nested */
    THINGSET_GROUP(THINGSET_ID_ROOT, 0x700, "Nested", THINGSET_NO_CALLBACK),
    THINGSET_ITEM_INT32(0x700, 0x701, "rBeginning", &nested_beginning, THINGSET_ANY_RW,
                        SUBSET_LIVE),
    THINGSET_GROUP(0x700, 0x702, "Obj1", THINGSET_NO_CALLBACK),
    THINGSET_ITEM_FLOAT(0x702, 0x703, "rItem1_V", &nested_obj1_item1, 1, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_FLOAT(0x702, 0x704, "rItem2_V", &nested_obj1_item2, 1, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_INT32(0x700, 0x705, "rBetween", &nested_between, THINGSET_ANY_RW, 0),
    THINGSET_GROUP(0x700, 0x706, "Obj2", THINGSET_NO_CALLBACK),
    THINGSET_ITEM_FLOAT(0x706, 0x707, "rItem1_V", &nested_obj2_item1, 1, THINGSET_ANY_RW, 0),
    THINGSET_ITEM_FLOAT(0x706, 0x708, "rItem2_V", &nested_obj2_item2, 1, THINGSET_ANY_RW,
                        SUBSET_LIVE),
    THINGSET_ITEM_INT32(0x700, 0x709, "rEnd", &nested_end, THINGSET_ANY_RW, 0),

    /* Subset */
    THINGSET_SUBSET(THINGSET_ID_ROOT, 0x800, "mLive", SUBSET_LIVE, THINGSET_ANY_RW),
};

size_t data_objects_size = ARRAY_SIZE(data_objects);
