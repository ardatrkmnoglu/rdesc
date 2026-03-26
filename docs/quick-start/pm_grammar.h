/**
 * @file pm_grammar.h
 */

#ifndef PM_GRAMMAR_H
#define PM_GRAMMAR_H

//! [Grammar declaration]
#include <rdesc/grammar.h>

/* let's use `pm` as abbreviation for Pipe Math. */
/** production count */
#define PM_PRODUCTION_COUNT 9
/** max alternative count */
#define PM_MAX_ALTERNATIVE_COUNT 2
/** max alternative size */
#define PM_MAX_ALTERNATIVE_SIZE 4

/**
 * @anchor pm_grammar_definition
 *
 * @snippet pm_grammar.c Grammar
 */
extern struct rdesc_grammar_symbol pm_grammar[PM_PRODUCTION_COUNT]
					     [PM_MAX_ALTERNATIVE_COUNT + 1]
					     [PM_MAX_ALTERNATIVE_SIZE + 1];
//! [Grammar declaration]

//! [Token definition]
/** token enum */
enum pm_tk {
	TK_NUM,  // ⟨Num⟩
	TK_IDENT,  // ⟨Identifier⟩
	TK_LPAREN /* ( */, TK_RPAREN /* ) */,
	TK_PIPE /* | */, TK_CARET /* ^ */,
	TK_COMMA /* , */, TK_SEMI /* ; */,
};
//! [Token definition]

//! [Nonterminal definition]
/** nonterminal enum */
enum pm_nt {
	/* ⟨Stmt⟩, ⟨Expr⟩ */
	NT_STMT, NT_EXPR,

	/* ⟨Num⟩ (^ ⟨Num⟩)* */
	NT_EXPONENTIATION_EXPR, NT_EXPONENTIATION_EXPR_REST,
	/* ⟨FunctionCall⟩ (| ⟨FunctionCall⟩)* */
	NT_PIPE_EXPR, NT_PIPE_EXPR_REST,

	/* We will define ⟨PipeExpr⟩ as right-associative for now. Later, we
	 * will use flip function to convert pipe operator left-associative. */

	/* ⟨Expr⟩ (, ⟨Expr⟩)* */
	NT_FUNCTION_ARG_LS, NT_FUNCTION_ARG_LS_REST,

	NT_FUNCTION_CALL,
};
//! [Nonterminal definition]


#endif
