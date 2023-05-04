/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset/thingset.h"
#include "thingset_internal.h"

#include <zephyr/logging/log.h>
#include <zephyr/toolchain/common.h>

#include <stdio.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(thingset, CONFIG_THINGSET_LOG_LEVEL);

/* pointers to iterable section data */
STRUCT_SECTION_START_EXTERN(thingset_data_object);
STRUCT_SECTION_END_EXTERN(thingset_data_object);

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
}

void thingset_init_global(struct thingset_context *ts)
{
    /* duplicates are checked at compile-time */

    ts->data_objects = TYPE_SECTION_START(thingset_data_object);
    STRUCT_SECTION_COUNT(thingset_data_object, &ts->num_objects);
    ts->auth_flags = THINGSET_USR_MASK;
}

int thingset_process_message(struct thingset_context *ts, const uint8_t *msg, size_t msg_len,
                             uint8_t *rsp, size_t rsp_size)
{
    if (msg == NULL || msg_len < 1) {
        return -THINGSET_ERR_BAD_REQUEST;
    }

    ts->msg = msg;
    ts->msg_len = msg_len;
    ts->msg_pos = 0;

    /* desires (without response) */
    if (ts->msg[0] == THINGSET_TXT_DESIRE) {
        return thingset_txt_desire(ts);
    }
    else if (ts->msg[0] == THINGSET_BIN_DESIRE) {
        return thingset_bin_desire(ts);
    }

    if (rsp == NULL || rsp_size < 4) {
        /* response buffer with at least 4 bytes required to fit minimum response */
        return -THINGSET_ERR_INTERNAL_SERVER_ERR;
    }

    ts->rsp = rsp;
    ts->rsp_size = rsp_size;
    ts->rsp_pos = 0;

    /* requests ordered with expected highest probability first */
    switch (ts->msg[0]) {
        case THINGSET_TXT_GET_FETCH:
            return thingset_txt_get_fetch(ts);
        case THINGSET_TXT_UPDATE:
            return thingset_txt_update(ts);
        case THINGSET_TXT_EXEC:
            return thingset_txt_exec(ts);
        case THINGSET_TXT_CREATE:
            return thingset_txt_create(ts);
        case THINGSET_TXT_DELETE:
            return thingset_txt_delete(ts);
        case THINGSET_BIN_GET:
            return thingset_bin_get(ts);
        case THINGSET_BIN_FETCH:
            return thingset_bin_fetch(ts);
        case THINGSET_BIN_UPDATE:
            return thingset_bin_update(ts);
        case THINGSET_BIN_EXEC:
            return thingset_bin_exec(ts);
        case THINGSET_BIN_CREATE:
            return thingset_bin_create(ts);
        case THINGSET_BIN_DELETE:
            return thingset_bin_delete(ts);
        default:
            return -THINGSET_ERR_BAD_REQUEST;
    }
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

    endpoint->index = INDEX_NONE;

    if (path_len == 0) {
        endpoint->object = NULL;
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
                endpoint->index = INDEX_NEW;
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

int thingset_serialize_path(struct thingset_context *ts, char *buf, size_t size,
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
        pos = thingset_serialize_path(ts, buf, size, parent_obj);
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
