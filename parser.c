// date: 05.04 2020

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/*

Operators:
+ -
* / %
( )
[ ]
& | ^
< > >= <= != ==
?:
;


Addressing:
[20,e]     // value of row 20 column E
[f]        // sum of column F
[3]        // sum of row 3
[*a,*b]    // use registers a and b as row and column numbers
[^0]       // sum column 0 (A)
[*[0,A]]   // use value of 0A as row number
[*(a?1:0)] // use expression as row number
[6,a>]     // calculate value at 6A
[8,b<]     // move execution to 8B
[0,0%]     // format text at column 0A
[0,0%yes]  // give the word 'here' to 0A format
[0,b>0,1]  // calculate 0B with a=0, b=1
*/


int parse(const char *s, int *val)
{
	return 0;
}
