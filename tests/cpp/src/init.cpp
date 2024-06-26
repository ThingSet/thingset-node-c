/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include <thingset.h>

#include "../../src/thingset_internal.h"

/* use same test data as for standard test, but compile with g++ */
#include "data.c"

ZTEST(thingset_init_cpp, test_init_data_objects)
{
    struct thingset_global_context ts_local;
    struct thingset_global_context ts_global;

    /* initialized with array declared in data.c */
    thingset_init(&ts_local, data_objects, data_objects_size);

    /* initialized using Zephyr iterable sections */
    thingset_init_global(&ts_global);

    zassert_equal(ts_local.num_objects, data_objects_size);
    zassert_equal(ts_global.num_objects, data_objects_size);

    zassert_mem_equal(ts_local.data_objects, ts_global.data_objects,
                      data_objects_size * sizeof(struct thingset_data_object));
}

ZTEST_SUITE(thingset_init_cpp, NULL, NULL, NULL, NULL, NULL);
