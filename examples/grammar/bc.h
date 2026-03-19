/**
 * @file bc.h
 * @brief Basic calculator.
 *
 * A lightweight arithmetic expression evaluator designed to demonstrate
 * the capabilities of librdesc in handling standard algebraic grammars.
 *
 * @note Disambiguation: This module is not related to the POSIX `bc`
 *       (arbitrary-precision calculator language). It is a minimal
 *       demonstration implementation named strictly for "Basic Calculator".
 *
 * Language features:
 * - Arithmetic operators (+, -, *, /)
 * - Operator precedence
 * - Grouping using parentheses ( ... ) to override standard precedence.
 * - Integer/floating point literals
 *
 * @see This header provides the exact same api with boolean_algebra.h. You may
 *      inspect source code of bc.h.
 */

#ifndef BC_H
#define BC_H
/** @cond */

#include "../../include/grammar.h"
#include "../../include/rule_macros.h"


#define BC_PRODUCTION_COUNT 12

#define BC_MAX_ALTERNATIVE_COUNT 3

#define BC_MAX_ALTERNATIVE_SIZE 4

enum bc_tk {
	TK_NUM = 1, TK_DOT,
	TK_MINUS, TK_PLUS, TK_MULT, TK_DIV,
	TK_LPAREN, TK_RPAREN, TK_ENDSYM,

	TK_DUMMY_AMBIGUITY_TRIGGER,
};

enum bc_nt {
	NT_UNSIGNED_NUM,

	NT_EXPR, NT_EXPR_REST, NT_EXPR_OP,
	NT_TERM, NT_TERM_REST, NT_TERM_OP,
	NT_FACTOR, NT_OPTSIGN,
	NT_ATOM,

	NT_STMT,
};

const char bc_tks[] = {
	'\0',
	'd', '.',
	'-', '+', '*', '/',
	'(', ')', ';',
	'?',

	'\0' /* required for exblex */
};

const char *const bc_nt_names[] = {
	"unsigned", "optsign", "sign",

	"expr", "expr_rest", "expr_op",
	"term", "term_rest", "term_op",
	"factor",

	"stmt",
};

static const struct rdesc_grammar_symbol
bc[BC_PRODUCTION_COUNT]
  [BC_MAX_ALTERNATIVE_COUNT + 1  /* +1 for end of production sentinel */]
  [BC_MAX_ALTERNATIVE_SIZE + 1  /* +1 for end of alternative sentinel */] = {
	/* <unsigned_num> ::= */ r(
		TK(NUM)
	alt	TK(DOT), TK(NUM)
	alt	TK(NUM), TK(DOT), TK(NUM)
	),

	/* <expr> ::= */
		rrr(EXPR, (NT(TERM)), (NT(EXPR_OP), NT(TERM))),
	/* <expr_op> ::= */ r(
		TK(PLUS)
	alt	TK(MINUS)
	),

	/* <term> ::= */
		rrr(TERM, (NT(FACTOR)), (NT(TERM_OP), NT(FACTOR))),
	/* <term_op> ::= */ r(
		TK(MULT)
	alt	TK(DIV)
	),

	/* <factor> ::= */ r(
		NT(OPTSIGN), NT(ATOM)
	),
	/* <optsign> ::= */ r(
		TK(MINUS)
	alt	TK(PLUS)
	alt	EPSILON
	),

	/* <atom> ::= */ r(
		NT(UNSIGNED_NUM)
	alt	TK(LPAREN), NT(EXPR), TK(RPAREN)
	alt	TK(LPAREN), NT(EXPR), TK(RPAREN), TK(DUMMY_AMBIGUITY_TRIGGER)
	),


	/* <stmt> ::= */ r(
		NT(EXPR), TK(ENDSYM)
	)
};


/** @endcond */
#endif
