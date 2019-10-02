/*
 * date:30.09. 2019
 * this file is not compiled, but includet by "isc.c"
 */
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>




struct termios  term_attr_old;
struct termios  term_attr_new;




void
terminal_init(void)
{
	tcgetattr(0, &term_attr_old);
	tcgetattr(0, &term_attr_new);
	term_attr_new.c_lflag = term_attr_new.c_lflag ^ ICANON ^ ECHO;
	term_attr_new.c_cc[VMIN] = 0;
	term_attr_new.c_cc[VTIME] = 1;
	term_attr_new.c_lflag &= ICANON;
	tcsetattr(0, TCSANOW, &term_attr_new);
}


void
terminal_reinit(void) {
	tcsetattr(0, TCSANOW, &term_attr_new);
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
