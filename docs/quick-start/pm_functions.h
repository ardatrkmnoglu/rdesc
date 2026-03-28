#ifndef PM_FUNCTIONS
#define PM_FUNCTIONS

#include <stddef.h>


/* Function interface. */
typedef double (*pm_function)(double lhs, size_t argc, double *argv);

/* List of built-in functions. */
extern pm_function pm_functions[];

/* Their names. */
extern const char *pm_function_names[];

/* Their count. */
extern size_t pm_function_count;


#endif
