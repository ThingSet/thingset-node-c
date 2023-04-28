/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test_utils.h"

#include <zephyr/sys/util.h>

#include <string.h>

size_t hex2bin_spaced(const char *hex, uint8_t *bin, size_t bin_size)
{
    int len = strlen(hex);
    size_t pos = 0;

    for (size_t i = 0; i < len; i += 3) {
        if (pos < bin_size) {
            hex2bin(&hex[i], 2, &bin[pos], 1);
            pos++;
        }
        else {
            return 0;
        }
    }

    return pos;
}

size_t bin2hex_spaced(const uint8_t *bin, size_t bin_size, char *hex, size_t hex_size)
{
    size_t pos = 0;

    for (size_t i = 0; i < bin_size; i++) {
        if (pos < hex_size + 3) {
            bin2hex(&bin[i], 1, &hex[pos], 3);
            hex[pos + 2] = ' ';
            pos += 3;
        }
        else {
            return 0;
        }
    }

    hex[pos - 1] = '\0';
    return pos - 1;
}
