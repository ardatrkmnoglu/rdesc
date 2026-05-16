#include "grammar.h"
#include "interpreter.h"
#include "functions.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//! [Interpreter]
#include <rdesc/rdesc.h>
#include <rdesc/cst_macros.h>
#include <rdesc/util.h>

/* We only call this function with nonterminals, starting from <stmt>. */
double pm_interpreter(struct rdesc_node n)
{
	switch (rid(n)) {
	case NT_STMT:
		/* continue evaluation from first child (<expr>) */
		return pm_interpreter(rchild(n, 0));

	case NT_EXPR:
		switch (ralt_idx(n)) {
		case 0:  /* <exponentiation_expr> "|" <pipe_expr>*/
			/* flip <pipe_expr> to left-associative order */
			rdesc_flip_left(n, 2);

			/* Start pipe evaluation from left-to-right. */
			return pm_interpret_pipe(rchild(n, 2),
					pm_interpreter(rchild(n, 0)));
		default:  /* <exponentiation_expr> */
			return pm_interpreter(rchild(n, 0));
		}

	case NT_EXPONENTIATION_EXPR:
		/* <num> <exponentiation_expr_rest> */
		return pow(pm_extract_num(rchild(n, 0)),
			   pm_interpreter(rchild(n, 1)));

	case NT_EXPONENTIATION_EXPR_REST:
		switch (ralt_idx(n)) {
		case 0:  /* "^" <num> <exponentiation_expr_rest> */
			return pow(pm_extract_num(rchild(n, 1)),
				   pm_interpreter(rchild(n, 2)));
		default:  /* E */
			return 1;
		}
	}

	return 0;  // GCOVR_EXCL_LINE: Unreachable
}
//! [Interpreter]


//! [Extracting numbers]
double pm_extract_num(struct rdesc_node num)
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
double pm_interpret_pipe(struct rdesc_node pipe,
			 double lhs  /* value at left-hand side of pipe */)
{
	switch (ralt_idx(pipe)) {
	case 0:  /* <pipe_expr> "|" <function_call> */
		return pm_interpret_function(rchild(pipe, 2),
			       pm_interpret_pipe(rchild(pipe, 0), lhs));

	default:  /* <function_call> */
		return pm_interpret_function(rchild(pipe, 0), lhs);
	}
}
//! [Interpreting pipe]

static void collect_function_args(struct rdesc_node args,
				  size_t *argc, double **argv)
{
	size_t current_idx;

	switch (rid(args)) {
	case NT_FUNCTION_ARG_LS:  /* <expr> <function_arg_ls_rest> */
		current_idx = (*argc)++;

		collect_function_args(rchild(args, 1), argc, argv);

		break;

	case NT_FUNCTION_ARG_LS_REST:
		switch (ralt_idx(args)) {
		case 0:  /* "," <expr> <function_arg_ls_rest> */
			current_idx = (*argc)++;

			collect_function_args(rchild(args, 2), argc, argv);

			break;

		case 1:  /* E */
			assert(*argv = malloc(sizeof(double) * *argc));

			return;
		}
	}

	(*argv)[current_idx] =
		pm_interpreter(rchild(args, rid(args) == NT_FUNCTION_ARG_LS ? 0 : 1));
}

double pm_interpret_function(struct rdesc_node function,
			     double lhs)
{
	char *function_name;

	/* child at index 0 is always function name (identifier) */
	memcpy(&function_name, rseminfo(rchild(function, 0)), sizeof(char *));

	size_t function_id;

	for (function_id = 0; function_id < pm_function_count; function_id++) {
		if (strcmp(function_name, pm_function_names[function_id]) == 0)
			break;
	}

	size_t argc = 0;
	double *argv = NULL;
	double result;

	if (ralt_idx(function) == 0) {  /* ident "(" <function_arg_ls> ")" */
		collect_function_args(rchild(function, 2), &argc, &argv);
	} else {  /* ident */  }

	if (function_id == pm_function_count) {
		fprintf(stderr, "Unknown function %s, ignoring\n", function_name);

		result = lhs;
	} else {
		result = pm_functions[function_id](lhs, argc, argv);
	}

	if (argv)
		free(argv);

	free(function_name);

	return result;
}
