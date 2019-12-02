/*
 * storage classes for intiger and string data
 */

/*
 * functions returning intigers as atatus will return 0 on success,
 * >0 on warning and -1 on error
 */


typedef int Box_int;

struct vsheet {
	struct vsheet *next;
	size_t cols;
	size_t rows;
	Box_int vals[];
};


struct vsheet  *vsheet_init(size_t);
void  vsheet_free(struct vsheet *);
int   vsheet_set_box(struct vsheet **, size_t, size_t, Box_int);
int   vsheet_add_rows(struct vsheet **, size_t);
Box_int  *vsheet_get_num(struct vsheet *, size_t, size_t);
int  vsheet_get_row_width(struct vsheet *, size_t);




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

/* EOF */
