/* Dump boolean algebra grammar to test dump_bnf feature. */

#include "../../include/grammar.h"
#include "../../include/util.h"
#include "../../src/common.h"

#include "../../examples/grammar/boolean_algebra.h"


int main(void)
{
	struct rdesc_grammar grammar;

	unwrap(rdesc_grammar_init(&grammar,
				  BALG_PRODUCTION_COUNT,
				  BALG_MAX_ALTERNATIVE_COUNT,
				  BALG_MAX_ALTERNATIVE_SIZE,
				  (struct rdesc_grammar_symbol *) balg));

	rdesc_dump_bnf(stdout, &grammar, balg_tk_names, balg_nt_names);

	rdesc_grammar_destroy(&grammar);
}
