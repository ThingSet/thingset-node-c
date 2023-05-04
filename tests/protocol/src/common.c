/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include "../../src/thingset_internal.h"

#include "test_utils.h"

static struct thingset_context ts;

ZTEST(thingset_common, test_endpoint_from_path)
{
    struct thingset_endpoint endpoint;
    char *path;
    int ret;

    path = "";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_equal(ret, 0);
    zassert_equal(endpoint.object, NULL);
    zassert_equal(endpoint.index, INDEX_NONE);

    path = "Types";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_false(ret < 0);
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0x200);
    zassert_equal(endpoint.index, INDEX_NONE);

    path = "Types/";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_false(ret < 0);
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0x200);
    zassert_equal(endpoint.index, INDEX_NONE);

    path = "Records";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_false(ret < 0);
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0x600);
    zassert_equal(endpoint.index, INDEX_NONE);

    path = "Records/1";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_false(ret < 0);
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0x600);
    zassert_equal(endpoint.index, 1);

    path = "Records/-";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_false(ret < 0);
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0x600);
    zassert_equal(endpoint.index, INDEX_NEW);

    path = "Nested/Obj2/rItem1_V";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_false(ret < 0);
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0x707);
    zassert_equal(endpoint.index, INDEX_NONE);

    path = "/";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_equal(ret, -THINGSET_ERR_NOT_A_GATEWAY);

    path = "Type";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_equal(ret, -THINGSET_ERR_NOT_FOUND);

    path = "Typess";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_equal(ret, -THINGSET_ERR_NOT_FOUND);
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_common, NULL, thingset_setup, NULL, NULL, NULL);
