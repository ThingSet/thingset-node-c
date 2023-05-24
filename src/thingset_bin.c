/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset/thingset.h"
#include "thingset_internal.h"

#include <zcbor_common.h>
#include <zcbor_decode.h>
#include <zcbor_encode.h>

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int bin_serialize_response(struct thingset_context *ts, uint8_t code, const char *msg, ...)
{
    va_list vargs;

    ts->rsp[0] = code;

    zcbor_update_state(ts->encoder, ts->rsp + 1, ts->rsp_size - 1);
    zcbor_nil_put(ts->encoder, NULL);

    if (THINGET_ERROR(code)) {
        if (msg != NULL) {
            /* zcbor uses memmove internally, so we can use the encoder buffer with an
             * offset for the string header for temporary storage of the message
             */
            uint8_t *msg_buf_start = ts->encoder->payload_mut + 2;
            size_t msg_buf_size = ts->encoder->payload_end - msg_buf_start;

            va_start(vargs, msg);
            int ret = vsnprintf((char *)msg_buf_start, msg_buf_size, msg, vargs);
            va_end(vargs);

            if (ret >= 0 && ret < msg_buf_size) {
                zcbor_tstr_encode_ptr(ts->encoder, msg_buf_start, msg_buf_size);
            }
            else {
                /* message did not fit: keep minimum message with error code only */
                zcbor_nil_put(ts->encoder, NULL);
            }
        }
        else {
            zcbor_nil_put(ts->encoder, NULL);
        }
    }

    return 0;
}

/**
 * @returns 0 or negative ThingSet reponse code in case of error
 */
static int bin_serialize_simple_value(zcbor_state_t *encoder, union thingset_data_pointer data,
                                      int type, int detail)
{
    bool success = true;

    switch (type) {
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
        case THINGSET_TYPE_U64:
            success = zcbor_uint64_put(encoder, *data.u64);
            break;
        case THINGSET_TYPE_I64:
            success = zcbor_int64_put(encoder, *data.i64);
            break;
#endif
        case THINGSET_TYPE_U32:
            success = zcbor_int32_put(encoder, *data.u32);
            break;
        case THINGSET_TYPE_I32:
            success = zcbor_int32_put(encoder, *data.i32);
            break;
        case THINGSET_TYPE_U16:
            success = zcbor_uint32_put(encoder, *data.u16);
            break;
        case THINGSET_TYPE_I16:
            success = zcbor_int32_put(encoder, *data.i16);
            break;
        case THINGSET_TYPE_U8:
            success = zcbor_uint32_put(encoder, *data.u8);
            break;
        case THINGSET_TYPE_I8:
            success = zcbor_int32_put(encoder, *data.i8);
            break;
        case THINGSET_TYPE_F32:
            if (detail == 0) { /* round to 0 decimals: use int */
                success = zcbor_int32_put(encoder, lroundf(*data.f32));
            }
            else {
                success = zcbor_float32_put(encoder, *data.f32);
            }
            break;
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
        case THINGSET_TYPE_DECFRAC:
            success = zcbor_tag_encode(encoder, 4); /* decimal fraction type */
            success = success && zcbor_list_start_encode(encoder, 2);
            success = success && zcbor_int32_put(encoder, -detail);       /* exponent */
            success = success && zcbor_int32_put(encoder, *data.decfrac); /* mantissa */
            success = success && zcbor_list_end_encode(encoder, 2);
            break;
#endif
        case THINGSET_TYPE_BOOL:
            success = zcbor_bool_put(encoder, *data.b);
            break;
        case THINGSET_TYPE_STRING:
            success = zcbor_tstr_put_term(encoder, data.str);
            break;
#if CONFIG_THINGSET_BYTE_STRING_TYPE_SUPPORT
        case THINGSET_TYPE_BYTES:
            success = zcbor_bstr_encode_ptr(encoder, data.bytes->bytes, data.bytes->num_bytes);
            break;
#endif
        default:
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }

    if (success) {
        return 0;
    }
    else {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

/**
 * @returns 0 or negative ThingSet reponse code in case of error
 */
static int bin_serialize_value(struct thingset_context *ts,
                               const struct thingset_data_object *object)
{
    bool success = false;
    int err;

    err = bin_serialize_simple_value(ts->encoder, object->data, object->type, object->detail);

    if (err != 0) {
        /* not a simple value */
        if (object->type == THINGSET_TYPE_GROUP) {
            success = zcbor_nil_put(ts->encoder, NULL);
        }
        else if (object->type == THINGSET_TYPE_RECORDS) {
            success = zcbor_uint32_put(ts->encoder, object->data.records->num_records);
        }
        else if (object->type == THINGSET_TYPE_FN_VOID || object->type == THINGSET_TYPE_FN_I32) {
            success = zcbor_list_start_encode(ts->encoder, UINT8_MAX);
            for (unsigned int i = 0; i < ts->num_objects; i++) {
                if (ts->data_objects[i].parent_id == object->id) {
                    zcbor_tstr_put_term(ts->encoder, ts->data_objects[i].name);
                }
            }
            success = success && zcbor_list_end_encode(ts->encoder, UINT8_MAX);
        }
        else if (object->type == THINGSET_TYPE_SUBSET) {
            success = zcbor_list_start_encode(ts->encoder, UINT8_MAX);
            for (unsigned int i = 0; i < ts->num_objects; i++) {
                if (ts->data_objects[i].subsets & object->data.subset) {
                    if (ts->endpoint.use_ids) {
                        success = success && zcbor_uint32_put(ts->encoder, ts->data_objects[i].id);
                    }
                    else {
                        /* zcbor uses memmove internally, so we can use the encoder buffer with an
                         * offset for the string header for temporary storage of the path
                         */
                        uint8_t *buf_path_start = ts->encoder->payload_mut + 2;
                        size_t buf_path_size = ts->encoder->payload_end - buf_path_start;
                        int path_len = thingset_serialize_path(ts, (char *)buf_path_start,
                                                               buf_path_size, &ts->data_objects[i]);
                        success =
                            success && zcbor_tstr_encode_ptr(ts->encoder, buf_path_start, path_len);
                    }
                }
            }
            success = success && zcbor_list_end_encode(ts->encoder, UINT8_MAX);
        }
        else if (object->type == THINGSET_TYPE_ARRAY) {
            struct thingset_array *array = object->data.array;

            success = zcbor_list_start_encode(ts->encoder, array->num_elements);

            size_t type_size = thingset_type_size(array->element_type);
            for (int i = 0; i < array->num_elements; i++) {
                /* using uint8_t pointer for byte-wise pointer arithmetics */
                union thingset_data_pointer data = { .u8 = array->elements.u8 + i * type_size };
                err |= bin_serialize_simple_value(ts->encoder, data, array->element_type,
                                                  array->decimals);
            }

            success =
                success && err == 0 && zcbor_list_end_encode(ts->encoder, array->num_elements);
        }
        else {
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
        }
    }

    if (err == 0 || success) {
        return 0;
    }
    else {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

static int bin_serialize_key_value(struct thingset_context *ts,
                                   const struct thingset_data_object *object)
{
    if (ts->endpoint.use_ids) {
        if (zcbor_uint32_put(ts->encoder, object->id) == false) {
            return -THINGSET_ERR_RESPONSE_TOO_LARGE;
        }
    }
    else {
        if (zcbor_tstr_put_term(ts->encoder, object->name) == false) {
            return -THINGSET_ERR_RESPONSE_TOO_LARGE;
        }
    }

    return ts->api->serialize_value(ts, object);
}

static int bin_serialize_map_start(struct thingset_context *ts)
{
    return zcbor_map_start_encode(ts->encoder, UINT8_MAX) ? 0 : -THINGSET_ERR_RESPONSE_TOO_LARGE;
}

static int bin_serialize_map_end(struct thingset_context *ts)
{
    return zcbor_map_end_encode(ts->encoder, UINT8_MAX) ? 0 : -THINGSET_ERR_RESPONSE_TOO_LARGE;
}

static int bin_serialize_list_start(struct thingset_context *ts)
{
    return zcbor_list_start_encode(ts->encoder, UINT8_MAX) ? 0 : -THINGSET_ERR_RESPONSE_TOO_LARGE;
}

static int bin_serialize_list_end(struct thingset_context *ts)
{
    return zcbor_list_end_encode(ts->encoder, UINT8_MAX) ? 0 : -THINGSET_ERR_RESPONSE_TOO_LARGE;
}

static void bin_serialize_finish(struct thingset_context *ts)
{
    ts->rsp_pos = ts->encoder->payload - ts->rsp;
}

/**
 * Parse endpoint and fill response buffer with response in case of error.
 * @returns 0 for success or negative ThingSet reponse code in case of error
 */
static int bin_parse_endpoint(struct thingset_context *ts)
{
    struct zcbor_string path;
    uint32_t id;
    int err = -THINGSET_ERR_NOT_FOUND;

    if (zcbor_tstr_decode(ts->decoder, &path) == true) {
        err = thingset_endpoint_by_path(ts, &ts->endpoint, path.value, path.len);
    }
    else if (zcbor_uint32_decode(ts->decoder, &id) == true && id <= UINT16_MAX) {
        err = thingset_endpoint_by_id(ts, &ts->endpoint, id);
    }

    if (err != 0) {
        ts->api->serialize_response(ts, -err, "Invalid endpoint");
        return err;
    }

    return 0;
}

int thingset_bin_fetch(struct thingset_context *ts)
{
    return ts->api->serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_update(struct thingset_context *ts)
{
    return ts->api->serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_exec(struct thingset_context *ts)
{
    return ts->api->serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_create(struct thingset_context *ts)
{
    return ts->api->serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_delete(struct thingset_context *ts)
{
    return ts->api->serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_desire(struct thingset_context *ts)
{
    return -THINGSET_ERR_NOT_IMPLEMENTED;
}

static int bin_serialize_subsets(struct thingset_context *ts, uint16_t subsets)
{
    return -THINGSET_ERR_NOT_IMPLEMENTED;
}

static int bin_serialize_report_header(struct thingset_context *ts, const char *path)
{
    return -THINGSET_ERR_NOT_IMPLEMENTED;
}

static struct thingset_api bin_api = {
    .serialize_response = bin_serialize_response,
    .serialize_value = bin_serialize_value,
    .serialize_key_value = bin_serialize_key_value,
    .serialize_map_start = bin_serialize_map_start,
    .serialize_map_end = bin_serialize_map_end,
    .serialize_list_start = bin_serialize_list_start,
    .serialize_list_end = bin_serialize_list_end,
    .serialize_subsets = bin_serialize_subsets,
    .serialize_report_header = bin_serialize_report_header,
    .serialize_finish = bin_serialize_finish,
};

inline void thingset_bin_setup(struct thingset_context *ts)
{
    ts->api = &bin_api;

    zcbor_new_decode_state(ts->decoder, ZCBOR_ARRAY_SIZE(ts->decoder), ts->msg + 1, ts->msg_len - 1,
                           1);

    zcbor_new_encode_state(ts->encoder, ZCBOR_ARRAY_SIZE(ts->encoder), ts->rsp + 1,
                           ts->rsp_size - 1, 1);
}

int thingset_bin_process(struct thingset_context *ts)
{
    int ret;

    thingset_bin_setup(ts);

    ret = bin_parse_endpoint(ts);
    if (ret != 0) {
        return ts->rsp_pos;
    }

    ts->api->serialize_response(ts, THINGSET_STATUS_CONTENT, NULL);

    /* requests ordered with expected highest probability first */
    switch (ts->msg[0]) {
        case THINGSET_BIN_GET:
            ret = thingset_common_get(ts);
            break;
        case THINGSET_BIN_FETCH:
            ret = thingset_bin_fetch(ts);
            break;
        case THINGSET_BIN_UPDATE:
            ret = thingset_bin_update(ts);
            break;
        case THINGSET_BIN_EXEC:
            ret = thingset_bin_exec(ts);
            break;
        case THINGSET_BIN_CREATE:
            ret = thingset_bin_create(ts);
            break;
        case THINGSET_BIN_DELETE:
            ret = thingset_bin_delete(ts);
            break;
        case THINGSET_BIN_DESIRE:
            ret = thingset_bin_desire(ts);
            break;
        default:
            return -THINGSET_ERR_BAD_REQUEST;
    }

    if (ts->msg[0] != THINGSET_BIN_DESIRE) {
        ts->api->serialize_finish(ts);
        return ts->rsp_pos;
    }
    else {
        ts->rsp_pos = 0;
        return ret;
    }
}
