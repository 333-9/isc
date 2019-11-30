#include "table.h"


int p_rand   (int, int);
int p_assign (int, int);
int p_mod    (int, int);
int p_div    (int, int);
int p_ls     (int, int);
int p_rs     (int, int);
int p_add    (int, int);
int p_sub    (int, int);
int p_and    (int, int);
int p_or     (int, int);
int p_xor    (int, int);
int p_mul    (int, int);
int p_max    (int, int);
int p_min    (int, int);

int  *parser_get_num(size_t r, size_t c);
int  parser_for_range(size_t r1, size_t r2, size_t c, int (*func)(int, int));
int  parser_assign_for_range(size_t r1, size_t r2, size_t c, int (*func)(int, int), int n);
