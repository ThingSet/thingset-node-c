/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* this must be included before thingset.h */
#include "jsmn.h"

#include <thingset.h>

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

#if CONFIG_THINGSET_JSON_STRING_ESCAPING
#include <zephyr/sys/util.h>
#endif

static inline int txt_serialize_start(struct thingset_context *ts, char c)
{
    if (ts->rsp_size > ts->rsp_pos + 2) {
        ts->rsp[ts->rsp_pos++] = c;
        return 0;
    }
    else {
        ts->rsp_pos = 0;
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

static inline int txt_serialize_end(struct thingset_context *ts, char c)
{
    if (ts->rsp_size > ts->rsp_pos + 3) {
        if (ts->rsp[ts->rsp_pos - 1] == ',') {
            ts->rsp_pos--;
        }
        ts->rsp[ts->rsp_pos++] = c;
        ts->rsp[ts->rsp_pos++] = ',';
        return 0;
    }
    else {
        ts->rsp_pos = 0;
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

static int txt_serialize_map_start(struct thingset_context *ts)
{
    return txt_serialize_start(ts, '{');
}

static int txt_serialize_map_end(struct thingset_context *ts)
{
    return txt_serialize_end(ts, '}');
}

static int txt_serialize_list_start(struct thingset_context *ts)
{
    return txt_serialize_start(ts, '[');
}

static int txt_serialize_list_end(struct thingset_context *ts)
{
    return txt_serialize_end(ts, ']');
}

static int txt_serialize_response(struct thingset_context *ts, uint8_t code, const char *msg, ...)
{
    va_list vargs;

    ts->rsp_pos = snprintf((char *)ts->rsp, ts->rsp_size, ":%.2X ", code);

    if (msg != NULL && ts->rsp_size > 7) {
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
        ts->rsp[ts->rsp_pos++] = ' ';
    }

    return 0;
}

/**
 * @returns Number of serialized bytes or negative ThingSet reponse code in case of error
 */
static int json_serialize_simple_value(char *buf, size_t size, union thingset_data_pointer data,
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
                /* explicit double-conversion required to please compiler */
                pos = snprintf(buf, size, "%.*f,", detail, (double)*data.f32);
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
#if CONFIG_THINGSET_JSON_STRING_ESCAPING
            buf[0] = '"';
            pos = 1;
            for (int data_pos = 0; data_pos < detail; data_pos++) {
                int remaining_chars = detail - data_pos;
                if (pos + remaining_chars + 2 >= size) {
                    /* indicate that the buffer is too small (similar to snprintf) and stop */
                    pos += remaining_chars + 2;
                    break;
                }

                if (data.str[data_pos] == '\0') {
                    break;
                }

                switch (data.str[data_pos]) {
                    case '\\':
                        buf[pos++] = '\\';
                        buf[pos++] = '\\';
                        break;
                    case '\"':
                        buf[pos++] = '\\';
                        buf[pos++] = '"';
                        break;
                    case '\b':
                        buf[pos++] = '\\';
                        buf[pos++] = 'b';
                        break;
                    case '\f':
                        buf[pos++] = '\\';
                        buf[pos++] = 'f';
                        break;
                    case '\n':
                        buf[pos++] = '\\';
                        buf[pos++] = 'n';
                        break;
                    case '\r':
                        buf[pos++] = '\\';
                        buf[pos++] = 'r';
                        break;
                    case '\t':
                        buf[pos++] = '\\';
                        buf[pos++] = 't';
                        break;
                    default:
                        buf[pos++] = data.str[data_pos];
                }
            }
            buf[pos++] = '"';
            buf[pos++] = ',';
#else
            pos = snprintf(buf, size, "\"%s\",", data.str);
#endif
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

static int txt_serialize_value(struct thingset_context *ts,
                               const struct thingset_data_object *object)
{
    char *buf = ts->rsp + ts->rsp_pos;
    size_t size = ts->rsp_size - ts->rsp_pos;
    int ret;

    int pos = json_serialize_simple_value(buf, size, object->data, object->type, object->detail);

    if (pos == -THINGSET_ERR_UNSUPPORTED_FORMAT) {
        /* not a simple value */
        if (object->type == THINGSET_TYPE_GROUP) {
            pos = snprintf(buf, size, "null,");
        }
        else if (object->type == THINGSET_TYPE_RECORDS) {
            if (IS_ENABLED(CONFIG_THINGSET_REPORT_RECORD_SERIALIZATION)
                && ts->rsp[0] == THINGSET_TXT_REPORT)
            {
                pos = snprintf(buf, size, "[");
                ts->rsp_pos++;
                for (unsigned int i = 0; i < object->data.records->num_records; i++) {
                    ret = thingset_common_serialize_record(ts, object, i);
                    pos = ts->rsp_pos;
                    buf[pos++] = ',';
                }
                /* thingset_common_serialize_record has already incremeneted rsp_pos, so we reset
                   the length of pos here, so that when we increment at the end, we are only
                   incrementing the small delta below */
                buf = ts->rsp + ts->rsp_pos;
                pos = 0;
                if (object->data.records->num_records > 0) {
                    pos--; /* remove trailing comma */
                }
                pos += snprintf(buf + pos, size - pos, "],");
            }
            else {
                pos = snprintf(buf, size, "%d,", object->data.records->num_records);
            }
        }
        else if (object->type == THINGSET_TYPE_FN_VOID || object->type == THINGSET_TYPE_FN_I32) {
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
                    ret = thingset_get_path(ts, buf + pos, size - pos, &ts->data_objects[i]);
                    if (ret <= 0) {
                        ts->rsp_pos = 0;
                        return ret;
                    }
                    pos += ret;
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
                pos += json_serialize_simple_value(buf + pos, size - pos, data, array->element_type,
                                                   array->decimals);
            }
            if (array->num_elements > 0) {
                pos--; /* remove trailing comma */
            }
            pos += snprintf(buf + pos, size - pos, "],");
        }
        else {
            ts->rsp_pos = 0;
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
        }
    }

    if (pos >= 0 && pos < size) {
        ts->rsp_pos += pos;
        return 0;
    }
    else {
        ts->rsp_pos = 0;
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

static int txt_serialize_path(struct thingset_context *ts,
                              const struct thingset_data_object *object)
{
    /* not used for text mode */

    return -THINGSET_ERR_INTERNAL_SERVER_ERR;
}

static int txt_serialize_string(struct thingset_context *ts, const char *buf, bool is_key)
{
    int len = snprintf(ts->rsp + ts->rsp_pos, ts->rsp_size - ts->rsp_pos, "\"%s\"%s", buf,
                       is_key ? ":" : ",");
    if (len >= 0 && len < ts->rsp_size - ts->rsp_pos) {
        ts->rsp_pos += len;
        return 0;
    }
    else {
        ts->rsp_pos = 0;
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

static int txt_serialize_name(struct thingset_context *ts,
                              const struct thingset_data_object *object)
{
    return txt_serialize_string(ts, object->name, false);
}

#ifdef CONFIG_THINGSET_METADATA_ENDPOINT
static int txt_serialize_metadata(struct thingset_context *ts,
                                  const struct thingset_data_object *object)
{
    int err = txt_serialize_map_start(ts);
    if (err) {
        return err;
    }

    if ((err = txt_serialize_string(ts, "name", true))) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    if ((err = txt_serialize_name(ts, object))) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    if ((err = txt_serialize_string(ts, "type", true))) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    char buf[128];
    int len = thingset_get_type_name(ts, object, (char *)&buf, sizeof(buf));
    if (len < 0) {
        return THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    if ((err = txt_serialize_string(ts, buf, false))) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }

    if ((err = txt_serialize_map_end(ts))) {
        return err;
    }

    return 0;
}
#endif /* CONFIG_THINGSET_METADATA_ENDPOINT */

static int txt_serialize_name_value(struct thingset_context *ts,
                                    const struct thingset_data_object *object)
{
    int err;

    err = txt_serialize_string(ts, object->name, true);
    if (err != 0) {
        return err;
    }

    return ts->api->serialize_value(ts, object);
}

static void txt_serialize_finish(struct thingset_context *ts)
{
    /* remove the trailing comma or space (in case of no payload) and terminate string */
    ts->rsp_pos--;
    ts->rsp[ts->rsp_pos] = '\0';
}

/**
 * @returns 0 or negative ThingSet reponse code in case of error
 */
static int txt_parse_endpoint(struct thingset_context *ts)
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

    int err = thingset_endpoint_by_path(ts, &ts->endpoint, path_begin, path_len);
    if (err != 0) {
        return err;
    }

    ts->msg_pos += path_len + 1;

    return 0;
}

/**
 * @returns 0 or negative ThingSet reponse code in case of error
 */
static int txt_parse_payload(struct thingset_context *ts)
{
    struct jsmn_parser parser;
    int ret;

    ts->msg_payload = ts->msg + ts->msg_pos;
    ts->tok_pos = 0;

    jsmn_init(&parser);

    ret = jsmn_parse(&parser, ts->msg_payload, ts->msg_len - ts->msg_pos, ts->tokens,
                     sizeof(ts->tokens));
    if (ret == JSMN_ERROR_NOMEM) {
        ts->rsp_pos = 0;
        return -THINGSET_ERR_REQUEST_TOO_LARGE;
    }
    else if (ret < 0) {
        /* other parsing error */
        ts->rsp_pos = 0;
        return -THINGSET_ERR_BAD_REQUEST;
    }

    ts->tok_count = ret;
    return 0;
}

int thingset_txt_get_fetch(struct thingset_context *ts)
{
    if (ts->tok_count == 0) {
        return thingset_common_get(ts);
    }
    else {
        return thingset_common_fetch(ts);
    }
}

static int txt_deserialize_simple_value(struct thingset_context *ts,
                                        union thingset_data_pointer data, int type, int detail,
                                        bool check_only)
{
    if (ts->tok_pos >= ts->tok_count) {
        return -THINGSET_ERR_DESERIALIZATION_FINISHED;
    }

    const char *buf = ts->msg_payload + ts->tokens[ts->tok_pos].start;
    size_t len = ts->tokens[ts->tok_pos].end - ts->tokens[ts->tok_pos].start;

    if (ts->tokens[ts->tok_pos].type != JSMN_PRIMITIVE
        && ts->tokens[ts->tok_pos].type != JSMN_STRING)
    {
        return -THINGSET_ERR_UNSUPPORTED_FORMAT;
    }

    errno = 0;
    switch (type) {
        case THINGSET_TYPE_F32:
            *data.f32 = strtod(buf, NULL);
            break;
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
        case THINGSET_TYPE_DECFRAC: {
            float tmp = strtod(buf, NULL);
            /* negative decimals and positive exponent */
            for (int16_t i = 0; i < -detail; i++) {
                tmp /= 10.0F;
            }
            /* positive decimals and negative exponent */
            for (int16_t i = 0; i > -detail; i--) {
                tmp *= 10.0F;
            }
            *data.decfrac = (int32_t)tmp;
            break;
        }
#endif
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
        case THINGSET_TYPE_U64:
            *data.u64 = strtoull(buf, NULL, 0);
            break;
        case THINGSET_TYPE_I64:
            *data.i64 = strtoll(buf, NULL, 0);
            break;
#endif
        case THINGSET_TYPE_U32:
            *data.u32 = strtoul(buf, NULL, 0);
            break;
        case THINGSET_TYPE_I32:
            *data.i32 = strtol(buf, NULL, 0);
            break;
        case THINGSET_TYPE_U16:
            *data.u16 = strtoul(buf, NULL, 0);
            break;
        case THINGSET_TYPE_I16:
            *data.i16 = strtol(buf, NULL, 0);
            break;
        case THINGSET_TYPE_U8:
            *data.u8 = strtoul(buf, NULL, 0);
            break;
        case THINGSET_TYPE_I8:
            *data.i8 = strtol(buf, NULL, 0);
            break;
        case THINGSET_TYPE_BOOL:
            if (buf[0] == 't' || buf[0] == '1') {
                *data.b = true;
            }
            else if (buf[0] == 'f' || buf[0] == '0') {
                *data.b = false;
            }
            else {
                return -THINGSET_ERR_UNSUPPORTED_FORMAT;
            }
            break;
        case THINGSET_TYPE_STRING:
            if (ts->tokens[ts->tok_pos].type != JSMN_STRING || (unsigned int)detail <= len) {
                return -THINGSET_ERR_REQUEST_TOO_LARGE;
            }
            if (!check_only) {
#if CONFIG_THINGSET_JSON_STRING_ESCAPING
                size_t data_pos = 0;
                for (int pos = 0; pos < len; pos++) {
                    if (buf[pos] == '\\') {
                        pos++;
                        switch (buf[pos]) {
                            case '"':
                            case '/':
                            case '\\':
                                data.str[data_pos++] = buf[pos];
                                break;
                            case 'b':
                                data.str[data_pos++] = '\b';
                                break;
                            case 'f':
                                data.str[data_pos++] = '\f';
                                break;
                            case 'n':
                                data.str[data_pos++] = '\n';
                                break;
                            case 'r':
                                data.str[data_pos++] = '\r';
                                break;
                            case 't':
                                data.str[data_pos++] = '\t';
                                break;
                            case 'u':
                                hex2bin(&buf[pos], 4, &data.str[data_pos++], 2);
                                break;
                            default:
                                /* this would be invalid JSON */
                                return -THINGSET_ERR_UNSUPPORTED_FORMAT;
                        }
                    }
                    else {
                        data.str[data_pos++] = buf[pos];
                    }
                }
#else
                strncpy(data.str, buf, len);
                data.str[len] = '\0';
#endif
            }
            break;
#if CONFIG_THINGSET_BYTES_TYPE_SUPPORT
        case THINGSET_TYPE_BYTES: {
            if (ts->tokens[ts->tok_pos].type != JSMN_STRING || data.bytes->max_bytes < len / 4 * 3)
            {
                return -THINGSET_ERR_REQUEST_TOO_LARGE;
            }
            if (!check_only) {
                struct thingset_bytes *bytes_buf = data.bytes;
                size_t byteslen;
                int err = base64_decode(bytes_buf->bytes, bytes_buf->max_bytes, &byteslen,
                                        (uint8_t *)buf, len);
                bytes_buf->num_bytes = byteslen;
                if (err != 0) {
                    return -THINGSET_ERR_UNSUPPORTED_FORMAT;
                }
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

    ts->tok_pos++;
    return 0;
}

static int txt_deserialize_value(struct thingset_context *ts,
                                 const struct thingset_data_object *object, bool check_only)
{
    int err =
        txt_deserialize_simple_value(ts, object->data, object->type, object->detail, check_only);

    if (err == -THINGSET_ERR_UNSUPPORTED_FORMAT && object->type == THINGSET_TYPE_ARRAY) {
        struct thingset_array *array = object->data.array;

        err = ts->api->deserialize_list_start(ts);
        if (err != 0) {
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
        }

        size_t type_size = thingset_type_size(array->element_type);
        int index = 0;
        do {
            /* using uint8_t pointer for byte-wise pointer arithmetics */
            union thingset_data_pointer data = { .u8 = array->elements.u8 + index * type_size };

            err = txt_deserialize_simple_value(ts, data, array->element_type, array->decimals,
                                               check_only);
            if (err != 0) {
                break;
            }
            index++;
        } while (index < array->max_elements);

        if (!check_only) {
            array->num_elements = index;
        }

        if (err == -THINGSET_ERR_DESERIALIZATION_FINISHED) {
            err = 0;
        };
    }

    return err;
}

int thingset_txt_desire(struct thingset_context *ts)
{
    return -THINGSET_ERR_NOT_IMPLEMENTED;
}

/* currently only supporting nesting of depth 2 (parent and grandparent != 0) */
static int txt_serialize_subsets(struct thingset_context *ts, uint16_t subsets)
{
    struct thingset_data_object *ancestors[2];
    int depth = 0;

    ts->rsp[ts->rsp_pos++] = '{';

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].subsets & subsets) {
            const uint16_t parent_id = ts->data_objects[i].parent_id;

            struct thingset_data_object *parent = NULL;
            if (depth > 0 && parent_id == ancestors[depth - 1]->id) {
                /* same parent as previous item */
                parent = ancestors[depth - 1];
            }
            else if (parent_id != 0) {
                /* parent needs to be searched in the object database */
                parent = thingset_get_object_by_id(ts, parent_id);
            }

            /* close object if previous object had different parent or grandparent */
            if (depth > 0 && parent_id != ancestors[depth - 1]->id
                && ((parent != NULL && parent->parent_id != ancestors[depth - 1]->id)
                    || parent_id == 0)) /* return to root */
            {
                ts->rsp[ts->rsp_pos - 1] = '}'; /* overwrite comma */
                ts->rsp[ts->rsp_pos++] = ',';
                depth--;
            }

            if (depth == 0 && parent != NULL) {
                if (parent->parent_id != 0) {
                    struct thingset_data_object *grandparent =
                        thingset_get_object_by_id(ts, parent->parent_id);
                    if (grandparent != NULL) {
                        ts->rsp_pos += snprintf(ts->rsp + ts->rsp_pos, ts->rsp_size - ts->rsp_pos,
                                                "\"%s\":{", grandparent->name);
                        ancestors[depth++] = grandparent;
                    }
                }
                ts->rsp_pos += snprintf(ts->rsp + ts->rsp_pos, ts->rsp_size - ts->rsp_pos,
                                        "\"%s\":{", parent->name);
                ancestors[depth++] = parent;
            }
            else if (depth > 0 && parent_id != ancestors[depth - 1]->id) {
                if (parent != NULL) {
                    ts->rsp_pos += snprintf(ts->rsp + ts->rsp_pos, ts->rsp_size - ts->rsp_pos,
                                            "\"%s\":{", parent->name);
                    ancestors[depth++] = parent;
                }
            }
            ts->rsp_pos += ts->api->serialize_key_value(ts, &ts->data_objects[i]);
        }
        if (ts->rsp_pos >= ts->rsp_size - 1 - depth) {
            return -THINGSET_ERR_RESPONSE_TOO_LARGE;
        }
    }

    ts->rsp_pos--; /* overwrite internal comma */

    while (depth >= 0) {
        ts->rsp[ts->rsp_pos++] = '}';
        depth--;
    }

    ts->rsp[ts->rsp_pos++] = ',';

    return 0;
}

static int txt_serialize_report_header(struct thingset_context *ts, const char *path)
{
    ts->rsp_pos = snprintf(ts->rsp, ts->rsp_size, "#%s ", path);
    if (ts->rsp_pos < 0 || ts->rsp_pos > ts->rsp_size) {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
    else {
        return 0;
    }
}

static void txt_deserialize_payload_reset(struct thingset_context *ts)
{
    ts->msg_pos = ts->msg_payload - ts->msg;

    txt_parse_payload(ts);
}

static int txt_deserialize_string(struct thingset_context *ts, const char **str_start,
                                  size_t *str_len)
{
    if (ts->tok_pos < ts->tok_count) {
        if (ts->tokens[ts->tok_pos].type == JSMN_STRING) {
            *str_start = ts->msg_payload + ts->tokens[ts->tok_pos].start;
            *str_len = ts->tokens[ts->tok_pos].end - ts->tokens[ts->tok_pos].start;
            ts->tok_pos++;
            return 0;
        }
        else {
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
        }
    }
    else {
        return -THINGSET_ERR_BAD_REQUEST;
    }
}

static int txt_deserialize_child(struct thingset_context *ts,
                                 const struct thingset_data_object **object)
{
    if (ts->tok_pos >= ts->tok_count) {
        return -THINGSET_ERR_DESERIALIZATION_FINISHED;
    }

    if (ts->tokens[ts->tok_pos].type != JSMN_STRING) {
        return -THINGSET_ERR_BAD_REQUEST;
    }

    const char *name = (char *)ts->msg_payload + ts->tokens[ts->tok_pos].start;
    size_t name_len = ts->tokens[ts->tok_pos].end - ts->tokens[ts->tok_pos].start;

    if (ts->endpoint.object->id == THINGSET_ID_METADATA) {
        int index;
        *object = thingset_get_object_by_path(ts, name, name_len, &index);
    }
    else {
        *object = thingset_get_child_by_name(ts, ts->endpoint.object->id, name, name_len);
    }

    if (*object == NULL) {
        return -THINGSET_ERR_NOT_FOUND;
    }

    ts->tok_pos++;
    return 0;
}

static int txt_deserialize_null(struct thingset_context *ts)
{
    if (ts->tok_pos < ts->tok_count) {
        jsmntok_t *token = &ts->tokens[ts->tok_pos];
        if (token->type == JSMN_PRIMITIVE
            && strncmp(ts->msg_payload + token->start, "null", token->end - token->start) == 0)
        {
            ts->tok_pos++;
            return 0;
        }
        else {
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
        }
    }
    else {
        return -THINGSET_ERR_BAD_REQUEST;
    }
}

static int txt_deserialize_list_start(struct thingset_context *ts)
{
    if (ts->tok_pos < ts->tok_count) {
        if (ts->tokens[ts->tok_pos].type == JSMN_ARRAY) {
            ts->tok_pos++;
            return 0;
        }
        else {
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
        }
    }
    else {
        return -THINGSET_ERR_BAD_REQUEST;
    }
}

static int txt_deserialize_map_start(struct thingset_context *ts)
{
    if (ts->tok_pos < ts->tok_count) {
        if (ts->tokens[ts->tok_pos].type == JSMN_OBJECT) {
            ts->tok_pos++;
            return 0;
        }
        else {
            return -THINGSET_ERR_UNSUPPORTED_FORMAT;
        }
    }
    else {
        return -THINGSET_ERR_BAD_REQUEST;
    }
}

static int txt_deserialize_skip(struct thingset_context *ts)
{
    if (ts->tok_pos < ts->tok_count) {
        ts->tok_pos++;
        return 0;
    }
    else {
        return -THINGSET_ERR_BAD_REQUEST;
    }
}

static int txt_deserialize_finish(struct thingset_context *ts)
{
    return ts->tok_count == ts->tok_pos ? 0 : -THINGSET_ERR_BAD_REQUEST;
}

static struct thingset_api txt_api = {
    .serialize_response = txt_serialize_response,
    .serialize_key = txt_serialize_name,
    .serialize_value = txt_serialize_value,
    .serialize_key_value = txt_serialize_name_value,
    .serialize_path = txt_serialize_path,
#ifdef CONFIG_THINGSET_METADATA_ENDPOINT
    .serialize_metadata = txt_serialize_metadata,
#endif
    .serialize_map_start = txt_serialize_map_start,
    .serialize_map_end = txt_serialize_map_end,
    .serialize_list_start = txt_serialize_list_start,
    .serialize_list_end = txt_serialize_list_end,
    .serialize_subsets = txt_serialize_subsets,
    .serialize_report_header = txt_serialize_report_header,
    .serialize_finish = txt_serialize_finish,
    .deserialize_payload_reset = txt_deserialize_payload_reset,
    .deserialize_string = txt_deserialize_string,
    .deserialize_null = txt_deserialize_null,
    .deserialize_list_start = txt_deserialize_list_start,
    .deserialize_map_start = txt_deserialize_map_start,
    .deserialize_child = txt_deserialize_child,
    .deserialize_value = txt_deserialize_value,
    .deserialize_skip = txt_deserialize_skip,
    .deserialize_finish = txt_deserialize_finish,
};

inline void thingset_txt_setup(struct thingset_context *ts)
{
    ts->api = &txt_api;
}

int thingset_txt_process(struct thingset_context *ts)
{
    int ret;

    thingset_txt_setup(ts);

    /* requests ordered with expected highest probability first */
    int (*request_fn)(struct thingset_context *ts);
    switch (ts->msg[0]) {
        case THINGSET_TXT_GET_FETCH:
            request_fn = thingset_txt_get_fetch;
            break;
        case THINGSET_TXT_UPDATE:
            request_fn = thingset_common_update;
            break;
        case THINGSET_TXT_EXEC:
            request_fn = thingset_common_exec;
            break;
        case THINGSET_TXT_CREATE:
            request_fn = thingset_common_create;
            break;
        case THINGSET_TXT_DELETE:
            request_fn = thingset_common_delete;
            break;
        case THINGSET_TXT_DESIRE:
            request_fn = thingset_txt_desire;
            break;
        default:
            return -THINGSET_ERR_BAD_REQUEST;
    }

    ret = txt_parse_endpoint(ts);
    if (ret != 0) {
        ts->api->serialize_response(ts, -ret, "Invalid endpoint");
        goto out;
    }

    ret = txt_parse_payload(ts);
    if (ret != 0) {
        ts->api->serialize_response(ts, -ret, "JSON parsing error");
        goto out;
    }

    ret = request_fn(ts);

out:
    if (ts->msg[0] != THINGSET_TXT_DESIRE) {
        if (ts->rsp_pos > 0) {
            ts->api->serialize_finish(ts);
        }
        return ts->rsp_pos;
    }
    else {
        ts->rsp_pos = 0;
        return ret;
    }
}
