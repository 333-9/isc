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

extern char *parser_input_str; /* defined in parser.y */


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

size_t  scroll_offset = 0;
char  *update_rows;




static int  setup(void);

static int  run(void);
static int  input(void);
static inline short int  *sel_get_num(void);
static void  move_selection(int, int);
static void  scroll_up(int);
static void  scroll_down(int);
static void  call_parser(void);
static void  draw_update_rows(void);
static void  draw_update_column(size_t);

static void  restore(void);
static void  die_line_num(int, const char *);
#define die(s)  die_line_num(__LINE__, s)




/* parser specific functions */

short int *
parser_get_num(size_t r, size_t c)
{
	if (r >= scroll_offset && r < (scroll_offset + rows)) {
		update_rows[r - scroll_offset] = 1;
	};
	return vsheet_get_num(sheet, r, c);
}

short int
parser_for_range(size_t r1, size_t r2, size_t c, int (*func)(int, int))
{
	short int *p, ret = 0, r;
	if (r1 > r2) {
		p = vsheet_get_num(sheet, r2, c);
		r = r2 - r1;
	} else {
		p = vsheet_get_num(sheet, r1, c);
		r = r1 - r2;
	};
	for (r += 1; r > 0; r--) {
		ret = (*func)(ret, *p);
		p += sheet->cols;
	};
	//draw_update_column(c);
	return ret;
}

short int
parser_assign_for_range(size_t r1, size_t r2, size_t c, int (*func)(int, int), int n)
{
	short int *p, r;
	if (r1 > r2) {
		p = vsheet_get_num(sheet, r2, c);
		r = r1 - r2;
	} else {
		p = vsheet_get_num(sheet, r1, c);
		r = r2 - r1;
	};
	//printf("%i, %i, %i; ",  r2, r1, r);
	for (r += 1; r > 0; r--) {
		*p = (*func)(*p, n);
		p += sheet->cols;
	};
	draw_update_column(c);
	return 0;
}

/* --- */




static int
setup(void)
{
	/* --- var init --- */
	parser_input_str = NULL;
	if ((sheet = vsheet_init(columns))          == NULL) die("falied to allocate");
	if ((sheet = vsheet_add_rows(sheet, rows)) == NULL) die("failed to realloc");
	if ((update_rows = calloc(1, rows))          == NULL) die("failed to allocate");
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
	sel1 = (struct position){ .col = 0, .row = 0 };
	move_selection(0, 0);
}




static int
run(void)
{
	while (input() >= 0) ;
}


/* returns -1 to exit loop, 0 to disable terminal buffer */
static int
input(void)
{
	char c;
	while (1) {
		while (read(0, &c, 1) <= 0) /* update everythinng */;
		switch (c) {
		case 'h': move_selection( 0, -1);  break;
		case 'l': move_selection( 0,  1);  break;
		case 'k': move_selection(-1,  0);  break;
		case 'j': move_selection( 1,  0);  break;
		case 'd':
			sheet = vsheet_set_box(sheet, scroll_offset + sel1.row, sel1.col, 0);
			move_selection(0, 0);
			break;
		case '=': call_parser(); break;
		case 's': return 0;
		case 'q': return -1;
		default:  putc('\a', stderr); break;
		};
	};
}


/* only passes a number under sel1 */
static inline short int *
sel_get_num(void) {
	return (sheet->vals + ((scroll_offset + (sel1.row)) * sheet->cols) + sel1.col);
}


/* (0,0) is valid and used to update the selected box value */
static void
move_selection(int r, int c)
{
#	define S_COL(c) ((6 * (c.col + 1)) + 1)
#	define S_ROW(r) (r.row + 2)
	int n = *sel_get_num();
	#ifdef CURSOR_NUMBERING /* cursor number highlighting */
	    char cn[2] = {0};
	    cn[0] = 'a' + sel1.col;
	    draw_box_num(Red, 5, S_ROW(sel1), 1, scroll_offset + sel1.row);
	    draw_box_str(Red, 5, 1, S_COL(sel1) + 2, cn);
	#else
	    draw_box_num(Red, 5, S_ROW(sel1), 1, scroll_offset + sel1.row);
	#endif
	draw_box_str(Black, 1, S_ROW(sel1), S_COL(sel1) - 1,     " ");
	draw_box_str(Black, 1, S_ROW(sel1), S_COL(1 + sel1) - 1, " ");
	if (!n) draw_box_str(Black, 5, S_ROW(sel1), S_COL(sel1), "     "); /* redraw number */
	else    draw_box_num(Grey,  5, S_ROW(sel1), S_COL(sel1), n);
	if        (c < 0 && (sel1.col + c) >= 0) { sel1.col += c; /* columns */
	} else if (c > 0 && (sel1.col + c) < (columns)) { sel1.col += c;
	} else if (r < 0) { scroll_up(r); /* rows */
	} else if (r > 0) { scroll_down(r);
	};
	#ifdef CURSOR_NUMBERING /* cursor number highlighting */
	    cn[0] = 'a' + sel1.col;
	    draw_box_num(Aqua,  5, S_ROW(sel1), 1, scroll_offset + sel1.row);
	    draw_box_str(Aqua, 5, 1, S_COL(sel1) + 2, cn);
	#else
	    draw_box_str(Aqua, 1, S_ROW(sel1), 1, ">");
	#endif
	draw_box_str(Aqua, 1, S_ROW(sel1), S_COL(sel1) - 1,     "[");
	draw_box_str(Aqua, 1, S_ROW(sel1), S_COL(1 + sel1) - 1, "]");
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
	draw_table_num(Grey, 5, rows, 7, 1,
	    1, columns, sheet->vals + ((rows + scroll_offset - 1) * sheet->cols));
	    /* draw new row */
	draw_box_num(Red, 5, 21, 1, scroll_offset + 20); /* redraw new row numbering */
	scroll_offset += 1;
}


static void
call_parser(void)
{
	int i;
	draw_box_str(Aqua, 2, rows + 2, 5, "= "); /* draw prompt */
	terminal_restore(); /* set terminal input to be normal */
	fputs("\x1b[?25h", stderr); /* make cursor visible */
	if (scanf("\n%m[^\n]", &parser_input_str) > 0) {
	    /* TODO: blank line is not recognized as input */
		i = yyparse(); /* call parser */
		if (i >= 0) *sel_get_num() = i; /* -1 is error */
		free(parser_input_str);
		draw_update_rows(); /* update rows with potensionally changed values */
	};
	terminal_reinit();
	fputs("\x1b[?25l", stderr); /* hide cursor */
	draw_box_str(Black, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	move_selection(0, 0);
}


static void
draw_update_rows(void)
{
	int i;
	for (i = 0; i < rows; i++) {
		if (update_rows[i] != 0) {
			update_rows[i] = 0;
			draw_table_num(Grey, 5, i + 2, 7, 1,
			    1, columns,
			    sheet->vals + ((i + scroll_offset) * sheet->cols));
		};
	};
}


void
draw_update_column(size_t c)
{
	int i, n;
	if (c > columns) return ;
	for (i = 0; i < rows; i++) {
		if ((n = *vsheet_get_num(sheet, i, c)) != 0) {
			draw_box_num(Grey, 5, i + 2, (6 * (c + 1)) + 1, n);
		};
	};
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
