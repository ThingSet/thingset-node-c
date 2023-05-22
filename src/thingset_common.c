/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset/thingset.h"
#include "thingset_internal.h"

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int thingset_common_serialize_group(struct thingset_context *ts,
                                    const struct thingset_data_object *object)
{
    int err;

    err = ts->api->serialize_map_start(ts);
    if (err != 0) {
        return err;
    }

    if (object->data.group_callback != NULL) {
        object->data.group_callback(THINGSET_CALLBACK_PRE_READ);
    }

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].parent_id == object->id
            && (ts->data_objects[i].access & THINGSET_READ_MASK))
        {
            err = ts->api->serialize_key_value(ts, &ts->data_objects[i]);
            if (err != 0) {
                return err;
            }
        }
    }

    if (object->data.group_callback != NULL) {
        object->data.group_callback(THINGSET_CALLBACK_POST_READ);
    }

    return ts->api->serialize_map_end(ts);
}

int thingset_common_serialize_record(struct thingset_context *ts,
                                     const struct thingset_data_object *object, int record_index)
{
    struct thingset_records *records = object->data.records;
    int err;

    err = ts->api->serialize_map_start(ts);
    if (err != 0) {
        return err;
    }

    /* record item definitions are expected to start behind record data object */
    const struct thingset_data_object *item = object + 1;
    while (item < &ts->data_objects[ts->num_objects] && item->parent_id == object->id) {
        /* create new object with data pointer including offset */
        uint8_t *data_ptr =
            (uint8_t *)records->records + record_index * records->record_size + item->data.offset;
        struct thingset_data_object obj = {
            item->parent_id, item->id, item->name, { .u8 = data_ptr }, item->type, item->detail,
        };

        err = ts->api->serialize_key_value(ts, &obj);
        if (err != 0) {
            return err;
        }

        item++;
    }

    return ts->api->serialize_map_end(ts);
}

int thingset_common_get(struct thingset_context *ts)
{
    int err;

    ts->api->serialize_response(ts, THINGSET_STATUS_CONTENT, NULL);

    switch (ts->endpoint.object->type) {
        case THINGSET_TYPE_GROUP:
            err = thingset_common_serialize_group(ts, ts->endpoint.object);
            break;
        case THINGSET_TYPE_FN_VOID:
        case THINGSET_TYPE_FN_I32:
            /* bad request, as we can't read exec object's values */
            err = -THINGSET_ERR_BAD_REQUEST;
            break;
        case THINGSET_TYPE_RECORDS:
            if (ts->endpoint.index != INDEX_NONE) {
                err = thingset_common_serialize_record(ts, ts->endpoint.object, ts->endpoint.index);
                break;
            }
            /* fallthrough */
        default:
            err = ts->api->serialize_value(ts, ts->endpoint.object);
            break;
    }

    if (err == 0) {
        return ts->rsp_pos;
    }
    else {
        return ts->api->serialize_response(ts, -err, NULL);
    }
}
