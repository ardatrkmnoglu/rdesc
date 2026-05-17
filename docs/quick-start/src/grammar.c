#include "grammar.h"

#include <rdesc/grammar.h>
#include <rdesc/rule_macros.h>

#include <stdlib.h>
#include <string.h>


/** @brief Pipe-Math grammar definition. */
//! [Basic rule macros]
struct rdesc_grammar_symbol pm_grammar[PM_NONTERMINAL_COUNT]
				      [PM_MAX_ALTERNATIVE_COUNT + 1]
				      [PM_MAX_ALTERNATIVE_SIZE + 1] = {
//! [Grammar]
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

//! [Nonterminal and terminal names]
const char *tk_names[] = {
	"\0",  /* We skip token with id 0, as we started enum from 1. */
	"@num", "@ident",  /* rdesc_dump_bnf utility wraps tokens with double
			    * quotes ("). Use @ to suppress automatic wrap. */
	"(", ")", "|", "^", ",", ";",
};

/** nonterminal names */
const char *nt_names[] = {
	"stmt", "expr",

	"exponentiation_expr", "exponentiation_expr_rest",
	"pipe_expr", "pipe_expr_rest",

	"function_arg_ls", "function_arg_ls_rest",

	"function_call",
};
//! [Nonterminal and terminal names]

//! [exblex tokens]
const char exblex_tks[] = {
	'\0',  /* exblex requires null termination at the beginning and end */
	'd',  /* num */ 'w',  /* ident */
	'(', ')', '|', '^', ',', ';',
	'\0'
};
//! [exblex tokens]


//! [Token destroyer]
void tk_destroyer(uint16_t id, void *seminfo)
{
	if (id == TK_NUM || id == TK_IDENT) {
		char *string;

		/* seminfo is a pointer to user-specific data, which is char *
		 * in our program. Extract the char * from void * by
		 * type-punning. */
		memcpy(&string, seminfo, sizeof(char *));

		free(string);
	}
}
//! [Token destroyer]


//! [Node printer]
#include <rdesc/cst_macros.h>

void node_printer(FILE *out, struct rdesc_node node)
{
	if (rtype(node) == RDESC_TOKEN) {
		if (rid(node) == TK_NUM || rid(node) == TK_IDENT) {
			char *string;

			memcpy(&string, rseminfo(node), sizeof(char *));

			fprintf(out, "[shape=box,label=<%s>]", string);
		} else {
			fprintf(out, "[shape=box,label=<%s>]", tk_names[rid(node)]);
		}
	} else {
		fprintf(out, "[label=\"%s\"]", nt_names[rid(node)]);
	}

}
//! [Node printer]
