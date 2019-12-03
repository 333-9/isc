/*
 * date:08.10. 2019
 * range operator functions
 */


#include <stdlib.h>

#include "table.h"
#include "isc.h"




int p_rand(int a, int b)  { return rand(); }
int p_assign(int a, int b)  { return b; }
int p_mod(int a, int b)  { return (a % b); } /* fails if b is 0 */
int p_div(int a, int b)  { return (a / b); } /* fails if b is 0 */
int p_ls (int a, int b)  { return (a << b); }
int p_rs (int a, int b)  { return (a << b); }

int p_add(int a, int b)  { return (a + b); }
int p_sub(int a, int b)  { return (a - b); }
int p_and(int a, int b)  { return (a? (b? a & b : a) : b); }
int p_or (int a, int b)  { return (a | b); }
int p_xor(int a, int b)  { return (a ^ b); }
int p_mul(int a, int b)  { return (a? (b? a * b : a) : b); }
int p_max(int a, int b)  { return (a > b? a : b); }
int p_min(int a, int b)  { return (a < b? (a?a:b) : (b?b:a)); }


static inline void
swap(size_t *a, size_t *b)
{
	size_t t;
	t = *a;
	*a = *b;
	*b = t;
}




int *
parser_get_num(size_t r, size_t c)
{
	if (r > row_changed_last)  row_changed_last  = r;
	if (r < row_changed_first) row_changed_first = r;
	return (int *) vsheet_get_num(sheet, r, c);
}


int
parser_for_range(size_t r1, size_t r2, size_t c, int (*func)(int, int))
{
	int ret = 0;
	Box_int *p;
	if (r1 > r2) swap(&r1, &r2);
	if (r1 < row_changed_first) row_changed_first = r1;
	if (r2 > row_changed_last)  row_changed_last  = r2;
	if (r2 >= sheet->rows) r2 = sheet->rows - 1;
	for (; r1 <= r2; r1++) {
		p = vsheet_get_num(sheet, r1, c);
		ret = (*func)(ret, (p ? *p : 0));
	};
	return ret;
}


int
parser_assign_for_range(size_t r1, size_t r2, size_t c, int (*func)(int, int), int n)
{
	Box_int *p;
	if (r1 > r2) swap(&r1, &r2);
	if (r1 < row_changed_first) row_changed_first = r1;
	if (r2 > row_changed_last)  row_changed_last  = r2;
	if (r2 >= sheet->rows  &&  vsheet_add_rows(&sheet, 1 + r2 - sheet->rows) < 0)
		die("Failed to realloc sheet");
	for (; r1 <= r2; r1++) {
		p = vsheet_get_num(sheet, r1, c);
		if (vsheet_set_box(&sheet, r1, c, (*func)((p ? *p : 0), n)) < 0)
			die("Failed to realloc"); /* not nescesary here */
	};
	return 0;
}

/* EOF */
