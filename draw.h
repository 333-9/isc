#ifndef DRAW_H
#define DRAW_H

enum Color {
	Black   = 0,
	Red     = 1,
	Green   = 2,
	Yellow  = 3,
	Blue    = 4,
	Magenta = 5,
	Aqua    = 6,
	White   = 7,
	Grey    = 8,
};


struct text_box {
	int width;
	enum Color color;
};




void draw_box_str(enum Color, int, short, short, const char *);

void draw_box_num(enum Color, int, short, short, short int);
void draw_column_num(enum Color, int, short, short, short int *, size_t);

/* smallest gap is 1, '0' is printed as nothing */
void draw_table_num(enum Color, int, short, short, short gap,
               size_t n_rows, size_t n_cols, short int *table);

void draw_row_numbering(enum Color, short, short, short, short);
void draw_column_numbering(enum Color, short, short, short, short);

#endif
