/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TESTS_COMMON_DATA_H_
#define TESTS_COMMON_DATA_H_

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

#endif /* TESTS_COMMON_DATA_H_ */
