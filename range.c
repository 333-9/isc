/*
 * wur@guardian
 * date:08.10. 2019
 * this cares about all the range operator functions
 */


#include <stdlib.h>

#include "table.h"




extern struct var_sheet  *sheet;
extern int row_changed_first = 0;
extern int row_changed_last = 0;


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
	int *p, ret = 0, r;
	if (r1 < row_changed_first) row_changed_first = r1;
	if (r2 > row_changed_last)  row_changed_last  = r2;
	if (r1 > r2) {
		p = vsheet_get_num(sheet, r2, c);
		if (r1 >= sheet->rows) r1 = sheet->rows - 1;
		r = r1 - r2;
	} else {
		p = vsheet_get_num(sheet, r1, c);
		if (r2 >= sheet->rows) r2 = sheet->rows - 1;
		r = r2 - r1;
	};
	if (p == NULL) return 0;
	for (r += 1; r > 0; r--) {
		ret = (*func)(ret, *p);
		p += sheet->cols;
	};
	return ret;
}


int
parser_assign_for_range(size_t r1, size_t r2, size_t c, int (*func)(int, int), int n)
{
	int *p, r;
	if (r1 < row_changed_first) row_changed_first = r1;
	if (r2 > row_changed_last)  row_changed_last  = r2;
	if (r1 > r2) {
		if (r1 >= sheet->rows) sheet = vsheet_add_rows(sheet, 1 + r1 - sheet->rows);
		p = vsheet_get_num(sheet, r2, c);
		r = r1 - r2;
	} else {
		if (r2 >= sheet->rows) sheet = vsheet_add_rows(sheet, 1 + r2 - sheet->rows);
		p = vsheet_get_num(sheet, r1, c);
		r = r2 - r1;
	};
	for (r += 1; r > 0; r--) { /* TODO: fix buffer overrun */
		*p = (*func)(*p, n);
		p += sheet->cols;
	};
	return 0;
}


