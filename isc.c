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


struct position {
	int col; /* starts at 0 */
	int row; /* starts at 0 */
};

struct {
	size_t height;
	size_t width;
} terminal_status;




size_t scroll_offset = 0;
struct var_sheet *sheet1 = NULL;

struct position sel1;
struct position sel2;




int input(void);
short int *sel_get_num(void);
void move_selection(int, int);
void scroll_up(int);
void scroll_down(int);

int setup(void);

int run(void);

void restore(void);




int
input(void)
{
	char c;
	while (1) {
		while (read(0, &c, 1) <= 0) ;
		switch (c) {
		case 'h': move_selection( 0, -1);  break;
		case 'l': move_selection( 0,  1);  break;
		case 'k': move_selection(-1,  0);  break;
		case 'j': move_selection( 1,  0);  break;
		case ' ':
			sheet1 = vsheet_set_box(sheet1, scroll_offset + sel1.row, sel1.col, random());
			move_selection(0, 0);
			break;
		case 'd':
			sheet1 = vsheet_set_box(sheet1, scroll_offset + sel1.row, sel1.col, 0);
			move_selection(0, 0);
			break;
		case 'r':
			draw_table_num(Grey, 5, 2, 7, 1, rows + 1, columns, sheet1->vals);
			break;
		case 's': return 0;
		case 'q': return -1;
		default:  break;
		};
	};
}


short int *
sel_get_num(void) {
	return (sheet1->vals + ((scroll_offset + (sel1.row)) * sheet1->cols) + sel1.col);
}


void
move_selection(int r, int c)
{
#	define S_COL(c) ((6 * (c.col + 1)) + 1)
#	define S_ROW(r) (r.row + 2)
	int n = *sel_get_num();
	draw_box_num(Red, 5, S_ROW(sel1), 1, scroll_offset + sel1.row);
	draw_box_str(Black, 1, S_ROW(sel1), S_COL(sel1) - 1,     " ");
	draw_box_str(Black, 1, S_ROW(sel1), S_COL(1 + sel1) - 1, " ");
	if (!n) draw_box_str(Black, 5, S_ROW(sel1), S_COL(sel1), "     ");
	else    draw_box_num(Grey,  5, S_ROW(sel1), S_COL(sel1), n);
	if        (c < 0 && (sel1.col + c) >= 0) { sel1.col += c;
	} else if (c > 0 && (sel1.col + c) < (columns)) { sel1.col += c;
	} else if (r < 0) { scroll_up(r);
	} else if (r > 0) { scroll_down(r);
	};
	draw_box_num(Red,  5, S_ROW(sel1), 1, scroll_offset + sel1.row);
	draw_box_str(Aqua, 1, S_ROW(sel1), 1, ">");
	draw_box_str(Aqua, 1, S_ROW(sel1), S_COL(sel1) - 1,     "[");
	draw_box_str(Aqua, 1, S_ROW(sel1), S_COL(1 + sel1) - 1, "]");
#	undef S_COL S_ROW
}


void
scroll_up(int r)
{
	if ((sel1.row + r) >= 0) return (void)(sel1.row += r);
	if (scroll_offset <= 0) return ;
	// TODO: make it variable (dependent on 'r')
	fputs("\x1b[T", stderr); /* scroll Down */
	draw_box_str(Black, 9, terminal_status.height - 1, 1, "\x1b[K");
	draw_column_numbering(Red, 1, 4, 'a', 'a' + (columns - 1));
	draw_box_num(Grey, 5, 1, 1, sheet1->rows);
	draw_box_str(Black, 9, 2, 6, "\x1b[K"); /* clear prev row numbering */
	scroll_offset -= 1;
	draw_box_num(Red, 5, 2, 1, scroll_offset);
	draw_table_num(Grey, 5, 2, 7, 1,
	    1, columns, sheet1->vals + (scroll_offset * sheet1->cols));
}


void
scroll_down(int r)
{
	if ((sel1.row + r) < (rows)) return (void)(sel1.row += r);
	// TODO: make it variable (dependent on 'r')
	fputs("\x1b[S", stderr); /* scroll Up */
	if ((sheet1->rows - 1) <= (sel1.row + scroll_offset))
		sheet1 = vsheet_add_rows(sheet1, 1);
	draw_column_numbering(Red, 1, 4, 'a', 'a' + (columns - 1));
	draw_box_str(Red, 6, 1, 1, "      ");
	draw_box_num(Grey, 5, 1, 1, sheet1->rows);
	draw_table_num(Grey, 5, rows, 7, 1,
	    1, columns, sheet1->vals + ((rows + scroll_offset - 1) * sheet1->cols));
	draw_box_num(Red, 5, 21, 1, scroll_offset + 20);
	scroll_offset += 1;
}




int
setup(void)
{
	terminal_init();
	terminal_buffer_enable();
	terminal_status.height = rows + 3;
	terminal_status.width  = columns;
	sheet1 = vsheet_init(columns);
	sheet1 = vsheet_add_rows(sheet1, rows);
	draw_column_numbering(Red, 1, 4, 'a', 'a' + columns - 1);
	draw_row_numbering(Red, 2, 1, 0, rows - 1);
	draw_box_num(Grey, 5, 1, 1, sheet1->rows);
	sel1 = (struct position){ .col = 0, .row = 0 };
	move_selection(0, 0);
}


int
run(void)
{
	while (input() >= 0) ;
}


void
restore(void)
{
	terminal_restore();
	terminal_buffer_disable();
	vsheet_free(sheet1);
}




int
main()
{
	setup();
	run();
	restore();
	return 0;
}
