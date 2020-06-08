/* date:28.09. 2019 */

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
	if (r > 0) fprintf(stderr, "\033[%hhu;%hhuH", r, c);
	for (; *format; format++) {
		if (*format != '%') {
			fputc(*format, stderr);
			continue;
		};
		fsz = strtol(++format, (char **) &format, 0);
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
