/*
 * wur@guardian
 * date:28.09. 2019
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "draw.h"


static long pow(a, b) {
	return(b == 1? a : a * pow(a, b - 1));
}


void
drawf(uint8_t r, uint8_t c, const char *format, ...)
{
	va_list args; va_start(args, format);
	char int_format_str[] = "%*_";
	int fsz, arr_sz, i, *arr;
	char *str;
	uint64_t num;
	if (r > 0) fprintf(stderr, "\033[%hu;%huH", r, c);
	for (; *format; format++) {
		if (*format != '%') {
			fputc(*format, stderr);
			continue;
		};
		fsz = strtol(++format, &format, 0);
		if (*format == '*') {
			fsz = va_arg(args, int);
			format++;
		};
		switch (*format) {
		case '\0':
			va_end(args);
			return ;
		case 's':
			str = va_arg(args, char *);
			if (strlen(str) <= fsz || fsz == 0)
				fprintf(stderr, "%s", str);
			else fprintf(stderr, "%.*s*", fsz -1, str);
			break;
		case 'a':
			int_format_str[2] = *++format;
			if (*format == '\0') format--;
			if (fsz) fsz--;
			else (fsz = 8);
			arr = va_arg(args, void *);
			arr_sz = va_arg(args, int);
			for (i = 0; i < arr_sz; i++) {
				putc(' ', stderr);
				if (!*arr)
					fprintf(stderr, "%*s", fsz, "");
				else if (*arr >= pow((int []){10, 16, 8} [(*format>>2 & 3) -1], fsz))
					for (num = 0; num < fsz; num++) fputc('*', stderr);
				else
					fprintf(stderr, int_format_str, fsz, *arr);
				arr += 1;
			};
			break;
		case 'd':
		case 'x':
		case 'o':
			num = va_arg(args, int);
			if (fsz == 0) {
				int_format_str[2] = *format;
				fprintf(stderr, int_format_str, -1, num);
				break;
			};
			if (num >= pow((int []){10, 16, 8} [(*format>>2 & 3) -1], fsz)) {
				for (num = 0; num < fsz; num++) fputc('*', stderr);
				break;
			};
			int_format_str[2] = *format;
			fprintf(stderr, int_format_str, fsz, num);
			break;
		case 'e':
			str = va_arg(args, char *);
			fprintf(stderr, "\033[%sm", str);
			break;
		case 'c':
			num = (unsigned) va_arg(args, int);
			fputc((char) num, stderr);
			break;
		case '%':
			fputc('%', stderr);
			break;
		default: break;
		};
	};
	va_end(args);
	return ;
}


void
draw_box_str(char color, int width, short row, short col, const char *text)
{
	if (width < 1) return ;
	if (strlen(text) > width) {
		fprintf(stderr, "\x1b[%hu;%huH\x1b[38;5;%hhum%-.*s*",
		    row, col, color, (width - 1), text);
	} else {
		fprintf(stderr, "\x1b[%hu;%huH\x1b[38;5;%hhum%-.*s",
		    row, col, color, width, text);
	};
}


void
draw_box_num(char color, int width, short row, short col, int num)
{
	if ((unsigned int) num < pow(10, width)) {
		fprintf(stderr, "\x1b[%hhu;%hhuH\x1b[38;5;%hhum%*u",
		    row, col, color, width, num);
	} else if (width > 0) {
		fprintf(stderr, "\x1b[%hhu;%hhuH\x1b[38;5;%hhum%.*s",
		    row, col, color, width, "******");
	};
}


void
draw_table_num(char color, int width, short row, short col, short gap,
               size_t n_rows, size_t n_cols, int *table)
{
	int r, c;
	char line[20];
	if (gap < 1) gap = 1;
	if (width >= 5) {
		fprintf(stderr, "\x1b[%hu;%huH\x1b[38;5;%hhum",
		    row, col, color);
		sprintf(line, "%s\x1b[%hiC", "%*u", gap);
		for (r = 1; r <= n_rows; r++) {
			for (c = 0; c < n_cols; c++) {
				if (*table == 0) {
					fprintf(stderr, "\x1b[%hiC", gap + width);
				} else if ((unsigned int) *table < pow(10, width)) {
					fprintf(stderr, line, width, *table);
				} else {
					fprintf(stderr, "%.*s\x1b[%hiC", width, "******", gap);
				};
				table++;
			};
			fprintf(stderr, "\x1b[%hu;%huH", row + r, col);
		};
	} else {
		return ;
	};
}


/*
void
draw_column_num(char color, int width, short row, short col, int *arr, size_t sz)
{
	int i;
	char line[20];
	if (width >= 5) {
		fprintf(stderr, "\x1b[%hhu;%hhuH\x1b[38;5;%hhum%*u",
		    row, col, color, width, arr[0]);
		sprintf(line, "\x1b[B\x1b[%huD%s", (short) width, "%*u");
		for (i = 1; i < sz; i++)
			fprintf(stderr, line, width, arr[i]);
	} else if (width > 0) {
		fprintf(stderr, "\x1b[%hhu;%hhuH\x1b[38;5;%hhum%-.*s",
		    row, col, color, width, "****");
		sprintf(line, "\x1b[B\x1b[%huD%.*s", (short) width, width, "****");
		for (i = 1; i < sz; i++)
			fputs(line, stderr);
	};
}
*/


void
draw_row_numbering(char c, short row, short col, short n1, short n2)
{
	int i;
	char line[20];
	//int width = num_char_size(n2);
	int width = 5;
	fprintf(stderr, "\x1b[%hhu;%hhuH\x1b[38;5;%hhum%*hu",
	    row, col, (char) c, width, n1);
	sprintf(line, "\x1b[B\x1b[%huD%s", width, "%*hu");
	for (i = n1 + 1; i <= n2; i++)
		fprintf(stderr, line, width, i);
}


void
draw_column_numbering(char c, short row, short col, short c1, short c2)
{
	//fputs("      ", stderr);
	fprintf(stderr, "\x1b[%hu;%huH\x1b[38;5;%hhum     %c", row, col, c, c1++);
	for (; c1 <= c2; c1++) {
		fputs("     ", stderr);
		putc(c1, stderr);
	};
}



/*
int
main()
{
	struct text_box b;
	b.width = 5;
	b.color = 7;
	short int arr[20];
	for (int i = 0; i < 20; i++) arr[i] = rand();
	for (int i = 1; i < 21; i++) {
		draw_box_num(&b, i, 1, rand() % 100000);
		draw_box_str(&(struct text_box) { 5, 8 }, i, 7, "milimeters");
	};
	draw_column_num(&(struct text_box) { 5, 2 }, 1, 7 + 6, arr, 20);
	return 0;
}
*/
