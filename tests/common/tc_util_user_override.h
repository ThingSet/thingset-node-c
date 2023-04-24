/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * @brief Overrides for Zephyr testing environment to get colored output.
 */

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define TC_PASS_STR ANSI_COLOR_GREEN "PASS" ANSI_COLOR_RESET
#define TC_FAIL_STR ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET
#define TC_SKIP_STR ANSI_COLOR_YELLOW "SKIP" ANSI_COLOR_RESET
