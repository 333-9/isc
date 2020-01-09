/*
 * date:04.12. 2019
 * basic regex engine with character classes
 */


enum Pattern {
	P_END = 0,
	P_CHAR,
	P_ANY,
	P_STAR,
	P_CC,
	P_NCC = P_CC + 1,
};


char  *compile(const char *str);
int    advance(const char *re, const char *str);
