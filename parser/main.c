#include "y.tab.h"
#include <stdio.h>

int
main()
{
	fprintf(stderr, "%i\n", (int)yyparse());
	return 0;
}
