#ifndef TABLE_H
#define TABLE_H

struct csheet {
	size_t height;
#	define Ncol 0x100
	int    *num[Ncol];
	char * *str[Ncol];
};
typedef struct csheet * Sheet;
typedef unsigned char Col;
typedef size_t Row;




Sheet  csheet_init(size_t);
void  csheet_free(Sheet);
int  csheet_add_rows(Sheet, size_t);
static int  csheet_add_num_col(Sheet, Col);
static int  csheet_add_str_col(Sheet, Col);
int  csheet_add_num(Sheet, Col, Row, int);
int  csheet_add_str(Sheet, Col, Row, const char *);
int  csheet_set_str(Sheet, Col, Row, char *);
int  csheet_get_num(Sheet, Col, Row);
char  *csheet_get_str(Sheet, Col, Row);




typedef int box_sz;

struct var_sheet {
	struct var_sheet *next;
	size_t cols;
	size_t rows;
	box_sz vals[];
};

struct var_sheet  *vsheet_init(size_t);
void  vsheet_free(struct var_sheet *);
struct var_sheet  *vsheet_set_box(struct var_sheet *, size_t, size_t, box_sz);
struct var_sheet  *vsheet_add_rows(struct var_sheet *, size_t);
box_sz  *vsheet_get_num(struct var_sheet *, size_t, size_t);




struct comment {
	size_t row;
	char   col;
	char *s;
};

struct comment  *comment_setup(char *, size_t, char, char, char *);


struct cmt_list {
#	define CMT_LIST_CHUNK 16
	size_t sz;
	struct comment list[];
};

struct cmt_list *cmt_list_init(void);
//int cmt_list_add(struct cmt_list **, size_t, char, char *); /* depricatesd */

struct comment *cmt_list_get(struct cmt_list **cl, int r, int c);
struct comment *cmt_list_new(struct cmt_list **cl, int r, int c);


#endif /* def TABLE_H */

/* EOF */
