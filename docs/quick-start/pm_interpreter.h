#ifndef PM_INTERPRETER_H
#define PM_INTERPRETER_H

#include <rdesc/rdesc.h>


double pm_interpreter(struct rdesc *p, struct rdesc_node *n);

double pm_interpreter_pipe(struct rdesc *p,
			   struct rdesc_node *pipe,
			   double lhs);

double pm_extract_num(struct rdesc_node *num);

#endif
