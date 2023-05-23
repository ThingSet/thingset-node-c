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
    int hex_len = strlen(hex);
    size_t bin_pos = 0;

    size_t hex_pos = 0;
    while (hex_pos < hex_len) {
        if (bin_pos < bin_size) {
            hex2bin(&hex[hex_pos], 2, &bin[bin_pos], 1);
            bin_pos++;
            hex_pos += 2;
        }
        else {
            return 0;
        }
        while (hex[hex_pos] == ' ' && hex_pos < hex_len) {
            hex_pos++;
        }
    }

    return bin_pos;
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
