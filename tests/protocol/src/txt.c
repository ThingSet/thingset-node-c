/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include "../../src/thingset_internal.h"

static struct thingset_context ts;

ZTEST(thingset_txt, test_response)
{
    char act[100];
    int len;

    char rsp_err[] = ":A0 \"test message 1\"";
    char rsp_changed[] = ":84";

    ts.rsp = act;
    ts.rsp_size = sizeof(act);

    len = thingset_txt_response(&ts, 0xA0, "test message %d", 1);
    zassert_equal(len, strlen(rsp_err));
    zassert_mem_equal(ts.rsp, rsp_err, strlen(ts.rsp), "act: %s\nexp: %s", ts.rsp, rsp_err);

    len = thingset_txt_response(&ts, 0x84, NULL);
    zassert_equal(len, strlen(rsp_changed));
    zassert_mem_equal(ts.rsp, rsp_changed, strlen(ts.rsp), "act: %s\nexp: %s", ts.rsp, rsp_changed);
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_txt, NULL, thingset_setup, NULL, NULL, NULL);
