#include "pm_functions.h"

#include <math.h>
#include <stddef.h>


static double pm_function_add(double lhs, size_t argc, double *argv)
{
	for (size_t i = 0; i < argc; i++)
		lhs += argv[i];

	return lhs;
}

static double pm_function_square(double lhs, size_t argc, double *argv)
{
	((void) argc);
	((void) argv);

	return lhs * lhs;
}

static double pm_function_log(double lhs, size_t argc, double *argv)
{
	((void) argc);
	((void) argv);

	return log(lhs);
}


pm_function pm_functions[] = {
	pm_function_add,
	pm_function_square,
	pm_function_log,
};

const char *pm_function_names[] = {
	"add",
	"square",
	"log",
};

size_t pm_function_count = 3;
