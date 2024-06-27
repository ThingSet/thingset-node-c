/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include <thingset.h>

extern struct thingset_data_object data_objects[];
extern size_t data_objects_size;

ZTEST(thingset_init, test_init_data_objects)
{
    struct thingset_global_context ts_local;
    struct thingset_global_context ts_global;

    /* initialized with array declared in data.c */
    thingset_init(&ts_local, data_objects, data_objects_size);

    /* initialized using Zephyr iterable sections */
    thingset_init_global(&ts_global);

    zassert_equal(ts_local.num_objects, data_objects_size);
    zassert_equal(ts_global.num_objects, data_objects_size);

    if (IS_ENABLED(CONFIG_THINGSET_OBJECT_LOOKUP_MAP)) {
        for (unsigned int i = 0; i < ts_local.num_objects; i++) {
            struct thingset_data_object local = ts_local.data_objects[i];
            struct thingset_data_object global = ts_global.data_objects[i];
            /* find size of object excluding pointer conveniently at the end */
            size_t size = sizeof(struct thingset_data_object) - sizeof(sys_snode_t);
            zassert_mem_equal(&local, &global, size);
        }
    }
    else {
        zassert_mem_equal(ts_local.data_objects, ts_global.data_objects,
                          data_objects_size * sizeof(struct thingset_data_object));
    }
}

ZTEST_SUITE(thingset_init, NULL, NULL, NULL, NULL, NULL);
