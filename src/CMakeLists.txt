# Copyright (c) The ThingSet Project Contributors
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named("thingset")

target_sources(thingset PRIVATE thingset.c)
target_sources(thingset PRIVATE thingset_bin.c)
target_sources(thingset PRIVATE thingset_common.c)
if(DEFINED CONFIG_THINGSET_TEXT_MODE)
    target_sources(thingset PRIVATE thingset_txt.c)
endif()
