// SPDX-FileCopyrightText: 2025-2026 Metehan Selvi <me@metehanselvi.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/**
 * @file grammar.h
 * @brief Grammar data structures.
 *
 * This header defines the core data structures used to represent a grammar in
 * librdesc.
 */

#ifndef RDESC_GRAMMAR_H
#define RDESC_GRAMMAR_H

#include "detail.h"

#include <stdint.h>


/**
 * @brief Grammar initialization instrumented with size assertion.
 *
 * This is the checked variant of `rdesc_grammar_init`. It ensures that the
 * provided `production_rules` array matches the expected size.
 */
#define rdesc_grammar_init_checked(grammar, \
				   production_count, \
				   max_alternative_count, \
				   max_alternative_size, \
				   production_rules) \
	_rdesc_priv_grammar_init_checked( \
		grammar, \
		production_count, max_alternative_count, max_alternative_size, \
		(struct rdesc_grammar_symbol *) production_rules, \
		sizeof(production_rules) / sizeof(struct rdesc_grammar_symbol) == \
			production_count * \
			(max_alternative_count + 1) * \
			(max_alternative_size + 1)\
	)


/**
 * @brief Grammar definition.
 *
 * The production rules are dimensioned as a 3D array where alternatives are
 * tried in order:
 * - [production_count][max_alternative_count][max_alternative_size]
 */
struct rdesc_grammar {
	/** @brief Grammar production rules. */
	const struct rdesc_grammar_symbol *rules;

	/** @brief Total number of nonterminals. */
	uint16_t production_count;

	/** @brief Maximum number of alternatives in a production rule. */
	uint16_t max_alternative_count;

	/** @brief Maximum length of an alternative. */
	uint16_t max_alternative_size;

	/**
	 * @brief Array of child capacities for each nonterminal.
	 *
	 * Specifies maximum children for each nonterminal's matched
	 * alternatives, used for CST stack memory allocation.
	 */
	uint16_t *child_caps;
};

/** @brief Symbol type discriminator for `rdesc_grammar_symbol`. */
enum rdesc_grammar_symbol_type {
	RDESC_TOKEN,
	RDESC_NONTERMINAL,
	/**
	 * @brief Sentinel marking the end of a production body or the end of
	 * all alternatives for a nonterminal.
	 */
	RDESC_SENTINEL,
};

/**
 * @brief A terminal or nonterminal representing the body (right side) of a
 * production rule.
 */
struct rdesc_grammar_symbol {
	enum rdesc_grammar_symbol_type ty  /** @brief Type of the symbol. */;

	int id  /** @brief Terminal, nonterminal, or sentinel identifier. */;
};


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes a grammar struct.
 *
 * `production_count` is equal to total number of nonterminals.
 */
int rdesc_grammar_init(struct rdesc_grammar *grammar,
		       uint16_t production_count,
		       uint16_t max_alternative_count,
		       uint16_t max_alternative_size,
		       const struct rdesc_grammar_symbol *production_rules) _rdesc_wur;

/** @brief Frees resources allocated by the grammar. */
void rdesc_grammar_destroy(struct rdesc_grammar *grammar);

/** @cond */
/* Use with rdesc_grammar_init checked. */
int _rdesc_priv_grammar_init_checked(struct rdesc_grammar *,
				     uint16_t, uint16_t, uint16_t,
				     const struct rdesc_grammar_symbol *,
				     int) _rdesc_wur;
/** @endcond */

#ifdef __cplusplus
}
#endif


#endif
