/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset/thingset.h"
#include "thingset_internal.h"

#include <stdarg.h>
#include <stdio.h>

int thingset_bin_serialize_response(struct thingset_context *ts, uint8_t code, const char *msg, ...)
{
    /** @todo Add support for custom messages */

    ts->rsp[0] = code;
    ts->rsp[1] = 0xF6;
    ts->rsp[2] = 0xF6;
    ts->rsp_pos = 3;

    return ts->rsp_pos;
}

int thingset_bin_get(struct thingset_context *ts)
{
    return thingset_bin_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_fetch(struct thingset_context *ts)
{
    return thingset_bin_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_update(struct thingset_context *ts)
{
    return thingset_bin_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_exec(struct thingset_context *ts)
{
    return thingset_bin_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_create(struct thingset_context *ts)
{
    return thingset_bin_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_delete(struct thingset_context *ts)
{
    return thingset_bin_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_desire(struct thingset_context *ts)
{
    return -THINGSET_ERR_NOT_IMPLEMENTED;
}

int thingset_bin_process(struct thingset_context *ts)
{
    /* requests ordered with expected highest probability first */
    switch (ts->msg[0]) {
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
        case THINGSET_BIN_DESIRE:
            return thingset_bin_desire(ts);
        default:
            return -THINGSET_ERR_BAD_REQUEST;
    }
}
