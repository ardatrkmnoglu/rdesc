// SPDX-FileCopyrightText: 2025-2026 Metehan Selvi <me@metehanselvi.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/* Public-facing implementation details (unstable). */

#ifndef RDESC_DETAIL_H
#define RDESC_DETAIL_H
/** @cond */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#if defined(__GNUC__) || defined(__clang__)
#define _rdesc_wur __attribute__((warn_unused_result))
#else
#define _rdesc_wur
#endif


struct rdesc {
	/* Grammar production rules. */
	const struct rdesc_grammar *grammar;

	/* Size in bytes allocated for each token's semantic information. */
	size_t seminfo_size;

	/* - Error Recovery -
	 *
	 * Extra space for holding a token in case of memory allocation error.
	 * Token will be copied to those fields for retry in next pump call.
	 */
	bool has_saved_tk;
	uint16_t saved_tk;
	void *saved_seminfo;

	/* - Navigation - */
	size_t cur  /* (current) Nonterminal being expanded; may not be
		     * the top element. */;
	uint16_t top_unwind  /* Stack's top node's unwind distance. */;

	/* Destructor method for tokens the parser owns. */
	void (*token_destroyer)(uint16_t, void *);

	/* Token stack used to store tokens temporarily during nonterminal
	 * backtracking. */
	struct rdesc_stack *token_stack;

	/* Underlying concrete syntax tree. */
	struct rdesc_stack *cst_stack;
};

struct rdesc_node {
	struct rdesc *p;
	struct _rdesc_priv_node *n;
};

/* These structs are private and should only be accessed via the provided
 * CST macros. */

#pragma pack(push, 1)

struct _rdesc_priv_tk {
	uint16_t _pad : 1;
	uint16_t id : 15;

	uint32_t seminfo  /* Semantic info starts here and extends into
			   * the flexible array member in _rdesc_priv_node. */;
};

struct _rdesc_priv_nt {
	uint16_t _pad : 1;
	uint16_t id : 15;

	uint16_t alt_idx;
	uint16_t child_count;
};

struct _rdesc_priv_node {
	/* ALSO CHANGE sizeof_node macro for any change in this struct. */
	size_t parent  /* Index of parent. */;
	uint16_t unwind_size  /* Previous node's unwind size (for backward
		               * navigation on the stack). */;

	union {
		uint16_t ty : 1  /* 0 for token and 1 for nonterminal. */;

		struct _rdesc_priv_tk tk;
		struct _rdesc_priv_nt nt;
	} n;
};

#pragma pack(pop)


/** @endcond */
#endif
