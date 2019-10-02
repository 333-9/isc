/*
 * file not includet in terminal.c
 * terminal.c is supposed to be compiled with isc.c
 */
#include <stdio.h>
#include <termios.h>

struct termios  term_attr_old;
struct termios  term_attr_new;
void terminal_init(void);
void terminal_reinit(void);
void terminal_restore(void);

void terminal_buffer_enable(void);
void terminal_buffer_disable(void);
