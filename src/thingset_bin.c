/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset/thingset.h"
#include "thingset_internal.h"

#include <stdarg.h>
#include <stdio.h>

int thingset_bin_response(struct thingset_context *ts, int code, const char *msg, ...)
{
    /** @todo Add support for custom messages */

    if (ts->rsp_size >= 3) {
        ts->rsp[0] = code;
        ts->rsp[1] = 0xF6;
        ts->rsp[2] = 0xF6;
        return 3;
    }
    else {
        return 0;
    }
}

int thingset_bin_get(struct thingset_context *ts)
{
    return thingset_bin_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_fetch(struct thingset_context *ts)
{
    return thingset_bin_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_update(struct thingset_context *ts)
{
    return thingset_bin_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_desire(struct thingset_context *ts)
{
    return thingset_bin_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_exec(struct thingset_context *ts)
{
    return thingset_bin_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_create(struct thingset_context *ts)
{
    return thingset_bin_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_bin_delete(struct thingset_context *ts)
{
    return thingset_bin_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}
