/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include <thingset.h>

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
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0);
    zassert_equal(endpoint.index, THINGSET_ENDPOINT_INDEX_NONE);

    path = "Types";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_false(ret < 0);
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0x200);
    zassert_equal(endpoint.index, THINGSET_ENDPOINT_INDEX_NONE);

    path = "Types/";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_false(ret < 0);
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0x200);
    zassert_equal(endpoint.index, THINGSET_ENDPOINT_INDEX_NONE);

    path = "Records";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_false(ret < 0);
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0x600);
    zassert_equal(endpoint.index, THINGSET_ENDPOINT_INDEX_NONE);

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
    zassert_equal(endpoint.index, THINGSET_ENDPOINT_INDEX_NEW);

    path = "Nested/Obj2/rItem1_V";
    ret = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_false(ret < 0);
    zassert_not_equal(endpoint.object, NULL);
    zassert_equal(endpoint.object->id, 0x707);
    zassert_equal(endpoint.index, THINGSET_ENDPOINT_INDEX_NONE);

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

ZTEST(thingset_common, test_serialize_path)
{
    struct thingset_data_object *obj;
    char buf[100];
    int len;

    obj = thingset_get_object_by_id(&ts, 0x10);
    len = thingset_serialize_path(&ts, buf, sizeof(buf), obj);
    zassert_true(len > 0);
    zassert_mem_equal(buf, "t_s", len);

    obj = thingset_get_object_by_id(&ts, 0x700);
    len = thingset_serialize_path(&ts, buf, sizeof(buf), obj);
    zassert_true(len > 0);
    zassert_mem_equal(buf, "Nested", len);

    obj = thingset_get_object_by_id(&ts, 0x702);
    len = thingset_serialize_path(&ts, buf, sizeof(buf), obj);
    zassert_true(len > 0);
    zassert_mem_equal(buf, "Nested/Obj1", len);

    obj = thingset_get_object_by_id(&ts, 0x705);
    len = thingset_serialize_path(&ts, buf, sizeof(buf), obj);
    zassert_true(len > 0);
    zassert_mem_equal(buf, "Nested/rBetween", len);

    obj = thingset_get_object_by_id(&ts, 0x707);
    len = thingset_serialize_path(&ts, buf, sizeof(buf), obj);
    zassert_true(len > 0);
    zassert_mem_equal(buf, "Nested/Obj2/rItem1_V", len);
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_common, NULL, thingset_setup, NULL, NULL, NULL);
