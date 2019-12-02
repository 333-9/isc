struct text_box {
	int width;
	int color;
};




void draw_box_str(char, int, short, short, const char *);

void draw_box_num(char, int, short, short, int);
//void draw_column_num(char, int, short, short, int *, size_t);
void draw_table_num(char, int, short, short, short gap,
               size_t n_rows, size_t n_cols, int *table);

void draw_row_numbering(char, short, short, short, short);
void draw_column_numbering(char, short, short, short, short);
