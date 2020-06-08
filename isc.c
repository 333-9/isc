/* date:29.09. 2019 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <setjmp.h>
// system specific:
#include <sys/ioctl.h>
#include <err.h>
#include <histedit.h>

#include "draw.h"
#include "table.h"
#include "parser.h"
#include "terminal.h"
#include "regex.h"

#include "config.h"




#define \
die(s)  die_line_num(__LINE__, s)




struct vsheet    *sheet = NULL;
struct cmt_list  *text  = NULL;

static jmp_buf  on_error;
char  *default_file = "out.csv";
const char  *parser_input_str;
int  row_changed_first;
int  row_changed_last;
int  rows  = 20;
int  scroll_offset = 0;
int  sel_col       = 0;
int  sel_row       = 0;
//int mark_col = 0;
//int mark_row = 0;
EditLine *editline;
const char *prompt;




static int   setup(char *[]);
static void  update_screen(void);
static int   get_term_rows(void);
static const char *get_el_prompt(void *);
static void  csv_read(char *);
static void  csv_write(const char *);
static int   csv_get_row_width(int);
static int   csv_get_last_row(void);
static int   csv_parse(char *);
static char *csv_parse_str(char *);

static int   run(void);
static void  move_selection(int, int);
static void  scroll_up(int);
static void  scroll_down(int);
static void  box_set_num(void);
static void  box_set_text(void);
static void  box_search(void);
static void  parse_text(void);
static void  update_text_row(int);
static void  update_cursor_text(int);
static void  update_text(int);
static void  update_nums(int, int);
static Box_int  *sel_get_num(void);

static void  restore(void);
void  die_line_num(int, const char *);



/* used with yacc
void
yyerror(int *not_used, char *s)
{
	static char *prev = NULL;
	if (s == NULL && prev != NULL) {
		drawf(1, 1, "\033[K%eParser: %s", "0;31", prev);
		parser_input_str = prev = NULL;
		while (yylex()) ;
	} else {
		prev = s;
	};
}
*/

/* === */




static int
setup(char *argv[])
{
	parser_input_str = NULL;
	rows = get_term_rows() - 2;
	sheet = vsheet_init(columns);
	if (sheet == NULL)
		err(1, "vsheet");
	if (vsheet_add_rows(&sheet, rows) < 0)
		err(1, "vsheet");
	text = cmt_list_init();
	if (text == NULL)
		err(1, "cmt_list");
	if (argv[1] != NULL && argv[1][0] != '-')
		default_file = argv[1];
	csv_read(default_file);
	/* --- */
	editline = el_init("isc", fdopen(0, "r"), stdout, stderr);
	if (editline == NULL)
		err(1, "editline");
	el_set(editline, EL_EDITOR, "vim");
	el_set(editline, EL_PROMPT, get_el_prompt);
	terminal_init();
	terminal_buffer_enable();
	update_screen();
	return 0;
}


static void
update_screen()
{
	int i;
	drawf(1, 1, "\033[J\n     %e", C_ind);
	for (i = 0; i < columns; i++)
		fprintf(stderr, "   %c  ", 'a' + i);
	for (i = 0; i < rows; i++)
		drawf(0, 0, "\n%e%5d%e%6ad", C_ind, scroll_offset + i, C_number,
		       vsheet_get_num(sheet, scroll_offset + i, 0), columns);
	update_text(0);
	move_selection(0, 0); /* draw cursor */
}


static int
get_term_rows(void)
{
	struct winsize sz;
	ioctl(0, TIOCGWINSZ, &sz);
	return  sz.ws_row;
}


static const char *
get_el_prompt(void *not_used)
{
	fputs("\e[?25h", stderr); /* enable cursor */
	return prompt;
}


/*
 * expects allocated sheet
 */
static void
csv_read(char *file_name)
{
	FILE *stream;
	char *mem = NULL;
	size_t i = 0;
	stream = fopen(file_name, "r");
	if (stream == NULL) err(1, "fopen");
	default_file = file_name;
	csv_parse(NULL);
	errno = 0;
	while (getline(&mem, &i, stream) > 0) {
		csv_parse(mem);
	};
	if (errno == ENOMEM) err(1, "getline");
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
	row_max = 1 + csv_get_last_row();
	for (r = 0; r < row_max; r++) {
		width = csv_get_row_width(r);
		for (c = 0 ;;) {
			str = cmt_list_str(&text, r, c);
			val = sheet->vals[(r * sheet->cols) + c];
			if (val) {
				if (str != NULL && *str != '\0')
					fprintf(stream, "%d\"%s\"", val, str);
				else fprintf(stream, "%d", val);
			} else if (str != NULL && *str != '\0') {
				fprintf(stream, "\"%s\"", str);
			};
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
			err(1, "vsheet");
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
			if (cmt == NULL) err(1, "cmt_list");
			cmt->s = strdup(++str);
			if (tmp == NULL) err(1, "strdup");
			else str = ++tmp;
			goto csv_parse_switch;
			break;
		case '>': /* FALLTHROW */
			str++;
		default:
			if ((tmp = strchr(str, ',')) == NULL) {
				str[strlen(str) - 1] = '\0';
				cmt = cmt_list_new(&text, row, col);
				if (cmt == NULL) err(1, "cmt_list");
				cmt->s = strdup(str);
				row += 1;
				col = 0;
				return 0;
			} else {
				*tmp = '\0';
				cmt = cmt_list_new(&text, row, col);
				if (cmt == NULL) err(1, "cmt_list");
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
	size_t tc = 0;
	while (1) {
		update_nums(row_changed_first, row_changed_last);
		/*update_text(0);*/
		row_changed_first = 0;
		row_changed_last  = 0;
		while (read(0, &c, 1) <= 0) ;
		switch (c) {
		case '0': /* FALLTHROW */
			if (count == 0) {
				count = -1;
				break;
			};
		case '1': case '2': case '3':
		case '4': case '5': case '6':
		case '7': case '8': case '9':
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
		case '>': prompt = "  > "; box_set_text(); break;
		case '=': prompt = "  = "; box_set_num();  break;
		case '/': prompt = "  / "; box_search();   break;
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
		case 'q':
			return 0;
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
#	define SCOL ((6 * (sel_col + 1)) + 1)
#	define SROW (sel_row + 3)
	int n;
	n = *sel_get_num();
	if(n) drawf(SROW, SCOL -1, " %e%5d ", C_number, n);
	else  drawf(SROW, SCOL -1, "       ");
	drawf(SROW, 1, "%e%5d", C_ind, scroll_offset + sel_row); /* row numbers */
	drawf(2, SCOL, "  %e%c  ", C_ind, 'a' + sel_col); /* column numbers */
	update_cursor_text(0);
	drawf(1, 1, "\033[K"); /* clear command line */
	sel_col = sel_col + c >= 0 ? sel_col +c : 0;
	if (sel_col >= columns) sel_col = columns - 1;
	if (r != 0) (r < 0 ? scroll_up(-r) : scroll_down(r));
	drawf(SROW, SCOL -1, "%e[\033[5C]", C_cursor); /* cursor */
	drawf(SROW, 1, "%e>%4d", C_cursor, scroll_offset + sel_row); /* row numbers */
	drawf(2, SCOL, "  %e%c  ", C_cursor, 'a' + sel_col); /* column numbers */
	update_cursor_text(1);
	//yyerror(NULL, NULL); /* draw error messages */
#	undef SCOL
#	undef SROW
}


static void
scroll_up(int r)
{
	int i;
	sel_row -= r;
	if (sel_row >= 0) return ;
	r = -sel_row;
	sel_row = 0;
	if (r > 8) {
		scroll_offset -= r;
		if (scroll_offset < 0) scroll_offset = 0;
		fputs("\033[1J", stderr);
		update_screen();
		return ;
	};
	for (; scroll_offset && r; r--) {
		fputs("\x1b[T", stderr); /* scroll Down */
		drawf(1, 1, "\033[K\n     %e", C_ind);
		for (i = 0; i < columns; i++) fprintf(stderr, "   %c  ", 'a' + i);
		drawf(3, 6, "\033[K");
		scroll_offset -= 1;
		drawf(3, 1, "%e%5d%e%6ad", C_ind, scroll_offset, C_number,
			vsheet_get_num(sheet, scroll_offset, 0), columns);
		update_text(1); /* draw text */
	};
}


static void
scroll_down(int r)
{
	int i;
	sel_row += r;
	if (sel_row < rows) {
		return ;
	} else {
		r = sel_row - rows + 1;
		sel_row = rows - 1;
		if (scroll_offset + rows + r >= sheet->rows)
			if (vsheet_add_rows(&sheet, r) < 0) die("Failed to realloc");
		if (r > 8) {
			scroll_offset += r;
			if (scroll_offset + rows >= 99999)
				scroll_offset = 99999 - rows;
			fputs("\033[1J", stderr);
			update_screen();
			return ;
		};
	};
	for (; r > 0 && (scroll_offset + rows) < 99999; r--) {
		fputs("\x1b[S", stderr); /* scroll Up */
		drawf(1, 1, "\033[K\n     %e", C_ind);
		for (i = 0; i < columns; i++) fprintf(stderr, "   %c  ", 'a' + i);
		scroll_offset += 1;
		drawf(rows +2, 1, "%e%5d%e%6ad", C_ind, scroll_offset + rows -1, C_number,
		         vsheet_get_num(sheet, scroll_offset + rows -1, 0), columns);
		update_text(-1); /* draw text */
	};
}


static void
box_set_num(void)
{
	int i;
	char *str;
	drawf(1, 1, "\033[K");
	str = el_gets(editline, &i);
	if (str != NULL) {
		i = 0;
		if (parse(str, &i) == 0) {
			*sel_get_num() = i;
		};
	};
	terminal_init();
	drawf(1, 1, "\033[K");
	drawf(1, 8, "%s", str);
	move_selection(1, 0);
}


static void
box_set_text(void)
{
	struct comment *cmt;
	int i;
	drawf(1, 1, "\033[K");
	if ((parser_input_str = el_gets(editline, &i)) != NULL) {
		cmt = cmt_list_new(&text, sel_row + scroll_offset, sel_col);
		if (cmt == NULL) {
			die("cmt_list error");
		} else if (*parser_input_str == '\0') {
			cmt->row = 0;
			cmt->col = 0;
			if (cmt->s != NULL) free(cmt->s);
			cmt->s = NULL;
		} else {
			cmt->row = sel_row + scroll_offset;
			cmt->col = sel_col;
			if (cmt->s != NULL) free(cmt->s);
			cmt->s = strdup(parser_input_str);
			cmt->s[i -1] = '\0';
		};
		parser_input_str = NULL;
	};
	terminal_init();
	drawf(1, 1, "\033[K");
	move_selection(0, 0);
}


static void
box_search(void)
{
	int i;
	char *str, *re;
	drawf(1, 1, "\033[K");
	if ((str = command_line_input("    / ", 80, NULL)) != NULL) {
		if (*str == '\0') return ;
		if ((re = compile(str)) == NULL) return ;
		free(str);
		drawf(1, 1, "\033[K");
		for (i = 0; i < text->sz; i++) {
			if (text->list[i].s == NULL) break;
			if (advance(re, text->list[i].s)) {
				sel_col = text->list[i].col;
				sel_row = text->list[i].row;
				if (sel_row > (scroll_offset + rows)) {
					scroll_offset = sel_row - rows + 1;
					sel_row = rows - 1;
				} else if (sel_row < scroll_offset) {
					scroll_offset = sel_row;
					sel_row = 0;
				} else {
					sel_row -= scroll_offset;
				};
				update_screen();
				free(re);
				return ;
			};
		};
		drawf(1, 1, "%ePattern not found !", C_error);
		free(re);
	};
}


static void
parse_text(void)
{
	int i;
	char *str;
	str = cmt_list_str(&text, sel_row + scroll_offset, sel_col);
	if (str == NULL) return ;
	if (parse(str, &i) == 0) {
		*sel_get_num() = i;
	};
	move_selection(0, 0);
}


static void
update_text_row(int row)
{
	int i, r, c, n;
	int tmp;
	cmt_list_update(&text);
	i = cmt_list_get_from(&text, scroll_offset + row, 0);
	for (; text->list[i].row == (scroll_offset + row); i++) {
		if  (text->list[i].s == NULL) return ;
		r = text->list[i].row + 3 - scroll_offset;
		c = ((text->list[i].col + 1) * 6) + 1;
		switch (text->list[i].s[0]) {
		case '!':
			drawf(r, c, "%e%s", C_text, text->list[i].s +1);
			tmp = strlen(text->list[i].s +1) / 6;
			i = cmt_list_get_from(&text, scroll_offset + row,
				text->list[i].col + tmp);
			break;
		case '=':
			n = *vsheet_get_num(sheet,
			    text->list[i].row, text->list[i].col);
			drawf(r, c, "%e%5d", C_special, n);
			break;
		default:
			drawf(r, c, "%e%5s", C_text, text->list[i].s);
			break;
		};
	};
}


static void
update_cursor_text(int update_command_line)
{
	char *str;
	int n;
	str = cmt_list_str(&text, sel_row + scroll_offset, sel_col);
	if (str != NULL) {
		update_text_row(sel_row);
		if (update_command_line) {
			drawf(1, 1, "\033[K%e [ %d ] %e%s",
			    C_number, *vsheet_get_num(sheet, sel_row + scroll_offset, sel_col),
			    C_text, str);
		};
	} else if (update_command_line) {
		drawf(1, 1, "\033[K%e [ %d ]",
		    C_number, *vsheet_get_num(sheet, sel_row + scroll_offset, sel_col));
	};
}


static void
update_text(int r)
{
	size_t i;
	if (r > 0) {
		update_text_row(0);
	} else if (r < 0) {
		update_text_row(rows -1);
	} else {
		for (i = 0; i < rows; i++)
			update_text_row(i);
	};
}


static void
update_nums(int ra, int rb)
{
	int i;
	if (ra >= rb) return ;
	else if (ra > scroll_offset + rows) return ;
	else if (rb < scroll_offset) return ;
	else if (rb > scroll_offset + rows) rb = scroll_offset + rows;
	fprintf(stderr, "\033[%sm", C_number);
	for (i = ra; i < rb; i++)
		drawf(i - scroll_offset + 3, 6,  "%6ad",
		      vsheet_get_num(sheet, i, 0), columns);
	update_text(0);
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
	terminal_buffer_disable();
	terminal_restore();
	vsheet_free(sheet);
	el_end(editline);
}


void
die_line_num(int n, const char *s)
{
	terminal_buffer_disable();
	terminal_restore();
	fprintf(stderr, "line %i: %s\f\r", n, s);
	longjmp(on_error, 1);
}




int
main(int argc, char *argv[])
{
	errno = 0;
	if (!setjmp(on_error)) {
		setup(argv);
		run();
		restore();
	} else {
		return 1;
	};
	return 0;
}
