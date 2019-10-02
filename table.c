/* * wur@guardian
 * date:26.09. 2019
 * the core commponents needet for spreadsheet calculator
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "table.h"




Sheet
sheet_init(size_t height)
{	struct sheet *s;
	if ((s = malloc(sizeof(struct sheet))) == NULL) return NULL;
	memset(s->num, 0, sizeof(void *) * Ncol);
	memset(s->str, 0, sizeof(void *) * Ncol);
	s->height = height;
	return s;
}


void
sheet_free(Sheet s)
{
	int i, si;
	for (i = 0; i < Ncol; i++) {
		free(s->num[i]);
		if (s->str[i] == NULL) break;
		for (si = 0; si < s->height; si++) {
			if (s->str[i][si] != NULL) free(s->str[i][si]);
		};
		free(s->str[i]);
	};
	free(s);
}


int
sheet_add_rows(Sheet s, size_t n)
{
	int i;
	s->height += n;
	for (i = 0; i < Ncol; i++) {
		if (s->num[i] != NULL &&
			(s->num[i] = realloc(s->num[i], s->height * sizeof(int))) == NULL)
			return -1;
		if (s->str[i] != NULL &&
			(s->str[i] = realloc(s->str[i], s->height * sizeof(char *))) == NULL)
			return -2;

	};
	return 0;
}


static int
sheet_add_num_col(Sheet s, unsigned char col) {
	if (s->num[col] != NULL) return 0;
	if ((s->num[col] = malloc(s->height * sizeof(int))) == NULL) return -1;
	memset(s->num[col], 0, s->height * sizeof(int));
	return 0;
}

static int
sheet_add_str_col(Sheet s, unsigned char col) {
	if (s->str[col] != NULL) return 0;
	if ((s->str[col] = malloc(s->height * sizeof(char *))) == NULL) return -1;
	memset(s->str[col], 0, s->height * sizeof(char *));
	return 0;
}


int
sheet_add_num(Sheet s, unsigned char col, size_t row, int num)
{
	if (sheet_add_num_col(s, col) < 0) return -1;
	s->num[col][row] = num;
}


int
sheet_add_str(Sheet s, unsigned char col, size_t row, const char *str)
{
	if (sheet_add_str_col(s, col) < 0) return -1;
	s->str[col][row] = strdup(str);
}


int
sheet_set_str(Sheet s, unsigned char col, size_t row, char *str)
{
	if (sheet_add_str_col(s, col) < 0) return -1;
	s->str[col][row] = str;
}


int
sheet_get_num(Sheet s, unsigned char col, size_t row)
{
	if (row >= s->height) return 0;
	if (s->num[col] == NULL) return 0;
	return (s->num[col][row]);
}


char *
sheet_get_str(Sheet s, unsigned char col, size_t row)
{
	if (row >= s->height) return 0;
	if (s->str[col] == NULL) return 0;
	return (s->str[col][row]);
}


int
sheet_do_col(Sheet s, unsigned char col, int (*func)(int, int))
{
	int i;
	int arg;
	for (i = 0; i < s->height; i++) {
		arg = (*func)(arg, s->num[col][i]);
	};
	return arg;
}


/* --------------------------------------------------------------------- */


struct var_sheet *
vsheet_init(size_t columns)
{	struct var_sheet *ret;
	ret = malloc(sizeof(struct var_sheet));
	if (ret == NULL) return NULL;
	ret->next = NULL;
	ret->cols = columns;
	ret->rows = 0;
	return ret;
}


void
vsheet_free(struct var_sheet *s) {
	free(s);
}


struct var_sheet *
vsheet_add_rows(struct var_sheet *s, size_t n)
{
	s->rows += n;
	s = realloc(s, sizeof(struct var_sheet) + (sizeof(box_sz) * s->cols * s->rows));
	memset(&(s->vals[s->cols * (s->rows - n)]), 0, sizeof(box_sz) * n * s->cols);
	return s;
}


struct var_sheet *
vsheet_set_box(struct var_sheet *s, size_t row, size_t col, box_sz val)
{
	if (col >= s->cols) return NULL;
	//if (row >= s->rows) s = vsheet_add_rows(s, s->rows - row + 1);
	s->vals[(row * s->cols) + col] = val;
	return s;
}




/*
int
main()
{
	Sheet s;
	s = sheet_init(0);
	sheet_add_rows(s, 2);
	if (sheet_add_num(s, 2, 1, 3) < 0) return 1;
	if (sheet_add_str(s, 2, 1, "aaa") < 0) return 1;
	if (sheet_add_num(s, 3, 1, 9) < 0) return 1;
	if (sheet_add_str(s, 3, 1, "bbb") < 0) return 1;
	printf("%i, %s\n", sheet_get_num(s, 2, 1), sheet_get_str(s, 2, 1));
	printf("%i, %s\n", sheet_get_num(s, 3, 1), sheet_get_str(s, 3, 1));
	sheet_free(s);
	return 0;
}
*/