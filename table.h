/*
 * storage classes for intiger and string data
 */

/* <0 error
 * =0 success
 * >0 warning
 */


typedef int Box_int;

struct vsheet {
	struct vsheet *next;
	size_t rows;
	size_t cols;
	size_t last_nonzero_row;
	size_t last_ind; /* last row given by *_get_num() */
	Box_int vals[];
};


struct vsheet  *vsheet_init(size_t);
void  vsheet_free(struct vsheet *);
void  vsheet_update(struct vsheet *);
Box_int  *vsheet_get_num(struct vsheet *, size_t, size_t);
int   vsheet_set_box(struct vsheet **, size_t, size_t, Box_int);
int   vsheet_add_rows(struct vsheet **, size_t);
int   vsheet_get_row_width(struct vsheet *, size_t);




#define CMT_LIST_CHUNK 16

struct comment {
	size_t row;
	size_t col;
	char *s;
};

struct comment  *comment_setup(char *, size_t, char, char, char *);


struct cmt_list {
	size_t sz;
	int i;
	struct comment  list[];
};

struct cmt_list *cmt_list_init(void);
struct comment  *cmt_list_new(struct cmt_list **, int, int);
size_t  cmt_list_get_from(struct cmt_list **, int, int);
char   *cmt_list_str(struct cmt_list **, size_t, size_t);
int     cmt_list_update(struct cmt_list **);


// --------------------------------------

/* 26.09. 2019 */


struct sheet {
	unsigned bottom;
	unsigned ind;
	int * val;
};

int   sheet_init (struct sheet *p);
void  sheet_free (struct sheet *p);
int * sheet_get  (struct sheet *p, unsigned i);
int * sheet_next (struct sheet *p);




struct _comment {
	unsigned size;
	unsigned i;
	int   *ind;  // SOA
	char **str;  // ^^^
};

int    _comment_init   (struct _comment *);
void   _comment_free   (struct _comment *);
char * _comment_set    (struct _comment *, unsigned, char *);
char * _comment_remove (struct _comment *, unsigned);
char * _comment_get    (struct _comment *, unsigned);
char * _comment_next   (struct _comment *);


/* EOF */
