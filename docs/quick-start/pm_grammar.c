#include "pm_grammar.h"

#include <rdesc/grammar.h>
#include <rdesc/rule_macros.h>


struct rdesc_grammar_symbol pm_grammar[PM_PRODUCTION_COUNT]
				      [PM_MAX_ALTERNATIVE_COUNT + 1]
				      [PM_MAX_ALTERNATIVE_SIZE + 1] = {
};
