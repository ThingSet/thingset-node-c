/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include <thingset.h>

#include "../../src/thingset_internal.h"

#include "test_utils.h"
#include "data.h"

static struct thingset_context ts;

struct object_callback_data {
    char **expected_hex;
    int *expected_size;
    int num_messages;
    int expected_num_messages;
};

static int object_callback(struct thingset_data_object *obj, void *callback_context)
{
    if (obj->type == THINGSET_TYPE_ARRAY_ELEMENT) {
        uint8_t buffer[THINGSET_TEST_BUF_SIZE];
        struct object_callback_data *data = (struct object_callback_data *)callback_context;
        int data_len = thingset_export_item(&ts, buffer, THINGSET_TEST_BUF_SIZE, obj, THINGSET_BIN_VALUES_ONLY);
        zassert_false(data_len < 0);
        zassert_true(data_len < 8);
        char *expected_hex = data->expected_hex[data->num_messages];
        uint8_t expected[THINGSET_TEST_BUF_SIZE];
        int exp_len = data->expected_size[data->num_messages];
        hex2bin_spaced(expected_hex, expected, exp_len);
        zassert_equal(exp_len, data_len);
        zassert_mem_equal(expected, buffer, exp_len);
        data->num_messages++;
    }
    return 0;
}

ZTEST(thingset_elemental_array_reports, test_serialize_float_array)
{
    char* expected_hex[3];
    expected_hex[0] = "82 00 FA BF 8C CC CD"; /* -1.1 */
    expected_hex[1] = "82 01 FA C0 0C CC CD"; /* -2.2 */
    expected_hex[2] = "82 02 FA C0 53 33 33"; /* -3.3 */

    int expected_sizes[] = { 7, 7, 7 };
    struct object_callback_data data = {
        .expected_hex = expected_hex,
        .expected_size = expected_sizes,
        .expected_num_messages = 3,
    };
    ts.elementwise_array_updates = true;
    int ret = thingset_for_object_in_subsets(&ts, SUBSET_ELEM_ARRAY_LIVE, object_callback, &data);
    ts.elementwise_array_updates = false;
    zassert_false(ret < 0);
    zassert_equal(3, data.num_messages, "Expected %d; was %d", 3, data.num_messages);
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_elemental_array_reports, NULL, thingset_setup, NULL, NULL, NULL);
