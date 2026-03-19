// SPDX-FileCopyrightText: 2025-2026 Metehan Selvi <me@metehanselvi.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../include/grammar.h"
#include "../include/rule_macros.h"
#include "common.h"
#include "test_instruments.h"

#include <stdint.h>
#include <stdlib.h>


/* tight coupled with: tests/integration/error_recovery.c:main
 * Check grammar initialization fail tests after a change in this function. */
int rdesc_grammar_init(struct rdesc_grammar *grammar,
		       uint16_t production_count,
		       uint16_t max_alternative_count,
		       uint16_t max_alternative_size,
		       const struct rdesc_grammar_symbol *rules)
{
	grammar->rules = rules;

	max_alternative_count += 1;  /* for end of production sentinel */
	max_alternative_size += 1;  /* for end of alternative sentinel */

	grammar->production_count = production_count;
	grammar->max_alternative_count = max_alternative_count;
	grammar->max_alternative_size = max_alternative_size;

	grammar->child_caps = xmalloc(sizeof(size_t) * production_count);

	if (!grammar->child_caps)
		return 1;

	for (size_t nt_id = 0; nt_id < production_count; nt_id++) {
		grammar->child_caps[nt_id] = 0;

		for (size_t alternative = 0;
		     alternative < max_alternative_count;
		     alternative++) {
			size_t len;
			struct rdesc_grammar_symbol sym;

			for (len = 0;
			     (sym = productions(*grammar)[nt_id][alternative][len]).ty !=
				RDESC_SENTINEL;
			     len++);

			if (len > grammar->child_caps[nt_id])
				grammar->child_caps[nt_id] = len;

			if (sym.id == EOP)
				break;
		}
	}

	return 0;
}

void rdesc_grammar_destroy(struct rdesc_grammar *grammar)
{
	free(grammar->child_caps);
}
