%{
#include <stdlib.h>
#include "y.tab.h"

extern int yylval;

%}

%option noyywrap


%%

[0-9]{1,5}       {
		yylval = atoi(yytext);
		return NUM;
	}

"<<"             { return L_Shift; }
">>"             { return R_Shift; }

[-+/*%&()^|<>?:=] { return *yytext; }

[a-zA-Z]         { yylval = (*yytext | 32) - 'a'; return Var_name; }

[ \t]+	         { ; }
\n               { return '$'; }

%%
