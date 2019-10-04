%{
#include <stdlib.h>
#include <stdio.h>

#include "parser/parser_defs.h"

int yylex();

void yyerror(char *s) {
	return ;
}

short vars[25] = {0};
%}

%token NUM
%token Var_name
%token L_Shift  R_Shift

%right '='
%left '?' ':'
%left '<' '>'
%left '+' '-'
%left '&' '^' '|'
%left L_Shift  R_Shift
%left '*' '/' '%'

%%

program:
	expr '$'         { return (unsigned short int) $1; }
|	/* NOP */        { return -1; }
;

expr: NUM { $$ = $1; }
|	expr '+' expr          { $$ = $1 + $3; }
|	expr '-' expr          { $$ = $1 - $3; }
|	expr '>' expr          { $$ = $1 > $3; }
|	expr '<' expr          { $$ = $1 < $3; }
|	expr '*' expr          { $$ = $1 * $3; }
|	expr '&' expr          { $$ = $1 & $3; }
|	expr '^' expr          { $$ = $1 ^ $3; }
|	expr '|' expr          { $$ = $1 | $3; }
|	expr L_Shift expr      { $$ = $1 << $3; }
|	expr R_Shift expr      { $$ = $1 >> $3; }
|	expr '?' expr ':' expr { $$ = $1 ? $3 : $5; }
|	'(' expr ')'           { $$ = $2; }
|	variable '=' ':' expr      { $$ = vars[$1]   = $4; }
|	variable '=' '+' expr      { $$ = vars[$1]  += $4; }
|	variable '=' '-' expr      { $$ = vars[$1]  -= $4; }
|	variable '=' '*' expr      { $$ = vars[$1]  *= $4; }
|	variable '=' '&' expr      { $$ = vars[$1]  &= $4; }
|	variable '=' '^' expr      { $$ = vars[$1]  ^= $4; }
|	variable '=' '|' expr      { $$ = vars[$1]  ^= $4; }
|	variable '=' L_Shift expr  { $$ = vars[$1] <<= $4; }
|	variable '=' R_Shift expr  { $$ = vars[$1] >>= $4; }
|	variable                   { $$ = vars[$1]; }
|	table_value '=' ':' expr      { $$ = *((int *)$1)   = $4; }
|	table_value '=' '+' expr      { $$ = *((int *)$1)  += $4; }
|	table_value '=' '-' expr      { $$ = *((int *)$1)  -= $4; }
|	table_value '=' '*' expr      { $$ = *((int *)$1)  *= $4; }
|	table_value '=' '&' expr      { $$ = *((int *)$1)  &= $4; }
|	table_value '=' '^' expr      { $$ = *((int *)$1)  ^= $4; }
|	table_value '=' '|' expr      { $$ = *((int *)$1)  ^= $4; }
|	table_value '=' L_Shift expr  { $$ = *((int *)$1) <<= $4; }
|	table_value '=' R_Shift expr  { $$ = *((int *)$1) >>= $4; }
|	table_value                   { $$ = *((int *) $1); }
|	expr '%' expr {
			if ($3 == 0) return -1;
			else $$ = $1 % $3;
		}
|	expr '/' expr {
			if ($3 == 0) return -1;
			else $$ = $1 % $3;
		}
|	{ return -1; }
;

variable: Var_name { $$ = $1; }

table_value: Var_name NUM { if(($$ = (int) parser_get_num($2, $1)) == NULL) return -1; }

%%
/* EOF */
