/*
 * date:30.09. 2019
 * this file is not compiled, but includet by "isc.c"
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>




struct termios  term_attr_old;




/*
 * str is expected to be at least sz bytes
 * sz must not be more than lex input max size (0x2000)
 */
char *
command_line_input(char *prompt, size_t sz, char *str)
{
	char c;
	int i = 0;
	if (str == NULL) str = malloc(sz);
	if (str == NULL) return NULL;
	fputs(prompt, stderr);
	fputs("\x1b[?25h", stderr); /* make cursor visible */
	str[0] = '\0';
	while (1) {
		while (read(0, &c, 1) <= 0) ;
		if (c == '\n' || c == 004) {
			break;
		} else if (c == 0x08 || c == 0x7f) { /* backspace */
			if (i != 0) i--;
			else continue;
			str[i] = '\0';
			fputs("\x1b[D \x1b[D", stderr);
		} else if (c < 0x7f && c > 0x1f) { /* valid character */
			if (i >= sz - 1) continue;
			str[i] = c;
			i++;
			str[i] = '\0';
			putc(c, stderr);
		} else if (c == 0x1b) { /* escape */
			/* no normal mode now */
			putc('\a', stderr);
		};
	};
	fputs("\x1b[?25l", stderr); /* hide cursor */
	return (str);
}


void
terminal_init(void)
{
	struct termios  attr_new = {
		.c_iflag = BRKINT | ICRNL,
		.c_oflag = ONLCR | OPOST,
		.c_cflag = 0,
		.c_lflag = ISIG,
		.c_cc[VEOF]  = 004,
		.c_cc[VEOL]  = 000,
		.c_cc[VINTR] = 003,
		.c_cc[VKILL] = 025,
		.c_cc[VMIN]  = 0,
		.c_cc[VTIME] = 1,
	};
	tcgetattr(0, &term_attr_old);
	tcsetattr(0, TCSANOW, &attr_new);
	fputs("\x1b[?1049h\x1b[?25l\x1b[H", stderr);
}


void
terminal_restore(void) {
	tcsetattr(0, TCSANOW, &term_attr_old);
}


void
terminal_buffer_enable(void) {
	fputs("\x1b[?1049h\x1b[?25l\x1b[H", stderr);
}


void
terminal_buffer_disable(void) {
	fputs("\x1b[?1049l\x1b[?25h", stderr);
}
