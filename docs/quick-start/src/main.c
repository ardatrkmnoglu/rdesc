#include "grammar.h"
#include "interpreter.h"

#include <rdesc/grammar.h>
#include <rdesc/util.h>
#include <rdesc/rdesc.h>

#include "../../../examples/lib/exblex.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
//! [Initializing the grammar] * @cond */
#include <rdesc/grammar.h>
/** @endcond */

struct rdesc_grammar grammar;

/* Returns non-zero value on memory allocation error, abort the program in this
 * case. */
assert(rdesc_grammar_init_checked(&grammar,
				  PM_PRODUCTION_COUNT,
				  PM_MAX_ALTERNATIVE_COUNT,
				  PM_MAX_ALTERNATIVE_SIZE,
				  pm_grammar) == 0);
//! [Initializing the grammar]


//! [Dump BNF] /** @cond */
#include <rdesc/util.h>
/** @endcond */

if (argc > 1 && strcmp(argv[1], "dump_bnf") == 0) {
	rdesc_dump_bnf(stdout, &grammar, tk_names, nt_names);
}
//! [Dump BNF]


//! [Initializing the parser] /** @cond */
#include <rdesc/rdesc.h>
/** @endcond */
struct rdesc parser;

/* Similar to grammar_init, this function also returns non-zero in case of
 * memory allocation failure. */
assert(rdesc_init(&parser, &grammar, sizeof(char *), tk_destroyer) == 0);
//! [Initializing the parser]


//! [Pumping]
struct exblex lexer;
enum rdesc_result res;

char buf[4096];

if (fgets(buf, 4096, stdin) == NULL)
	buf[0] = '\0';

exblex_init(&lexer, buf, exblex_tks);

/* Memory allocation assertion. */
assert(rdesc_start(&parser, NT_STMT) == 0);

do {
	uint16_t tk_id = exblex_next(&lexer);
	char *tk_seminfo = exblex_current_seminfo(&lexer);

	/* EOF before match */
	if (tk_id == 0) {
		res = RDESC_NOMATCH;
		break;
	}

	res = rdesc_pump(&parser, tk_id, &tk_seminfo);

	/* Retry if allocation failed. */
	while (res == RDESC_ENOMEM)
		res = rdesc_resume(&parser);
} while (res == RDESC_CONTINUE);
//! [Pumping]


//! [Dump CST] /** @cond */
#include <rdesc/util.h>
/** @endcond */

if (argc > 1 && strcmp(argv[1], "dump_cst") == 0 && res == RDESC_READY) {
	rdesc_dump_cst(stdout, rdesc_get_root(&parser), node_printer);
}
//! [Dump CST]


//! [Calling interpreter]
if (res == RDESC_READY) {
	printf(">> %.2lf\n", pm_interpreter(rdesc_get_root(&parser)));
}
//! [Calling interpreter]


//! [Graceful shutdown]
rdesc_destroy(&parser);
rdesc_grammar_destroy(&grammar);
//! [Graceful shutdown]
}
