/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include "../../src/thingset_internal.h"

#include "test_utils.h"

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

ZTEST(thingset_bin, test_get_root)
{
    const char req_hex[] = "01 00";
    const char rsp_exp_hex[] = "C1 F6 F6"; /* not yet implemented */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_fetch_root_names)
{
    const char req_hex[] = "05 00 F6";
    const char rsp_exp_hex[] = "C1 F6 F6"; /* not yet implemented */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_update_timestamp_zero)
{
    const char req_hex[] = "07 10 00";
    const char rsp_exp_hex[] = "C1 F6 F6"; /* not yet implemented */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_desire_timestamp_zero)
{
    const char req_hex[] = "1D 10 00";
    const char rsp_exp_hex[] = "C1 F6 F6"; /* not yet implemented */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_exec)
{
    const char req_hex[] = "02 00 00";     /* invalid request */
    const char rsp_exp_hex[] = "C1 F6 F6"; /* not yet implemented */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_create)
{
    const char req_hex[] = "06 00 00";     /* invalid request */
    const char rsp_exp_hex[] = "C1 F6 F6"; /* not yet implemented */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

ZTEST(thingset_bin, test_delete)
{
    const char req_hex[] = "04 00 00";     /* invalid request */
    const char rsp_exp_hex[] = "C1 F6 F6"; /* not yet implemented */

    THINGSET_ASSERT_REQUEST_HEX(req_hex, rsp_exp_hex);
}

static void *thingset_setup(void)
{
    thingset_init_global(&ts);

    return NULL;
}

ZTEST_SUITE(thingset_bin, NULL, thingset_setup, NULL, NULL, NULL);
