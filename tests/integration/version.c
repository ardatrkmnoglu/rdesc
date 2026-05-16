#include "../../include/rdesc.h"
#include "../../src/common.h"

#include <string.h>


int main(void)
{
	rdesc_assert(strcmp("0.3.0-preview", rdesc_version()) == 0,
		     "version mismatch");
	return 0;
}
