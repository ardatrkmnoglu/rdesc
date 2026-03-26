#include "pm_grammar.h"

#include <rdesc/grammar.h>
#include <rdesc/rule_macros.h>


/** @brief Pipe-Math grammar definition. */
struct rdesc_grammar_symbol pm_grammar[PM_PRODUCTION_COUNT]
				      [PM_MAX_ALTERNATIVE_COUNT + 1]
				      [PM_MAX_ALTERNATIVE_SIZE + 1] = {
//! [Grammar] [Basic rule macros]
/* <stmt> ::= */ r(
	NT(EXPR), TK(SEMI)
),
/* <expr> ::= */ r(
	NT(EXPONENTIATION_EXPR), TK(PIPE), NT(PIPE_EXPR)
alt	NT(EXPONENTIATION_EXPR)
)
//! [Basic rule macros]
,
//! [Recursive rule macros]
/* <exponentiation_expr> ::= */
	rrr(EXPONENTIATION_EXPR, (TK(NUM)), (TK(CARET), TK(NUM))),
/* this also expands to <exponentiation_expr_rest> */

/* <pipe_expr> ::= */
	rrr(PIPE_EXPR, (NT(FUNCTION_CALL)), (TK(PIPE), NT(FUNCTION_CALL))),
/* similarly, this also expands to <pipe_expr_rest> */

/* <function_arg_ls> ::= */
	rrr(FUNCTION_ARG_LS, (NT(EXPR)), (TK(COMMA), NT(EXPR)))
/* <function_arg_ls_rest> */
//! [Recursive rule macros]
,
//! [The rest]
/* <function_call> ::= */ r(
	TK(IDENT), TK(LPAREN), NT(FUNCTION_ARG_LS), TK(RPAREN)
alt	TK(IDENT)
)
//! [The rest]
//! [Grammar]
};
