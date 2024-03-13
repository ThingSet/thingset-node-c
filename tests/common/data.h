/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TESTS_COMMON_DATA_H_
#define TESTS_COMMON_DATA_H_

#define SUBSET_LIVE (1U << 0)
#define SUBSET_NVM  (2U << 0)

extern uint32_t timestamp;

extern bool b;
extern uint8_t u8;
extern int8_t i8;
extern uint16_t u16;
extern int16_t i16;
extern uint32_t u32;
extern int32_t i32;
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
extern uint64_t u64;
extern int64_t i64;
#endif
extern float f32;
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
extern int32_t decfrac;
#endif
extern char strbuf[300];
extern uint8_t bytes_buf[18];
extern struct thingset_bytes bytes_item;

extern bool bool_arr[100];
extern uint8_t u8_arr[100];
extern int8_t i8_arr[100];
extern uint16_t u16_arr[100];
extern int16_t i16_arr[100];
extern uint32_t u32_arr[100];
extern int32_t i32_arr[100];
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
extern uint64_t u64_arr[100];
extern int64_t i64_arr[100];
#endif
extern float f32_arr[100];
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
extern int32_t decfrac_arr[100];
#endif

extern bool fn_void_called;
extern bool fn_void_params_called;
extern bool fn_void_param_b;
extern char fn_i32_param_str[100];
extern int32_t fn_i32_param_num;

extern int group_callback_pre_read_count;
extern int group_callback_post_read_count;
extern int group_callback_pre_write_count;
extern int group_callback_post_write_count;

extern int dyn_records_callback_pre_read_count;
extern int dyn_records_callback_post_read_count;
extern int dyn_records_callback_index;

/*
 * Records
 */
struct child_struct
{
    uint32_t child_u32;
    float child_f32;
};

struct test_struct
{
    uint32_t timestamp;
    uint8_t unused_element;
    bool b;
    uint8_t u8;
    int8_t i8;
    uint16_t u16;
    int16_t i16;
    uint32_t u32;
    int32_t i32;
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    uint64_t u64;
    int64_t i64;
#endif
    float f32;
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
    int32_t decfrac;
#endif
    char strbuf[300];
    float f32_arr[3];
    struct child_struct nested[2];
};

extern struct test_struct records[5];

/*
 * Dynamic Records
 */
struct test_dyn_struct
{
    uint32_t index;
};

#endif /* TESTS_COMMON_DATA_H_ */
