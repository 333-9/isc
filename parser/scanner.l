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
("//".*)*\n { return '$'; }

%%
