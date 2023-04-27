/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include "../../src/thingset_internal.h"

static struct thingset_context ts;

ZTEST(thingset_bin, test_response)
{
    char act[100];
    int len;

    uint8_t rsp_changed[] = { 0x84, 0xF6, 0xF6 };

    ts.rsp = act;
    ts.rsp_size = sizeof(act);

    len = thingset_bin_response(&ts, 0x84, NULL);
    zassert_equal(len, sizeof(rsp_changed));
    zassert_mem_equal(ts.rsp, rsp_changed, sizeof(rsp_changed));
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_bin, NULL, thingset_setup, NULL, NULL, NULL);
