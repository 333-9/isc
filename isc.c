/*
 * wur@guardian
 * date:29.09. 2019
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "draw.h"
#include "table.h"
#include "terminal.h"
#include "y.tab.h"

#include "config.h"




#define \
die(s)  die_line_num(__LINE__, s)





char  *parser_input_str;
int  row_changed_first;
int  row_changed_last;


int rows = 20;
char *default_file = "out.csv";




int mark_col = 0;
int mark_row = 0;
int sel_col = 0;
int sel_row = 0;
size_t  scroll_offset = 0;

struct {
	size_t height;
	size_t width;
} terminal_status; /* future functionality, for now everything depends on rows */

struct vsheet  *sheet = NULL;
struct cmt_list   *text  = NULL;




static int  setup(char *[]);
static int  get_term_size(void);
static void  csv_read(char *);
static void  csv_write(const char *);
static int  csv_get_row_width(int);
static int  csv_get_last_row(void);
static int  csv_parse(char *);
static char  *csv_parse_str(char *);

static int   run(void);
static void  move_selection(int, int);
static void  scroll_up(int);
static void  scroll_down(int);
static void  box_set_num(void);
static void  box_add_num(void);
static void  box_set_text(void);
static void  parse_text(void);
static void  update_cursor_text(int);
static void  update_text(int);
static void  update_nums(int, int);
static Box_int  *sel_get_num(void);

static void  restore(void);
void  die_line_num(int, const char *);




extern yylex(void);

void
yyerror(int *not_used, char *s)
{
	static char *prev = NULL;
	if (s == NULL && prev != NULL) {
		draw_box_str(C_error,  9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
		draw_box_str(C_error,  7, rows + 2, 1, "Parser:");
		draw_box_str(C_error,200, rows + 2, 9, prev); /* draw error message */
		prev = NULL;
		parser_input_str = NULL;
		while (yylex()) ; /* clear lex input */
	} else {
		prev = s;
	};
}




static int
setup(char *argv[])
{
	/* --- var init --- */
	parser_input_str = NULL;
	rows = get_term_size() - 2;
	if ((sheet = vsheet_init(columns)) == NULL) die("falied to allocate");
	if (vsheet_add_rows(&sheet, rows) < 0) die("failed to realloc");
	if ((text = cmt_list_init()) == NULL) die("falied to allocate");
	if (argv[1] != NULL) { /* csv_read expects allocated sheet */
		csv_read(argv[1]);
	};
	/* --- term init --- */
	terminal_init();
	terminal_buffer_enable();
	/* - draw - */
	draw_column_numbering(C_ind, 1, 4, 'a', 'a' + columns - 1);
	draw_row_numbering(C_ind, 2, 1, 0, rows - 1);
	draw_table_num(C_number, 5, 2, 7, 1, sheet->rows, columns, sheet->vals);
	move_selection(0, 0); /* draw cursor */
}


static int
get_term_size(void)
{
	struct winsize sz;
	ioctl(0, TIOCGWINSZ, &sz);
	return sz.ws_row;
}


static void
csv_read(char *file_name)
{
	FILE *stream;
	char *mem = NULL;
	size_t i = 0;
	if ((stream = fopen(file_name, "r")) == NULL) die("failed to open file");
	default_file = file_name;
	csv_parse(NULL);
	while (getline(&mem, &i, stream) > 0) {
		csv_parse(mem);
		//putc('\n', stderr);
	};
	fclose(stream);
	free(mem);
}


static void
csv_write(const char *file_name)
{
	int val;
	char *str;
	int r, c, width, row_max;
	FILE *stream;
	if ((stream = fopen(file_name, "w")) == NULL) die("failed to open file");
	cmt_list_update(&text);
	vsheet_update(sheet);
	row_max = csv_get_last_row();
	for (r = 0; r < row_max; r++) {
		width = csv_get_row_width(r);
		for (c = 0 ;;) {
			str = cmt_list_str(&text, r, c);
			val = sheet->vals[(r * sheet->cols) + c];
			if (val) {
				if (str != NULL && *str != '\0')
					fprintf(stream, "%lli\"%s\"", val, str);
				else fprintf(stream, "%lli", val);
			} else if (str != NULL && *str != '\0') {
				fprintf(stream, "\"%s\"", str);
			};
			//if (++c >= sheet->cols) break;
			if (++c > width) break;
			fputc(',', stream);
		};
		putc('\n', stream);
	};
	fclose(stream);
}


static int
csv_get_row_width(int r)
{
	size_t i;
	size_t wt, wv;
	i = cmt_list_get_from(&text, r, 0);
	for (; i < text->sz && text->list[i].row == r; i++) 
		if (text->list[i].s == NULL) break;
	if (i != 0) wt = text->list[--i].col;
	else wt = 0;
	wv = vsheet_get_row_width(sheet, r);
	if (wt > wv) return wt;
	else return wv;
}


static int
csv_get_last_row()
{
	int r;
	int i;
	r = sheet->last_nonzero_row;
	i = cmt_list_get_from(&text, r, 0);
	if (text->list[i].row > r) {
		for (; i < text->sz; i++) {
			if (text->list[i].s == NULL) return r + 1;
			r = text->list[i].row;
		};
	};
	return r;
}


static int
csv_parse(char *str)
{
	struct comment  *cmt;
	long long int  value;
	char  *tmp;
	static int  row = 0;
	static int  col = 0;
	if (str == NULL) {
		row = col = 0;
		return 0;
	};
	if (*str == '#') return 0;
	for (;;) {
		value = strtoll(str, &tmp, 0);
		if (vsheet_set_box(&sheet, row, col, value) < 0)
			die("failed to realloc");
		//fprintf(stderr, " %lli ", value);
		str = tmp;
csv_parse_switch:
		switch (*str) {
		case '#':
		case '\n':
		case '\0':
			row += 1;
			col = 0;
			return 0;
		case ',':
			col += 1;
			str++;
			break;
		case '\'':
		case '"':
		case '`':
			tmp = csv_parse_str(str);
			cmt = cmt_list_new(&text, row, col);
			if (cmt == NULL) return 1;
			cmt->s = strdup(++str);
			if (tmp == NULL) return 0;
			else str = ++tmp;
			goto csv_parse_switch;
			break;
		case '>': /* FALLTHROW */
			str++;
		default:
			if ((tmp = strchr(str, ',')) == NULL) {
				str[strlen(str) - 1] = '\0'; /* should never segfault */
				cmt = cmt_list_new(&text, row, col);
				if (cmt == NULL) return 1;
				cmt->s = strdup(str);
				row += 1;
				col = 0;
				return 0;
			} else {
				*tmp = '\0';
				cmt = cmt_list_new(&text, row, col);
				if (cmt == NULL) return 1;
				cmt->s = strdup(str);
				col += 1;
				str = tmp + 1;
			};
		};
	};
}


static char *
csv_parse_str(char *str)
{
	char quot;
	quot = *str;
	str++;
	for (; *str; str++) {
		if (*str == '\\') {
			str++;
			if (str == '\0') return NULL;
		} else if (*str == quot || *str == '\n') {
			*str = '\0';
			return str;
		};
	};
	return NULL;
}




static int
run(void)
{
	char c;
	int count = 0;
	while (1) {
		update_nums(row_changed_first, row_changed_last);
		update_text(0);
		row_changed_first = 0;
		row_changed_last  = 0;
		while (read(0, &c, 1) <= 0) {
			rows = get_term_size() - 2;
			if (rows + scroll_offset > sheet->rows  &&
			    vsheet_add_rows(&sheet, (rows + scroll_offset) - sheet->rows) < 0)
				die("failed to realloc");
		};
		switch (c) {
		case '0': /* FALLTHROW */
			if (count == 0) {
				count = -1;
				break;
			};
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (count < 0) {
				count = c - '0';
				break;
			} else if (count >= 1000000) {
				break;
			} else {
				count *= 10;
				count += c - '0';
				break;
			};
		case 'h': move_selection( 0, -(count ? count : 1)); count = 0; break;
		case 'l': move_selection( 0,  (count ? count : 1)); count = 0; break;
		case 'j': move_selection( (count ? count : 1),  0); count = 0; break;
		case 'k': move_selection(-(count ? count : 1),  0); count = 0; break;
		case '>': box_set_text(); break;
		case '=': box_set_num();  break;
		case '+': box_add_num();  break;
		case '\n':
			if (count != 0) {
				*sel_get_num() = count == -1 ? 0 : count;
				count = 0;
				move_selection(1, 0);
				break;
			} else {
				parse_text();
				break;
			};
		case 'w': csv_write(default_file); break;
		case 'q': return 0;
		default:
			putc('\a', stderr);
			count = 0;
			break;
		};
	};
}


/* (0,0) is valid and used to update the selected box */
static void
move_selection(int r, int c)
{
#	define SCOL(a) ((6 * (a + 1)) + 1)
#	define SROW(a) (a + 2)
	int n;
	/* remove prev cursor: */
	{
		n = *sel_get_num();
		draw_box_num(C_ind, 5, SROW(sel_row), 1, scroll_offset + sel_row);
		draw_column_numbering(C_ind, 1, SCOL(sel_col) - 3, 'a' + sel_col, 0);
		draw_box_str(Black, 1, SROW(sel_row), SCOL(sel_col) - 1,     " ");
		draw_box_str(Black, 1, SROW(sel_row), SCOL(1 + sel_col) - 1, " ");
		/* draw prev cursor box: */
		if (!n) draw_box_str(C_number, 5, SROW(sel_row), SCOL(sel_col), "     ");
		else    draw_box_num(C_number,  5, SROW(sel_row), SCOL(sel_col), n);
		update_cursor_text(0); /* draw prev text */
	}
	draw_box_str(Black, 9,  rows + 2, 1, "\x1b[K"); /* clear comand line area */
	/* move cursor: */
	if        (c < 0) { sel_col = ((sel_col + c) >= 0) ? sel_col + c : 0;
	} else if (c > 0) { sel_col = ((sel_col + c) < (columns)) ? sel_col + c : columns - 1;
	} else if (r < 0) { scroll_up(-r);
	} else if (r > 0) { scroll_down(r);
	};
	/* draw cursor: */
	{
		draw_box_num(C_cursor, 5, SROW(sel_row), 1, scroll_offset + sel_row);
		draw_column_numbering(C_cursor, 1, SCOL(sel_col) - 3, 'a' + sel_col, 0);
		draw_box_str(C_cursor, 1, SROW(sel_row), 1, ">");
		draw_box_str(C_cursor, 1, SROW(sel_row), SCOL(sel_col) - 1,     "[");
		draw_box_str(C_cursor, 1, SROW(sel_row), SCOL(1 + sel_col) - 1, "]");
		update_cursor_text(1); /* draw text */
	}
	yyerror(NULL, NULL); /* draw error message */
#	undef S_COL
#	undef S_ROW
}


static void
scroll_up(int r)
{
	sel_row -= r;
	if (sel_row >= 0) {
		return ;
	};
	r = -sel_row;
	sel_row = 0;
	for (; scroll_offset && r; r--) {
		fputs("\x1b[T", stderr); /* scroll Down */
		draw_box_str(Black, 9, rows + 2, 1, "\x1b[K"); /* clear command line area */
		draw_column_numbering(C_ind, 1, 4, 'a', 'a' + (columns - 1)); /* redraw top row */
		//draw_box_num(Grey, 5, 1, 1, sheet->rows); /* draw sheet size */
		draw_box_str(Black, 9, 2, 6, "\x1b[K"); /* clear prev row numbering */
		scroll_offset -= 1;
		draw_box_num(C_ind, 5, 2, 1, scroll_offset); /* draw new row numbering */
		draw_table_num(C_number, 5, 2, 7, 1,
		    1, columns, vsheet_get_num(sheet, scroll_offset, 0)); /* draw new row */
		update_text(1); /* draw text */
	};
}


static void
scroll_down(int r)
{
	if ((sel_row + r) < (rows)) {
		sel_row += r;
		return ;
	} else {
		r -= rows - sel_row;
		r += 1;
		sel_row = rows - 1;
	};
	for (; r > 0 && (scroll_offset + sel_row) < 99999; r--) {
		fputs("\x1b[S", stderr); /* scroll Up */
		if ((sheet->rows - 1) <= (sel_row + scroll_offset)) {
			if (vsheet_add_rows(&sheet, 1) < 0) /* realloc sheet */
				die("Failed to realloc");
		};
		draw_column_numbering(C_ind, 1, 4, 'a', 'a' + (columns - 1)); /* redraw top row */
		draw_box_str(C_ind, 6, 1, 1, "      "); /* clear top left corner */
		//draw_box_num(Grey, 5, 1, 1, sheet->rows); /* draw sheet size */
		draw_table_num(C_number, 5, rows + 1, 7, 1,
		    1, columns, vsheet_get_num(sheet, scroll_offset + rows, 0)); /* draw new row */
		draw_box_num(C_ind, 5, rows + 1, 1, scroll_offset + rows); /* redraw row numbering */
		scroll_offset += 1;
		update_text(-1); /* draw text */
	};
}


static void
box_set_num(void)
{
	int i;
	draw_box_str(C_command, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	if ((parser_input_str = command_line_input("    = ", 80, NULL)) != NULL) {
		i = *sel_get_num();
		if (yyparse(&i) == 0) { /* call parser */
			*sel_get_num() = i;
		};
		free(parser_input_str);
		parser_input_str = NULL;
	};
	draw_box_str(Black, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	move_selection(1, 0);
}


static void
box_add_num(void)
{
	int i;
	draw_box_str(C_command, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	if ((parser_input_str = command_line_input("    + ", 80, NULL)) != NULL) {
		i = *sel_get_num();
		if (yyparse(&i) == 0) { /* call parser */
			*sel_get_num() += i;
		};
		free(parser_input_str);
		parser_input_str = NULL;
	};
	draw_box_str(Black, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	move_selection(0, 0);
}


static void
box_set_text(void)
{
	struct comment *cmt;
	draw_box_str(C_command, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	if ((parser_input_str = command_line_input("    > ", 80, NULL)) != NULL) {
		cmt = cmt_list_new(&text, sel_row + scroll_offset, sel_col);
		if (cmt == NULL) {
			free(parser_input_str);
			parser_input_str = NULL;
			/* TODO: save the file and exit */
			die("cmt_list error");
		} else if (*parser_input_str == '\0') {
			free(parser_input_str);
			parser_input_str = NULL;
			cmt->row = 0;
			cmt->col = 0;
			if (cmt->s != NULL) free(cmt->s);
			cmt->s = NULL;
		} else {
			cmt->row = sel_row + scroll_offset;
			cmt->col = sel_col;
			if (cmt->s != NULL) free(cmt->s);
			cmt->s = parser_input_str;
		};
	};
	draw_box_str(Black, 9, rows + 2, 1, "\x1b[K"); /* clear comand line area */
	move_selection(0, 0);
}


static void
parse_text(void)
{
	int i;
	parser_input_str = cmt_list_str(&text, sel_row + scroll_offset, sel_col);
	if (parser_input_str == NULL) return ;
	i = *sel_get_num();
	if (yyparse(&i) == 0) {
		*sel_get_num() = i;
	};
	move_selection(0, 0);
}


static void
update_cursor_text(int update_command_line)
{
	char *str;
	str = cmt_list_str(&text, sel_row + scroll_offset, sel_col);
	if (str != NULL) {
		draw_box_str(C_text, 5, sel_row + 2, ((sel_col + 1) * 6) + 1, str);
		if (update_command_line) {
			draw_box_str(Black, 9,  rows + 2, 1, "\x1b[K"); /* clear comand line area */
			fprintf(stderr, "\x1b[38;5;%um %s  \x1b[38;5;%um[ %i ]",
			    C_text, str, C_number,
			    *vsheet_get_num(sheet, sel_row + scroll_offset, sel_col));
			    /* draw command line */
		};
	} else if (update_command_line) {
		draw_box_str(C_number, 9,  rows + 2, 1, "\x1b[K"); /* clear comand line area */
		fprintf(stderr, " \x1b[38;5;%um[ %i ]", C_number,
		    *vsheet_get_num(sheet, sel_row + scroll_offset, sel_col));
		    /* draw command line */
	};
}


static void
update_text(int r)
{
	size_t i, stop;
	cmt_list_update(&text);
	if (r >= 1) {
		i = cmt_list_get_from(&text, scroll_offset, 0);
		stop = scroll_offset + 1;
	} else if (r <= -1) {
		i = cmt_list_get_from(&text, scroll_offset + rows - 1, 0);
		stop = scroll_offset + rows;
	} else {
		i = cmt_list_get_from(&text, scroll_offset, 0);
		stop = scroll_offset + rows;
	};
	for (; i < text->sz; i++) {
		if  (text->list[i].s == NULL ||
		    (text->list[i].row >= stop))
			return ;
		draw_box_str(C_text, 5,
		    text->list[i].row + 2 - scroll_offset,
		    ((text->list[i].col + 1) * 6) + 1,
		    text->list[i].s);
	};
}


static void
update_nums(int ra, int rb)
{
	if (ra == rb) return ;
	else if (ra > scroll_offset + rows) return ;
	else if (rb < scroll_offset) return ;
	else if (rb > scroll_offset + rows)
		 rb = scroll_offset + rows;
	draw_table_num(C_number, 5, (ra - scroll_offset) + 2, 7, 1,
	    rb - ra + 1, columns, vsheet_get_num(sheet, ra, 0));
}


static Box_int *
sel_get_num()
{
	Box_int *p;
	p = vsheet_get_num(sheet, scroll_offset + sel_row, sel_col);
	if (p == NULL) die("Selection out of bounds");
	return p;
}





static void
restore(void)
{
	terminal_restore();
	terminal_buffer_disable();
	vsheet_free(sheet);
}


void
die_line_num(int n, const char *s)
{
	terminal_restore();
	terminal_buffer_disable();
	fprintf(stderr, "line %i: %s\n", n, s);
	exit(1);
}




int
main(int argc, char *argv[])
{
	setup(argv);
	run();
	restore();
	return 0;
}
