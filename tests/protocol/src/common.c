/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include <thingset.h>

#include "../../src/thingset_internal.h"

#include "data.h"
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
    len = thingset_get_path(&ts, buf, sizeof(buf), obj);
    zassert_true(len > 0);
    zassert_mem_equal(buf, "t_s", len);

    obj = thingset_get_object_by_id(&ts, 0x700);
    len = thingset_get_path(&ts, buf, sizeof(buf), obj);
    zassert_true(len > 0);
    zassert_mem_equal(buf, "Nested", len);

    obj = thingset_get_object_by_id(&ts, 0x702);
    len = thingset_get_path(&ts, buf, sizeof(buf), obj);
    zassert_true(len > 0);
    zassert_mem_equal(buf, "Nested/Obj1", len);

    obj = thingset_get_object_by_id(&ts, 0x705);
    len = thingset_get_path(&ts, buf, sizeof(buf), obj);
    zassert_true(len > 0);
    zassert_mem_equal(buf, "Nested/rBetween", len);

    obj = thingset_get_object_by_id(&ts, 0x707);
    len = thingset_get_path(&ts, buf, sizeof(buf), obj);
    zassert_true(len > 0);
    zassert_mem_equal(buf, "Nested/Obj2/rItem1_V", len);
}

/* Paths are not null-terminated, so parsing must not read beyond the given length. */
static char path_without_termination[] = { 'T', 'y', 'p', 'e', 's', '/', 'w', 'B', 'o', 'o', 'l' };

ZTEST(thingset_common, test_endpoint_from_path_without_termination)
{
    struct thingset_endpoint endpoint;
    int err;

    err = thingset_endpoint_by_path(&ts, &endpoint, path_without_termination,
                                    sizeof(path_without_termination));
    zassert_equal(err, 0, "act: 0x%X", -err);
    zassert_equal(endpoint.object->id, 0x201);

    /* a shorter length must select the group instead of the item */
    err = thingset_endpoint_by_path(&ts, &endpoint, path_without_termination, 5);
    zassert_equal(err, 0, "act: 0x%X", -err);
    zassert_equal(endpoint.object->id, 0x200);

    /* an empty path selects the root object */
    err = thingset_endpoint_by_path(&ts, &endpoint, path_without_termination, 0);
    zassert_equal(err, 0, "act: 0x%X", -err);
    zassert_equal(endpoint.object->id, 0);

    /* an empty path must not be dereferenced when looked up directly */
    int index;
    zassert_is_null(thingset_get_object_by_path(&ts, path_without_termination, 0, &index));
}

/* A record index in a path must be limited to the range of valid indices. */
ZTEST(thingset_common, test_endpoint_from_path_record_index)
{
    struct thingset_endpoint endpoint;
    char *path;
    int err;

    path = "Records/1";
    err = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_equal(err, 0, "act: 0x%X", -err);
    zassert_equal(endpoint.index, 1);

    path = "Records/65535";
    err = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_equal(err, 0, "act: 0x%X", -err);
    zassert_equal(endpoint.index, 65535);

    /* an index above the range of uint16_t must be rejected instead of overflowing */
    path = "Records/65536";
    err = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_equal(err, -THINGSET_ERR_NOT_FOUND, "act: 0x%X", -err);

    path = "Records/99999999999999999999";
    err = thingset_endpoint_by_path(&ts, &endpoint, path, strlen(path));
    zassert_equal(err, -THINGSET_ERR_NOT_FOUND, "act: 0x%X", -err);
}

/* Importing a record must reject indices outside of the records array. */
ZTEST(thingset_common, test_import_record_invalid_index)
{
    struct thingset_endpoint endpoint;
    uint8_t data[THINGSET_TEST_BUF_SIZE];
    int err;

    /* {0x602: false} */
    int data_len = hex2bin_spaced("A1 19 06 02 F4", data, sizeof(data));

    err = thingset_endpoint_by_path(&ts, &endpoint, "Records/1", strlen("Records/1"));
    zassert_equal(err, 0, "act: 0x%X", -err);

    /* only two records exist */
    endpoint.index = 2;
    err = thingset_import_record(&ts, data, data_len, &endpoint, THINGSET_BIN_IDS_VALUES);
    zassert_equal(err, -THINGSET_ERR_NOT_FOUND, "act: 0x%X", -err);

    endpoint.index = THINGSET_ENDPOINT_INDEX_NONE;
    err = thingset_import_record(&ts, data, data_len, &endpoint, THINGSET_BIN_IDS_VALUES);
    zassert_equal(err, -THINGSET_ERR_NOT_FOUND, "act: 0x%X", -err);

    /* an endpoint that is not a records object must be rejected as well */
    err = thingset_endpoint_by_path(&ts, &endpoint, "Types", strlen("Types"));
    zassert_equal(err, 0, "act: 0x%X", -err);
    endpoint.index = 0;
    err = thingset_import_record(&ts, data, data_len, &endpoint, THINGSET_BIN_IDS_VALUES);
    zassert_equal(err, -THINGSET_ERR_NOT_FOUND, "act: 0x%X", -err);
}

/* Dynamic records keep only one record in memory, so the index must not be used as an offset. */
ZTEST(thingset_common, test_import_dyn_record)
{
    struct thingset_endpoint endpoint;
    uint8_t data[THINGSET_TEST_BUF_SIZE];
    int err;

    /* {0x681: 42} */
    int data_len = hex2bin_spaced("A1 19 06 81 18 2A", data, sizeof(data));

    err = thingset_endpoint_by_path(&ts, &endpoint, "DynRecords/7", strlen("DynRecords/7"));
    zassert_equal(err, 0, "act: 0x%X", -err);
    zassert_equal(endpoint.index, 7);

    err = thingset_import_record(&ts, data, data_len, &endpoint, THINGSET_BIN_IDS_VALUES);
    zassert_equal(err, 0, "act: 0x%X", -err);

    zassert_equal(dyn_records.index, 42);

    dyn_records.index = 0;
}

/* Rejecting an unsupported format must not leave the context locked. */
ZTEST(thingset_common, test_export_subsets_unsupported_format)
{
    uint8_t buf[THINGSET_TEST_BUF_SIZE];
    int err;

    err = thingset_export_subsets(&ts, buf, sizeof(buf), SUBSET_LIVE, THINGSET_BIN_IDS_ONLY);
    zassert_equal(err, -THINGSET_ERR_NOT_IMPLEMENTED, "act: 0x%X", -err);

    /* a leaked lock would make this time out and fail with an internal server error */
    err = thingset_export_subsets(&ts, buf, sizeof(buf), SUBSET_LIVE, THINGSET_BIN_IDS_VALUES);
    zassert_true(err > 0, "act: 0x%X", -err);
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_common, NULL, thingset_setup, NULL, NULL, NULL);
