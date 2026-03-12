#include "../../include/cst_macros.h"


int main(void)
{
#ifndef rvariant
	"macro should be defined"--;
#endif
#ifndef rchild
	"macro should be defined"--;
#endif

#include "../../include/cst_macros.h"

#ifdef rid
	"macro should have undefined"--;
#endif
#ifdef rchild_count
	"macro should have undefined"--;
#endif

#include "../../include/cst_macros.h"

#ifndef rtype
	"macro should be defined"--;
#endif

#include "../../include/cst_macros.h"

#ifdef rseminfo
	"macro should have undefined"--;
#endif
}
