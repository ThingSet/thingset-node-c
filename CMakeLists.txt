# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0
#
# This CMake file is picked by the Zephyr build system because it is defined
# as the module CMake entry point (see zephyr/module.yml).

zephyr_library_named("thingset")

zephyr_include_directories(include)
zephyr_include_directories(lib)

add_subdirectory_ifdef(CONFIG_THINGSET src)

# linker files required for auto-generation of ts_data_objects array
if(DEFINED CONFIG_THINGSET_IMMUTABLE_OBJECTS)
    zephyr_linker_sources(SECTIONS zephyr/thingset_iterables_rom.ld)
else()
    zephyr_linker_sources(DATA_SECTIONS zephyr/thingset_iterables_ram.ld)
endif()
