# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

target_sources(app PRIVATE data.c)
target_sources(app PRIVATE test_utils.c)
