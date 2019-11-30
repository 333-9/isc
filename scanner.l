%{
#include <stdlib.h>
#include <stdio.h>

#include "y.tab.h"

extern int yylval;
extern char *parser_input_str;


#undef YY_INPUT
#define YY_INPUT(b, m, n) (m = _input(b, n))
size_t _input(char *buf, int sz) {
	size_t size;
	if (parser_input_str == NULL) {
		return 0;
	} else if (sz > (size = 1 + strlen(parser_input_str))) {
		strcpy(buf, parser_input_str);
		buf[size] = '\n';
		return (size + 1);
	} else {
		strncpy(buf, parser_input_str, sz);
		return (sz);
	};
}

%}

%option noyywrap


%%


\'[ -~]\'     { yylval = *(yytext + 1); return NUM; }
"0"           { yylval = 0; return NUM; }
[1-9][0-9]*   { yylval = strtol(yytext, NULL, 10);  return NUM; }
0[0-9]+       { yylval = strtol(yytext, NULL,  8);  return NUM; }
0x[0-9a-f]+   { yylval = strtol(yytext, NULL, 16);  return NUM; }

[a-zA-Z]    { yylval = (*yytext | 32) - 'a'; return Var_name; }

"min"    return F_min;
"max"    return F_max;
"count"  return F_count;

"pow"    return F_pow;
"abs"    return F_abs;
"avg"    return F_avg;

"<<"    return O_lsh;
">>"    return O_rsh;
"**"    return O_pow;

"!"|"not"     return L_not;
"&&"|"and"    return L_and;
"||"|"or"     return L_or;
"^^"|"xor"    return L_xor;

"=="    return C_eq;
"!="    return C_ne;
">="    return C_ge;
"<="    return C_le;

"="     return A_set;
"+="    return A_add;
"-="    return A_sub;
"*="    return A_mul;
"/="    return A_div;
"%="    return A_mod;
"&="    return A_and;
"|="    return A_or;
"^="    return A_xor;
"<<="   return A_ls;
">>="   return A_rs;

[-+/*%&^|?:<>~,._]   { return *yytext; }
[)(\[\]\{\}]        { return *yytext; }

[ \t]+	  { ; }
("//".*)*\n    { return EOL; } 
%%
