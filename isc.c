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




unsigned data_size = 0;
struct sheet *data = NULL;
struct text   text;

int no_data = 0;

jmp_buf         on_parser_error;
const char *    err_msg = "";
static jmp_buf  on_error;


struct row_update {
	int first;
	int last;
} // screen update
update = { -1 };

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
static int   csv_parse(char *, int);
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
static int  *sel_get_num(void);

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

int *
data_io(int r, int c)
{
	int *p;
	p = sheet_get(data, r * columns + c);
	if (p == NULL) return &no_data;
	if /**/ (update.first < 0) update.last  = update.first = r;
	else if (r < update.first) update.first = r;
	else if (r > update.last)  update.last  = r;
	return p;
}


/* === */




static int
setup(char *argv[])
{
	ex.r = -1;
	ex.c = -1;
	rows = get_term_rows() - 2;
	data_size = 1;
	data = malloc(sizeof(struct sheet));
	if (sheet_init(data) < 0)     err(1, "memory allocation error");
	if (comment_init(&text) < 0)  err(1, "memory allocation error");
	csv_read("out.csv");
	/* --- */
	editline = el_init("isc", fdopen(0, "r"), stdout, stderr);
	if (editline == NULL) err(1, "editline");
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
	int r, c;
	int *num;
	drawf(1, 1, "\033[J\n     %e", C_ind);
	for (c = 0; c < columns; c++) {
		fprintf(stderr, "   %c  ", 'a' + c);
	};
	num = sheet_get(data, scroll_offset * columns);
	num = num ? num : &no_data;
	for (r = 0; r < rows; r++) {
		drawf(0, 0, "\n%e%5d%e", C_ind, scroll_offset + r, C_number);
		for (c = 0; c < columns; c++) {
			if (*num) drawf(0, 0, "%6d", *num);
			else      drawf(0, 0, "      ");
			num = sheet_next(data);
			num = num ? num : &no_data;
		};
	};
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


/* expects allocated sheet */
static void
csv_read(char *file_name)
{
	FILE *stream;
	char *mem = NULL;
	size_t i = 0;
	int r = 0;
	stream = fopen(file_name, "r");
	if (stream == NULL) err(1, "fopen");
	errno = 0;
	while (getline(&mem, &i, stream) > 0)
		csv_parse(mem, r++);
	if (errno == ENOMEM) err(1, "csv_read");
	fclose(stream);
	free(mem);
}


static int
csv_parse(char *line, int r)
{
	int   *num;
	char  *str;
	char  *end;
	int c = 0;
	if (line == NULL) return 1;
	if (*line == '#') return 0;
	num = sheet_get(data, columns * r);
	for (;;) {
		*num = strtol(line, &end, 0);
		num = sheet_next(data);
		if (num == NULL) err(1, "parse-num");
		line = end;
		switch (*line) {
		case '#':
		case '\n':
		case '\0':
			return 0;
		case ',':
			c += 1;
			line++;
			break;
		case '\'':
		case '"':
		case '`':
			end = csv_parse_str(line);
			if (end == NULL) return 1;
			str = malloc((size_t) end - (size_t) line);
			if (str == NULL) err(1, "parse-str");
			strcpy(str, line +1);
			if (comment_set(&text, columns * r + c, str) < 0)
				err(1, "parse-text");
			line = end + 1;
			if (line[0] != ',') return 1;
			line++;
			break;
		default:
			return 1;
		};
	};
}


static char *
csv_parse_str(char *str)
{
	char quot;
	quot = *str++;
	for (; *str; str++) {
		if (*str == '\\') {
			if (*++str == '\0') return NULL;
		} else if (*str == quot || *str == '\n') {
			*str = '\0';
			return str;
		};
	};
	return NULL;
}


static void
csv_write(const char *file_name)
{
	int  *num;
	char *str;
	int   off = 0;
	int   r, c;
	int   width, height;
	FILE *stream;
	// setup
	stream = fopen(file_name, "w");
	if (stream == NULL)
	    die("failed to open file");
	height = csv_get_last_row();
	text.last = 0;
	text.index = -1;
	data->ind = 0;
	// write
	for (r = 0; r < height; r++, off += columns) {
		width = csv_get_row_width(r);
		num = sheet_get(data, off);
		str = comment_get(&text, off);
		for (c = 0 ; c < width; c++) {
			if (*num) fprintf(stream, "%d", *num);
			if (*str) fprintf(stream, "'%s'", str);
			putc(',', stream);
			num = sheet_next(data);
			str = comment_next(&text);
		};
		if (width == 0) putc(',', stream);
		putc('\n', stream);
	};
	fclose(stream);
}


static int
csv_get_row_width(int r)
{
	return 0;
#if 0
	int i;
	int off = r * columns;
	int max = 0;
	for (i = 0; i < columns; i++)
		if (data->val[off + i]) max = i;
	for (i = 0; text.ind[text.i +i] < off + i; i++) {
		if (text.ind[text.i +i] < 0) return max + 1;
		if (text.ind[text.i +i] > off +max)
			max = text.ind[text.i +i] - off;
	};
	return max + 1;
#endif
}


static int
csv_get_last_row()
{
	return 0;
#if 0
	int r, max, *num;
	for (r = 0; r < text.size; r++)
		if (text.ind[r] < 0) break;
	max = !r ? 0 : text.ind[r -1];
	for (r = max, num = sheet_get(data, r * columns); num != NULL;) {
		if(*num) max = r;
		num = sheet_get(data, r * columns);
	};
	return max + 1;
#endif
}




static int
run(void)
{
	int i;
	const char *str;
	char c;
	int *num;
	int n = 0;
//
	if (setjmp(on_parser_error)) {
		drawf(1, 1, "\033[K%e%s", C_error, err_msg);
	};
	while (1) {
		for (i = 0; i < 5 && ex.r >= 0; i++) {
			str = comment_get(&text, columns * ex.r + ex.c);
			//drawf(1, 1, "\033[K%e < %d > %e%s", C_number,
			//    *sheet_get(data, columns * ex.r + ex.c), C_text, str);
			num = data_io(ex.r, ex.c);
			*num = parse(str);
		};
		if (update.first >= 0) {
			update_nums(update.first, update.last);
			update.first = -1;
		};
		while (read(0, &c, 1) <= 0) ;
		switch (c) {
		case '0': /* FALLTHROW */
			if (n == 0) { move_selection(0, -99); break; };
		case '1': case '2': case '3':
		case '4': case '5': case '6':
		case '7': case '8': case '9':
			if (n < 0) {
				n = c - '0';
				break;
			} else if (n >= 100000) {
				break;
			} else {
				n *= 10;
				n += c - '0';
				break;
			};
		case 'h': move_selection( 0, -(n ? n : 1)); n = 0; break;
		case 'l': move_selection( 0,  (n ? n : 1)); n = 0; break;
		case 'j': move_selection( (n ? n : 1),  0); n = 0; break;
		case 'k': move_selection(-(n ? n : 1),  0); n = 0; break;
		case '>': prompt = "  > "; box_set_text(); break;
		case '=': prompt = "  = "; box_set_num();  break;
		case '/': prompt = "  / "; box_search();   break;
		case '\n':
			if (n != 0) {
				*sel_get_num() = n;
				n = 0;
				move_selection(1, 0);
				break;
			} else {
				parse_text();
				break;
			};
		case 'w': csv_write("out.csv"); break;
		case 'q':
			return 0;
		default:
			putc('\a', stderr);
			n = 0;
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
			sheet_get(data, scroll_offset * columns), columns);
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
		if (columns * (scroll_offset + rows + r) >= data->bottom)
			die("<out of space>");
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
		         sheet_get(data, columns * (scroll_offset + rows -1)), columns);
		update_text(-1); /* draw text */
	};
}


static void
box_set_num(void)
{
	int i, n;
	const char *str;
	drawf(1, 1, "\033[K");
	str = el_gets(editline, &i);
	terminal_init();
	if (str != NULL) {
		ex.r = sel_row + scroll_offset;
		ex.c = sel_col;
		n = parse(str);
		*sel_get_num() = n;
	};
	drawf(1, 1, "\033[K");
	drawf(1, 8, "%s", str);
	move_selection(1, 0);
}


static void
box_set_text(void)
{
	int i;
	const char *str;
	char *str_;
	drawf(1, 1, "\033[K");
	str = el_gets(editline, &i);
	terminal_init();
	if (str != NULL && str[0] == '\0') {
		free(comment_remove(&text, columns * sel_row + sel_col));
	} else if (str != NULL) {
		str_ = strdup(str);
		str_[i-1] = '\0';
		comment_set(&text, columns * sel_row + sel_col, str_);
	};
	drawf(1, 1, "\033[K");
	update_nums(sel_row + scroll_offset, sel_row + scroll_offset);
	update_text_row(sel_row + scroll_offset);
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
		for (i = 0; i < text.size && text.str[i]; i++) {
			if (!advance(re, text.str[i])) continue;
			sel_col = text.ind[i] % columns;
			sel_row = text.ind[i] / columns;
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
		drawf(1, 1, "%ePattern not found !", C_error);
		free(re);
	};
}


static void
parse_text(void)
{
	int i, n;
	char *str;
	str = comment_get(&text, columns * (sel_row + scroll_offset) + sel_col);
	ex.r = sel_row + scroll_offset;
	ex.c = sel_col;
	n = parse(str);
	*sel_get_num() = n;
	move_selection(0, 0);
}


static void
update_text_row(int row)
{
	int i, r, c, n;
	int width;
	char *str, *p;
	r = scroll_offset + row +3;
	c = 7;
	str = comment_get(&text, columns * (scroll_offset + row));
	for (i = 0; i < columns; i++) {
		switch (*str) {
		case '\0':
			break;
		case '!':
			drawf(r, c, "%e%s", C_text, str +1);
			width = strlen(str +1) / 6;
			while (width--) str = comment_next(&text);
			break;
		case '=':
			n = *sheet_get(data, text.index);
			drawf(r, c, "%e%5d%e", C_special, n, C_none);
			break;
		case '{':
		case '[':
			p = strchr(str, ']');
			if (p == NULL) break;
			*p = 0;
			drawf(r, c, "%e%5s", C_button, str +1);
			*p = ']';
			break;
		default:
			drawf(r, c, "%e%5s", C_text, str);
			break;
		};
		c += 6;
		str = comment_next(&text);
	};
}


static void
update_cursor_text(int update_command_line)
{
	char *str;
	int n;
	str = comment_get(&text, columns * (sel_row + scroll_offset) + sel_col);
	if (str != NULL) {
		update_text_row(sel_row);
		if (update_command_line) {
			drawf(1, 1, "\033[K%e [ %d ] %e%s", C_number,
			    *sheet_get(data, columns * (sel_row + scroll_offset) + sel_col),
			    C_text, str);
		};
	} else if (update_command_line) {
		drawf(1, 1, "\033[K%e [ %d ]", C_number,
		    *sheet_get(data, columns * (sel_row + scroll_offset) + sel_col));
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
	if (ra > rb) return ;
	else if (ra > scroll_offset + rows) return ;
	else if (rb < scroll_offset) return ;
	else if (rb > scroll_offset + rows) rb = scroll_offset + rows;
	fprintf(stderr, "\033[%sm", C_number);
	for (i = ra; i <= rb; i++) {
		drawf(i - scroll_offset + 3, 6,  "%6ad",
		      sheet_get(data, i * columns), columns);
		update_text_row(i + scroll_offset);
	};
	//update_text(0);
	if (ra <= scroll_offset + sel_row
	&&  rb >= scroll_offset + sel_row)
		move_selection(0, 0);
}


static int *
sel_get_num()
{
	int *p;
	p = sheet_get(data, columns * (scroll_offset + sel_row) + sel_col);
	if (p == NULL) die("Selection out of bounds");
	return p;
}







static void
restore(void)
{
	terminal_buffer_disable();
	terminal_restore();
	sheet_free(data);
	comment_free(&text);
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
