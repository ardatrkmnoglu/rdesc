#include "pm_grammar.h"
#include "pm_interpreter.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>


//! [Interpreter]
#include <rdesc/rdesc.h>
#include <rdesc/cst_macros.h>
#include <rdesc/util.h>

/* We only call this function with nonterminals, starting from <stmt>. */
double pm_interpreter(struct rdesc *p,
		      struct rdesc_node *n)
{
	switch (rid(n)) {
	case NT_STMT:
		/* continue evaluation from first child (<expr>) */
		return pm_interpreter(p, rchild(p, n, 0));

	case NT_EXPR:
		switch (ralt_idx(n)) {
		case 0:  /* <exponentiation_expr> "|" <pipe_expr>*/
			/* flip <pipe_expr> to left-associative order */
			rdesc_flip_left(p, n, 2);

			/* Start pipe evaluation from left-to-right. */
			return pm_interpret_pipe(p,
					rchild(p, n, 2),
					pm_interpreter(p, rchild(p, n, 0)));
		default:  /* <exponentiation_expr> */
			return pm_interpreter(p, rchild(p, n, 0));
		}

	case NT_EXPONENTIATION_EXPR:
		/* <num> <exponentiation_expr_rest> */
		return pow(pm_extract_num(rchild(p, n, 0)),
			   pm_interpreter(p, rchild(p, n, 1)));

	case NT_EXPONENTIATION_EXPR_REST:
		switch (ralt_idx(n)) {
		case 0:  /* "^" <num> <exponentiation_expr_rest> */
			return pow(pm_extract_num(rchild(p, n, 1)),
				   pm_interpreter(p, rchild(p, n, 2)));
		default:  /* E */
			return 1;
		}
	}

	return 0;  // GCOVR_EXCL_LINE: Unreachable
}
//! [Interpreter]


//! [Extracting numbers]
double pm_extract_num(struct rdesc_node *num)
{
	char *seminfo;

	/* seminfo stores a pointer to number, so it is a char **.
	 *
	 * Aside: We need to use memcpy to retrieve char * from char ** to not
	 * break strict-aliasing rule. */
	memcpy(&seminfo, rseminfo(num), sizeof(char *));

	double seminfo_num = atof(seminfo);

	free(seminfo);

	return seminfo_num;
}
//! [Extracting numbers]


//! [Interpreting pipe]
/* Called for <pipe_expr>. */
double pm_interpret_pipe(struct rdesc *p,
			 struct rdesc_node *pipe,
			 double lhs  /* value at left-hand side of pipe */)
{
	switch (ralt_idx(pipe)) {
	case 0:  /* <pipe_expr> "|" <function_call> */
		return pm_interpret_pipe(p, rchild(p, pipe, 0),
					 pm_interpret_function(p, rchild(p, pipe, 2), lhs));

	default:  /* <function_call> */
		return pm_interpret_function(p, rchild(p, pipe, 0), lhs);
	}
}
//! [Interpreting pipe]

double pm_interpret_function(struct rdesc *p,
			     struct rdesc_node *pipe,
			     double lhs)
{
	((void) p);
	((void) pipe);
	((void) lhs);
	return lhs;
}
