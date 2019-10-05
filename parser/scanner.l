%{
#include <stdlib.h>
#include <stdio.h>
#include "y.tab.h"

extern int yylval;
extern char *parser_input_str;
//int gg_give_input(char *buf, int *r_sz, int sz);

#undef YY_INPUT
#define YY_INPUT(b, m, n) gg_give_input(b, &(m), n)
int gg_give_input(char *buf, int *r_sz, int sz) {
	static char *text = NULL, *p = NULL;
	static size_t size = 0;
	if (text != parser_input_str) {
		text = parser_input_str;
		p = text;
		size = strlen(p);
	};
	if (size == 0) {
		*r_sz = 1;
		*buf = '\n';
		return -1;
	} else if (sz > size) {
		*r_sz = size;
		memcpy(buf, p, size);
		size = 0;
	} else {
		size -= sz;
		memcpy(buf, p, sz);
		p += sz;
		*r_sz = sz;
	};
	return 0;
}

%}

%option noyywrap


%%

[0-9]{1,5} {
		yylval = atoi(yytext);
		return NUM;
	}

[a-zA-Z]  { yylval = (*yytext | 32) - 'a'; return Var_name; }

"<<"   { return L_Shift; }
">>"   { return R_Shift; }
"&&"   { return AND; }
"||"   { return OR;  }
"^^"   { return XOR; }
"=="   { return EQ_cmp; }
"!="   { return NE_cmp; }
">="   { return GE_cmp; }
"<="   { return LE_cmp; }

"+="   { return AO_add; }
"-="   { return AO_sub; }
"*="   { return AO_mul; }
"/="   { return AO_div; }
"%="   { return AO_mod; }
"&="   { return AO_and; }
"|="   { return AO_or;  }
"^="   { return AO_xor; }
"<<="  { return AO_ls;  }
">>="  { return AO_rs;  }

[-+/*%&^|?:=!<>~]   { return *yytext; }
[()\[\]\{\}]        { return *yytext; }

[ \t]+	{ ; }
\n|"//" { return '$'; }

%%
