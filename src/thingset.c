/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <thingset.h>

#include "thingset_internal.h"

#include <zephyr/logging/log.h>
#include <zephyr/toolchain/common.h>

#include <stdio.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(thingset, CONFIG_THINGSET_LOG_LEVEL);

/* pointers to iterable section data */
#ifdef STRUCT_SECTION_START_EXTERN
STRUCT_SECTION_START_EXTERN(thingset_data_object);
STRUCT_SECTION_END_EXTERN(thingset_data_object);
#else
/* Zephyr versions below v3.4 are missing some macros... */
extern struct thingset_data_object _thingset_data_object_list_start[];
extern struct thingset_data_object _thingset_data_object_list_end[];
#define TYPE_SECTION_START(secname) _CONCAT(_##secname, _list_start)
#endif /* STRUCT_SECTION_START_EXTERN */

/* dummy objects to avoid using NULL pointer for root object or _Paths/_Types overlays */
static struct thingset_data_object root_object = THINGSET_GROUP(0, 0, "", NULL);
static struct thingset_data_object paths_object =
    THINGSET_GROUP(0, THINGSET_ID_PATHS, "_Paths", NULL);
static struct thingset_data_object types_object =
    THINGSET_GROUP(0, THINGSET_ID_METADATA, "_Types", NULL);

static void check_id_duplicates(const struct thingset_data_object *objects, size_t num)
{
    for (unsigned int i = 0; i < num; i++) {
        for (unsigned int j = i + 1; j < num; j++) {
            if (objects[i].id == objects[j].id) {
                LOG_ERR("Duplicate data object ID 0x%X.\n", objects[i].id);
            }
        }
    }
}

void thingset_init(struct thingset_context *ts, struct thingset_data_object *objects,
                   size_t num_objects)
{
    check_id_duplicates(objects, num_objects);

    ts->data_objects = objects;
    ts->num_objects = num_objects;
    ts->auth_flags = THINGSET_USR_MASK;

    k_sem_init(&ts->lock, 1, 1);
}

void thingset_init_global(struct thingset_context *ts)
{
    /* duplicates are checked at compile-time */

    ts->data_objects = TYPE_SECTION_START(thingset_data_object);
    STRUCT_SECTION_COUNT(thingset_data_object, &ts->num_objects);
    ts->auth_flags = THINGSET_USR_MASK;

    k_sem_init(&ts->lock, 1, 1);
}

int thingset_process_message(struct thingset_context *ts, const uint8_t *msg, size_t msg_len,
                             uint8_t *rsp, size_t rsp_size)
{
    int ret;

    if (msg == NULL || msg_len < 1) {
        return -THINGSET_ERR_BAD_REQUEST;
    }

    if (rsp == NULL || rsp_size < 4) {
        /* response buffer with at least 4 bytes required to fit minimum response */
        return -THINGSET_ERR_INTERNAL_SERVER_ERR;
    }

    if (k_sem_take(&ts->lock, K_MSEC(THINGSET_CONTEXT_LOCK_TIMEOUT_MS)) != 0) {
        LOG_ERR("ThingSet context lock timed out");
        return -THINGSET_ERR_INTERNAL_SERVER_ERR;
    }

    ts->msg = msg;
    ts->msg_len = msg_len;
    ts->msg_pos = 0;

    ts->rsp = rsp;
    ts->rsp_size = rsp_size;
    ts->rsp_pos = 0;

    if (ts->msg[0] >= 0x20) {
        ret = thingset_txt_process(ts);
    }
    else {
        ret = thingset_bin_process(ts);
    }

    k_sem_give(&ts->lock);

    return ret;
}

int thingset_export_subsets(struct thingset_context *ts, uint8_t *buf, size_t buf_size,
                            uint16_t subsets, enum thingset_data_format format)
{
    int ret;

    if (k_sem_take(&ts->lock, K_MSEC(THINGSET_CONTEXT_LOCK_TIMEOUT_MS)) != 0) {
        LOG_ERR("ThingSet context lock timed out");
        return -THINGSET_ERR_INTERNAL_SERVER_ERR;
    }

    ts->rsp = buf;
    ts->rsp_size = buf_size;
    ts->rsp_pos = 0;

    switch (format) {
        case THINGSET_TXT_NAMES_VALUES:
            thingset_txt_setup(ts);
            break;
        case THINGSET_BIN_IDS_VALUES:
            ts->endpoint.use_ids = true;
            thingset_bin_setup(ts, 0);
            break;
        default:
            return -THINGSET_ERR_NOT_IMPLEMENTED;
    }

    ret = ts->api->serialize_subsets(ts, subsets);

    ts->api->serialize_finish(ts);

    if (ret == 0) {
        ret = ts->rsp_pos;
    }

    k_sem_give(&ts->lock);

    return ret;
}

int thingset_export_item(struct thingset_context *ts, uint8_t *buf, size_t buf_size,
                         const struct thingset_data_object *obj, enum thingset_data_format format)
{
    int ret;

    if (k_sem_take(&ts->lock, K_MSEC(THINGSET_CONTEXT_LOCK_TIMEOUT_MS)) != 0) {
        LOG_ERR("ThingSet context lock timed out");
        return -THINGSET_ERR_INTERNAL_SERVER_ERR;
    }

    ts->rsp = buf;
    ts->rsp_size = buf_size;
    ts->rsp_pos = 0;

    switch (format) {
        case THINGSET_TXT_VALUES_ONLY:
            thingset_txt_setup(ts);
            break;
        case THINGSET_BIN_VALUES_ONLY:
            ts->endpoint.use_ids = true;
            thingset_bin_setup(ts, 0);
            break;
        default:
            ret = -THINGSET_ERR_NOT_IMPLEMENTED;
            goto out;
    }

    ret = ts->api->serialize_value(ts, obj);

    ts->api->serialize_finish(ts);

    if (ret == 0) {
        ret = ts->rsp_pos;
    }

out:
    k_sem_give(&ts->lock);

    return ret;
}

struct thingset_data_object *thingset_iterate_subsets(struct thingset_context *ts, uint16_t subset,
                                                      struct thingset_data_object *start_obj)
{
    if (start_obj == NULL) {
        start_obj = ts->data_objects;
    }

    struct thingset_data_object *end_obj = ts->data_objects + ts->num_objects;
    for (struct thingset_data_object *obj = start_obj; obj < end_obj; obj++) {
        if (obj->subsets & subset) {
            return obj;
        }
    }

    return NULL;
}

int thingset_import_data(struct thingset_context *ts, const uint8_t *data, size_t len,
                         uint8_t auth_flags, enum thingset_data_format format)
{
    int err;

    if (k_sem_take(&ts->lock, K_MSEC(THINGSET_CONTEXT_LOCK_TIMEOUT_MS)) != 0) {
        LOG_ERR("ThingSet context lock timed out");
        return -THINGSET_ERR_INTERNAL_SERVER_ERR;
    }

    ts->msg = data;
    ts->msg_len = len;
    ts->msg_pos = 0;
    ts->rsp = NULL;
    ts->rsp_size = 0;
    ts->rsp_pos = 0;

    switch (format) {
        case THINGSET_BIN_IDS_VALUES:
            ts->endpoint.use_ids = true;
            thingset_bin_setup(ts, 0);
            ts->msg_payload = data;
            ts->api->deserialize_payload_reset(ts);
            err = thingset_bin_import_data(ts, auth_flags, format);
            break;
        default:
            err = -THINGSET_ERR_NOT_IMPLEMENTED;
            break;
    }

    k_sem_give(&ts->lock);

    return err;
}

int thingset_import_record(struct thingset_context *ts, const uint8_t *data, size_t len,
                           struct thingset_endpoint *endpoint, enum thingset_data_format format)
{
    int err;

    if (k_sem_take(&ts->lock, K_MSEC(THINGSET_CONTEXT_LOCK_TIMEOUT_MS)) != 0) {
        LOG_ERR("ThingSet context lock timed out");
        return -THINGSET_ERR_INTERNAL_SERVER_ERR;
    }

    ts->msg = data;
    ts->msg_len = len;
    ts->msg_pos = 0;
    ts->rsp = NULL;
    ts->rsp_size = 0;
    ts->rsp_pos = 0;

    ts->endpoint = *endpoint;

    switch (format) {
        case THINGSET_TXT_NAMES_VALUES:
            thingset_txt_setup(ts);
            ts->msg_payload = data;
            ts->api->deserialize_payload_reset(ts);
            break;
        case THINGSET_BIN_IDS_VALUES:
            ts->endpoint.use_ids = true;
            thingset_bin_setup(ts, 0);
            ts->msg_payload = data;
            ts->api->deserialize_payload_reset(ts);
            break;
        default:
            err = -THINGSET_ERR_NOT_IMPLEMENTED;
            goto out;
    }

    err = ts->api->deserialize_map_start(ts);
    if (err != 0) {
        goto out;
    }

    const struct thingset_data_object *item;
    while ((err = ts->api->deserialize_child(ts, &item)) != -THINGSET_ERR_DESERIALIZATION_FINISHED)
    {
        if (err != 0) {
            goto out;
        }

        struct thingset_records *records = ts->endpoint.object->data.records;
        uint8_t *record_ptr =
            (uint8_t *)records->records + ts->endpoint.index * records->record_size;

        if (item->type == THINGSET_TYPE_ARRAY) {
            struct thingset_array *arr = item->data.array;
            struct thingset_array arr_offset = {
                { .u8 = record_ptr + arr->elements.offset },
                arr->element_type,
                arr->decimals,
                arr->max_elements,
                arr->num_elements,
            };
            struct thingset_data_object item_offset = {
                item->parent_id,          item->id,   item->name,
                { .array = &arr_offset }, item->type, item->detail,
            };
            err = ts->api->deserialize_value(ts, &item_offset, false);
        }
        else {
            struct thingset_data_object item_offset = {
                item->parent_id, item->id,     item->name, { .u8 = record_ptr + item->data.offset },
                item->type,      item->detail,
            };
            err = ts->api->deserialize_value(ts, &item_offset, false);
        }

        if (err != 0) {
            goto out;
        }
    }

    err = ts->api->deserialize_finish(ts);

out:
    k_sem_give(&ts->lock);

    return err;
}

int thingset_report_path(struct thingset_context *ts, char *buf, size_t buf_size, const char *path,
                         enum thingset_data_format format)
{
    int err;

    if (k_sem_take(&ts->lock, K_MSEC(THINGSET_CONTEXT_LOCK_TIMEOUT_MS)) != 0) {
        LOG_ERR("ThingSet context lock timed out");
        return -THINGSET_ERR_INTERNAL_SERVER_ERR;
    }

    ts->rsp = buf;
    ts->rsp_size = buf_size;
    ts->rsp_pos = 0;

    err = thingset_endpoint_by_path(ts, &ts->endpoint, path, strlen(path));
    if (err != 0) {
        goto out;
    }
    else if (ts->endpoint.object == NULL) {
        err = -THINGSET_ERR_BAD_REQUEST;
        goto out;
    }

    switch (format) {
        case THINGSET_TXT_NAMES_VALUES:
            thingset_txt_setup(ts);
            break;
        case THINGSET_BIN_IDS_VALUES:
            ts->endpoint.use_ids = true;
            thingset_bin_setup(ts, 1);
            break;
        default:
            err = -THINGSET_ERR_NOT_IMPLEMENTED;
            goto out;
    }

    err = ts->api->serialize_report_header(ts, path);
    if (err != 0) {
        goto out;
    }

    switch (ts->endpoint.object->type) {
        case THINGSET_TYPE_GROUP:
            err = thingset_common_serialize_group(ts, ts->endpoint.object);
            break;
        case THINGSET_TYPE_SUBSET:
            err = ts->api->serialize_subsets(ts, ts->endpoint.object->data.subset);
            break;
        case THINGSET_TYPE_FN_VOID:
        case THINGSET_TYPE_FN_I32:
            /* bad request, as we can't read exec object's values */
            err = -THINGSET_ERR_BAD_REQUEST;
            break;
        case THINGSET_TYPE_RECORDS:
            if (ts->endpoint.index != THINGSET_ENDPOINT_INDEX_NONE) {
                err = thingset_common_serialize_record(ts, ts->endpoint.object, ts->endpoint.index);
                break;
            }
            /* fallthrough */
        default:
            err = ts->api->serialize_value(ts, ts->endpoint.object);
            break;
    }

    ts->api->serialize_finish(ts);

    if (err == 0) {
        err = ts->rsp_pos;
    }

out:
    k_sem_give(&ts->lock);

    return err;
}

void thingset_set_authentication(struct thingset_context *ts, uint8_t flags)
{
    ts->auth_flags = flags;
}

void thingset_set_update_callback(struct thingset_context *ts, const uint16_t subsets,
                                  void (*update_cb)(void))
{
    ts->update_subsets = subsets;
    ts->update_cb = update_cb;
}

struct thingset_data_object *thingset_get_child_by_name(struct thingset_context *ts,
                                                        uint16_t parent_id, const char *name,
                                                        size_t len)
{
    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].parent_id == parent_id
            && strncmp(ts->data_objects[i].name, name, len) == 0
            // without length check foo and fooBar would be recognized as equal
            && strlen(ts->data_objects[i].name) == len)
        {
            return &(ts->data_objects[i]);
        }
    }

    return NULL;
}

struct thingset_data_object *thingset_get_object_by_id(struct thingset_context *ts, uint16_t id)
{
    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].id == id) {
            return &(ts->data_objects[i]);
        }
    }

    return NULL;
}

int thingset_endpoint_by_path(struct thingset_context *ts, struct thingset_endpoint *endpoint,
                              const char *path, size_t path_len)
{
    struct thingset_data_object *object = NULL;
    const char *start = path;
    const char *end;
    uint16_t parent = 0;

    endpoint->index = THINGSET_ENDPOINT_INDEX_NONE;
    endpoint->use_ids = false;

    if (path_len == 0) {
        endpoint->object = &root_object;
        return 0;
    }

    if (start[0] == '/') {
        return -THINGSET_ERR_NOT_A_GATEWAY;
    }

    /* maximum depth of 10 assumed */
    for (int i = 0; i < 10; i++) {
        end = strchr(start, '/');
        if (end == NULL || end >= path + path_len) {
            /* reached at the end of the path */
            if (object != NULL && object->type == THINGSET_TYPE_RECORDS && start[0] >= '0'
                && start[0] <= '9')
            {
                /* numeric ID to select index in an array of records */
                endpoint->index = strtoul(start, NULL, 0);
                endpoint->object = object;
            }
            else if (start[0] == '-') {
                /* non-existent element behind the last array element */
                endpoint->index = THINGSET_ENDPOINT_INDEX_NEW;
                endpoint->object = object;
            }
            else {
                object = thingset_get_child_by_name(ts, parent, start, path + path_len - start);
            }
            break;
        }
        else if (end == path + path_len - 1) {
            /* path ends with slash */
            object = thingset_get_child_by_name(ts, parent, start, end - start);
            break;
        }
        else {
            /* go further down the path */
            object = thingset_get_child_by_name(ts, parent, start, end - start);
            if (object) {
                parent = object->id;
                start = end + 1;
            }
            else {
                break;
            }
        }
    }

    endpoint->object = object;

    if (object == NULL) {
        return -THINGSET_ERR_NOT_FOUND;
    }

    return 0;
}

int thingset_endpoint_by_id(struct thingset_context *ts, struct thingset_endpoint *endpoint,
                            uint16_t id)
{
    struct thingset_data_object *object;
    endpoint->index = THINGSET_ENDPOINT_INDEX_NONE;
    endpoint->use_ids = true;

    if (id == 0) {
        endpoint->object = &root_object;
        return 0;
    }
    else if (id == THINGSET_ID_PATHS) {
        endpoint->object = &paths_object;
        return 0;
    }
    else if (id == THINGSET_ID_METADATA) {
        endpoint->object = &types_object;
        return 0;
    }

    object = thingset_get_object_by_id(ts, id);
    if (object != NULL) {
        /* check that the found endpoint is not part of a record (cannot be queried like this) */
        struct thingset_data_object *parent = thingset_get_object_by_id(ts, object->parent_id);
        if (parent == NULL || parent->type != THINGSET_TYPE_RECORDS) {
            endpoint->object = object;
            return 0;
        }
    }

    return -THINGSET_ERR_NOT_FOUND;
}

int thingset_get_path(struct thingset_context *ts, char *buf, size_t size,
                      const struct thingset_data_object *obj)
{
    int pos = 0;
    if (obj->parent_id != 0) {
        struct thingset_data_object *parent_obj = thingset_get_object_by_id(ts, obj->parent_id);
        if (parent_obj == NULL) {
            return -THINGSET_ERR_NOT_FOUND;
        }

        /*
         * Recursive implementation acceptable because the depth is automatically limited by actual
         * data structure nesting depth.
         */
        pos = thingset_get_path(ts, buf, size, parent_obj);
        if (pos < 0) {
            /* propagate errors back */
            return pos;
        }
        buf[pos++] = '/';
    }

    pos += snprintf(buf + pos, size - pos, "%s", obj->name);

    if (pos < size) {
        return pos;
    }
    else {
        return -THINGSET_ERR_RESPONSE_TOO_LARGE;
    }
}

char *type_to_type_name(const enum thingset_type type)
{
    switch (type) {
        case THINGSET_TYPE_BOOL:
            return "bool";
        case THINGSET_TYPE_U8:
            return "u8";
        case THINGSET_TYPE_I8:
            return "i8";
        case THINGSET_TYPE_U16:
            return "u16";
        case THINGSET_TYPE_I16:
            return "i16";
        case THINGSET_TYPE_U32:
            return "u32";
        case THINGSET_TYPE_I32:
            return "i32";
        case THINGSET_TYPE_U64:
            return "u64";
        case THINGSET_TYPE_I64:
            return "i64";
        case THINGSET_TYPE_F32:
            return "f32";
        case THINGSET_TYPE_DECFRAC:
            return "decimal";
        case THINGSET_TYPE_STRING:
            return "string";
        case THINGSET_TYPE_BYTES:
            return "buffer";
        case THINGSET_TYPE_RECORDS:
            return "record";
        case THINGSET_TYPE_GROUP:
            return "group";
        case THINGSET_TYPE_SUBSET:
            return "subset";
        /* in theory, these last three will never be hit */
        case THINGSET_TYPE_ARRAY:
            return "array";
        case THINGSET_TYPE_FN_VOID:
            return "()->()";
        case THINGSET_TYPE_FN_I32:
            return "()->(i32)";
        default:
            return "";
    }
}

static int get_function_arg_types(struct thingset_context *ts, uint16_t parent_id, char *buf,
                                  size_t size)
{
    int len = 0;
    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].parent_id == parent_id) {
            if (len > 0) {
                if (size < 2) {
                    return -1;
                }
                len += sprintf(buf, ", ");
                size -= 2;
                buf += 2;
            }
            char *elementType = type_to_type_name(ts->data_objects[i].type);
            if (len > size) {
                return -1;
            }
            len += sprintf(buf, "%s", elementType);
            buf += len;
            size -= len;
        }
    }
    return len;
}

int thingset_get_type_name(struct thingset_context *ts, const struct thingset_data_object *obj,
                           char *buf, size_t size)
{
    switch (obj->type) {
        case THINGSET_TYPE_ARRAY:
            char *elementType = type_to_type_name(obj->data.array->element_type);
            if (sizeof(elementType) > size) {
                return -1;
            }
            return sprintf(buf, "%s[]", elementType);
        case THINGSET_TYPE_FN_VOID:
        case THINGSET_TYPE_FN_I32:
            sprintf(buf, "(");
            int len = 1 + get_function_arg_types(ts, obj->id, buf + 1, size - 1);
            if (len < 0) {
                return -1;
            }
            if (size - len < 8) { /* enough space to finish? */
                return -1;
            }
            buf += len;
            size -= len;
            switch (obj->type) {
                case THINGSET_TYPE_FN_VOID:
                    len += sprintf(buf, ")->()");
                    break;
                case THINGSET_TYPE_FN_I32:
                    len += sprintf(buf, ")->(i32)");
                    break;
                default:
                    break;
            }
            return len;
        default:
            char *type = type_to_type_name(obj->type);
            return sprintf(buf, "%s", type);
    }
}
