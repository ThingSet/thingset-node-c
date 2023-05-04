/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THINGSET_THINGSET_H_
#define THINGSET_THINGSET_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define JSMN_HEADER
#include "jsmn.h"

#ifndef _ARRAY_SIZE
/* custom ARRAY_SIZE to avoid redefinition warning if thingset.h is included before Zephr headers */
#define _ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Function codes (binary mode) */
#define THINGSET_BIN_GET    0x01
#define THINGSET_BIN_EXEC   0x02
#define THINGSET_BIN_DELETE 0x04
#define THINGSET_BIN_FETCH  0x05
#define THINGSET_BIN_CREATE 0x06
#define THINGSET_BIN_UPDATE 0x07
#define THINGSET_BIN_DESIRE 0x1D
#define THINGSET_BIN_REPORT 0x1F

/* Function codes (text mode) */
#define THINGSET_TXT_GET_FETCH '?'
#define THINGSET_TXT_EXEC      '!'
#define THINGSET_TXT_DELETE    '-'
#define THINGSET_TXT_CREATE    '+'
#define THINGSET_TXT_UPDATE    '='
#define THINGSET_TXT_DESIRE    '@'
#define THINGSET_TXT_REPORT    '#'

/* Status codes (success) */
#define THINGSET_STATUS_CREATED 0x81
#define THINGSET_STATUS_DELETED 0x82
#define THINGSET_STATUS_CHANGED 0x84
#define THINGSET_STATUS_CONTENT 0x85

/* Status codes (client errors) */
#define THINGSET_ERR_BAD_REQUEST        0xA0
#define THINGSET_ERR_UNAUTHORIZED       0xA1
#define THINGSET_ERR_FORBIDDEN          0xA3
#define THINGSET_ERR_NOT_FOUND          0xA4
#define THINGSET_ERR_METHOD_NOT_ALLOWED 0xA5
#define THINGSET_ERR_REQUEST_INCOMPLETE 0xA8
#define THINGSET_ERR_CONFLICT           0xA9
#define THINGSET_ERR_REQUEST_TOO_LARGE  0xAD
#define THINGSET_ERR_UNSUPPORTED_FORMAT 0xAF

/* Status codes (server errors) */
#define THINGSET_ERR_INTERNAL_SERVER_ERR 0xC0
#define THINGSET_ERR_NOT_IMPLEMENTED     0xC1
#define THINGSET_ERR_GATEWAY_TIMEOUT     0xC4
#define THINGSET_ERR_NOT_A_GATEWAY       0xC5

/* Status codes (ThingSet specific errors) */
#define THINGSET_ERR_RESPONSE_TOO_LARGE 0xE1

#define THINGET_ERROR(code)   (code >= 0xA0)
#define THINGET_SUCCESS(code) (code >= 0x80 && code < 0xA0)

/* Reserved data object IDs */
#define THINGSET_ID_ROOT        0x00
#define THINGSET_ID_TIME        0x10
#define THINGSET_ID_IDS         0x16
#define THINGSET_ID_PATHS       0x17
#define THINGSET_ID_METADATAURL 0x18
#define THINGSET_ID_NODEID      0x1D

/*
 * Macros for defining data object array elements.
 */

/** Create data item for bool variable. */
#define THINGSET_ITEM_BOOL(parent_id, id, name, bool_ptr, access, subsets) \
    { \
        parent_id, id, name, { .b = bool_ptr }, THINGSET_TYPE_BOOL, 0, access, subsets \
    }

/** Create data item for uint8_t variable. */
#define THINGSET_ITEM_UINT8(parent_id, id, name, uint8_ptr, access, subsets) \
    { \
        parent_id, id, name, { .u8 = uint8_ptr }, THINGSET_TYPE_U8, 0, access, subsets \
    }

/** Create data item for int8_t variable. */
#define THINGSET_ITEM_INT8(parent_id, id, name, int8_ptr, access, subsets) \
    { \
        parent_id, id, name, { .i8 = int8_ptr }, THINGSET_TYPE_I8, 0, access, subsets \
    }

/** Create data item for uint16_t variable. */
#define THINGSET_ITEM_UINT16(parent_id, id, name, uint16_ptr, access, subsets) \
    { \
        parent_id, id, name, { .u16 = uint16_ptr }, THINGSET_TYPE_U16, 0, access, subsets \
    }

/** Create data item for int16_t variable. */
#define THINGSET_ITEM_INT16(parent_id, id, name, int16_ptr, access, subsets) \
    { \
        parent_id, id, name, { .i16 = int16_ptr }, THINGSET_TYPE_I16, 0, access, subsets \
    }

/** Create data item for uint32_t variable. */
#define THINGSET_ITEM_UINT32(parent_id, id, name, uint32_ptr, access, subsets) \
    { \
        parent_id, id, name, { .u32 = uint32_ptr }, THINGSET_TYPE_U32, 0, access, subsets \
    }

/** Create data item for int32_t variable. */
#define THINGSET_ITEM_INT32(parent_id, id, name, int32_ptr, access, subsets) \
    { \
        parent_id, id, name, { .i32 = int32_ptr }, THINGSET_TYPE_I32, 0, access, subsets \
    }

/** Create data item for uint64_t variable. */
#define THINGSET_ITEM_UINT64(parent_id, id, name, uint64_ptr, access, subsets) \
    { \
        parent_id, id, name, { .u64 = uint64_ptr }, THINGSET_TYPE_U64, 0, access, subsets \
    }

/** Create data item for int64_t variable. */
#define THINGSET_ITEM_INT64(parent_id, id, name, int64_ptr, access, subsets) \
    { \
        parent_id, id, name, { .i64 = int64_ptr }, THINGSET_TYPE_I64, 0, access, subsets \
    }

/** Create data item for float variable. */
#define THINGSET_ITEM_FLOAT(parent_id, id, name, float_ptr, decimals, access, subsets) \
    { \
        parent_id, id, name, { .f32 = float_ptr }, THINGSET_TYPE_F32, decimals, access, subsets \
    }

/**
 * Create data item for decimal fraction variable. The mantissa is internally stored as int32_t.
 * The value is converted into a float (JSON) or decimal fraction type (CBOR) for the protocol,
 * based on the specified (fixed) exponent, which is equivalent to the decimal digits.
 */
#define THINGSET_ITEM_DECFRAC(parent_id, id, name, mantissa_ptr, decimals, access, subsets) \
    { \
        parent_id, id, name, { .i32 = mantissa_ptr }, THINGSET_TYPE_DECFRAC, decimals, access, \
            subsets \
    }

/**
 * Create data item for a string buffer. The string must be null-terminated and buf_size contains
 * the maximum length of the buffer including the null-termination character.
 */
#define THINGSET_ITEM_STRING(parent_id, id, name, char_ptr, buf_size, access, subsets) \
    { \
        parent_id, id, name, { .str = char_ptr }, THINGSET_TYPE_STRING, buf_size, access, subsets \
    }

/**
 * Create data item for a byte buffer containing arbitrary binary data. In contrast to string
 * buffers, no null-termination is used.
 */
#define THINGSET_ITEM_BYTES(parent_id, id, name, bytes_ptr, access, subsets) \
    { \
        parent_id, id, name, { .bytes = bytes_ptr }, THINGSET_TYPE_BYTES, 0, access, subsets \
    }

/** Create an executable data object (function) with void return value. */
#define THINGSET_FN_VOID(parent_id, id, name, void_fn_ptr, access) \
    { \
        parent_id, id, name, { .void_fn = void_fn_ptr }, THINGSET_TYPE_FN_VOID, 0, access, 0 \
    }

/** Create an executable data object (function) with int32_t return value. */
#define THINGSET_FN_INT32(parent_id, id, name, int32_fn_ptr, access) \
    { \
        parent_id, id, name, { .i32_fn = int32_fn_ptr }, THINGSET_TYPE_FN_I32, 0, access, 0 \
    }

/** Create a data object pointing to a struct thingset_array. */
#define THINGSET_ITEM_ARRAY(parent_id, id, name, array_ptr, access, subsets) \
    { \
        parent_id, id, name, { .array = array_ptr }, THINGSET_TYPE_ARRAY, 0, access, subsets \
    }

/** Create a data object pointing to a struct thingset_records. */
#define THINGSET_RECORDS(parent_id, id, name, records_ptr, access, subsets) \
    { \
        parent_id, id, name, { .records = records_ptr }, THINGSET_TYPE_RECORDS, 0, access, subsets \
    }

/** Create a subset data object for the provided subset flag. */
#define THINGSET_SUBSET(parent_id, id, name, subset_flag, access) \
    { \
        parent_id, id, name, { .subset = subset_flag }, THINGSET_TYPE_SUBSET, 0, access, 0 \
    }

/** Create a group for hierarchical structuring of the data. */
#define THINGSET_GROUP(parent_id, id, name, void_fn_cb_ptr) \
    { \
        parent_id, id, name, { .void_fn = void_fn_cb_ptr }, THINGSET_TYPE_GROUP, 0, \
            THINGSET_READ_MASK \
    }

/** Create record data item for bool variable. */
#define THINGSET_RECORD_ITEM_BOOL(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_BOOL \
    }

/** Create record item for uint64_t variable. */
#define THINGSET_RECORD_ITEM_UINT64(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, THINGSET_TYPE_U64 \
    }

/** Create record item for int64_t variable. */
#define THINGSET_RECORD_ITEM_INT64(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, THINGSET_TYPE_I64 \
    }

/** Create record data item for uint32_t variable. */
#define THINGSET_RECORD_ITEM_UINT32(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, THINGSET_TYPE_U32 \
    }

/** Create record data item for int32_t variable. */
#define THINGSET_RECORD_ITEM_INT32(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, THINGSET_TYPE_I32 \
    }

/** Create record data item for uint16_t variable. */
#define THINGSET_RECORD_ITEM_UINT16(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, THINGSET_TYPE_U16 \
    }

/** Create record data item for int16_t variable. */
#define THINGSET_RECORD_ITEM_INT16(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, THINGSET_TYPE_I16 \
    }

/** Create record data item for uint8_t variable. */
#define THINGSET_RECORD_ITEM_UINT8(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, THINGSET_TYPE_U8 \
    }

/** Create record data item for int8_t variable. */
#define THINGSET_RECORD_ITEM_INT8(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, THINGSET_TYPE_I8 \
    }

/** Create record data item for float variable. */
#define THINGSET_RECORD_ITEM_FLOAT(parent_id, id, name, struct_type, struct_member, decimals) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_F32, decimals \
    }

/** Create record data item for decimal fraction variable. */
#define THINGSET_RECORD_ITEM_DECFRAC(parent_id, id, name, struct_type, struct_member, decimals) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_DECFRAC, decimals \
    }

/** Create record data item for string variable. */
#define THINGSET_RECORD_ITEM_STRING(parent_id, id, name, struct_type, struct_member, buf_size) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_STRING, buf_size \
    }

/*
 * Macros for defining data objects using Zephyr iterable sections.
 *
 * These macros can be used to populate the data objects array from multiple independent files.
 *
 * See also: https://docs.zephyrproject.org/latest/kernel/iterable_sections/index.html
 */

/** @cond INTERNAL_HIDDEN */

#ifdef CONFIG_THINGSET_IMMUTABLE_OBJECTS
#define MAYBE_CONST const
#else
#define MAYBE_CONST
#endif

#define _THINGSET_ADD_ITERABLE_SECTION(type, parent_id, id, ...) \
    MAYBE_CONST STRUCT_SECTION_ITERABLE(thingset_data_object, _CONCAT(obj_, id)) = \
        _CONCAT(THINGSET_, type)(parent_id, id, __VA_ARGS__)
/** @endcond */

#define THINGSET_ADD_ITEM_BOOL(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_BOOL, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_UINT64(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_UINT64, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_INT64(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_INT64, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_UINT32(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_UINT32, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_INT32(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_INT32, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_UINT16(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_UINT16, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_INT16(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_INT16, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_UINT8(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_UINT8, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_INT8(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_INT8, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_FLOAT(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_FLOAT, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_DECFRAC(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_DECFRAC, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_STRING(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_STRING, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_BYTES(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_BYTES, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_ITEM_ARRAY(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_ARRAY, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_FN_VOID(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(FN_VOID, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_FN_INT32(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(FN_INT32, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_SUBSET(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(SUBSET, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_GROUP(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(GROUP, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORDS(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORDS, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_BOOL(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_BOOL, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_UINT64(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_UINT64, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_INT64(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_INT64, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_UINT32(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_UINT32, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_INT32(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_INT32, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_UINT16(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_UINT16, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_INT16(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_INT16, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_UINT8(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_UINT8, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_INT8(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_INT8, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_FLOAT(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_FLOAT, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_DECFRAC(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_DECFRAC, parent_id, id, __VA_ARGS__)
#define THINGSET_ADD_RECORD_ITEM_STRING(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_STRING, parent_id, id, __VA_ARGS__)

/** Define a struct thingset_bytes to be used with THINGSET_ITEM_BYTES() */
#define THINGSET_DEFINE_BYTES(var_name, buffer, used_bytes) \
    struct thingset_bytes var_name = { buffer, sizeof(buffer), used_bytes };

/** Define a struct thingset_array to expose uint8_t arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_BOOL_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .b = array }, THINGSET_TYPE_BOOL, 0, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_array to expose uint8_t arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_UINT8_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .u8 = array }, THINGSET_TYPE_U8, 0, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_array to expose int8_t arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_INT8_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .i8 = array }, THINGSET_TYPE_I8, 0, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_array to expose uint16_t arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_UINT16_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .u16 = array }, THINGSET_TYPE_U16, 0, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_array to expose int16_t arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_INT16_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .i16 = array }, THINGSET_TYPE_I16, 0, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_array to expose uint32_t arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_UINT32_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .u32 = array }, THINGSET_TYPE_U32, 0, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_array to expose int32_t arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_INT32_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .i32 = array }, THINGSET_TYPE_I32, 0, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_array to expose uint64_t arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_UINT64_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .u64 = array }, THINGSET_TYPE_U64, 0, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_array to expose int64_t arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_INT64_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .i64 = array }, THINGSET_TYPE_I64, 0, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_array to expose float arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_FLOAT_ARRAY(var_name, array, decimals, used_elements) \
    struct thingset_array var_name = { \
        { .f32 = array }, THINGSET_TYPE_F32, decimals, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_array to expose dec frac arrays with THINGSET_ITEM_ARRAY() */
#define THINGSET_DEFINE_DECFRAC_ARRAY(var_name, array, decimals, used_elements) \
    struct thingset_array var_name = { \
        { .decfrac = array }, THINGSET_TYPE_DECFRAC, decimals, _ARRAY_SIZE(array), used_elements, \
    };

/** Define a struct thingset_records to be used with THINGSET_RECORDS() */
#define THINGSET_DEFINE_RECORDS(var_name, records, used_records) \
    struct thingset_records var_name = { records, sizeof(__typeof__(*records)), \
                                         ARRAY_SIZE(records), used_records };

/*
 * Defines to make data object definitions more explicit
 */
#define THINGSET_NO_CALLBACK NULL /**< No callback assigned to group */

/*
 * Access right macros for data objects
 */

/** @cond INTERNAL_HIDDEN */

#define THINGSET_ROLE_USR (1U << 0) /**< Normal user */
#define THINGSET_ROLE_EXP (1U << 1) /**< Expert user */
#define THINGSET_ROLE_MFR (1U << 2) /**< Manufacturer */
#define THINGSET_ROLE_ANY (THINGSET_USR_R | THINGSET_EXP_R | THINGSET_MFR_R)

#define THINGSET_READ_MASK  0x0F /**< Read flags stored in 4 least-significant bits */
#define THINGSET_WRITE_MASK 0xF0 /**< Write flags stored in 4 most-significant bits */

/* Masks for different roles */
#define THINGSET_USR_MASK (THINGSET_ROLE_USR << 4 | THINGSET_ROLE_USR)
#define THINGSET_EXP_MASK (THINGSET_ROLE_EXP << 4 | THINGSET_ROLE_EXP)
#define THINGSET_MFR_MASK (THINGSET_ROLE_MFR << 4 | THINGSET_ROLE_MFR)

#define THINGSET_READ(roles)       ((roles)&THINGSET_READ_MASK)
#define THINGSET_WRITE(roles)      (((roles) << 4) & THINGSET_WRITE_MASK)
#define THINGSET_READ_WRITE(roles) (THINGSET_READ(roles) | THINGSET_WRITE(roles))

/** @endcond */

#define THINGSET_USR_R THINGSET_READ(THINGSET_ROLE_USR) /**< Read-only access for normal user */
#define THINGSET_EXP_R THINGSET_READ(THINGSET_ROLE_EXP) /**< Read-only access for expert user */
#define THINGSET_MFR_R THINGSET_READ(THINGSET_ROLE_MFR) /**< Read-only access for manufacturer */
#define THINGSET_ANY_R THINGSET_READ(THINGSET_ROLE_ANY) /**< Read-only access for any user */

#define THINGSET_USR_W THINGSET_WRITE(THINGSET_ROLE_USR) /**< Write-only access for normal user */
#define THINGSET_EXP_W THINGSET_WRITE(THINGSET_ROLE_EXP) /**< Write-only access for expert user */
#define THINGSET_MFR_W THINGSET_WRITE(THINGSET_ROLE_MFR) /**< Write-only access for manuf. */
#define THINGSET_ANY_W THINGSET_WRITE(THINGSET_ROLE_ANY) /**< Write-only access for any user */

#define THINGSET_USR_RW \
    THINGSET_READ_WRITE(THINGSET_ROLE_USR) /**< Read/write access for normal user */
#define THINGSET_EXP_RW \
    THINGSET_READ_WRITE(THINGSET_ROLE_EXP) /**< Read/write access for expert user */
#define THINGSET_MFR_RW \
    THINGSET_READ_WRITE(THINGSET_ROLE_MFR) /**< Read/write access for manufacturer */
#define THINGSET_ANY_RW \
    THINGSET_READ_WRITE(THINGSET_ROLE_ANY) /**< Read/write access for any user */

/** ThingSet data object ID (16-bit) */
typedef uint16_t thingset_object_id_t;

/**
 * Internal C data types (used together with union thingset_data_pointer)
 */
enum thingset_type
{
    THINGSET_TYPE_BOOL,    /**< bool */
    THINGSET_TYPE_U8,      /**< uint8_t */
    THINGSET_TYPE_I8,      /**< int8_t */
    THINGSET_TYPE_U16,     /**< uint16_t */
    THINGSET_TYPE_I16,     /**< int16_t */
    THINGSET_TYPE_U32,     /**< uint32_t */
    THINGSET_TYPE_I32,     /**< int32_t */
    THINGSET_TYPE_U64,     /**< uint64_t */
    THINGSET_TYPE_I64,     /**< int64_t */
    THINGSET_TYPE_F32,     /**< float */
    THINGSET_TYPE_DECFRAC, /**< decimal fraction */
    THINGSET_TYPE_STRING,  /**< String buffer (UTF-8 text) */
    THINGSET_TYPE_BYTES,   /**< Byte buffer (binary data) */
    THINGSET_TYPE_ARRAY,   /**< Array */
    THINGSET_TYPE_RECORDS, /**< Records (array of arbitrary struct objects) */
    THINGSET_TYPE_GROUP,   /**< Internal object to describe data hierarchy */
    THINGSET_TYPE_SUBSET,  /**< Subset of data items */
    THINGSET_TYPE_FN_VOID, /**< Function with void return value */
    THINGSET_TYPE_FN_I32,  /**< Function with int32_t return value */
};

/**
 * Helper function to determine the size of above ThingSet types in bytes
 */
static inline size_t thingset_type_size(uint8_t type)
{
    uint8_t sizes[] = { sizeof(bool),    sizeof(uint8_t),  sizeof(int8_t),  sizeof(uint16_t),
                        sizeof(int16_t), sizeof(uint32_t), sizeof(int32_t), sizeof(uint64_t),
                        sizeof(int64_t), sizeof(float),    sizeof(int32_t) };
    return type < sizeof(sizes) ? sizes[type] : 0;
}

/**
 * Union for type-checking of provided data item variable pointers through the macros.
 */
union thingset_data_pointer {
    bool *b;
    uint8_t *u8;
    int8_t *i8;
    uint16_t *u16;
    int16_t *i16;
    uint32_t *u32;
    int32_t *i32;
    uint64_t *u64;
    int64_t *i64;
    float *f32;
    int32_t *decfrac;
    char *str;
    struct thingset_bytes *bytes;
    struct thingset_array *array;
    struct thingset_records *records;
    size_t offset;
    uint32_t subset;
    void (*void_fn)();
    int32_t (*i32_fn)();
};

/**
 * Data structure to specify a binary data buffer
 */
struct thingset_bytes
{
    const uint8_t *bytes;     /**< Pointer to the bytes buffer */
    const uint16_t max_bytes; /**< Maximum number of bytes in the buffer */
    uint16_t num_bytes;       /**< Actual number of bytes in the buffer */
};

/**
 * Data structure to specify an array data object
 */
struct thingset_array
{
    const union thingset_data_pointer elements; /**< Pointer to the first element of the array */
    const enum thingset_type element_type; /**< Type of the array elements (enum thingset_type) */
    const int16_t decimals;                /**< See detail in struct thingset_data_object */
    const uint16_t max_elements;           /**< Maximum number of elements in the array */
    uint16_t num_elements;                 /**< Actual number of elements in the array */
};

/**
 * Data structure to specify records of an arbitrary struct
 */
struct thingset_records
{
    const void *records;        /** Pointer to the first record */
    const size_t record_size;   /** Size of one record in bytes */
    const uint16_t max_records; /** Maximum number of records in the array */
    uint16_t num_records;       /** Actual number of records in the array */
};

/**
 * ThingSet data object struct.
 */
struct thingset_data_object
{
    /**
     * ID of parent object
     */
    const thingset_object_id_t parent_id;

    /**
     * Data object ID
     */
    const thingset_object_id_t id;

    /**
     * Data object name
     */
    const char *name;

    /**
     * Pointer to the variable containing the data. The variable type must match the type as
     * specified.
     *
     * For record items, the offset of this item inside the struct is stored (in bytes).
     */
    const union thingset_data_pointer data;

    /**
     * One of THINGSET_TYPE_INT32, _FLOAT, ...
     */
    const enum thingset_type type : 5;

    /**
     * Variable storing different detail information depending on the data type
     *
     * - FLOAT32: Decimal digits (precision) to use during serialization to JSON.
     *
     * - DECFRAC: Exponent for conversion between internal unit and unit exposed via ThingSet
     *   (equivalent to decimal digits for FLOAT32).
     *   Formula: internal value = 10^exponent * ThingSet value
     *   Example: If a voltage measurement is internally stored as an integer in mV, use exponent 3
     *   to convert to the SI base unit V as exposed via ThingSet.
     *
     * - STRING: Size of the internal buffer in bytes.
     */
    const int32_t detail : 12;

    /**
     * Flags to define read/write access
     */
    const uint32_t access : 8;

    /**
     * Flags to assign data item to different data item subsets (e.g. for publication messages)
     */
    MAYBE_CONST uint32_t subsets : 7;
};

/**
 * ThingSet context.
 *
 * Stores and handles all data objects exposed to different communication interfaces.
 */
struct thingset_context
{
    /**
     * Array of objects database provided during initialization
     */
    struct thingset_data_object *data_objects;

    /**
     * Number of objects in the data_objects array
     */
    size_t num_objects;

    /**
     * Pointer to the incoming message buffer (request or desire, provided by process function)
     */
    const uint8_t *msg;

    /**
     * Length of the incoming message
     */
    size_t msg_len;

    /**
     * Position in the message currently being parsed
     */
    size_t msg_pos;

    /**
     * Pointer to the response buffer (provided by process function)
     */
    uint8_t *rsp;

    /**
     * Size of response buffer (i.e. maximum length)
     */
    size_t rsp_size;

    /**
     * Current position inside the response (equivalent to length of the response at end of
     * processing)
     */
    size_t rsp_pos;

    /**
     * Pointer to the start of JSON payload in the request
     */
    char *json_str;

    /**
     * JSON tokes in json_str parsed by JSMN
     */
    jsmntok_t tokens[CONFIG_THINGSET_NUM_JSON_TOKENS];

    /**
     * Number of JSON tokens parsed by JSMN
     */
    int tok_count;

    /**
     * Stores current authentication status (authentication as "normal" user as default)
     */
    uint8_t auth_flags;

    /**
     * Stores current authentication status (authentication as "normal" user as default)
     */
    uint8_t update_subsets;

    /**
     * Callback to be called from patch function if a value belonging to update_subsets
     * was changed
     */
    void (*update_cb)(void);
};

/**
 * Initialize a ThingSet context.
 *
 * @param ts Pointer to ThingSet context.
 * @param objects Pointer to array containing the ThingSet object database
 * @param num_objects Number of elements in that array
 */
void thingset_init(struct thingset_context *ts, struct thingset_data_object *objects,
                   size_t num_objects);

/**
 * Initialize a ThingSet context using Zephyr iterable sections.
 *
 * Data objects defined using THINGSET_ADD_* macros will be magically added to the object database
 * by the linker.
 *
 * @param ts Pointer to ThingSet context.
 */
void thingset_init_global(struct thingset_context *ts);

/**
 * Process ThingSet request or desire.
 *
 * This function also detects if text mode (JSON) or binary mode (CBOR) is used.
 *
 * @param ts Pointer to ThingSet context.
 * @param msg Pointer to the ThingSet message (request or desire)
 * @param msg_len Length of the message
 * @param rsp Pointer to the buffer where the response should be stored (if any)
 * @param rsp_size Size of the response buffer
 *
 * @retval Length of the response written to the buffer after processing a request
 * @retval 0 if the message was empty or a desire was processed successfully (no response)
 * @retval Negative ThingSet response code if a desire could not be processed successfully
 */
int thingset_process_message(struct thingset_context *ts, const uint8_t *msg, size_t msg_len,
                             uint8_t *rsp, size_t rsp_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THINGSET_THINGSET_H_ */
