/*
 * 26.09. 2019
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "table.h"


#define VSHI(s, r, c) ((r * s->cols) + c)


int cmt_sort_compare(const void *, const void *);




struct vsheet *
vsheet_init(size_t columns)
{	struct vsheet *ret;
	ret = malloc(sizeof(struct vsheet));
	if (ret == NULL) return NULL;
	ret->next = NULL;
	ret->rows = 0;
	ret->cols = columns;
	ret->last_ind = 0;
	ret->last_nonzero_row = 0;
	return ret;
}


void
vsheet_free(struct vsheet *s) {
	free(s);
}


int
vsheet_add_rows(struct vsheet **s, size_t n)
{
	(*s)->rows += n;
	*s = realloc(*s, sizeof(struct vsheet) + (sizeof(Box_int) * (*s)->cols * (*s)->rows));
	if (*s == NULL) return -1;
	memset(&((*s)->vals[(*s)->cols * ((*s)->rows - n)]), 0, sizeof(Box_int) * n * (*s)->cols);
	return 0;
}


int
vsheet_set_box(struct vsheet **s, size_t r, size_t c, Box_int val)
{
	if (c >= (*s)->cols) return 1;
	if (r >= (*s)->rows) {
		if (vsheet_add_rows(s, r - (*s)->rows + 1) < 0) return -1;
	};
	if (val && (r > (*s)->last_nonzero_row)) (*s)->last_nonzero_row = r;
	(*s)->vals[VSHI((*s), r, c)] = val;
	return 0;
}


Box_int *
vsheet_get_num(struct vsheet *s, size_t r, size_t c)
{
	if (r >= s->rows || c >= s->cols) return NULL;
	vsheet_update(s);
	//if (r > s->last_nonzero_row) s->last_nonzero_row = r;
	s->last_ind = VSHI(s, r, c);
	return (s->vals + s->last_ind);
}


int
vsheet_get_row_width(struct vsheet *s, size_t r)
{
	int i, wt = 0;
	if (r >= s->rows) return 0; /* width defaults to 0 */
	for (i = 0; i < s->cols; i++) {
		if (s->vals[VSHI(s, r, i)] != 0) wt = i;
	};
	return wt;
}


void
vsheet_update(struct vsheet *s)
{
	if (s->vals[s->last_ind] == 0) return ;
	if (s->last_ind / s->cols > s->last_nonzero_row) {
		s->last_nonzero_row = s->last_ind / s->cols;
		if (s->last_ind % s->cols) s->last_nonzero_row += 1;
	};
}




struct comment *
comment_setup(char *str, size_t row, char col, char type, char *metadata)
{	struct comment *cmt;
	if ((cmt = malloc(sizeof(struct comment))) == NULL) return NULL;
	cmt->row = row;
	cmt->col = col;
	if ((cmt->s = malloc(strlen(str) + strlen(metadata) + 2)) == NULL) return NULL;
	cmt->s[0] = type;
	strcpy(cmt->s + 1, metadata);
	strcat(cmt->s + 1, str);
	return (cmt);
};


struct cmt_list *
cmt_list_init(void)
{	struct cmt_list *ret;
	int i;
	ret = malloc(sizeof(struct cmt_list) +
	            CMT_LIST_CHUNK * sizeof(struct comment));
	if (ret == NULL) return NULL;
	ret->sz = CMT_LIST_CHUNK;
	for (i = 0; i < ret->sz; i++) {
		ret->list[i].s = NULL;
	};
	return ret;
}


size_t
cmt_list_get_from(struct cmt_list **cl, int r, int c)
{
	size_t i;
	for (i = 0; i < (*cl)->sz; i++) {
		if ((*cl)->list[i].s == NULL) break;
		if (r < (*cl)->list[i].row)
			return i;
		if ((r == (*cl)->list[i].row)
		&&  (c <= (*cl)->list[i].col))
			return i;
	};
	return ((*cl)->sz - 1);
}


int
cmt_sort_compare(
    const void *arga,
    const void *argb)
{
	const struct comment *a, *b;
	a = arga;
	b = argb;
	if (b->s == NULL && a->s == NULL)  return (!a->s) - (!b->s);
	if (a->row != b->row)  return a->row - b->row;
	if (a->col != b->col)  return a->col - b->col;
	return 0;
}


int
cmt_list_update(struct cmt_list **cl)
{
	qsort((*cl)->list, (*cl)->sz, sizeof(struct comment), cmt_sort_compare);
	return 0;
}


char *
cmt_list_str(struct cmt_list **cl, size_t r, size_t c) /* expects sorted list */
{
	int i;
	if (c >= 0) { /* search for specific comment */
		for (i = 0; i < (*cl)->sz; i++) {
			if ((*cl)->list[i].s   != NULL &&
			    (*cl)->list[i].row == r &&
			    (*cl)->list[i].col == c )
			{
				return ((*cl)->list + i)->s;
			};
		};
	};
	return "";
}


struct comment *
cmt_list_new(struct cmt_list **cl, int r, int c)
{
	int i;
	struct cmt_list *clp = *cl;
	if (c >= 0) { /* search for specific comment */
		for (i = 0; i < clp->sz; i++) {
			if (clp->list[i].s   != NULL &&
			    clp->list[i].row == r &&
			    clp->list[i].col == c ) {
				return (clp->list + i);
			} else if (clp->list[i].s == NULL) {
				clp->list[i].row = r;
				clp->list[i].col = c;
				return (clp->list + i);
			};
		};
	} else {
		/* TODO:search for first free comment in the list */
	};
	return NULL;
}
