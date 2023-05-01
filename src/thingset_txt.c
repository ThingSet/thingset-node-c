/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset/thingset.h"
#include "thingset_internal.h"

#include <stdarg.h>
#include <stdio.h>

int thingset_txt_serialize_response(struct thingset_context *ts, int code, const char *msg, ...)
{
    va_list vargs;

    ts->rsp_pos = snprintf((char *)ts->rsp, ts->rsp_size, ":%.2X", code);

    if (msg != NULL && ts->rsp_size > 7) {
        ts->rsp[ts->rsp_pos++] = ' ';
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
        ts->rsp[ts->rsp_pos] = '\0';
    }

    return ts->rsp_pos;
}

int thingset_txt_get_fetch(struct thingset_context *ts)
{
    return thingset_txt_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_txt_update(struct thingset_context *ts)
{
    return thingset_txt_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_txt_exec(struct thingset_context *ts)
{
    return thingset_txt_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_txt_create(struct thingset_context *ts)
{
    return thingset_txt_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_txt_delete(struct thingset_context *ts)
{
    return thingset_txt_serialize_response(ts, THINGSET_ERR_NOT_IMPLEMENTED, NULL);
}

int thingset_txt_desire(struct thingset_context *ts)
{
    return -THINGSET_ERR_NOT_IMPLEMENTED;
}
