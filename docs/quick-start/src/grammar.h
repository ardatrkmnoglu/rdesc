#ifndef PM_GRAMMAR_H
#define PM_GRAMMAR_H

#include <rdesc/rdesc.h>

#include <stdint.h>
#include <stdio.h>

//! [Grammar declaration]
#include <rdesc/grammar.h>

/* let's use `pm` as abbreviation for Pipe-Math. */
/** total number of nonterminals */
#define PM_NONTERMINAL_COUNT 9
/** max alternative count */
#define PM_MAX_ALTERNATIVE_COUNT 2
/** max alternative size */
#define PM_MAX_ALTERNATIVE_SIZE 4

extern struct rdesc_grammar_symbol pm_grammar[PM_NONTERMINAL_COUNT]
					     [PM_MAX_ALTERNATIVE_COUNT + 1]
					     [PM_MAX_ALTERNATIVE_SIZE + 1];
//! [Grammar declaration]

/** token enum */
//! [Token definition]
enum pm_tk {
	TK_NUM = 1,  // ⟨Num⟩
	TK_IDENT,  // ⟨Identifier⟩
	TK_LPAREN /* ( */, TK_RPAREN /* ) */,
	TK_PIPE /* | */, TK_CARET /* ^ */,
	TK_COMMA /* , */, TK_SEMI /* ; */,
};
//! [Token definition]

/** nonterminal enum */
//! [Nonterminal definition]
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

/** token names */
extern const char *tk_names[];

/** nonterminal names */
extern const char *nt_names[];

/** exblex token rules */
extern const char exblex_tks[];

/** free tokens constructed by exblex */
void tk_destroyer(uint16_t id, void *);

/** print out tokens */
void node_printer(FILE *out, struct rdesc_node node);


#endif
