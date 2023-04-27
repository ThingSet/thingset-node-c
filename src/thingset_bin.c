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
