#include "../../include/rdesc.h"
#include "../../src/common.h"

#include <string.h>


int main(void)
{
	rdesc_assert(strcmp("0.2.0-rc.2", rdesc_version()) == 0,
		     "version mismatch");
	return 0;
}
