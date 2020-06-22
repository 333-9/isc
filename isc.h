/* isc.h */


struct data  data;
struct text  text;
int no_data = 0;



jmp_buf         on_parser_error;
static jmp_buf  on_error;
const char *    err_msg = "";


const int  columns;
int  rows;
int  scroll_offset;
int  sel_col;
int  sel_row;



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
