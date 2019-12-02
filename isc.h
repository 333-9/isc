/*
 * isc.h
 */


char  *parser_input_str;
int  row_changed_first;
int  row_changed_last;

int rows;

int mark_col;
int mark_row;
int sel_col;
int sel_row;
size_t  scroll_offset;

struct vsheet  *sheet;
struct cmt_list   *text ;




#define die(s)  die_line_num(__LINE__, s)

void  die_line_num(int, const char *);

/*
static void  move_selection(int, int);
static void  scroll_up(int);
static void  scroll_down(int);
static void  box_set_num(void);
static void  box_add_num(void);
static void  box_set_text(void);
static void  parse_text(void);
static void  update_cursor_text(int);
static void  update_text(void);
static void  update_nums(int, int);
*/
