/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THINGSET_H_
#define THINGSET_H_

/**
 * @file
 */

/** @cond INTERNAL_HIDDEN */
#define JSMN_HEADER
/** @endcond */
#include "jsmn.h"

#include <zephyr/kernel.h>

#include <zcbor_common.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @cond INTERNAL_HIDDEN */
#ifndef _ARRAY_SIZE
/* custom ARRAY_SIZE to avoid redefinition warning if thingset.h is included before Zephr headers */
#define _ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif
/** @endcond */

#ifdef __cplusplus
extern "C" {
#endif

/* Function codes (binary mode) */
#define THINGSET_BIN_GET    0x01 /**< Function code for GET request in binary mode. */
#define THINGSET_BIN_EXEC   0x02 /**< Function code for EXEC request in binary mode. */
#define THINGSET_BIN_DELETE 0x04 /**< Function code for DELETE request in binary mode. */
#define THINGSET_BIN_FETCH  0x05 /**< Function code for FETCH request in binary mode. */
#define THINGSET_BIN_CREATE 0x06 /**< Function code for CREATE request in binary mode. */
#define THINGSET_BIN_UPDATE 0x07 /**< Function code for UPDATE request in binary mode. */
#define THINGSET_BIN_DESIRE 0x1D /**< Function code for DESIRE in binary mode. */
#define THINGSET_BIN_REPORT 0x1F /**< Function code for REPORT in binary mode. */

/* Function codes (text mode) */
#define THINGSET_TXT_GET_FETCH '?' /**< Function code for GET and FETCH requests in text mode. */
#define THINGSET_TXT_EXEC      '!' /**< Function code for EXEC request in text mode. */
#define THINGSET_TXT_DELETE    '-' /**< Function code for DELETE request in text mode. */
#define THINGSET_TXT_CREATE    '+' /**< Function code for CREATE request in text mode. */
#define THINGSET_TXT_UPDATE    '=' /**< Function code for UPDATE request in text mode. */
#define THINGSET_TXT_DESIRE    '@' /**< Function code for DESIRE in text mode. */
#define THINGSET_TXT_REPORT    '#' /**< Function code for REPORT in text mode. */

/* Status codes (success) */
#define THINGSET_STATUS_CREATED 0x81 /**< Response code for successful CREATE requests. */
#define THINGSET_STATUS_DELETED 0x82 /**< Response code for successful DELETE requests. */
#define THINGSET_STATUS_CHANGED 0x84 /**< Response code for successful EXEC/UPDATE requests. */
#define THINGSET_STATUS_CONTENT 0x85 /**< Response code for successful GET/FETCH requests. */

/* Status codes (client errors) */
#define THINGSET_ERR_BAD_REQUEST        0xA0 /**< Error code: Bad request. */
#define THINGSET_ERR_UNAUTHORIZED       0xA1 /**< Error code: Authentication needed. */
#define THINGSET_ERR_FORBIDDEN          0xA3 /**< Error code: Access forbidden. */
#define THINGSET_ERR_NOT_FOUND          0xA4 /**< Error code: Data object not found. */
#define THINGSET_ERR_METHOD_NOT_ALLOWED 0xA5 /**< Error code: Method not allowed. */
#define THINGSET_ERR_REQUEST_INCOMPLETE 0xA8 /**< Error code: Request incomplete. */
#define THINGSET_ERR_CONFLICT           0xA9 /**< Error code: Conflict. */
#define THINGSET_ERR_REQUEST_TOO_LARGE  0xAD /**< Error code: Request not fitting into buffer. */
#define THINGSET_ERR_UNSUPPORTED_FORMAT 0xAF /**< Error code: Format for an item not supported. */

/* Status codes (server errors) */
#define THINGSET_ERR_INTERNAL_SERVER_ERR 0xC0 /**< Error code: Generic catch-all response. */
#define THINGSET_ERR_NOT_IMPLEMENTED     0xC1 /**< Error code: Request method not implemented. */
#define THINGSET_ERR_GATEWAY_TIMEOUT     0xC4 /**< Error code: Node cannot be reached. */
#define THINGSET_ERR_NOT_A_GATEWAY       0xC5 /**< Error code: Node is not a gateway. */

/* Status codes (ThingSet specific errors) */
#define THINGSET_ERR_RESPONSE_TOO_LARGE 0xE1 /**< Error code: Response not fitting into buffer. */

/* Internal status codes */
#define THINGSET_ERR_DESERIALIZATION_FINISHED 0xF0 /**< Internal indication: Parsing finished. */

#define THINGSET_ERROR(code) (code >= 0xA0) /**< Check if provided code indicates an error. */
#define THINGSET_SUCCESS(code) \
    (code >= 0x80 && code < 0xA0) /**< Check if provided code indicates success. */

/* Reserved data object IDs */
#define THINGSET_ID_ROOT        0x00 /**< Root object for a node (empty path) */
#define THINGSET_ID_TIME        0x10 /**< Unix timestamp `t_s` */
#define THINGSET_ID_IDS         0x16 /**< `_Ids` overlay */
#define THINGSET_ID_PATHS       0x17 /**< `_Paths` overlay */
#define THINGSET_ID_METADATAURL 0x18 /**< URL for extended metadata information: `cMetadataURL` */
#define THINGSET_ID_METADATA    0x19 /**< `_Metadata` overlay */
#define THINGSET_ID_NODEID      0x1D /**< String containing the node ID: `cNodeID` */

/*
 * Macros for defining data object array elements.
 */

/**
 * Initialize struct thingset_data_object to expose `bool` variable via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param bool_ptr Pointer to the `bool` variable
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_BOOL(parent_id, id, name, bool_ptr, access, subsets) \
    { \
        parent_id, id, name, { .b = bool_ptr }, THINGSET_TYPE_BOOL, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose `uint8_t` variable via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param uint8_ptr Pointer to the `uint8_t` variable
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_UINT8(parent_id, id, name, uint8_ptr, access, subsets) \
    { \
        parent_id, id, name, { .u8 = uint8_ptr }, THINGSET_TYPE_U8, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose `int8_t` variable via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param int8_ptr Pointer to the `int8_t` variable
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_INT8(parent_id, id, name, int8_ptr, access, subsets) \
    { \
        parent_id, id, name, { .i8 = int8_ptr }, THINGSET_TYPE_I8, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose `uint16_t` variable via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param uint16_ptr Pointer to the `uint16_t` variable
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_UINT16(parent_id, id, name, uint16_ptr, access, subsets) \
    { \
        parent_id, id, name, { .u16 = uint16_ptr }, THINGSET_TYPE_U16, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose `int16_t` variable via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param int16_ptr Pointer to the `int16_t` variable
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_INT16(parent_id, id, name, int16_ptr, access, subsets) \
    { \
        parent_id, id, name, { .i16 = int16_ptr }, THINGSET_TYPE_I16, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose `uint32_t` variable via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param uint32_ptr Pointer to the `uint32_t` variable
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_UINT32(parent_id, id, name, uint32_ptr, access, subsets) \
    { \
        parent_id, id, name, { .u32 = uint32_ptr }, THINGSET_TYPE_U32, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose `int32_t` variable via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param int32_ptr Pointer to the `int32_t` variable
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_INT32(parent_id, id, name, int32_ptr, access, subsets) \
    { \
        parent_id, id, name, { .i32 = int32_ptr }, THINGSET_TYPE_I32, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose `uint64_t` variable via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param uint64_ptr Pointer to the `uint64_t` variable
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_UINT64(parent_id, id, name, uint64_ptr, access, subsets) \
    { \
        parent_id, id, name, { .u64 = uint64_ptr }, THINGSET_TYPE_U64, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose `int64_t` variable via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param int64_ptr Pointer to the `int64_t` variable
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_INT64(parent_id, id, name, int64_ptr, access, subsets) \
    { \
        parent_id, id, name, { .i64 = int64_ptr }, THINGSET_TYPE_I64, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose `float` variable via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param float_ptr Pointer to the `float` variable
 * @param decimals Number of decimal digits to be serialized in text mode
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_FLOAT(parent_id, id, name, float_ptr, decimals, access, subsets) \
    { \
        parent_id, id, name, { .f32 = float_ptr }, THINGSET_TYPE_F32, decimals, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose decimal fraction variable via ThingSet.
 *
 * The mantissa is internally stored as int32_t.
 * The value is converted into a float (JSON) or decimal fraction type (CBOR) for the protocol,
 * based on the specified number of decimal digits.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param mantissa_ptr Pointer to the `int32_t` variable
 * @param decimals Number of decimal digits to be serialized in text mode
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_DECFRAC(parent_id, id, name, mantissa_ptr, decimals, access, subsets) \
    { \
        parent_id, id, name, { .i32 = mantissa_ptr }, THINGSET_TYPE_DECFRAC, decimals, access, \
            subsets \
    }

/**
 * Initialize struct thingset_data_object to expose a null-terminated character string buffer via
 * ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param char_ptr Pointer to the `char` buffer
 * @param buf_size Maximum length of the buffer including the null-termination character
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_STRING(parent_id, id, name, char_ptr, buf_size, access, subsets) \
    { \
        parent_id, id, name, { .str = char_ptr }, THINGSET_TYPE_STRING, buf_size, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose a byte buffer containing arbitrary binary data
 * via ThingSet.
 *
 * In contrast to string buffers, no null-termination is used.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param bytes_ptr Pointer to the struct thingset_bytes object
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_BYTES(parent_id, id, name, bytes_ptr, access, subsets) \
    { \
        parent_id, id, name, { .bytes = bytes_ptr }, THINGSET_TYPE_BYTES, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose a function with `void` return value as an
 * executable item via ThingSet.
 *
 * If the function should have input parameters, child object with this object as their parent_id
 * have to be defined.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param void_fn_ptr Pointer to the `void` function
 * @param access Flags to define read/write access for this data object
 */
#define THINGSET_FN_VOID(parent_id, id, name, void_fn_ptr, access) \
    { \
        parent_id, id, name, { .void_fn = void_fn_ptr }, THINGSET_TYPE_FN_VOID, 0, access, 0 \
    }

/**
 * Initialize struct thingset_data_object to expose a function with `int32_t` return value as an
 * executable item via ThingSet.
 *
 * If the function should have input parameters, child object with this object as their parent_id
 * have to be defined.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param int32_fn_ptr Pointer to the `int32_t` function
 * @param access Flags to define read/write access for this data object
 */
#define THINGSET_FN_INT32(parent_id, id, name, int32_fn_ptr, access) \
    { \
        parent_id, id, name, { .i32_fn = int32_fn_ptr }, THINGSET_TYPE_FN_I32, 0, access, 0 \
    }

/**
 * Initialize struct thingset_data_object to expose an array of simple values via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param array_ptr Pointer to the struct thingset_array object
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_ITEM_ARRAY(parent_id, id, name, array_ptr, access, subsets) \
    { \
        parent_id, id, name, { .array = array_ptr }, THINGSET_TYPE_ARRAY, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose records of similar data via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param records_ptr Pointer to the struct thingset_records object
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_RECORDS(parent_id, id, name, records_ptr, access, subsets) \
    { \
        parent_id, id, name, { .records = records_ptr }, THINGSET_TYPE_RECORDS, 0, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose dynamic records of similar data via ThingSet.
 *
 * In contrast to normal records, dynamic records don't keep the data for all records in memory.
 * Instead, the data must be retrieved dynamically in the THINGSET_CALLBACK_PRE_READ callback and
 * stored in the referenced struct object. This type of records can be used to store large amounts
 * of data in an external flash or EEPROM and only read the data on demand.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param records_ptr Pointer to the struct thingset_records object
 * @param access Flags to define read/write access for this data object
 * @param subsets Subset(s) this data object belongs to
 */
#define THINGSET_DYN_RECORDS(parent_id, id, name, records_ptr, access, subsets) \
    { \
        parent_id, id, name, { .records = records_ptr }, THINGSET_TYPE_RECORDS, \
            THINGSET_DETAIL_DYN_RECORDS, access, subsets \
    }

/**
 * Initialize struct thingset_data_object to expose a subset item via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param subset_flag Single subset flag to be used
 * @param access Flags to define read/write access for this data object
 */
#define THINGSET_SUBSET(parent_id, id, name, subset_flag, access) \
    { \
        parent_id, id, name, { .subset = subset_flag }, THINGSET_TYPE_SUBSET, 0, access, 0 \
    }

/**
 * Initialize struct thingset_data_object to group hierarchical data via ThingSet.
 *
 * @param parent_id ID of the parent data object of type `GROUP`
 * @param id ID of this data object
 * @param name String literal with the data object name
 * @param update_callback Pointer to a function to be called before/after read/write operations.
 */
#define THINGSET_GROUP(parent_id, id, name, update_callback) \
    { \
        parent_id, id, name, { .group_callback = update_callback }, THINGSET_TYPE_GROUP, 0, \
            THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose `bool` record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `bool` used for this item
 */
#define THINGSET_RECORD_ITEM_BOOL(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_BOOL, 0, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose `uint64_t` record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `uint64_t` used for this item
 */
#define THINGSET_RECORD_ITEM_UINT64(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_U64, 0, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose `int64_t` record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `int64_t` used for this item
 */
#define THINGSET_RECORD_ITEM_INT64(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_I64, 0, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose `uint32_t` record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `uint32_t` used for this item
 */
#define THINGSET_RECORD_ITEM_UINT32(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_U32, 0, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose `int32_t` record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `int32_t` used for this item
 */
#define THINGSET_RECORD_ITEM_INT32(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_I32, 0, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose `uint16_t` record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `uint16_t` used for this item
 */
#define THINGSET_RECORD_ITEM_UINT16(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_U16, 0, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose `int16_t` record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `int16_t` used for this item
 */
#define THINGSET_RECORD_ITEM_INT16(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_I16, 0, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose `uint8_t` record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `uint8_t` used for this item
 */
#define THINGSET_RECORD_ITEM_UINT8(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, THINGSET_TYPE_U8, \
            0, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose `int8_t` record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `int8_t` used for this item
 */
#define THINGSET_RECORD_ITEM_INT8(parent_id, id, name, struct_type, struct_member) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, THINGSET_TYPE_I8, \
            0, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose `float` record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `float` used for this item
 * @param decimals Number of decimal digits to be serialized in text mode
 */
#define THINGSET_RECORD_ITEM_FLOAT(parent_id, id, name, struct_type, struct_member, decimals) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_F32, decimals, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose decimal fraction record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `int32_t` used as the mantissa for this item
 * @param decimals Number of decimal digits to be serialized in text mode
 */
#define THINGSET_RECORD_ITEM_DECFRAC(parent_id, id, name, struct_type, struct_member, decimals) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_DECFRAC, decimals, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose a null-terminated character string buffer
 * record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `char *` used for this item
 * @param buf_size Maximum length of the buffer including the null-termination character
 */
#define THINGSET_RECORD_ITEM_STRING(parent_id, id, name, struct_type, struct_member, buf_size) \
    { \
        parent_id, id, name, { .offset = offsetof(struct_type, struct_member) }, \
            THINGSET_TYPE_STRING, buf_size, THINGSET_READ_MASK \
    }

/**
 * Initialize struct thingset_data_object to expose an array record item.
 *
 * @param parent_id ID of the parent data object of type `RECORDS`
 * @param id ID of this data object (same for all records of this kind)
 * @param name String literal with the data object name
 * @param array_ptr Pointer to the struct thingset_array object
 */
#define THINGSET_RECORD_ITEM_ARRAY(parent_id, id, name, array_ptr) \
    { \
        parent_id, id, name, { .array = array_ptr }, THINGSET_TYPE_ARRAY, 0, THINGSET_READ_MASK \
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

/**
 * Add data item for `bool` variable to global iterable section.
 *
 * See #THINGSET_ITEM_BOOL for parameter description.
 */
#define THINGSET_ADD_ITEM_BOOL(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_BOOL, parent_id, id, __VA_ARGS__)

/**
 * Add data item for `uint64_t` variable to global iterable section.
 *
 * See #THINGSET_ITEM_UINT64 for parameter description.
 */
#define THINGSET_ADD_ITEM_UINT64(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_UINT64, parent_id, id, __VA_ARGS__)

/**
 * Add data item for `int64_t` variable to global iterable section.
 *
 * See #THINGSET_ITEM_INT64 for parameter description.
 */
#define THINGSET_ADD_ITEM_INT64(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_INT64, parent_id, id, __VA_ARGS__)

/**
 * Add data item for `uint32_t` variable to global iterable section.
 *
 * See #THINGSET_ITEM_UINT32 for parameter description.
 */
#define THINGSET_ADD_ITEM_UINT32(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_UINT32, parent_id, id, __VA_ARGS__)

/**
 * Add data item for `int32_t` variable to global iterable section.
 *
 * See #THINGSET_ITEM_INT32 for parameter description.
 */
#define THINGSET_ADD_ITEM_INT32(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_INT32, parent_id, id, __VA_ARGS__)

/**
 * Add data item for `uint16_t` variable to global iterable section.
 *
 * See #THINGSET_ITEM_UINT16 for parameter description.
 */
#define THINGSET_ADD_ITEM_UINT16(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_UINT16, parent_id, id, __VA_ARGS__)

/**
 * Add data item for `int16_t` variable to global iterable section.
 *
 * See #THINGSET_ITEM_INT16 for parameter description.
 */
#define THINGSET_ADD_ITEM_INT16(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_INT16, parent_id, id, __VA_ARGS__)

/**
 * Add data item for `uint8_t` variable to global iterable section.
 *
 * See #THINGSET_ITEM_UINT8 for parameter description.
 */
#define THINGSET_ADD_ITEM_UINT8(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_UINT8, parent_id, id, __VA_ARGS__)

/**
 * Add data item for `int8_t` variable to global iterable section.
 *
 * See #THINGSET_ITEM_INT8 for parameter description.
 */
#define THINGSET_ADD_ITEM_INT8(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_INT8, parent_id, id, __VA_ARGS__)

/**
 * Add data item for `float` variable to global iterable section.
 *
 * See #THINGSET_ITEM_FLOAT for parameter description.
 */
#define THINGSET_ADD_ITEM_FLOAT(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_FLOAT, parent_id, id, __VA_ARGS__)

/**
 * Add data item for decimal fraction variable to global iterable section.
 *
 * See #THINGSET_ITEM_DECFRAC for parameter description.
 */
#define THINGSET_ADD_ITEM_DECFRAC(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_DECFRAC, parent_id, id, __VA_ARGS__)

/**
 * Add data item for character string buffer to global iterable section.
 *
 * See #THINGSET_ITEM_STRING for parameter description.
 */
#define THINGSET_ADD_ITEM_STRING(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_STRING, parent_id, id, __VA_ARGS__)

/**
 * Add data item for byte string buffer to global iterable section.
 *
 * See #THINGSET_ITEM_BYTES for parameter description.
 */
#define THINGSET_ADD_ITEM_BYTES(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_BYTES, parent_id, id, __VA_ARGS__)

/**
 * Add data item for an array to global iterable section.
 *
 * See #THINGSET_ITEM_ARRAY for parameter description.
 */
#define THINGSET_ADD_ITEM_ARRAY(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(ITEM_ARRAY, parent_id, id, __VA_ARGS__)

/**
 * Add executable item for `void` function to global iterable section.
 *
 * See #THINGSET_FN_VOID for parameter description.
 */
#define THINGSET_ADD_FN_VOID(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(FN_VOID, parent_id, id, __VA_ARGS__)

/**
 * Add executable item for `int32_t` function to global iterable section.
 *
 * See #THINGSET_FN_INT32 for parameter description.
 */
#define THINGSET_ADD_FN_INT32(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(FN_INT32, parent_id, id, __VA_ARGS__)

/**
 * Add subset item to global iterable section.
 *
 * See #THINGSET_SUBSET for parameter description.
 */
#define THINGSET_ADD_SUBSET(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(SUBSET, parent_id, id, __VA_ARGS__)

/**
 * Add group data object to global iterable section.
 *
 * See #THINGSET_GROUP for parameter description.
 */
#define THINGSET_ADD_GROUP(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(GROUP, parent_id, id, __VA_ARGS__)

/**
 * Add records object to global iterable section.
 *
 * See #THINGSET_RECORDS for parameter description.
 */
#define THINGSET_ADD_RECORDS(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORDS, parent_id, id, __VA_ARGS__)

/**
 * Add dynamic records object to global iterable section.
 *
 * See #THINGSET_DYN_RECORDS for parameter description.
 */
#define THINGSET_ADD_DYN_RECORDS(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(DYN_RECORDS, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `bool` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_BOOL for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_BOOL(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_BOOL, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `uint64_t` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_UINT64 for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_UINT64(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_UINT64, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `int64_t` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_INT64 for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_INT64(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_INT64, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `uint32_t` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_UINT32 for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_UINT32(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_UINT32, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `int32_t` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_INT32 for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_INT32(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_INT32, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `uint16_t` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_UINT16 for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_UINT16(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_UINT16, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `int16_t` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_INT16 for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_INT16(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_INT16, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `uint8_t` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_UINT8 for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_UINT8(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_UINT8, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `int8_t` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_INT8 for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_INT8(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_INT8, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `float` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_FLOAT for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_FLOAT(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_FLOAT, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type decimal fraction to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_DECFRAC for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_DECFRAC(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_DECFRAC, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type `char *` to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_STRING for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_STRING(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_STRING, parent_id, id, __VA_ARGS__)

/**
 * Add record member of type array to global iterable section.
 *
 * See #THINGSET_RECORD_ITEM_ARRAY for parameter description.
 */
#define THINGSET_ADD_RECORD_ITEM_ARRAY(parent_id, id, ...) \
    _THINGSET_ADD_ITERABLE_SECTION(RECORD_ITEM_ARRAY, parent_id, id, __VA_ARGS__)

/**
 * Define a struct thingset_bytes to be used with #THINGSET_ITEM_BYTES
 *
 * @param var_name Name of the created variable of struct thingset_bytes
 * @param buffer Existing `uint8_t` array (must be an array, pointer not allowed)
 * @param used_bytes Currently used elements in the buffer
 */
#define THINGSET_DEFINE_BYTES(var_name, buffer, used_bytes) \
    struct thingset_bytes var_name = { buffer, sizeof(buffer), used_bytes };

/**
 * Define a struct thingset_array to expose `bool` arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param array Existing fixed-size array of type `bool` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_BOOL_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .b = array }, THINGSET_TYPE_BOOL, 0, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose `uint8_t` arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param array Existing fixed-size array of type `uint8_t` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_UINT8_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .u8 = array }, THINGSET_TYPE_U8, 0, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose `int8_t` arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param array Existing fixed-size array of type `int8_t` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_INT8_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .i8 = array }, THINGSET_TYPE_I8, 0, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose `uint16_t` arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param array Existing fixed-size array of type `uint16_t` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_UINT16_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .u16 = array }, THINGSET_TYPE_U16, 0, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose `int16_t` arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param array Existing fixed-size array of type `int16_t` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_INT16_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .i16 = array }, THINGSET_TYPE_I16, 0, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose `uint32_t` arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param array Existing fixed-size array of type `uint32_t` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_UINT32_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .u32 = array }, THINGSET_TYPE_U32, 0, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose `int32_t` arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param array Existing fixed-size array of type `int32_t` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_INT32_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .i32 = array }, THINGSET_TYPE_I32, 0, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose `uint64_t` arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param array Existing fixed-size array of type `uint64_t` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_UINT64_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .u64 = array }, THINGSET_TYPE_U64, 0, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose `int64_t` arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param array Existing fixed-size array of type `int64_t` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_INT64_ARRAY(var_name, array, used_elements) \
    struct thingset_array var_name = { \
        { .i64 = array }, THINGSET_TYPE_I64, 0, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose `float` arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param decimals Number of decimal digits to be serialized in text mode
 * @param array Existing fixed-size array of type `float` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_FLOAT_ARRAY(var_name, decimals, array, used_elements) \
    struct thingset_array var_name = { \
        { .f32 = array }, THINGSET_TYPE_F32, decimals, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose decimal fraction arrays with #THINGSET_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param decimals Number of decimal digits to be serialized in text mode
 * @param array Existing fixed-size array of type `int32_t` (must be an array and not a pointer)
 * @param used_elements Currently used elements in the array
 */
#define THINGSET_DEFINE_DECFRAC_ARRAY(var_name, decimals, array, used_elements) \
    struct thingset_array var_name = { \
        { .decfrac = array }, THINGSET_TYPE_DECFRAC, decimals, _ARRAY_SIZE(array), used_elements, \
    };

/**
 * Define a struct thingset_array to expose `bool` arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `bool` array used for this item
 */
#define THINGSET_DEFINE_RECORD_BOOL_ARRAY(var_name, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_BOOL, \
        0, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_array to expose `uint8_t` arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `uint8_t` array used for this item
 */
#define THINGSET_DEFINE_RECORD_UINT8_ARRAY(var_name, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_U8, \
        0, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_array to expose `int8_t` arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `int8_t` array used for this item
 */
#define THINGSET_DEFINE_RECORD_INT8_ARRAY(var_name, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_I8, \
        0, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_array to expose `uint16_t` arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `uint16_t` array used for this item
 */
#define THINGSET_DEFINE_RECORD_UINT16_ARRAY(var_name, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_U16, \
        0, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_array to expose `int16_t` arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `int16_t` array used for this item
 */
#define THINGSET_DEFINE_RECORD_INT16_ARRAY(var_name, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_I16, \
        0, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_array to expose `uint32_t` arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `uint32_t` array used for this item
 */
#define THINGSET_DEFINE_RECORD_UINT32_ARRAY(var_name, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_U32, \
        0, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_array to expose `int32_t` arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `int32_t` array used for this item
 */
#define THINGSET_DEFINE_RECORD_INT32_ARRAY(var_name, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_I32, \
        0, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_array to expose `uint64_t` arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `uint64_t` array used for this item
 */
#define THINGSET_DEFINE_RECORD_UINT64_ARRAY(var_name, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_U64, \
        0, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_array to expose `int64_t` arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `int64_t` array used for this item
 */
#define THINGSET_DEFINE_RECORD_INT64_ARRAY(var_name, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_I64, \
        0, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_array to expose `float` arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param decimals Number of decimal digits to be serialized in text mode
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `float` array used for this item
 */
#define THINGSET_DEFINE_RECORD_FLOAT_ARRAY(var_name, decimals, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_F32, \
        decimals, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_array to expose decimal fraction arrays with #THINGSET_RECORD_ITEM_ARRAY
 *
 * @param var_name Name of the created struct thingset_array variable
 * @param decimals Number of decimal digits to be serialized in text mode
 * @param struct_type Type of the struct used for the records (e.g. `struct my_record`)
 * @param struct_member Struct member of type `int32_t` array used for this item
 */
#define THINGSET_DEFINE_RECORD_DECFRAC_ARRAY(var_name, decfrac, struct_type, struct_member) \
    struct thingset_array var_name = { \
        { .offset = offsetof(struct_type, struct_member) }, \
        THINGSET_TYPE_F32, \
        decimals, \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
        _ARRAY_SIZE(((struct_type *)0)->struct_member), \
    };

/**
 * Define a struct thingset_records to be used with #THINGSET_RECORDS
 *
 * @param var_name Name of the created struct thingset_records variable
 * @param records Existing fixed-size array of custom struct containing the records
 * @param used_records Number of currently used records
 */
#define THINGSET_DEFINE_RECORDS(var_name, records, used_records) \
    struct thingset_records var_name = { records, sizeof(__typeof__(*records)), \
                                         ARRAY_SIZE(records), used_records, \
                                         THINGSET_NO_CALLBACK };

/**
 * Define a struct thingset_records to be used with #THINGSET_DYN_RECORDS
 *
 * @param var_name Name of the created struct thingset_records variable
 * @param record Pointer to existing struct object containing the dynamically created data
 * @param available_records Number of available records
 * @param callback Pointer to a function to be called before/after read/write operations.
 */
#define THINGSET_DEFINE_DYN_RECORDS(var_name, record, available_records, callback) \
    struct thingset_records var_name = { record, sizeof(__typeof__(*record)), 1, \
                                         available_records, callback };

/*
 * Defines to make data object definitions more explicit
 */
#define THINGSET_NO_CALLBACK NULL /**< No callback assigned to group */

/** @cond INTERNAL_HIDDEN */
#define THINGSET_DETAIL_DYN_RECORDS -1
/** @endcond */

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

#define THINGSET_READ(roles)       ((roles) & THINGSET_READ_MASK)
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

#define THINGSET_ENDPOINT_INDEX_NONE (-1) /**< No index provided for endpoint */
#define THINGSET_ENDPOINT_INDEX_NEW  (-2) /**< Non-existent element behind the last element */

#define THINGSET_CONTEXT_LOCK_TIMEOUT_MS (1000)

/** ThingSet data object ID (16-bit) */
typedef uint16_t thingset_object_id_t;

/**
 * Indication for which reason a callback assigned to a group was called.
 *
 * The reason is passed to the callback as a parameter, such that the application can perform
 * desired actions, e.g. update data from ADC before it is read/serialized or write data to an
 * EEPROM after it was written/deserialized.
 */
enum thingset_callback_reason
{
    THINGSET_CALLBACK_PRE_READ,   /**< Function was called before serializing data of the group */
    THINGSET_CALLBACK_POST_READ,  /**< Function was called after serializing data of the group */
    THINGSET_CALLBACK_PRE_WRITE,  /**< Function was called before deserializing data of the group */
    THINGSET_CALLBACK_POST_WRITE, /**< Function was called after deserializing data of the group */
};

/** Function to be called before/after read/write operations to groups. */
typedef void (*thingset_group_callback_t)(enum thingset_callback_reason cb_reason);

/** Function to be called before/after read/write operations to records. */
typedef void (*thingset_records_callback_t)(enum thingset_callback_reason cb_reason, int index);

/** @cond INTERNAL_HIDDEN */

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
    bool *b;                          /**< Pointer to bool variable */
    uint8_t *u8;                      /**< Pointer to uint8_t variable */
    int8_t *i8;                       /**< Pointer to int8_t variable */
    uint16_t *u16;                    /**< Pointer to uint16_t variable */
    int16_t *i16;                     /**< Pointer to int16_t variable */
    uint32_t *u32;                    /**< Pointer to uint32_t variable */
    int32_t *i32;                     /**< Pointer to int32_t variable */
    uint64_t *u64;                    /**< Pointer to uint64_t variable */
    int64_t *i64;                     /**< Pointer to int64_t variable */
    float *f32;                       /**< Pointer to float variable */
    int32_t *decfrac;                 /**< Pointer to decimal fraction mantissa */
    char *str;                        /**< Pointer to string buffer */
    struct thingset_bytes *bytes;     /**< Pointer to thingset_bytes struct */
    struct thingset_array *array;     /**< Pointer to thingset_array struct */
    struct thingset_records *records; /**< Pointer to thingset_records struct */
    size_t offset;                    /**< Offset for record elements */
    uint32_t subset;                  /**< Subset flag(s) */
    void (*void_fn)();                /**< Pointer to function with void return value */
    int32_t (*i32_fn)();              /**< Pointer to function with int32_t return value */
    /** Pointer to group callback function */
    thingset_group_callback_t group_callback;
};

/** @endcond */

/**
 * Enum to define ThingSet data format (used by export/import and report functions).
 */
enum thingset_data_format
{
    THINGSET_TXT_NAMES_VALUES, /**< Text names and values (JSON) */
    THINGSET_TXT_NAMES_ONLY,   /**< Text names only (JSON) */
    THINGSET_TXT_VALUES_ONLY,  /**< Text values only (JSON) */
    THINGSET_BIN_IDS_VALUES,   /**< Binary IDs and values (CBOR) */
    THINGSET_BIN_NAMES_VALUES, /**< Binary names and values (CBOR) */
    THINGSET_BIN_IDS_ONLY,     /**< Binary IDs only (CBOR) */
    THINGSET_BIN_VALUES_ONLY,  /**< Binary values only (CBOR) */
};

/**
 * Data structure to specify a binary data buffer
 */
struct thingset_bytes
{
    uint8_t *bytes;           /**< Pointer to the bytes buffer */
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
    const void *records;        /**< Pointer to the first record */
    const size_t record_size;   /**< Size of one record in bytes */
    const uint16_t max_records; /**< Maximum number of records in the array */
    uint16_t num_records;       /**< Actual number of records in the array */
    thingset_records_callback_t callback;
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
     * Flags to assign data item to different data item subsets (e.g. for reports)
     */
    MAYBE_CONST uint32_t subsets : 7;
};

/**
 * Data to describe the endpoint parsed from a ThingSet request
 */
struct thingset_endpoint
{
    /** Pointer to the data object in memory (must never be NULL) */
    struct thingset_data_object *object;
    /** Index number or THINGSET_ENDPOINT_INDEX_NONE or THINGSET_ENDPOINT_INDEX_NEW */
    int index;
    /** Use names or IDs (relevant for binary mode) */
    bool use_ids;
};

/* Forward-declaration of internal ThingSet API struct (defined in thingset_internal.h) */
struct thingset_api;

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
     * Semaphore to lock this context and avoid race conditions if the context may be used by
     * multiple threads in parallel.
     */
    struct k_sem lock;

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
     * Pointer to the start of the payload in the message buffer
     */
    const uint8_t *msg_payload;

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
     * Function pointers to mode-specific implementation (text or binary)
     */
    struct thingset_api *api;

    /**
     * State information for data processing, either for text mode or binary mode depending on the
     * assigned api.
     */
    union {
        /* Text mode */
        struct
        {
            /** JSON tokens in msg_payload parsed by JSMN */
            jsmntok_t tokens[CONFIG_THINGSET_NUM_JSON_TOKENS];

            /** Number of JSON tokens parsed by JSMN */
            size_t tok_count;

            /** Current position of the parsing process */
            size_t tok_pos;
        };
        /* Binary mode */
        struct
        {
            /** CBOR encoder states for binary mode */
            zcbor_state_t encoder[4];

            /** CBOR decoder states for binary mode */
            zcbor_state_t decoder[4];
        };
    };

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

    /**
     * Endpoint used for the current message
     */
    struct thingset_endpoint endpoint;
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
 * The string in the buffer will be null-terminated in case of text mode, but the termination
 * character is not included in the returned length.
 *
 * @param ts Pointer to ThingSet context.
 * @param msg Pointer to the ThingSet message (request or desire)
 * @param msg_len Length of the message
 * @param rsp Pointer to the buffer where the response should be stored (if any)
 * @param rsp_size Size of the response buffer
 *
 * @retval rsp_len Length of the response written to the buffer after processing a request
 * @retval 0 If the message was empty or a desire was processed successfully (no response)
 * @retval err Negative ThingSet response code if a desire could not be processed successfully
 */
int thingset_process_message(struct thingset_context *ts, const uint8_t *msg, size_t msg_len,
                             uint8_t *rsp, size_t rsp_size);

/**
 * Retrieve data for given subset(s).
 *
 * This function does not return a complete ThingSet message, but only the payload data as a
 * name/value map. It can be used e.g. to store data in the EEPROM or other non-volatile memory.
 *
 * The string in the buffer will be null-terminated, but the termination character is not included
 * in the returned length.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the data should be stored
 * @param buf_size Size of the buffer, i.e. maximum allowed length of the data
 * @param subsets Flags to select which subset(s) of data items should be exported
 * @param format Protocol data format to be used (text, binary with IDs or binary with names)
 *
 * @return Actual length of the data or negative ThingSet response code in case of error.
 */
int thingset_export_subsets(struct thingset_context *ts, uint8_t *buf, size_t buf_size,
                            uint16_t subsets, enum thingset_data_format format);

/**
 * Export id, value and/or name of a single data item.
 *
 * This function is typically used together with thingset_iterate_subsets to export items of a
 * subset one by one, e.g. for publishing them via CAN.
 *
 * The string in the buffer will be null-terminated, but the termination character is not included
 * in the returned length.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the data should be stored
 * @param buf_size Size of the buffer, i.e. maximum allowed length of the data
 * @param obj Pointer to data item which should be exported
 * @param format Protocol data format to be used (text, binary with IDs or binary with names)
 *
 * @return Actual length of the data or negative ThingSet response code in case of error.
 */
int thingset_export_item(struct thingset_context *ts, uint8_t *buf, size_t buf_size,
                         const struct thingset_data_object *obj, enum thingset_data_format format);

/**
 * Iterate over all objects of given subset(s).
 *
 * @param ts Pointer to ThingSet context.
 * @param subsets Flags to select which subset(s) of data items should be iterated over
 * @param start_obj Data object to start searching (use NULL to start at the beginning)
 *
 * @returns Pointer to the next object found or NULL if end of data objects was reached
 */
struct thingset_data_object *thingset_iterate_subsets(struct thingset_context *ts, uint16_t subset,
                                                      struct thingset_data_object *start_obj);

/**
 * Import data into data objects.
 *
 * This function can be used to initialize data objects from previously exported data (using
 * thingset_export_subsets function) and stored in the EEPROM or other non-volatile memory.
 *
 * @param ts Pointer to ThingSet context.
 * @param data Buffer containing ID/value map that should be written to the data objects
 * @param len Length of the data in the buffer
 * @param auth_flags Authentication flags to be used in this function (to override auth_flags)
 * @param format Protocol data format to be used (text, binary with IDs or binary with names)
 *
 * @returns 0 for success or negative ThingSet response code in case of error
 */
int thingset_import_data(struct thingset_context *ts, const uint8_t *data, size_t len,
                         uint8_t auth_flags, enum thingset_data_format format);

/**
 * Import data into a record.
 *
 * @param ts Pointer to ThingSet context.
 * @param data Buffer containing ID/value map that should be written to the record
 * @param len Length of the data in the buffer
 * @param endpoint Records endpoint (including index) to be used
 * @param format Protocol data format to be used (text, binary with IDs or binary with names)
 *
 * @returns 0 for success or negative ThingSet response code in case of error
 */
int thingset_import_record(struct thingset_context *ts, const uint8_t *data, size_t len,
                           struct thingset_endpoint *endpoint, enum thingset_data_format format);

/**
 * Generate a report for a given path.
 *
 * @note Searching the object database to find the path and items to be published based on the
 * path provides the most user-friendly API, but is not the most efficient way to generate the
 * report. A more efficient method which caches the pointers to the data objects may be added
 * in the future.
 *
 * The string in the buffer will be null-terminated, but the termination character is not included
 * in the returned length.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the report should be stored
 * @param buf_size Size of the buffer, i.e. maximum allowed length of the report
 * @param path Path of subset/group/record to be published
 * @param format Protocol data format to be used (text, binary with IDs or binary with names)
 *
 * @return Actual length of the report or negative ThingSet response code in case of error
 */
int thingset_report_path(struct thingset_context *ts, char *buf, size_t buf_size, const char *path,
                         enum thingset_data_format format);

/**
 * Set current authentication level.
 *
 * The authentication flags must match with read/write access flags of the a data objects.
 *
 * @param ts Pointer to ThingSet context.
 * @param flags Flags to define authentication level (1 = access allowed)
 */
void thingset_set_authentication(struct thingset_context *ts, uint8_t flags);

/**
 * Configure a callback for notification if data belonging to specified subset(s) was updated.
 *
 * @param ts Pointer to ThingSet context.
 * @param subsets Flags to select which subset(s) of data items should be considered
 * @param update_cb Callback to be called after an update.
 */
void thingset_set_update_callback(struct thingset_context *ts, const uint16_t subsets,
                                  void (*update_cb)(void));

/**
 * Get the endpoint from a provided path.
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to the struct thingset_endpoint to store the result.
 * @param path Relative path with multiple object names separated by forward slash.
 * @param len Length of the entire path.
 *
 * @return 0 if successful or negative ThingSet error code to be reported
 */
int thingset_endpoint_by_path(struct thingset_context *ts, struct thingset_endpoint *endpoint,
                              const char *path, size_t len);

/**
 * Get the endpoint from a provided ID.
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to the struct thingset_endpoint to store the result.
 * @param id Numeric ID of the ThingSet object.
 *
 * @return 0 if successful or negative ThingSet error code to be reported
 */
int thingset_endpoint_by_id(struct thingset_context *ts, struct thingset_endpoint *endpoint,
                            uint16_t id);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THINGSET_H_ */
