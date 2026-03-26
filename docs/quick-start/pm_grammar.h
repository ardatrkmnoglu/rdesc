#ifndef PM_GRAMMAR_H
#define PM_GRAMMAR_H

//! [Grammar declaration]
#include <rdesc/grammar.h>

/* lets use `pm` as abbreviation for Pipe Math. */
#define PM_PRODUCTION_COUNT 10
#define PM_MAX_ALTERNATIVE_COUNT 10
#define PM_MAX_ALTERNATIVE_SIZE 10

extern struct rdesc_grammar_symbol pm_grammar[PM_PRODUCTION_COUNT]
					     [PM_MAX_ALTERNATIVE_COUNT + 1]
					     [PM_MAX_ALTERNATIVE_SIZE + 1];
//! [Grammar declaration]

//! [Token definition]
enum pm_tk {
	TK_NUM,  // ⟨Num⟩
	TK_IDENT,  // ⟨Identifier⟩
	TK_LPAREN /* ( */, TK_RPAREN /* ) */,
	TK_PIPE /* | */, TK_CARET /* ^ */,
	TK_COMMA /* , */, TK_SEMI /* ; */,
};
//! [Token definition]


#endif
