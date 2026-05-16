#ifndef PM_INTERPRETER_H
#define PM_INTERPRETER_H

#include <rdesc/rdesc.h>


double pm_interpreter(struct rdesc_node n);

double pm_interpret_pipe(struct rdesc_node pipe, double lhs);

double pm_interpret_function(struct rdesc_node function_call, double lhs);

double pm_extract_num(struct rdesc_node num);


#endif
