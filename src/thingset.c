/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset/thingset.h"

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

    switch (ts->req[0]) {
        case THINGSET_TXT_GET_FETCH:
        case THINGSET_TXT_EXEC:
        case THINGSET_TXT_DELETE:
        case THINGSET_TXT_CREATE:
        case THINGSET_TXT_UPDATE:
        case THINGSET_TXT_DESIRE:
            LOG_INF("text request: %c", ts->req[0]);
            return snprintf(rsp, rsp_size, ":%0X", THINGSET_ERR_NOT_IMPLEMENTED);
        case THINGSET_BIN_GET:
        case THINGSET_BIN_EXEC:
        case THINGSET_BIN_DELETE:
        case THINGSET_BIN_FETCH:
        case THINGSET_BIN_CREATE:
        case THINGSET_BIN_UPDATE:
        case THINGSET_BIN_DESIRE:
            LOG_INF("binary request: 0x%2X", ts->req[0]);
            rsp[1] = THINGSET_ERR_NOT_IMPLEMENTED;
            return 1;
        default:
            /* not a thingset command: ignore and set response to empty string */
            rsp[0] = '\0';
            return 0;
    }
}
