/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include "thingset/thingset.h"

#include "../../src/thingset_internal.h"

extern struct thingset_data_object data_objects[];
extern size_t data_objects_size;

ZTEST(thingset_init, test_init_data_objects)
{
    struct thingset_context ts_local;
    struct thingset_context ts_global;

    /* initialized with array declared in data.c */
    thingset_init(&ts_local, data_objects, data_objects_size);

    /* initialized using Zephyr iterable sections */
    thingset_init_global(&ts_global);

    zassert_equal(ts_local.num_objects, data_objects_size);
    zassert_equal(ts_global.num_objects, data_objects_size);

    zassert_mem_equal(ts_local.data_objects, ts_global.data_objects,
                      data_objects_size * sizeof(struct thingset_data_object));
}

ZTEST_SUITE(thingset_init, NULL, NULL, NULL, NULL, NULL);
