// date: 05.04 2020

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

extern int *data_io(int, int);
extern jmp_buf on_parser_error;
extern const char *err_msg;


typedef struct pair {
	int i;
	const char *s;
} pair;


struct {
	long r, c;
} ex = { 0 };


static void nextt          (const char **);
static int  parse_num      (const char **);
static int  parse_fact     (const char **);
static int  parse_sum      (const char **);
static int  parse_bitsum   (const char **);
static int  parse_function (const char **);
static int  parse_expr     (const char **);
static void parse_return   (const char *);
int parse (const char *);


static void
nextt(const char **s)
{
	while (**s && isspace(**s)) *s += 1;
}

static void
skipparen(const char **s)
{
	const char *p;
	if (**s == '[') {
		p = strchr(*s, ']');
		if (p == NULL) return ;
		*s = p + 1;
	};
	if (**s == '{') {
		p = strchr(*s, '}');
		if (p == NULL) return ;
		*s = p + 1;
	};
}


static void
m_err(const char *e)
{
	ex.r = ex.c = -1;
	err_msg = e;
	longjmp(on_parser_error, 1);
}


static int
parse_num(const char **s_)
{
	long r, c, n;
	const char *end;
	const char *s = *s_;
//
	if (isalpha(*s)) { // register
		r = ex.r;
		c = *s - 'a';
		n = *data_io(r, c);
		s += 1;
	} else if (!strncmp(s, "++", 2)) { // ++
		r = ex.r + 1;
		while (isspace(*s)) s++;
		if (isalpha(*s))
			c = *s++ - 'a';
		else
			c = ex.c;
		n = *data_io(r, c);
		//printf("%ld, %ld\n", r, c);
	} else { // 20a
		r = strtol(s, (char **) &end, 0);
		s = end;
		if (isalpha(*s)) {
			c = *s++ - 'a';
			n = *data_io(r, c);
			//printf("%ld, %ld\n", r, c);
		} else if (*s == '.') {
			// subdata
			n = 0;
		} else {
			n = r;
		};
	};
	*s_ = s;
	return n;
}


static int
parse_fact(const char **s)
{
	int l, r;
	char c;
//
	nextt(s);
	if (**s == '(') {
		*s += 1;
		l = parse_expr(s);
		if (**s != ')') return *(*s = "");
		*s += 1;
	} else {
		l = parse_num(s);
	};
	nextt(s);
	if (**s == 0 || strchr("*/%", **s) == NULL)
		return l;
	c = **s;
	*s += 1;
	r = parse_fact(s);
	if /**/ (c == '*') l *= r;
	else if (r == 0)  m_err("division by zero");
	else if (c == '/') l /= r;
	else if (c == '%') l %= r;
	return l;
}


static int
parse_sum(const char **s)
{
	int l, r;
	char c;
//
	l = parse_fact(s);
	nextt(s);
	if (**s == 0 || strchr("+-", **s) == NULL)
		return l;
	c = **s;
	*s += 1;
	r = parse_sum(s);
	if /**/ (c == '+') return l + r;
	else if (c == '-') return l - r;
	return 0; // unreached
}


static int
parse_bitsum(const char **s)
{
	int l, r;
	char c;
//
	l = parse_sum(s);
	nextt(s);
	if (**s == 0 || strchr("&|^", **s) == NULL)
		return l;
	c = **s;
	*s += 1;
	r = parse_bitsum(s);
	if /**/ (c == '&') return l & r;
	else if (c == '|') return l | r;
	else if (c == '^') return l ^ r;
	return 0; // unreached
}


static int
parse_function(const char **s)
{
	int l, r;
	const char *t;
//
	l = parse_bitsum(s);
	nextt(s);
	if ((*s)[0] == 0 || (*s)[1] == 0
	||  strchr("rlgen", **s) == NULL)
		return l;
	t = *s;
	*s += 2;
	nextt(s);
	r = parse_function(s);
	if /**/ (!strncmp("lt", t, 2)) return l <  r;
	else if (!strncmp("gt", t, 2)) return l >  r;
	else if (!strncmp("le", t, 2)) return l <= r;
	else if (!strncmp("ge", t, 2)) return l >= r;
	else if (!strncmp("eq", t, 2)) return l == r;
	else if (!strncmp("ne", t, 2)) return l != r;
	else if (!strncmp("ls", t, 2)) return l << r;
	else if (!strncmp("rs", t, 2)) return l >> r;
	else  m_err("function not found");
	return 0; // unreached
}


static int
parse_expr(const char **s)
{
	int l, r, i;
	const char *p;
//
	nextt(s);
	l = parse_function(s);
	nextt(s);
	if (**s == '?') {
		*s += 1;
		if (l) {
			nextt(s);
			r = parse_expr(s);
			nextt(s);
			if (**s != ':') return l;
			p = strpbrk(*s, ";>");
			if (p != NULL)
				*s = p;
			else
				*s = "";
			return r;
		} else {
			for (i = 1, p = *s ;;) {
				p = strpbrk(p, "?:");
				if (p == NULL) m_err("'?:' operator incomplete");
				if (*p == '?') i++;
				else i--;
				p += 1;
				if (i <= 0) break;
			};
			*s = p;
			return parse_expr(s);
		};
	} else {
		return l;
	};
}


static void
parse_return(const char *s)
{
	const char *end;
	nextt(&s);
	if (isalpha(*s)) { ex.c = *s - 'a'; return;
	} else if (*s == '+') {  ex.r += 1;  s += 1 + (s[1] == '+');
	} else if (*s == '-') {  ex.r -= 1;  s += 1 + (s[1] == '-');
	} else if (isdigit(*s)) {
		ex.r = strtol(s, (char **) &end, 0);
		s = end;
	};
	nextt(&s);
	if (isalpha(*s)) ex.c = *s - 'a';
}


int
parse(const char *s)
{
	int n;
	char c;
//
	if (*s == '\0') m_err("no command");
	if (ex.r < 0) return 0;
//
	nextt(&s);
	if (*s == '=') s += 1;
	skipparen(&s);
	n = parse_expr(&s);
	nextt(&s);
//
	if (*s == '>') {
		s += 1;
		if (*s == '?') {
			if (!n) {
				s = strchr(s, ':');
				if (s == NULL) {
					ex.r = -1;
					ex.c = -1;
					return n;
				};
			};
			parse_return(++s);
			*data_io(ex.r, ex.c) = n;
			return n;
		};
		if (strchr("!&|", *s)) c = *s++;
		else c = 0;
		parse_return(s);
		*data_io(ex.r, ex.c) = n;
		if /**/ (c == '!')  return n;
		else if (c == '&' && n)  return n;
		else if (c == '|' && !n) return n;
	} else if (*s == ';') {
		s += 1;
		if (*s == '?') {
			if (!n) {
				s = strchr(s, ':');
				if (s == NULL) {
					ex.r = -1;
					ex.c = -1;
					return n;
				};
			};
			parse_return(++s);
			return n;
		};
		parse_return(++s);
		if /**/ (*s == '&' && n)  return n;
		else if (*s == '|' && !n) return n;
		else  return n;
	};
	ex.r = -1;
	ex.c = -1;
	return n;
}

/*
1 b  // row 1, column 2
2 + 4  >  10 a  // redirect result
2 + 4  >! 10 a  // redirect and execute
12 / 8 >| 12 b  // redirect, execute if zero
50 / 1 >& 04 a  // redirect, execute if nonzero
11 + a ;  20 b  // execute
a % 3  ;& 10 f  // if result nonzero execute
a % 2  ;| 10 f  // if result zero execute


>   // redirect
>!  // redirect and execute
>|  // redirect, execute if zero
>&  // redirect, execute if nonzero
>?: // conditionally redirect and execute ??

;   // execute
;|  // execute if zero
;&  // execute if nonzero
;?: // conditionally execute ??


// */

/*

Operators:
    ( ) * / %
    + -
    & | ^
    gt lt ge le  ee ne  ls rs
    ?:

// */
