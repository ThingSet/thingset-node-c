/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <thingset.h>

#include "thingset_internal.h"

#include <zcbor_common.h>
#include <zcbor_decode.h>
#include <zcbor_encode.h>

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void bin_decoder_init(struct thingset_context *ts, const uint8_t *payload,
                             size_t payload_len)
{
    zcbor_new_decode_state(ts->decoder, ZCBOR_ARRAY_SIZE(ts->decoder), payload, payload_len, 1,
                           NULL, 0);

    /* required to accept incoming data which does not use the most compact encoding */
    ts->decoder->constant_state->enforce_canonical = false;
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

int bin_serialize_response(struct thingset_context *ts, uint8_t code, const char *msg, ...)
{
    va_list vargs;

    ts->rsp[0] = code;

    zcbor_update_state(ts->encoder, ts->rsp + 1, ts->rsp_size - 1);
    zcbor_nil_put(ts->encoder, NULL);

    if (THINGSET_ERROR(code)) {
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
                zcbor_tstr_encode_ptr(ts->encoder, msg_buf_start, ret);
            }
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
            success = zcbor_uint32_put(encoder, *data.u32);
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
            if (IS_ENABLED(CONFIG_THINGSET_ENCODE_ZERO_DECIMAL_FLOATS_AS_INTEGERS) && detail == 0)
            { /* round to 0 decimals: use int */
                success = zcbor_int32_put(encoder, lroundf(*data.f32));
            }
            else {
                success = zcbor_float32_put(encoder, *data.f32);
            }
            break;
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
        case THINGSET_TYPE_DECFRAC:
            success = zcbor_tag_put(encoder, ZCBOR_TAG_DECFRAC_ARR);
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
            success = zcbor_tstr_put_term(encoder, data.str, detail);
            break;
#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT
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

static int bin_serialize_path(struct thingset_context *ts,
                              const struct thingset_data_object *object)
{
    /* zcbor uses memmove internally, so we can use the encoder buffer with an
     * offset for the string header for temporary storage of the path
     */
    uint8_t *buf_path_start = ts->encoder->payload_mut + 2;
    size_t buf_path_size = ts->encoder->payload_end - buf_path_start;

    int path_len = thingset_get_path(ts, (char *)buf_path_start, buf_path_size, object);
    if (path_len < 0) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    return zcbor_tstr_encode_ptr(ts->encoder, buf_path_start, path_len)
               ? 0
               : -THINGSET_ERR_RESPONSE_TOO_LARGE;
}

#ifdef CONFIG_THINGSET_METADATA_ENDPOINT
static int bin_serialize_metadata(struct thingset_context *ts,
                                  const struct thingset_data_object *object)
{
    int err = bin_serialize_map_start(ts);
    if (err) {
        return err;
    }

    const char name[] = "name";
    if (!zcbor_tstr_put_lit(ts->encoder, name)) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    if (!zcbor_tstr_encode_ptr(ts->encoder, object->name, strlen(object->name))) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    const char type[] = "type";
    if (!zcbor_tstr_put_lit(ts->encoder, type)) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    char buf[128];
    int len = thingset_get_type_name(ts, object, buf, sizeof(buf));
    if (len < 0) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
    if (!zcbor_tstr_encode_ptr(ts->encoder, buf, len)) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    if ((err = bin_serialize_map_end(ts))) {
        return err;
    }

    return 0;
}
#endif /* CONFIG_THINGSET_METADATA_ENDPOINT */

static int bin_serialize_value(struct thingset_context *ts,
                               const struct thingset_data_object *object)
{
    bool success = false;
    int err;

    err = bin_serialize_simple_value(ts->encoder, object->data, object->type, object->detail);
    if (err != -THINGSET_ERR_UNSUPPORTED_FORMAT) {
        return err;
    }

    /* not a simple value */
    if (object->type == THINGSET_TYPE_GROUP) {
        success = zcbor_nil_put(ts->encoder, NULL);
    }
    else if (object->type == THINGSET_TYPE_RECORDS) {
        if (IS_ENABLED(CONFIG_THINGSET_REPORT_RECORD_SERIALIZATION)
            && ts->rsp[0] == THINGSET_BIN_REPORT)
        {
            /* serialise all records */
            success = zcbor_list_start_encode(ts->encoder, UINT8_MAX);
            for (unsigned int i = 0; i < object->data.records->num_records; i++) {
                err = thingset_common_serialize_record(ts, object, i);
            }
            success = success && zcbor_list_end_encode(ts->encoder, UINT8_MAX);
        }
        else {
            success = zcbor_uint32_put(ts->encoder, object->data.records->num_records);
        }
    }
    else if (object->type == THINGSET_TYPE_FN_VOID || object->type == THINGSET_TYPE_FN_I32) {
        success = zcbor_list_start_encode(ts->encoder, UINT8_MAX);
        for (unsigned int i = 0; i < ts->num_objects; i++) {
            if (ts->data_objects[i].parent_id == object->id) {
                zcbor_tstr_encode_ptr(ts->encoder, ts->data_objects[i].name,
                                      strlen(ts->data_objects[i].name));
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
                    success = success && (bin_serialize_path(ts, &ts->data_objects[i]) == 0);
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
            err =
                bin_serialize_simple_value(ts->encoder, data, array->element_type, array->decimals);
            if (err != 0) {
                /* finish up to leave encoder in defined state */
                zcbor_list_end_encode(ts->encoder, array->num_elements);
                return err;
            }
        }

        success = success && zcbor_list_end_encode(ts->encoder, array->num_elements);
    }
    else {
        return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }

    if (success) {
        return 0;
    }
    else {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

static int bin_serialize_key(struct thingset_context *ts, const struct thingset_data_object *object)
{
    if (ts->endpoint.use_ids) {
        if (zcbor_uint32_put(ts->encoder, object->id) == false) {
            return -THINGSET_ERR_RESPONSE_TOO_LARGE;
        }
    }
    else {
        if (zcbor_tstr_encode_ptr(ts->encoder, object->name, strlen(object->name)) == false) {
            return -THINGSET_ERR_RESPONSE_TOO_LARGE;
        }
    }

    return 0;
}

static int bin_serialize_key_value(struct thingset_context *ts,
                                   const struct thingset_data_object *object)
{
    int err = ts->api->serialize_key(ts, object);
    if (err != 0) {
        return err;
    }

    return ts->api->serialize_value(ts, object);
}

static void bin_serialize_finish(struct thingset_context *ts)
{
    ts->rsp_pos = ts->encoder->payload - ts->rsp;
    if (ts->rsp_pos == 2 && ts->rsp[1] == 0xF6) {
        /* message with empty payload */
        ts->rsp[ts->rsp_pos++] = 0xF6;
    }
}

/**
 * Parse endpoint and fill response buffer with response in case of error.
 *
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
    else if (zcbor_list_start_decode(ts->decoder) == true) {
        if (zcbor_uint32_decode(ts->decoder, &id) == true && id <= UINT16_MAX) {
            err = thingset_endpoint_by_id(ts, &ts->endpoint, id);
            if (err == 0) {
                if (!zcbor_int32_decode(ts->decoder, &ts->endpoint.index) || ts->endpoint.index < 0
                    || !zcbor_list_end_decode(ts->decoder))
                {
                    err = -THINGSET_ERR_BAD_REQUEST;
                }
                /* else: ID and index found, return 0 */
            }
        }
    }

    if (err != 0) {
        ts->api->serialize_response(ts, -err, "Invalid endpoint");
        return err;
    }

    ts->msg_payload = ts->decoder->payload;

    /* re-initialize decoder for payload parsing */
    bin_decoder_init(ts, ts->msg_payload, ts->msg_len - (ts->msg_payload - ts->msg));

    return 0;
}

int thingset_bin_desire(struct thingset_context *ts)
{
    return -THINGSET_ERR_NOT_IMPLEMENTED;
}

int thingset_bin_export_subsets_progressively(struct thingset_context *ts, uint16_t subsets,
                                              unsigned int *index, size_t *len)
{
    if (*index == 0) {
        size_t num_elements = 0;
        for (int i = 0; i < ts->num_objects; i++) {
            if (ts->data_objects[i].subsets & subsets) {
                num_elements++;
            }
        }
        zcbor_map_start_encode(ts->encoder, num_elements);
    }

    while (*index < ts->num_objects) {
        if (ts->data_objects[*index].subsets & subsets) {
            /* update last length in case next serialisation runs out of room */
            *len = ts->rsp_pos;
            int ret = bin_serialize_key_value(ts, &ts->data_objects[*index]);
            if (ret == -THINGSET_ERR_RESPONSE_TOO_LARGE) {
                if (ts->rsp_pos > 0) {
                    /* reset pointer to position before we encoded this key-value
                     * pair and ask for more data
                     */
                    ts->encoder->payload_mut = ts->rsp;
                    ts->rsp_pos = 0;
                    return 1;
                }
                else {
                    /* this element alone is too large to fit the buffer */
                    return -THINGSET_ERR_RESPONSE_TOO_LARGE;
                }
            }
            else if (ret < 0) {
                return ret;
            }
        }
        (*index)++;
        ts->rsp_pos = ts->encoder->payload - ts->rsp;
        *len = ts->rsp_pos;
    }

    /* NB. there is no corresponding call to `zcbor_map_end_encode` because
       we have already specified the exact number of elements in our call to
       `zcbor_map_start_encode` above. */

    ts->api->serialize_finish(ts);
    *len = ts->rsp_pos;
    return 0;
}

static int bin_serialize_subsets(struct thingset_context *ts, uint16_t subsets)
{
    bool success;

    success = zcbor_map_start_encode(ts->encoder, UINT8_MAX);

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].subsets & subsets) {
            bin_serialize_key_value(ts, &ts->data_objects[i]);
        }
    }

    success = success && zcbor_map_end_encode(ts->encoder, UINT8_MAX);

    if (success) {
        return 0;
    }
    else {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

static int bin_serialize_report_header(struct thingset_context *ts, const char *path)
{
    bool success;

    ts->rsp[0] = THINGSET_BIN_REPORT;

    if (ts->endpoint.use_ids) {
        if (ts->endpoint.index == THINGSET_ENDPOINT_INDEX_NONE) {
            success = zcbor_uint32_put(ts->encoder, ts->endpoint.object->id);
        }
        else {
            success = zcbor_list_start_encode(ts->encoder, UINT8_MAX);
            success |= zcbor_uint32_put(ts->encoder, ts->endpoint.object->id);
            success |= zcbor_uint32_put(ts->encoder, ts->endpoint.index);
            success |= zcbor_list_end_encode(ts->encoder, UINT8_MAX);
        }
    }
    else {
        success = zcbor_tstr_encode_ptr(ts->encoder, path, strlen(path));
    }

    return success ? 0 : -THINGSET_ERR_RESPONSE_TOO_LARGE;
}

static void bin_deserialize_payload_reset(struct thingset_context *ts)
{
    bin_decoder_init(ts, ts->msg_payload, ts->msg_len - (ts->msg_payload - ts->msg));
}

static int bin_deserialize_string(struct thingset_context *ts, const char **str_start,
                                  size_t *str_len)
{
    struct zcbor_string str;
    if (zcbor_tstr_decode(ts->decoder, &str) == true) {
        *str_start = str.value;
        *str_len = str.len;
        return 0;
    }
    else {
        return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }
}

static int bin_deserialize_null(struct thingset_context *ts)
{
    return zcbor_nil_expect(ts->decoder, NULL) ? 0 : -THINGSET_ERR_UNSUPPORTED_FORMAT;
}

static int bin_deserialize_child(struct thingset_context *ts,
                                 const struct thingset_data_object **object)
{
    struct zcbor_string name;
    uint32_t id;

    if (ts->decoder->payload_end == ts->decoder->payload || ts->decoder->elem_count == 0) {
        return -THINGSET_ERR_DESERIALIZATION_FINISHED;
    }

    if (zcbor_tstr_decode(ts->decoder, &name) == true) {
        *object = thingset_get_child_by_name(ts, ts->endpoint.object->id, name.value, name.len);
        if (*object == NULL) {
            return -THINGSET_ERR_NOT_FOUND;
        }
    }
    else if (zcbor_uint32_decode(ts->decoder, &id) == true && id <= UINT16_MAX) {
        *object = thingset_get_object_by_id(ts, id);
        if (*object == NULL) {
            return -THINGSET_ERR_NOT_FOUND;
        }
        else if (ts->endpoint.object->id != THINGSET_ID_PATHS
                 && ts->endpoint.object->id != THINGSET_ID_METADATA
                 && (*object)->parent_id != ts->endpoint.object->id)
        {
            return -THINGSET_ERR_BAD_REQUEST;
        }
    }
    else {
        return -THINGSET_ERR_BAD_REQUEST;
    }

    return 0;
}

static int bin_deserialize_list_start(struct thingset_context *ts)
{
    return zcbor_list_start_decode(ts->decoder) ? 0 : -THINGSET_ERR_UNSUPPORTED_FORMAT;
}

static int bin_deserialize_map_start(struct thingset_context *ts)
{
    return zcbor_map_start_decode(ts->decoder) ? 0 : -THINGSET_ERR_UNSUPPORTED_FORMAT;
}

static int bin_deserialize_simple_value(struct thingset_context *ts,
                                        union thingset_data_pointer data, int type, int detail,
                                        bool check_only)
{
    bool success;

    if (ts->decoder->payload_end == ts->decoder->payload) {
        return -THINGSET_ERR_DESERIALIZATION_FINISHED;
    }

    switch (type) {
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
        case THINGSET_TYPE_U64:
            success = zcbor_uint64_decode(ts->decoder, data.u64);
            break;
        case THINGSET_TYPE_I64:
            success = zcbor_int64_decode(ts->decoder, data.i64);
            break;
#endif
        case THINGSET_TYPE_U32:
            success = zcbor_uint32_decode(ts->decoder, data.u32);
            break;
        case THINGSET_TYPE_I32:
            success = zcbor_int32_decode(ts->decoder, data.i32);
            break;
        case THINGSET_TYPE_U16:
            success = zcbor_uint_decode(ts->decoder, data.u16, 2);
            break;
        case THINGSET_TYPE_I16:
            success = zcbor_int_decode(ts->decoder, data.i16, 2);
            break;
        case THINGSET_TYPE_U8:
            success = zcbor_uint_decode(ts->decoder, data.u8, 1);
            break;
        case THINGSET_TYPE_I8:
            success = zcbor_int_decode(ts->decoder, data.i8, 1);
            break;
        case THINGSET_TYPE_F32:
            success = zcbor_float16_32_decode(ts->decoder, data.f32);
            if (!success) {
                /* try integer type */
                int32_t tmp;
                if (zcbor_int32_decode(ts->decoder, &tmp) == true) {
                    *data.f32 = tmp;
                    success = true;
                }
            }
            break;
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
        case THINGSET_TYPE_DECFRAC: {
            int32_t exponent = -detail;
            int32_t *mantissa = data.decfrac;
            if (zcbor_tag_expect(ts->decoder, ZCBOR_TAG_DECFRAC_ARR)) {
                success = zcbor_list_start_decode(ts->decoder);
                int32_t mantissa_tmp;
                int32_t exponent_received;
                success = success && zcbor_int32_decode(ts->decoder, &exponent_received);
                success = success && zcbor_int32_decode(ts->decoder, &mantissa_tmp);

                for (int i = exponent_received; i < exponent; i++) {
                    mantissa_tmp /= 10;
                }
                for (int i = exponent_received; i > exponent; i--) {
                    mantissa_tmp *= 10;
                }
                *mantissa = mantissa_tmp;
            }
            else {
                /* try integer and float types */
                int32_t i32;
                float f32;
                if (zcbor_int32_decode(ts->decoder, &i32) == true) {
                    for (int i = 0; i < exponent; i++) {
                        i32 /= 10;
                    }
                    for (int i = 0; i > exponent; i--) {
                        i32 *= 10;
                    }
                    *mantissa = i32;
                    success = true;
                }
                else if (zcbor_float16_32_decode(ts->decoder, &f32) == true) {
                    for (int i = 0; i < exponent; i++) {
                        f32 /= 10.0F;
                    }
                    for (int i = 0; i > exponent; i--) {
                        f32 *= 10.0F;
                    }
                    *mantissa = (int32_t)f32;
                    success = true;
                }
                else {
                    success = false;
                }
            }
            break;
        }
#endif
        case THINGSET_TYPE_BOOL:
            success = zcbor_bool_decode(ts->decoder, data.b);
            break;
        case THINGSET_TYPE_STRING: {
            struct zcbor_string str;
            success = zcbor_tstr_decode(ts->decoder, &str);
            if (success && str.len < detail) {
                if (!check_only) {
                    strncpy(data.str, str.value, str.len);
                    data.str[str.len] = '\0';
                }
            }
            else {
                success = false;
            }
            break;
        }
#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT
        case THINGSET_TYPE_BYTES: {
            struct thingset_bytes *bytes_buf = data.bytes;
            struct zcbor_string bstr;
            success = zcbor_bstr_decode(ts->decoder, &bstr);
            if (success && bstr.len <= bytes_buf->max_bytes) {
                if (!check_only) {
                    memcpy(bytes_buf->bytes, bstr.value, bstr.len);
                    bytes_buf->num_bytes = bstr.len;
                }
            }
            else {
                success = false;
            }
            break;
        }
#endif
        default:
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }

    return success ? 0 : -THINGSET_ERR_UNSUPPORTED_FORMAT;
}

static int bin_deserialize_value(struct thingset_context *ts,
                                 const struct thingset_data_object *object, bool check_only)
{
    int err =
        bin_deserialize_simple_value(ts, object->data, object->type, object->detail, check_only);

    if (err == -THINGSET_ERR_UNSUPPORTED_FORMAT) {
        bool success;

        switch (object->type) {
            case THINGSET_TYPE_ARRAY:
                struct thingset_array *array = object->data.array;

                success = zcbor_list_start_decode(ts->decoder);
                if (!success) {
                    err = -THINGSET_ERR_UNSUPPORTED_FORMAT;
                    break;
                }

                size_t type_size = thingset_type_size(array->element_type);
                int index = 0;
                do {
                    /* using uint8_t pointer for byte-wise pointer arithmetics */
                    union thingset_data_pointer data = { .u8 = array->elements.u8 + index * type_size };

                    err = bin_deserialize_simple_value(ts, data, array->element_type, array->decimals,
                                                    check_only);
                    if (err != 0) {
                        break;
                    }
                    index++;
                } while (index < array->max_elements);

                if (!check_only) {
                    array->num_elements = index;
                }

                success = zcbor_list_end_decode(ts->decoder);
                if (success) {
                    err = 0;
                }
                break;

            case THINGSET_TYPE_RECORDS:
                struct thingset_records *records = object->data.records;
                uint32_t id;

                success = zcbor_list_start_decode(ts->decoder);
                for (unsigned int i = 0; i < records->num_records; i++) {
                    success = zcbor_map_start_decode(ts->decoder);
                    if (!success) {
                        err = -THINGSET_ERR_UNSUPPORTED_FORMAT;
                        break;
                    }

                    while (zcbor_uint32_decode(ts->decoder, &id) && id < UINT16_MAX) {
                        struct thingset_data_object *element = thingset_get_object_by_id(ts, id);
                        if (element == NULL) {
                            zcbor_any_skip(ts->decoder, NULL);
                            continue;
                        }
                        union thingset_data_pointer data = { .u8 = ((uint8_t *)records->records)
                                                                + (i * records->record_size)
                                                                + element->data.offset };
                        err = bin_deserialize_simple_value(ts, data, element->type, element->detail,
                                                        check_only);
                    }

                    success = zcbor_map_end_decode(ts->decoder);
                }

                success = zcbor_list_end_decode(ts->decoder);
                if (success) {
                    err = 0;
                }
                break;

            default:
                err = -THINGSET_ERR_UNSUPPORTED_FORMAT;
                break;
        }
    }

    return err;
}

static int bin_deserialize_skip(struct thingset_context *ts)
{
    return zcbor_any_skip(ts->decoder, NULL) ? 0 : -THINGSET_ERR_BAD_REQUEST;
}

static int bin_deserialize_finish(struct thingset_context *ts)
{
    return ts->decoder->payload_end == ts->decoder->payload ? 0 : -THINGSET_ERR_BAD_REQUEST;
}

static struct thingset_api bin_api = {
    .serialize_response = bin_serialize_response,
    .serialize_key = bin_serialize_key,
    .serialize_value = bin_serialize_value,
    .serialize_key_value = bin_serialize_key_value,
    .serialize_path = bin_serialize_path,
#ifdef CONFIG_THINGSET_METADATA_ENDPOINT
    .serialize_metadata = bin_serialize_metadata,
#endif
    .serialize_map_start = bin_serialize_map_start,
    .serialize_map_end = bin_serialize_map_end,
    .serialize_list_start = bin_serialize_list_start,
    .serialize_list_end = bin_serialize_list_end,
    .serialize_subsets = bin_serialize_subsets,
    .serialize_report_header = bin_serialize_report_header,
    .serialize_finish = bin_serialize_finish,
    .deserialize_payload_reset = bin_deserialize_payload_reset,
    .deserialize_string = bin_deserialize_string,
    .deserialize_null = bin_deserialize_null,
    .deserialize_child = bin_deserialize_child,
    .deserialize_list_start = bin_deserialize_list_start,
    .deserialize_map_start = bin_deserialize_map_start,
    .deserialize_value = bin_deserialize_value,
    .deserialize_skip = bin_deserialize_skip,
    .deserialize_finish = bin_deserialize_finish,
};

inline void thingset_bin_setup(struct thingset_context *ts, size_t rsp_buf_offset)
{
    ts->api = &bin_api;

    bin_decoder_init(ts, ts->msg + 1, ts->msg_len - 1);

    zcbor_new_encode_state(ts->encoder, ZCBOR_ARRAY_SIZE(ts->encoder), ts->rsp + rsp_buf_offset,
                           ts->rsp_size - rsp_buf_offset, 1);
}

int thingset_bin_import_data_progressively(struct thingset_context *ts, uint8_t auth_flags,
                                           size_t size, uint32_t *last_id, size_t *consumed)
{
    if (*last_id == 0) {
        int err = ts->api->deserialize_map_start(ts);
        if (err) {
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
        }
    }

    /* reset decoder state, but use current decoder payload position and element count
     * (this handles both the first case, where we've decoded the two bytes of the map start,
     * and subsequent cases, where we set the payload pointer back to the start of the buffer)
     */
    zcbor_new_decode_state(ts->decoder, ZCBOR_ARRAY_SIZE(ts->decoder), ts->decoder->payload_mut,
                           size - (ts->decoder->payload - ts->msg), ts->decoder->elem_count, NULL,
                           0);
    ts->decoder->constant_state->enforce_canonical = false;

    uint32_t id;
    size_t successfully_parsed_bytes = 0;
    zcbor_state_t state = *ts->decoder;
    while (zcbor_uint32_decode(ts->decoder, &id)) {
        if (id <= UINT16_MAX) {
            const struct thingset_data_object *object = thingset_get_object_by_id(ts, id);
            if (object != NULL && (object->access & THINGSET_WRITE_MASK & auth_flags) != 0) {
                if (ts->api->deserialize_value(ts, object, false) == 0) {
                    successfully_parsed_bytes = ts->decoder->payload - ts->msg;
                }
                else {
                    if (id == *last_id) {
                        /* we got stuck here last time, so no point going back and asking
                            for more data; just skip it and move on */
                        if (zcbor_any_skip(ts->decoder, NULL)) {
                            state = *ts->decoder;
                            successfully_parsed_bytes = ts->decoder->payload - ts->msg;
                        }
                        else {
                            /* if we can't even skip the element, the data must be corrupted */
                            *consumed = successfully_parsed_bytes;
                            return -THINGSET_ERR_REQUEST_INCOMPLETE;
                        }
                    }
                    else {
                        /* reset decoder position to the beginning of the buffer */
                        ts->decoder->payload = ts->msg;
                        ts->decoder->elem_count = state.elem_count;
                        *consumed = successfully_parsed_bytes;
                        *last_id = id;
                        return 1; /* ask for more data */
                    }
                }
            }
            else {
                /* did not find this object in lookup or was not writable */
                if (zcbor_any_skip(ts->decoder, NULL)) {
                    state = *ts->decoder;
                    successfully_parsed_bytes = ts->decoder->payload - ts->msg;
                }
                else {
                    /* reincrement element count because ID will be parsed again */
                    ts->decoder->elem_count = state.elem_count;
                }
            }
            state = *ts->decoder;
            *last_id = id;
        }
    }

    *consumed = successfully_parsed_bytes;
    if (*consumed == 0 && size > 0) {
        /* if we didn't manage to consume anything at this point, the data must be completely
         * invalid, because it means we didn't even parse an ID, so just bail out
         */
        return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }
    bool finished = ts->decoder->payload == ts->decoder->payload_end;
    ts->decoder->payload = ts->msg; /* reset decoder position */
    return finished ? 0 : 1;
}

int thingset_bin_import_data(struct thingset_context *ts, uint8_t auth_flags,
                             enum thingset_data_format format)
{
    int err;

    err = ts->api->deserialize_map_start(ts);
    if (err != 0) {
        return err;
    }

    uint32_t id;
    while (zcbor_uint32_decode(ts->decoder, &id)) {
        if (id <= UINT16_MAX) {
            const struct thingset_data_object *object = thingset_get_object_by_id(ts, id);
            if (object != NULL) {
                if ((object->access & THINGSET_WRITE_MASK & auth_flags) != 0) {
                    err = ts->api->deserialize_value(ts, object, false);
                    if (err == 0) {
                        continue;
                    }
                }
            }
        }
        /* silently ignore this item if it caused an error */
        zcbor_any_skip(ts->decoder, NULL);
    }

    return ts->api->deserialize_finish(ts);
}

int thingset_bin_import_report(struct thingset_context *ts, uint8_t auth_flags, uint16_t subset)
{
    uint32_t id = 0;

    if (ts->msg_payload[0] != THINGSET_BIN_REPORT) {
        return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }

    zcbor_uint32_decode(ts->decoder, &id);
    if (id != subset) {
        return -THINGSET_ERR_NOT_FOUND;
    }

    return thingset_bin_import_data(ts, auth_flags, subset);
}

int thingset_bin_process(struct thingset_context *ts)
{
    int ret;

    thingset_bin_setup(ts, 1);

    ret = bin_parse_endpoint(ts);
    if (ret != 0) {
        ts->api->serialize_finish(ts);
        return ts->rsp_pos;
    }

    ts->api->serialize_response(ts, THINGSET_STATUS_CONTENT, NULL);

    /* requests ordered with expected highest probability first */
    switch (ts->msg[0]) {
        case THINGSET_BIN_GET:
            ret = thingset_common_get(ts);
            break;
        case THINGSET_BIN_FETCH:
            ret = thingset_common_fetch(ts);
            break;
        case THINGSET_BIN_UPDATE:
            ret = thingset_common_update(ts);
            break;
        case THINGSET_BIN_EXEC:
            ret = thingset_common_exec(ts);
            break;
        case THINGSET_BIN_CREATE:
            ret = thingset_common_create(ts);
            break;
        case THINGSET_BIN_DELETE:
            ret = thingset_common_delete(ts);
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
