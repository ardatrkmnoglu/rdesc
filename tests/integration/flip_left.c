#include "../../include/cst_macros.h"
#include "../../include/grammar.h"
#include "../../include/rdesc.h"
#include "../../include/util.h"
#include "../../src/common.h"

#include "../../examples/grammar/bc.h"

#include <stdint.h>


int main(void)
{
	struct rdesc_grammar grammar;
	struct rdesc p;

	unwrap(rdesc_grammar_init(&grammar,
				  BC_NT_COUNT, BC_NT_VARIANT_COUNT, BC_NT_BODY_LENGTH,
				  (struct rdesc_grammar_symbol *) bc));

	unwrap(rdesc_init(&p, &grammar, 0, 0));

	unwrap(rdesc_start(&p, NT_STMT));

	rdesc_assert(rdesc_pump(&p, TK_NUM, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_PLUS, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_NUM, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_MINUS, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_NUM, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_PLUS, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_NUM, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_ENDSYM, NULL) == RDESC_READY,);

	rdesc_flip_left(&p, rdesc_root(&p), 0);

	rdesc_destroy(&p);
	rdesc_grammar_destroy(&grammar);
}
