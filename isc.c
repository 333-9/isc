/*
 * wur@guardian
 * date:29.09. 2019
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "draw.h"
#include "table.h"
#include "terminal.h"
#include "y.tab.h"

#include "config.h"




#define \
sel_get_num() (sheet->vals + ((scroll_offset + (sel1.row)) * sheet->cols) + sel1.col)
#define \
die(s)  die_line_num(__LINE__, s)




extern char  *parser_input_str;

extern int  row_changed_first;
extern int  row_changed_last;




struct position {
	int col;
	int row;
};
struct position  sel1; /* main cursor */
struct position  sel2;

struct {
	size_t height;
	size_t width;
} terminal_status; /* future functionality, for now everything depends on rows */


struct var_sheet  *sheet = NULL;
struct cmt_list   *s_text = NULL;

size_t  scroll_offset = 0;




static int  setup(void);

static int   run(void);
static void  move_selection(int, int);
static void  scroll_up(int);
static void  scroll_down(int);
static void  call_parser(void);
static void  parse_text(void);
static void  set_box_text(void);
static void  update_cursor_text(int);
static void  update_text(void);
static void  update_nums(int, int);

static void  restore(void);
static void  die_line_num(int, const char *);




void
yyerror(int *not_used, char *s) {
	static char *prev = NULL;
	if (s == NULL && prev != NULL) {
		draw_box_str(Black, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
		draw_box_str(Red,   7, rows + 2, 1, "Parser:");
		draw_box_str(Red, 200, rows + 2, 9, prev); /* draw error message */
		prev = NULL;
	} else {
		prev = s;
	};
}




static int
setup(void)
{
	/* --- var init --- */
	parser_input_str = NULL;
	if ((sheet = vsheet_init(columns))          == NULL) die("falied to allocate");
	if ((sheet = vsheet_add_rows(sheet, rows))  == NULL) die("failed to realloc");
	//if ((update_rows = calloc(1, rows))         == NULL) die("failed to allocate");
	if ((s_text = cmt_list_init())              == NULL) die("falied to allocate");
	/* --- term init --- */
	terminal_init(); /* set input mode (call termios) */
	terminal_buffer_enable();
	/* static teerminal status (just a placeholder): */
	terminal_status.height = rows + 3;
	terminal_status.width  = columns;
	/* docorations: */
	draw_column_numbering(Red, 1, 4, 'a', 'a' + columns - 1);
	draw_row_numbering(Red, 2, 1, 0, rows - 1);
	//draw_box_num(Grey, 5, 1, 1, sheet->rows); /* draw sheet size */
	sel1 = (struct position){ .col = 0, .row = 0 }; /* default position */
	move_selection(0, 0); /* draw cursor */
}




static int
run(void)
{
	char c;
	while (1) {
		while (read(0, &c, 1) <= 0) {
			update_text();
			update_nums(row_changed_first, row_changed_last);
			row_changed_first = 0;
			row_changed_last  = 0;
		};
		switch (c) {
		case 'h': move_selection( 0, -1);  break;
		case 'l': move_selection( 0,  1);  break;
		case 'k': move_selection(-1,  0);  break;
		case 'j': move_selection( 1,  0);  break;
		case '=': call_parser(); break;
		case '>': set_box_text(); break;
		case '\n': parse_text();  break;
		case 'd':
			sheet = vsheet_set_box(sheet, scroll_offset + sel1.row, sel1.col, 0);
			move_selection(0, 0);
			break;
		case 'q': return 0;
		default:  putc('\a', stderr); break;
		};
	};
}


/* (0,0) is valid and used to update the selected box value */
static void
move_selection(int r, int c)
{
#	define S_COL(c) ((6 * (c.col + 1)) + 1)
#	define S_ROW(r) (r.row + 2)
	int n = *sel_get_num();
	/* remove prev cursor: */
	draw_box_num(Red, 5, S_ROW(sel1), 1, scroll_offset + sel1.row);
	draw_box_str(Black, 1, S_ROW(sel1), S_COL(sel1) - 1,     " ");
	draw_box_str(Black, 1, S_ROW(sel1), S_COL(1 + sel1) - 1, " ");
	/* draw prev cursor box: */
	if (!n) draw_box_str(Black, 5, S_ROW(sel1), S_COL(sel1), "     ");
	else    draw_box_num(Grey,  5, S_ROW(sel1), S_COL(sel1), n);
	draw_box_str(Black, 9,  rows + 2, 1, "\x1b[K"); /* clear comand line area */
	update_cursor_text(0); /* draw prev text */
	/* move cursor: */
	if        (c < 0 && (sel1.col + c) >= 0) { sel1.col += c;
	} else if (c > 0 && (sel1.col + c) < (columns)) { sel1.col += c;
	} else if (r < 0) { scroll_up(r); /* rows */
	} else if (r > 0) { scroll_down(r);
	};
	/* draw cursor: */
	draw_box_str(Aqua, 1, S_ROW(sel1), 1, ">");
	draw_box_str(Aqua, 1, S_ROW(sel1), S_COL(sel1) - 1,     "[");
	draw_box_str(Aqua, 1, S_ROW(sel1), S_COL(1 + sel1) - 1, "]");
	update_cursor_text(1); /* draw text */
	yyerror(NULL, NULL); /* draw error message */
#	undef S_COL
#	undef S_ROW
}


static void
scroll_up(int r)
{
	if ((sel1.row + r) >= 0) return (void)(sel1.row += r);
	if (scroll_offset <= 0) return ;
	// TODO: make it variable (dependent on 'r')
	fputs("\x1b[T", stderr); /* scroll Down */
	draw_box_str(Black, 9, rows + 2, 1, "\x1b[K"); /* clear command line area */
	draw_column_numbering(Red, 1, 4, 'a', 'a' + (columns - 1)); /* redraw top row */
	//draw_box_num(Grey, 5, 1, 1, sheet->rows); /* draw sheet size */
	draw_box_str(Black, 9, 2, 6, "\x1b[K"); /* clear prev row numbering */
	scroll_offset -= 1;
	draw_box_num(Red, 5, 2, 1, scroll_offset); /* draw new row numbering */
	draw_table_num(Grey, 5, 2, 7, 1,
	    1, columns, sheet->vals + (scroll_offset * sheet->cols)); /* draw new row */
}


static void
scroll_down(int r)
{
	if ((sel1.row + r) < (rows)) return (void)(sel1.row += r);
	// TODO: make it variable (dependent on 'r')
	fputs("\x1b[S", stderr); /* scroll Up */
	if ((sheet->rows - 1) <= (sel1.row + scroll_offset))
		sheet = vsheet_add_rows(sheet, 1); /* realloc sheet */
	draw_column_numbering(Red, 1, 4, 'a', 'a' + (columns - 1)); /* redraw top row */
	draw_box_str(Red, 6, 1, 1, "      "); /* clear top left corner */
	//draw_box_num(Grey, 5, 1, 1, sheet->rows); /* draw sheet size */
	draw_table_num(Grey, 5, rows + 1, 7, 1,
	    1, columns, sheet->vals + ((rows + scroll_offset) * sheet->cols));
	    /* draw new row */
	draw_box_num(Red, 5, 21, 1, scroll_offset + 20); /* redraw new row numbering */
	scroll_offset += 1;
}


static void
call_parser(void)
{
	int i;
	draw_box_str(Aqua, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	if ((parser_input_str = command_line_input("    = ", 80, NULL)) != NULL) {
		if (yyparse(&i) == 0) { /* call parser */
			*sel_get_num() = i;
		};
		free(parser_input_str);
		parser_input_str = NULL;
	};
	draw_box_str(Black, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	move_selection(0, 0);
}


static void
parse_text(void)
{
	int i;
	struct comment *cmt;
	cmt = cmt_list_get(&s_text, sel1.row + scroll_offset, sel1.col);
	if (cmt == NULL || cmt->s == NULL) return ;
	parser_input_str = cmt->s;
	if (yyparse(&i) == 0) { /* call parser */
		*sel_get_num() = i;
	};
	move_selection(0, 0);
}


static void
set_box_text(void)
{
	struct comment *cmt;
	draw_box_str(Aqua, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	if ((parser_input_str = command_line_input("    > ", 80, NULL)) != NULL) {
		cmt = cmt_list_new(&s_text, sel1.row + scroll_offset, sel1.col);
		if (cmt == NULL) {
			free(parser_input_str);
			parser_input_str = NULL;
		} else {
			cmt->row = sel1.row + scroll_offset;
			cmt->col = sel1.col;
			if (cmt->s != NULL) free(cmt->s); /* TODO: line editing */
			cmt->s = parser_input_str;
		};
	};
	draw_box_str(Black, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	move_selection(0, 0);
}


static inline void
update_cursor_text(int update_command_line)
{	struct comment *cmt;
	cmt = cmt_list_get(&s_text, sel1.row + scroll_offset, sel1.col);
	if (cmt != NULL && cmt->s != NULL) {
		draw_box_str(Green, 5, sel1.row + 2, ((sel1.col + 1) * 6) + 1,
		    cmt->s);
		if (update_command_line) {
			draw_box_str(Black, 9,  rows + 2, 1, "\x1b[K"); /* clear comand line area */
			draw_box_str(Green, 80, rows + 2, 1, cmt->s); /* draw it into command line */
		};
	};
}


static void
update_text(void)
{
	int i;
	for (i = 0; i < s_text->sz; i++) {
		if  (s_text->list[i].s != NULL &&
		    (s_text->list[i].row) <= (scroll_offset + rows - 1) &&
		    (s_text->list[i].row) >= scroll_offset) {
			draw_box_str(Green, 5,
			    s_text->list[i].row + 2 - scroll_offset,
			    ((s_text->list[i].col + 1) * 6) + 1,
			    s_text->list[i].s);
		};
	};
}


static void
update_nums(int a, int b)
{
	if (a == b) return ;
	else if (a > scroll_offset + rows) return ;
	else if (b < scroll_offset) return ;
	else if (b > scroll_offset + rows)
		 b = scroll_offset + rows;
	draw_table_num(Grey, 5, (a - scroll_offset) + 2, 7, 1,
	    b - a + 1, columns, vsheet_get_num(sheet, a, 0));
}




static void
restore(void)
{
	terminal_restore();
	terminal_buffer_disable();
	vsheet_free(sheet);
}


/* used in die() macro */
static void
die_line_num(int n, const char *s)
{
	terminal_restore();
	terminal_buffer_disable();
	fprintf(stderr, "line %i: %s", n, s);
	exit(1);
}




int
main()
{
	setup();
	run();
	restore();
	return 0;
}
