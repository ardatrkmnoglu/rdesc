/* Seminfo ownership tests. */

#include "../../include/grammar.h"
#include "../../include/rdesc.h"
#include "../../src/common.h"

#include "../../examples/grammar/boolean_algebra.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


// GCOVR_EXCL_START
void abort_destroyer(uint16_t id, void *seminfo)
{
	((void) id); ((void) seminfo);
	rdesc_assert(0, "this destroyer should not called");
}
// GCOVR_EXCL_STOP

void destroyer(uint16_t id, void *seminfo)
{
	((void) id);
	void *ptr;

	memcpy(&ptr, seminfo, sizeof(void *));

	free(ptr);
}


int main(void)
{
	struct rdesc_grammar grammar;
	struct rdesc p;

	unwrap(rdesc_grammar_init_checked(&grammar,
					 BALG_PRODUCTION_COUNT,
					 BALG_MAX_ALTERNATIVE_COUNT,
					 BALG_MAX_ALTERNATIVE_SIZE,
					 balg));
	unwrap(rdesc_init(&p, &grammar, sizeof(void *), abort_destroyer));

	/* Seminfo destructor is not called in complete parse. */
	unwrap(rdesc_start(&p, NT_STMT));
	rdesc_assert(rdesc_pump(&p, TK_SEMI, NULL) == RDESC_READY,);

	unwrap(rdesc_start(&p, NT_STMT));
	rdesc_assert(rdesc_pump(&p, TK_SEMI, NULL) == RDESC_READY,);

	rdesc_reset(&p);

	unwrap(rdesc_start(&p, NT_STMT));
	rdesc_assert(rdesc_pump(&p, TK_SEMI, NULL) == RDESC_READY,);

	rdesc_destroy(&p);

	/* In incomplete parse, seminfo destructor is called. */
	unwrap(rdesc_init(&p, &grammar, sizeof(void *), destroyer));

	unwrap(rdesc_start(&p, NT_STMT));

	void *seminfo = malloc(1);
	rdesc_assert(rdesc_pump(&p, TK_IDENT, &seminfo) == RDESC_CONTINUE,);

	rdesc_destroy(&p);
	rdesc_grammar_destroy(&grammar);
}
