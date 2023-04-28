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
}

void thingset_init_global(struct thingset_context *ts)
{
    /* duplicates are checked at compile-time */

    ts->data_objects = TYPE_SECTION_START(thingset_data_object);
    STRUCT_SECTION_COUNT(thingset_data_object, &ts->num_objects);
}

int thingset_process_request(struct thingset_context *ts, const uint8_t *req, size_t req_len,
                             uint8_t *rsp, size_t rsp_size)
{
    if (req == NULL || req_len < 1) {
        return 0;
    }

    ts->req = req;
    ts->req_len = req_len;
    ts->rsp = rsp;
    ts->rsp_size = rsp_size;

    /* ordered with expected highest probability first */
    switch (ts->req[0]) {
        case THINGSET_TXT_GET_FETCH:
            return thingset_txt_get_fetch(ts);
        case THINGSET_TXT_UPDATE:
            return thingset_txt_update(ts);
        case THINGSET_TXT_DESIRE:
            return thingset_txt_desire(ts);
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
        case THINGSET_BIN_DESIRE:
            return thingset_bin_desire(ts);
        case THINGSET_BIN_EXEC:
            return thingset_bin_exec(ts);
        case THINGSET_BIN_CREATE:
            return thingset_bin_create(ts);
        case THINGSET_BIN_DELETE:
            return thingset_bin_delete(ts);
        default:
            /* not a ThingSet request: ignore and set response to empty string */
            rsp[0] = '\0';
            return 0;
    }
}
