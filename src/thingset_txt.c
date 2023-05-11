/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* this must be included before thingset.h */
#include "jsmn.h"

#include "thingset/thingset.h"
#include "thingset_internal.h"

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT
#include <zephyr/sys/base64.h>
#endif

int thingset_txt_serialize_response(struct thingset_context *ts, uint8_t code, const char *msg, ...)
{
    va_list vargs;

    ts->rsp_pos = snprintf((char *)ts->rsp, ts->rsp_size, ":%.2X", code);

    if (msg != NULL && ts->rsp_size > 7) {
        ts->rsp[ts->rsp_pos++] = ' ';
        ts->rsp[ts->rsp_pos++] = '"';
        va_start(vargs, msg);
        ts->rsp_pos +=
            vsnprintf((char *)ts->rsp + ts->rsp_pos, ts->rsp_size - ts->rsp_pos, msg, vargs);
        va_end(vargs);
        if (ts->rsp_pos + 1 < ts->rsp_size) {
            ts->rsp[ts->rsp_pos++] = '"';
        }
        else {
            /* message did not fit: keep minimum message with error code only */
            ts->rsp_pos = 3;
        }
        ts->rsp[ts->rsp_pos] = '\0';
    }

    return ts->rsp_pos;
}

/**
 * @returns Number of serialized bytes or negative ThingSet reponse code in case of error
 */
static int txt_serialize_simple_value(char *buf, size_t size, union thingset_data_pointer data,
                                      int type, int detail)
{
    int pos;

    switch (type) {
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
        case THINGSET_TYPE_U64:
            pos = snprintf(buf, size, "%" PRIu64 ",", *data.u64);
            break;
        case THINGSET_TYPE_I64:
            pos = snprintf(buf, size, "%" PRIi64 ",", *data.i64);
            break;
#endif
        case THINGSET_TYPE_U32:
            pos = snprintf(buf, size, "%" PRIu32 ",", *data.u32);
            break;
        case THINGSET_TYPE_I32:
            pos = snprintf(buf, size, "%" PRIi32 ",", *data.i32);
            break;
        case THINGSET_TYPE_U16:
            pos = snprintf(buf, size, "%" PRIu16 ",", *data.u16);
            break;
        case THINGSET_TYPE_I16:
            pos = snprintf(buf, size, "%" PRIi16 ",", *data.i16);
            break;
        case THINGSET_TYPE_U8:
            pos = snprintf(buf, size, "%" PRIu8 ",", *data.u8);
            break;
        case THINGSET_TYPE_I8:
            pos = snprintf(buf, size, "%" PRIi8 ",", *data.i8);
            break;
        case THINGSET_TYPE_F32:
            if (isnan(*data.f32) || isinf(*data.f32)) {
                /* JSON spec does not support NaN and Inf, so we need to use null instead */
                pos = snprintf(buf, size, "null,");
                break;
            }
            else {
                pos = snprintf(buf, size, "%.*f,", detail, *data.f32);
                break;
            }
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
        case THINGSET_TYPE_DECFRAC:
            pos = snprintf(buf, size, "%" PRIi32 "e%" PRIi16 ",", *data.decfrac, -detail);
            break;
#endif
        case THINGSET_TYPE_BOOL:
            pos = snprintf(buf, size, "%s,", *data.b == true ? "true" : "false");
            break;
        case THINGSET_TYPE_STRING:
            pos = snprintf(buf, size, "\"%s\",", data.str);
            break;
#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT
        case THINGSET_TYPE_BYTES: {
            size_t strlen;
            int err = base64_encode((uint8_t *)buf + 1, size - 4, &strlen, data.bytes->bytes,
                                    data.bytes->num_bytes);
            if (err == 0) {
                buf[0] = '\"';
                buf[strlen + 1] = '\"';
                buf[strlen + 2] = ',';
                buf[strlen + 3] = '\0';
                pos = strlen + 3;
            }
            else {
                pos = snprintf(buf, size, "null,");
            }
            break;
        }
#endif
        default:
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }

    if (pos >= 0 && pos < size) {
        return pos;
    }
    else {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

static int txt_serialize_value(struct thingset_context *ts, char *buf, size_t size,
                               const struct thingset_data_object *object)
{
    int pos = txt_serialize_simple_value(buf, size, object->data, object->type, object->detail);

    if (pos < 0) {
        /* not a simple value */
        if (object->type == THINGSET_TYPE_FN_VOID || object->type == THINGSET_TYPE_FN_I32) {
            pos = snprintf(buf, size, "[");
            for (unsigned int i = 0; i < ts->num_objects; i++) {
                if (ts->data_objects[i].parent_id == object->id) {
                    pos += snprintf(buf + pos, size - pos, "\"%s\",", ts->data_objects[i].name);
                }
            }
            if (pos > 1) {
                pos--; /* remove trailing comma */
            }
            pos += snprintf(buf + pos, size - pos, "],");
        }
        else if (object->type == THINGSET_TYPE_SUBSET) {
            pos = snprintf(buf, size, "[");
            for (unsigned int i = 0; i < ts->num_objects; i++) {
                if (ts->data_objects[i].subsets & object->data.subset) {
                    buf[pos++] = '"';
                    pos += thingset_serialize_path(ts, buf + pos, size - pos, &ts->data_objects[i]);
                    buf[pos++] = '"';
                    buf[pos++] = ',';
                }
            }
            if (pos > 1) {
                pos--; /* remove trailing comma */
            }
            pos += snprintf(buf + pos, size - pos, "],");
        }
        else if (object->type == THINGSET_TYPE_ARRAY && object->data.array != NULL) {
            struct thingset_array *array = object->data.array;
            pos = snprintf(buf, size, "[");
            size_t type_size = thingset_type_size(array->element_type);
            for (int i = 0; i < array->num_elements; i++) {
                /* using uint8_t pointer for byte-wise pointer arithmetics */
                union thingset_data_pointer data = { .u8 = array->elements.u8 + i * type_size };
                pos += txt_serialize_simple_value(buf + pos, size - pos, data, array->element_type,
                                                  array->decimals);
            }
            if (array->num_elements > 0) {
                pos--; /* remove trailing comma */
            }
            pos += snprintf(buf + pos, size - pos, "],");
        }
        else if (object->type == THINGSET_TYPE_GROUP) {
            pos = snprintf(buf, size, "null,");
        }
        else if (object->type == THINGSET_TYPE_RECORDS) {
            pos = snprintf(buf, size, "%d,", object->data.records->num_records);
        }
    }

    if (pos >= 0 && pos < size) {
        return pos;
    }
    else {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

static int txt_serialize_name_value(struct thingset_context *ts, char *buf, size_t size,
                                    const struct thingset_data_object *object)
{
    int len_name = snprintf(buf, size, "\"%s\":", object->name);
    if (len_name < 0 || len_name >= size) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    int len_value = txt_serialize_value(ts, &buf[len_name], size - len_name, object);
    if (len_value < 0) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    return len_name + len_value;
}

static int txt_serialize_group(struct thingset_context *ts, char *buf, size_t size,
                               const struct thingset_data_object *object)
{
    int pos = 1;

    if (size < 3) { /* "{},"*/
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    buf[0] = '{';

    if (object->data.group_callback != NULL) {
        object->data.group_callback(THINGSET_CALLBACK_PRE_READ);
    }

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].parent_id == object->id
            && (ts->data_objects[i].access & THINGSET_READ_MASK))
        {
            int ret = txt_serialize_name_value(ts, buf + pos, size - pos - 1, &ts->data_objects[i]);
            if (ret >= 0) {
                pos += ret;
            }
            else {
                return -THINGSET_ERR_RESPONSE_TOO_LARGE;
            }
        }
    }

    if (object->data.group_callback != NULL) {
        object->data.group_callback(THINGSET_CALLBACK_POST_READ);
    }

    if (pos > 1) {
        pos--; /* remove internal trailing comma */
    }
    buf[pos++] = '}';
    buf[pos++] = ','; /* overrides \0 character (no length check required here) */

    return pos;
}

static int txt_serialize_record(struct thingset_context *ts, char *buf, size_t size,
                                const struct thingset_data_object *object, int record_index)
{
    struct thingset_records *records = object->data.records;
    int pos = 1;

    if (size < 3) { /* "{},"*/
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    buf[0] = '{';

    /* record item definitions are expected to start behind record data object */
    const struct thingset_data_object *item = object + 1;
    while (item < &ts->data_objects[ts->num_objects] && item->parent_id == object->id) {
        int len_name = snprintf(buf + pos, size - pos, "\"%s\":", item->name);
        if (len_name < 0 || len_name >= (size - pos)) {
            return -THINGSET_ERR_RESPONSE_TOO_LARGE;
        }

        /* using uint8_t pointer for byte-wise pointer arithmetics */
        union thingset_data_pointer data = {
            .u8 = (uint8_t *)records->records + record_index * records->record_size
                  + item->data.offset,
        };
        int len_value = txt_serialize_simple_value(buf + len_name + pos, size - len_name - pos,
                                                   data, item->type, item->detail);
        if (len_value < 0) {
            return -THINGSET_ERR_RESPONSE_TOO_LARGE;
        }

        pos += len_name + len_value;
        item++;
    }

    if (pos > 1) {
        pos--; /* remove internal trailing comma */
    }
    buf[pos++] = '}';
    buf[pos++] = ','; /* overrides \0 character (no length check required here) */

    return pos;
}

/**
 * @returns Number of parsed bytes or negative ThingSet reponse code in case of error
 */
static int txt_parse_endpoint(struct thingset_context *ts, struct thingset_endpoint *endpoint)
{
    char *path_begin = (char *)ts->msg + 1;
    char *path_end = memchr(path_begin, ' ', ts->msg_len - 1);
    int path_len;

    if (path_end != NULL) {
        path_len = path_end - path_begin;
    }
    else {
        path_len = ts->msg_len - 1;
    }

    int ret = thingset_endpoint_by_path(ts, endpoint, path_begin, path_len);
    if (ret < 0) {
        return ret;
    }

    return path_len;
}

/**
 * @returns Number of tokens or negative ThingSet reponse code in case of error
 */
static int txt_parse_payload(struct thingset_context *ts, int pos)
{
    struct jsmn_parser parser;

    jsmn_init(&parser);

    ts->json_str = (char *)ts->msg + pos;
    ts->tok_count =
        jsmn_parse(&parser, ts->json_str, ts->msg_len - pos, ts->tokens, sizeof(ts->tokens));

    if (ts->tok_count == JSMN_ERROR_NOMEM) {
        return -THINGSET_ERR_REQUEST_TOO_LARGE;
    }
    else if (ts->tok_count < 0) {
        /* other parsing error */
        return -THINGSET_ERR_BAD_REQUEST;
    }

    return ts->tok_count;
}

/**
 * Parse endpoint and payload and fill response buffer with response in case of error.
 * @return number of payload tokens or negative error code
 */
static int txt_parse_request(struct thingset_context *ts, struct thingset_endpoint *endpoint)
{
    int path_len = txt_parse_endpoint(ts, endpoint);
    if (path_len < 0) {
        thingset_txt_serialize_response(ts, -path_len, "Invalid endpoint");
        return path_len;
    }

    int num_tokens = txt_parse_payload(ts, path_len + 1);
    if (num_tokens < 0) {
        thingset_txt_serialize_response(ts, -num_tokens, "JSON parsing error");
    }

    return num_tokens;
}

static int thingset_txt_fetch(struct thingset_context *ts, struct thingset_endpoint *endpoint)
{
    int tok = 0; /* current token */
    int pos;

    /* initialize response with success message */
    pos = thingset_txt_serialize_response(ts, THINGSET_STATUS_CONTENT, NULL);
    pos += snprintf((char *)ts->rsp + pos, ts->rsp_size - pos, " [");

    if (ts->tok_count == 1 && ts->tokens[0].type == JSMN_PRIMITIVE
        && strncmp(ts->json_str + ts->tokens[0].start, "null",
                   ts->tokens[0].end - ts->tokens[0].start)
               == 0)
    {
        /* fetch names */
        for (unsigned int i = 0; i < ts->num_objects; i++) {
            if ((ts->data_objects[i].access & THINGSET_READ_MASK)
                && (ts->data_objects[i].parent_id == endpoint->object->id))
            {
                pos += snprintf((char *)ts->rsp + pos, ts->rsp_size - pos, "\"%s\",",
                                ts->data_objects[i].name);

                if (pos >= ts->rsp_size - 1) {
                    return thingset_txt_serialize_response(ts, THINGSET_ERR_RESPONSE_TOO_LARGE,
                                                           NULL);
                }

                tok++; /* increase token to indicate that the array is not empty */
            }
        }
    }
    else if (ts->tokens[0].type == JSMN_ARRAY) {
        /* fetch values */
        tok++;

        if (endpoint->object->data.group_callback != NULL) {
            endpoint->object->data.group_callback(THINGSET_CALLBACK_PRE_READ);
        }

        while (tok < ts->tok_count) {
            if (ts->tokens[tok].type != JSMN_STRING) {
                return thingset_txt_serialize_response(ts, THINGSET_ERR_BAD_REQUEST,
                                                       "Only string elements allowed");
            }

            char *item = ts->json_str + ts->tokens[tok].start;
            size_t item_len = ts->tokens[tok].end - ts->tokens[tok].start;

            const struct thingset_data_object *object =
                thingset_get_child_by_name(ts, endpoint->object->id, item, item_len);

            if (object == NULL) {
                return thingset_txt_serialize_response(ts, THINGSET_ERR_NOT_FOUND,
                                                       "Item %.*s not found", item_len, item);
            }
            else if (object->type == THINGSET_TYPE_GROUP) {
                return thingset_txt_serialize_response(ts, THINGSET_ERR_BAD_REQUEST,
                                                       "%.*s is a group", item_len, item);
            }

            if ((object->access & THINGSET_READ_MASK & ts->auth_flags) == 0) {
                if (object->access & THINGSET_READ_MASK) {
                    return thingset_txt_serialize_response(ts, THINGSET_ERR_UNAUTHORIZED,
                                                           "Authentication required for %.*s",
                                                           item_len, item);
                }
                else {
                    return thingset_txt_serialize_response(
                        ts, THINGSET_ERR_FORBIDDEN, "Reading %.*s forbidden", item_len, item);
                }
            }

            pos += txt_serialize_value(ts, (char *)&ts->rsp[pos], ts->rsp_size - pos, object);

            if (pos >= ts->rsp_size - 2) {
                return thingset_txt_serialize_response(ts, THINGSET_ERR_RESPONSE_TOO_LARGE, NULL);
            }

            tok++;
        }

        if (endpoint->object->data.group_callback != NULL) {
            endpoint->object->data.group_callback(THINGSET_CALLBACK_POST_READ);
        }
    }
    else {
        return thingset_txt_serialize_response(ts, THINGSET_ERR_BAD_REQUEST, "Invalid payload");
    }

    if (tok > 0) {
        pos--; /* remove trailing comma */
    }
    ts->rsp[pos++] = ']';
    ts->rsp[pos] = '\0';

    ts->rsp_pos = pos;
    return pos;
}

static int thingset_txt_get(struct thingset_context *ts, struct thingset_endpoint *endpoint)
{
    int pos, ret;

    pos = thingset_txt_serialize_response(ts, THINGSET_STATUS_CONTENT, NULL);
    ts->rsp[pos++] = ' '; /* rsp size at least 4 bytes: check not required */

    switch (endpoint->object->type) {
        case THINGSET_TYPE_GROUP:
            ret = txt_serialize_group(ts, (char *)ts->rsp + pos, ts->rsp_size - pos,
                                      endpoint->object);
            break;
        case THINGSET_TYPE_FN_VOID:
        case THINGSET_TYPE_FN_I32:
            /* bad request, as we can't read exec object's values */
            ret = -THINGSET_ERR_BAD_REQUEST;
            break;
        case THINGSET_TYPE_RECORDS:
            if (endpoint->index != INDEX_NONE) {
                ret = txt_serialize_record(ts, (char *)ts->rsp + pos, ts->rsp_size - pos,
                                           endpoint->object, endpoint->index);
                break;
            }
            /* fallthrough */
        default:
            ret = txt_serialize_value(ts, (char *)ts->rsp + pos, ts->rsp_size - pos,
                                      endpoint->object);
            break;
    }

    if (ret >= 0) {
        pos += ret - 1; /* remove trailing comma */
        ts->rsp[pos] = '\0';
        return pos;
    }
    else {
        return thingset_txt_serialize_response(ts, ret, NULL);
    }
}

int thingset_txt_get_fetch(struct thingset_context *ts)
{
    struct thingset_endpoint endpoint;

    int payload_tokens = txt_parse_request(ts, &endpoint);
    if (payload_tokens == 0) {
        return thingset_txt_get(ts, &endpoint);
    }
    else if (payload_tokens > 0) {
        return thingset_txt_fetch(ts, &endpoint);
    }
    else {
        return ts->rsp_pos;
    }
}

/** @return Number of tokens deserialized or negative ThingSet response code in case of error */
int thingset_txt_deserialize_value(struct thingset_context *ts, char *buf, size_t len,
                                   jsmntype_t type, const struct thingset_data_object *object)
{
    if (type != JSMN_PRIMITIVE && type != JSMN_STRING) {
        return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }

    errno = 0;
    switch (object->type) {
        case THINGSET_TYPE_F32:
            *object->data.f32 = strtod(buf, NULL);
            break;
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
        case THINGSET_TYPE_DECFRAC: {
            float tmp = strtod(buf, NULL);
            /* positive exponent */
            for (int16_t i = 0; i < object->detail; i++) {
                tmp /= 10.0F;
            }
            /* negative exponent */
            for (int16_t i = 0; i > object->detail; i--) {
                tmp *= 10.0F;
            }
            *object->data.decfrac = (int32_t)tmp;
            break;
        }
#endif
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
        case THINGSET_TYPE_U64:
            *object->data.u64 = strtoull(buf, NULL, 0);
            break;
        case THINGSET_TYPE_I64:
            *object->data.i64 = strtoll(buf, NULL, 0);
            break;
#endif
        case THINGSET_TYPE_U32:
            *object->data.u32 = strtoul(buf, NULL, 0);
            break;
        case THINGSET_TYPE_I32:
            *object->data.i32 = strtol(buf, NULL, 0);
            break;
        case THINGSET_TYPE_U16:
            *object->data.u16 = strtoul(buf, NULL, 0);
            break;
        case THINGSET_TYPE_I16:
            *object->data.i16 = strtol(buf, NULL, 0);
            break;
        case THINGSET_TYPE_U8:
            *object->data.u8 = strtoul(buf, NULL, 0);
            break;
        case THINGSET_TYPE_I8:
            *object->data.i8 = strtol(buf, NULL, 0);
            break;
        case THINGSET_TYPE_BOOL:
            if (buf[0] == 't' || buf[0] == '1') {
                *object->data.b = true;
            }
            else if (buf[0] == 'f' || buf[0] == '0') {
                *object->data.b = false;
            }
            else {
                return -THINGSET_ERR_UNSUPPORTED_FORMAT;
            }
            break;
        case THINGSET_TYPE_STRING:
            if (type != JSMN_STRING || (unsigned int)object->detail <= len) {
                return -THINGSET_ERR_REQUEST_TOO_LARGE;
            }
            strncpy(object->data.str, buf, len);
            object->data.str[len] = '\0';
            break;
#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT
        case THINGSET_TYPE_BYTES: {
            if (type != JSMN_STRING || object->data.bytes->max_bytes < len / 4 * 3) {
                return -THINGSET_ERR_REQUEST_TOO_LARGE;
            }
            struct thingset_bytes *bytes_buf = object->data.bytes;
            size_t byteslen;
            int err = base64_decode(bytes_buf->bytes, bytes_buf->max_bytes, &byteslen,
                                    (uint8_t *)buf, len);
            bytes_buf->num_bytes = byteslen;
            if (err != 0) {
                return -THINGSET_ERR_UNSUPPORTED_FORMAT;
            }
            break;
        }
#endif
        default:
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }

    if (errno == ERANGE) {
        return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }

    return 1; /* value always contained in one token (arrays not yet supported) */
}

int thingset_txt_update(struct thingset_context *ts)
{
    struct thingset_endpoint endpoint;

    int payload_tokens = txt_parse_request(ts, &endpoint);
    if (payload_tokens < 0) {
        return ts->rsp_pos;
    }
    else if (payload_tokens < 3 || ts->tokens[0].type != JSMN_OBJECT) {
        return thingset_txt_serialize_response(ts, THINGSET_ERR_BAD_REQUEST,
                                               "Map with data required");
    }

    int tok = 1; /* current token (skipping JSMN_OBJECT) */
    bool updated = false;

    /* loop through all elements to check if request is valid */
    while (tok + 1 < ts->tok_count) {

        if (ts->tokens[tok].type != JSMN_STRING
            || (ts->tokens[tok + 1].type != JSMN_PRIMITIVE
                && ts->tokens[tok + 1].type != JSMN_STRING))
        {
            return thingset_txt_serialize_response(ts, THINGSET_ERR_BAD_REQUEST, NULL);
        }

        char *item = ts->json_str + ts->tokens[tok].start;
        size_t item_len = ts->tokens[tok].end - ts->tokens[tok].start;

        const struct thingset_data_object *object =
            thingset_get_child_by_name(ts, endpoint.object->id, item, item_len);

        if (object == NULL) {
            return thingset_txt_serialize_response(ts, THINGSET_ERR_NOT_FOUND,
                                                   "Item %.*s not found", item_len, item);
        }

        if ((object->access & THINGSET_WRITE_MASK & ts->auth_flags) == 0) {
            if (object->access & THINGSET_WRITE_MASK) {
                return thingset_txt_serialize_response(ts, THINGSET_ERR_UNAUTHORIZED,
                                                       "Authentication required for %.*s", item_len,
                                                       item);
            }
            else {
                return thingset_txt_serialize_response(ts, THINGSET_ERR_FORBIDDEN,
                                                       "Item %.*s is read-only", item_len, item);
            }
        }

        tok++;

        /* extract the value and check buffer lengths */
        size_t value_len = ts->tokens[tok].end - ts->tokens[tok].start;
        if (object->type == THINGSET_TYPE_STRING) {
            if (value_len < (size_t)object->detail) {
                /* provided string fits into data object buffer */
                tok += 1;
                continue;
            }
            else {
                return thingset_txt_serialize_response(ts, THINGSET_ERR_REQUEST_TOO_LARGE, NULL);
            }
        }
        else if (object->type == THINGSET_TYPE_BYTES) {
#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT
            if (value_len / 4 * 3 <= object->data.bytes->max_bytes) {
                /* decoded base64-encoded string fits into data object buffer */
                tok += 1;
                continue;
            }
            else {
                return thingset_txt_serialize_response(ts, THINGSET_ERR_REQUEST_TOO_LARGE, NULL);
            }
#else
            return thingset_txt_serialize_response(ts, THINGSET_ERR_UNSUPPORTED_FORMAT, NULL);
#endif
        }
        else {
            /*
             * Test format of simple data types (up to 64-bit) by deserializing the value into a
             * dummy object of the same type.
             *
             * Caution: This does not work for strings and byte buffers.
             */
            uint8_t dummy_data[8];
            struct thingset_data_object dummy_object = {
                0, 0, "Dummy", { .u8 = dummy_data }, object->type, object->detail
            };

            int res =
                thingset_txt_deserialize_value(ts, ts->json_str + ts->tokens[tok].start, value_len,
                                               ts->tokens[tok].type, &dummy_object);
            if (res == 0) {
                return thingset_txt_serialize_response(ts, THINGSET_ERR_UNSUPPORTED_FORMAT, NULL);
            }
            tok += res;
        }
    }

    if (endpoint.object->data.group_callback != NULL) {
        endpoint.object->data.group_callback(THINGSET_CALLBACK_PRE_WRITE);
    }

    /* actually write data */
    tok = 1;
    while (tok + 1 < ts->tok_count) {

        const struct thingset_data_object *object = thingset_get_child_by_name(
            ts, endpoint.object->id, ts->json_str + ts->tokens[tok].start,
            ts->tokens[tok].end - ts->tokens[tok].start);

        tok++;

        tok += thingset_txt_deserialize_value(ts, &ts->json_str[ts->tokens[tok].start],
                                              ts->tokens[tok].end - ts->tokens[tok].start,
                                              ts->tokens[tok].type, object);

        if (ts->update_subsets & object->subsets) {
            updated = true;
        }
    }

    if (updated && ts->update_cb != NULL) {
        ts->update_cb();
    }

    if (endpoint.object->data.group_callback != NULL) {
        endpoint.object->data.group_callback(THINGSET_CALLBACK_POST_WRITE);
    }

    return thingset_txt_serialize_response(ts, THINGSET_STATUS_CHANGED, NULL);
}

int thingset_txt_exec(struct thingset_context *ts)
{
    struct thingset_endpoint endpoint;
    int tok = 0;
    int pos;

    int payload_tokens = txt_parse_request(ts, &endpoint);
    if (payload_tokens < 0) {
        return ts->rsp_pos;
    }
    else if (payload_tokens > 0) {
        if (ts->tokens[0].type != JSMN_ARRAY) {
            return thingset_txt_serialize_response(ts, THINGSET_ERR_BAD_REQUEST,
                                                   "Invalid parameters");
        }
        tok++; /* skip JSMN_ARRAY token */
    }

    if ((endpoint.object->access & THINGSET_WRITE_MASK)
        && (endpoint.object->type == THINGSET_TYPE_FN_VOID
            || endpoint.object->type == THINGSET_TYPE_FN_I32))
    {
        /* object is generally executable, but are we authorized? */
        if ((endpoint.object->access & THINGSET_WRITE_MASK & ts->auth_flags) == 0) {
            return thingset_txt_serialize_response(ts, THINGSET_ERR_UNAUTHORIZED,
                                                   "Authentication required");
        }
    }
    else {
        return thingset_txt_serialize_response(ts, THINGSET_ERR_FORBIDDEN, "%s is not executable",
                                               endpoint.object->name);
    }

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].parent_id == endpoint.object->id) {
            if (tok >= ts->tok_count) {
                /* more child objects found than parameters were passed */
                return thingset_txt_serialize_response(ts, THINGSET_ERR_BAD_REQUEST,
                                                       "Not enough parameters");
            }
            int res = thingset_txt_deserialize_value(ts, ts->json_str + ts->tokens[tok].start,
                                                     ts->tokens[tok].end - ts->tokens[tok].start,
                                                     ts->tokens[tok].type, &ts->data_objects[i]);
            if (res < 0) {
                /* deserializing the value was not successful */
                return thingset_txt_serialize_response(ts, THINGSET_ERR_UNSUPPORTED_FORMAT, NULL);
            }
            tok += res;
        }
    }

    if (ts->tok_count > tok) {
        /* more parameters passed than child objects found */
        return thingset_txt_serialize_response(ts, THINGSET_ERR_BAD_REQUEST, "Too many parameters");
    }

    pos = thingset_txt_serialize_response(ts, THINGSET_STATUS_CHANGED, NULL);

    /* if we got here, finally create function pointer and call function */
    if (endpoint.object->type == THINGSET_TYPE_FN_I32) {
        int32_t ret = endpoint.object->data.i32_fn();
        union thingset_data_pointer data = { .i32 = &ret };
        ts->rsp[pos++] = ' ';
        pos += txt_serialize_simple_value(&ts->rsp[pos], ts->rsp_size - pos, data,
                                          THINGSET_TYPE_I32, 0);
        ts->rsp[--pos] = '\0'; /* remove trailing comma again */
    }
    else {
        endpoint.object->data.void_fn();
    }

    return pos;
}

static int thingset_txt_create_delete(struct thingset_context *ts, bool create)
{
    struct thingset_endpoint endpoint;

    int payload_tokens = txt_parse_request(ts, &endpoint);
    if (payload_tokens < 0) {
        return ts->rsp_pos;
    }
    else if (endpoint.object->id == 0) {
        return thingset_txt_serialize_response(ts, THINGSET_ERR_BAD_REQUEST,
                                               "Endpoint item required");
    }
    else if (payload_tokens != 1) {
        return thingset_txt_serialize_response(ts, THINGSET_ERR_BAD_REQUEST,
                                               "Only single value supported");
    }

    if (endpoint.object->type == THINGSET_TYPE_ARRAY) {
        return thingset_txt_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED,
                                               "Arrays not yet supported");
    }
    else if (endpoint.object->type == THINGSET_TYPE_SUBSET) {
#if CONFIG_THINGSET_IMMUTABLE_OBJECTS
        return thingset_txt_serialize_response(ts, THINGSET_ERR_METHOD_NOT_ALLOWED,
                                               "Subset is immutable");
#else
        if (ts->tokens[0].type == JSMN_STRING) {
            struct thingset_endpoint element;
            int ret = thingset_endpoint_by_path(ts, &element, ts->json_str + ts->tokens[0].start,
                                                ts->tokens[0].end - ts->tokens[0].start);
            if (ret >= 0 && element.index == INDEX_NONE) {
                if (create) {
                    element.object->subsets |= endpoint.object->data.subset;
                    return thingset_txt_serialize_response(ts, THINGSET_STATUS_CREATED, NULL);
                }
                else {
                    element.object->subsets &= ~endpoint.object->data.subset;
                    return thingset_txt_serialize_response(ts, THINGSET_STATUS_DELETED, NULL);
                }
            }
            return thingset_txt_serialize_response(ts, THINGSET_ERR_NOT_FOUND, NULL);
        }
        else {
            return thingset_txt_serialize_response(ts, THINGSET_ERR_UNSUPPORTED_FORMAT, NULL);
        }
#endif /* CONFIG_THINGSET_IMMUTABLE_OBJECTS */
    }

    return thingset_txt_serialize_response(ts, THINGSET_ERR_METHOD_NOT_ALLOWED, NULL);
}

int thingset_txt_create(struct thingset_context *ts)
{
    return thingset_txt_create_delete(ts, true);
}

int thingset_txt_delete(struct thingset_context *ts)
{
    return thingset_txt_create_delete(ts, false);
}

int thingset_txt_desire(struct thingset_context *ts)
{
    return -THINGSET_ERR_NOT_IMPLEMENTED;
}
