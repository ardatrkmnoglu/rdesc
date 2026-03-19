// SPDX-FileCopyrightText: 2025-2026 Metehan Selvi <me@metehanselvi.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/* Dependency injection for error emulation. */

#ifndef TEST_INSTRUMENTS_H
#define TEST_INSTRUMENTS_H
#ifdef TEST_INSTRUMENTS

#include <stdbool.h>

/* Counters to memory allocation failures. */
extern int multipush_fail_at;

extern int realloc_fail_at;

extern int malloc_fail_at;

bool rdesc_test_instruments_check_failure(int *state, int count);

#define xmalloc(size) (rdesc_test_instruments_check_failure(&malloc_fail_at, 1) ? \
	NULL : malloc(size))

#define xrealloc(ptr, size) (rdesc_test_instruments_check_failure(&realloc_fail_at, 1) ? \
	NULL : realloc(ptr, size))

#define xmultipush(c) rdesc_test_instruments_check_failure(&multipush_fail_at, c)

#else

#define xmalloc malloc
#define xrealloc realloc
#define xmultipush(c) false

#endif
#endif
