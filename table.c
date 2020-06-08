/*
 * 26.09. 2019
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "table.h"


#define VSHI(s, r, c) ((r * s->cols) + c)

int cmt_sort_compare(const void *, const void *);




// -------------------




/*
struct sheet {
	unsigned bottom;
	unsigned ind;
	int * val;
};
*/

const unsigned sheet_size = sizeof(int) * 2048;


int
sheet_init(struct sheet *p)
{
	p->bottom = 0;
	p->ind = 0;
	p->val = malloc(sheet_size);
	return -(p->val == NULL);
}


void
sheet_free(struct sheet *p)
{
	free(p->val);
	p->bottom = 0;
	p->val = NULL;
}


int *
sheet_get(struct sheet *p, unsigned i)
{
	if (i >= p->bottom || p->val == NULL)
		return NULL;
	p->ind = i;
	return p->val + i;
}


int *
sheet_next(struct sheet *p)
{
	if (p->ind + 1 >= p->bottom || p->val == NULL)
		return NULL;
	p->ind += 1;
	return p->val + p->ind;
}




// -------------------




int
_comment_init(struct _comment *c)
{
	c->size = 16;
	c->i = 0;
	c->ind = malloc(sizeof(c->ind[0]) * c->size);
	c->str = malloc(sizeof(c->str[0]) * c->size);
	if (c->ind == NULL || c->str == NULL)
		return -1;
	memset(c->ind, 0xff, sizeof(c->ind[0]) * c->size);
	memset(c->str, 0,    sizeof(c->ind[0]) * c->size);
	return 0;
}


void
_comment_free(struct _comment *c)
{
	c->size = 0;
	c->i = 0;
	free(c->ind);  c->ind = NULL;
	free(c->str);  c->str = NULL;
	return ;
}


static int
_comment_realloc(struct _comment *c, int fact)
{
	void *ind, *str;
	if (fact < 0) {
		if (c->ind[c->size / 2 -1] != -1) return 1;
		if (c->size <= 16) return 1;
		c->size /= 2;
		ind = realloc(c->ind, sizeof(c->ind[0]) * c->size);
		str = realloc(c->str, sizeof(c->str[0]) * c->size);
		if (ind == NULL || str == NULL) return -1;
		c->ind = ind;
		c->str = str;
		return 0;
	} else if (fact > 0) {
		ind = realloc(c->ind, sizeof(c->ind[0]) * c->size * 2);
		str = realloc(c->str, sizeof(c->str[0]) * c->size * 2);
		if (ind == NULL || str == NULL) return -1;
		c->ind = ind;
		c->str = str;
		memset(c->ind + c->size, 0xff, sizeof(c->ind[0]) * c->size);
		memset(c->str + c->size, 0,    sizeof(c->ind[0]) * c->size);
		c->size *= 2;
		return 0;
	} else {
		return 1;
	};
}


static int
_comment_ind_cmp(const int *a, const int *b ) {
	return (*b < 0) ? -!(*a < 0) : *a - *b;
}


static unsigned
_comment_search(struct _comment *c, unsigned ind)
{
	void *p;
	if (c->ind[0] == -1) return 0;
	p = bsearch(&ind, c->ind, c->size, sizeof(c->ind[0]),
	    (int (*)(const void *, const void *)) &_comment_ind_cmp);
	return (unsigned) (p - (size_t) c->ind);
}


char *
_comment_set(struct _comment *c, unsigned ind, char *str)
{
	unsigned i;
	i = _comment_search(c, ind);
	if (c->ind[i] != ind)
		return (c->str[c->ind[i] = ind] = str);
	if (c->ind[c->size -1] != -1)
		if (_comment_realloc(c, +1) < 0) return NULL;
	memmove(c->ind +i+1, c->ind +i, sizeof(c->ind[0]) * (c->size -1));
	memmove(c->str +i+1, c->str +i, sizeof(c->str[0]) * (c->size -1));
	c->ind[i] = ind;
	c->str[i] = str;
	return c->str[i];
}


char *
_comment_remove(struct _comment *c, unsigned ind)
{
	unsigned i;
	char *str;
	i = _comment_search(c, ind);
	if (c->ind[i] != ind) return NULL;
	str = c->str[i];
	if (_comment_realloc(c, -1) < 0) return NULL;
	memmove(c->ind +i, c->ind +i+1, sizeof(c->ind[0]) * (c->size -1));
	memmove(c->str +i, c->str +i+1, sizeof(c->str[0]) * (c->size -1));
	c->str[c->size -1] = NULL;
	c->ind[c->size -1] = -1;
	return str;
}


char *
_comment_get(struct _comment *c, unsigned ind)
{
	char *str;
	if (ind > c->size || c->ind[ind] < 0)
		return NULL;
	c->i = _comment_search(c, ind);
	if (c->ind[c->i] != ind) {
		c->i = 0;
		return NULL;
	};
	return c->str[c->i];
}


char *
_comment_next(struct _comment *c)
{
	char *str;
	if (c->i >= c->size) return NULL;
	str = c->str[c->i];
	c->i += (c->ind[c->i] >= 0);
	return str;
}




// -------------------  old  -------------------




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




// ------------------




// NOTE: not used
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
cmt_sort_compare(const void *a_, const void *b_)
{
	const struct comment *a = a_;
	const struct comment *b = b_;
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
