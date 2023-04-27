/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset/thingset.h"
#include "thingset_internal.h"

#include <stdarg.h>
#include <stdio.h>

int thingset_txt_response(struct thingset_context *ts, int code, const char *msg, ...)
{
    va_list vargs;
    int pos = 0;

    pos = snprintf((char *)ts->rsp, ts->rsp_size, ":%.2X \"", code);

    if (msg != NULL) {
        va_start(vargs, msg);
        pos += vsnprintf((char *)ts->rsp + pos, ts->rsp_size - pos - 1, msg, vargs);
        va_end(vargs);
        ts->rsp[pos++] = '"';
        ts->rsp[pos] = '\0';
    }
    else {
        ts->rsp[3] = '\0';
        pos = 3;
    }

    if (pos < ts->rsp_size) {
        return pos;
    }
    else {
        return 0;
    }
}
