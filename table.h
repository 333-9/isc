struct sheet {
	size_t height;
#	define Ncol 0x100
	int    *num[Ncol];
	char * *str[Ncol];
};
typedef struct sheet * Sheet;
typedef unsigned char Col;
typedef size_t Row;


typedef short int box_sz;

struct var_sheet {
	struct var_sheet *next;
	size_t cols;
	size_t rows;
	box_sz vals[];
};




struct sheet *sheet_init(size_t);
void sheet_free(Sheet);
int sheet_add_rows(Sheet, size_t);
static int sheet_add_num_col(Sheet, Col);
static int sheet_add_str_col(Sheet, Col);
int sheet_add_num(Sheet, Col, Row, int);
int sheet_add_str(Sheet, Col, Row, const char *);
int sheet_set_str(Sheet, Col, Row, char *);
int   sheet_get_num(Sheet, Col, Row);
char *sheet_get_str(Sheet, Col, Row);


struct var_sheet * vsheet_init(size_t);
void               vsheet_free(struct var_sheet *);
struct var_sheet * vsheet_set_box(struct var_sheet *, size_t, size_t, box_sz);
struct var_sheet * vsheet_add_rows(struct var_sheet *, size_t);
