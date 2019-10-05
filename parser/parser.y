%{
#include <stdlib.h>
#include <stdio.h>

char *parser_input_str;

short int *parser_get_num(size_t r, size_t c);
short int parser_for_range(size_t r1, size_t r2, size_t c, int (*func)(int, int));

short vars[25] = {0};


int yylex();

void yyerror(char *s) {
	return ;
}

int p_add(int a, int b)  { return (a + b); }
int p_sub(int a, int b)  { return (a - b); }
int p_and(int a, int b)  { return ((a?a:0xffff) & (b?b:0xffff)); }
int p_or (int a, int b)  { return (a | b); }
int p_xor(int a, int b)  { return (a ^ b); }
int p_mul(int a, int b)  { return ((a?a:1) * (b?b:1)); }
int p_max(int a, int b)  { return (a > b? a       : b); }
int p_min(int a, int b)  { return (a < b? (a?a:b) : (b?b:a)); }

%}

%token NUM
%token Var_name

%token L_Shift  R_Shift
%token GE_cmp LE_cmp EQ_cmp NE_cmp
%token AND OR XOR

%token AO_add AO_sub
%token AO_mul AO_div AO_mod
%token AO_and AO_or  AO_xor
%token AO_ls  AO_rs 

%right '='
%right AO_add AO_sub
%right AO_mul AO_div AO_mod
%right AO_and AO_or  AO_xor
%right AO_ls  AO_rs 
%left '?' ':'
%left GE_cmp LE_cmp EQ_cmp NE_cmp
%left AND OR XOR
%left '<' '>'
%left '+' '-'
%left '&' '^' '|'
%left L_Shift  R_Shift
%left '*' '/' '%'
%left '!'

%%

program:
	expr '$'         { return (unsigned short int) $1; }
|	/* NOP */        { return -1; }
;

variable: Var_name { $$ = $1; }

table_value: Var_name NUM { if(($$ = (int) parser_get_num($2, $1)) == NULL) return -1; }

expr: NUM { $$ = $1; }
|	expr '+' expr      { $$ = $1 + $3; }
|	expr '-' expr      { $$ = $1 - $3; }
|	expr '*' expr      { $$ = $1 * $3; }
|	expr '&' expr      { $$ = $1 & $3; }
|	expr '^' expr      { $$ = $1 ^ $3; }
|	expr '|' expr      { $$ = $1 | $3; }
|	expr L_Shift expr  { $$ = $1 << $3; }
|	expr R_Shift expr  { $$ = $1 >> $3; }
|	expr '%' expr      { if ($3 == 0) return -1; else $$ = $1 % $3; }
|	expr '/' expr      { if ($3 == 0) return -1; else $$ = $1 / $3; }
|	expr '>' expr      { $$ = $1 > $3; }
|	expr '<' expr      { $$ = $1 < $3; }
|	expr EQ_cmp expr   { $$ = $1 == $3; }
|	expr NE_cmp expr   { $$ = $1 != $3; }
|	expr GE_cmp expr   { $$ = $1 >= $3; }
|	expr LE_cmp expr   { $$ = $1 <= $3; }
|	expr AND expr      { $$ = $1 && $3; }
|	expr OR expr       { $$ = $1 || $3; }
|	expr XOR expr      { $$ = ($1?1:0) ^ ($3?1:0); }
|	'!' expr           { $$ = !$2; }
|	expr '?' expr ':' expr  { $$ = $1 ? $3 : $5; }
|	'(' expr ')'            { $$ = $2; }
|	variable                  { $$ = vars[$1]; }
|	variable '='    expr      { $$ = vars[$1]   = $3; }
|	variable AO_add expr      { $$ = vars[$1]  += $3; }
|	variable AO_sub expr      { $$ = vars[$1]  -= $3; }
|	variable AO_mul expr      { $$ = vars[$1]  *= $3; }
|	variable AO_div expr      { if (!$3) return -1;  $$ = vars[$1] /= $3; }
|	variable AO_mod expr      { if (!$3) return -1;  $$ = vars[$1] %= $3; }
|	variable AO_and expr      { $$ = vars[$1]  &= $3; }
|	variable AO_or  expr      { $$ = vars[$1]  |= $3; }
|	variable AO_xor expr      { $$ = vars[$1]  ^= $3; }
|	variable AO_ls  expr      { $$ = vars[$1] >>= $3; }
|	variable AO_rs  expr      { $$ = vars[$1] >>= $3; }
|	table_value                  { $$ = *((int *) $1); }
|	table_value '='    expr      { $$ = *((int *) $1)   = $3; }
|	table_value AO_add expr      { $$ = *((int *) $1)  += $3; }
|	table_value AO_sub expr      { $$ = *((int *) $1)  -= $3; }
|	table_value AO_mul expr      { $$ = *((int *) $1)  *= $3; }
|	table_value AO_div expr      { if (!$3) return -1;  $$ = *((int *) $1) /= $3; }
|	table_value AO_mod expr      { if (!$3) return -1;  $$ = *((int *) $1) %= $3; }
|	table_value AO_and expr      { $$ = *((int *) $1)  &= $3; }
|	table_value AO_or  expr      { $$ = *((int *) $1)  ^= $3; }
|	table_value AO_xor expr      { $$ = *((int *) $1)  |= $3; }
|	table_value AO_ls  expr      { $$ = *((int *) $1) <<= $3; }
|	table_value AO_rs  expr      { $$ = *((int *) $1) >>= $3; }
|	'[' '+' Var_name NUM ':' NUM ']'   { $$ = parser_for_range($4, $6, $3, &p_add); }
|	'[' '-' Var_name NUM ':' NUM ']'   { $$ = parser_for_range($4, $6, $3, &p_sub); }
|	'[' '>' Var_name NUM ':' NUM ']'   { $$ = parser_for_range($4, $6, $3, &p_max); }
|	'[' '<' Var_name NUM ':' NUM ']'   { $$ = parser_for_range($4, $6, $3, &p_min); }
|	'[' '&' Var_name NUM ':' NUM ']'   { $$ = parser_for_range($4, $6, $3, &p_and); }
|	'[' '^' Var_name NUM ':' NUM ']'   { $$ = parser_for_range($4, $6, $3, &p_xor); }
|	'[' '|' Var_name NUM ':' NUM ']'   { $$ = parser_for_range($4, $6, $3, &p_or ); }
|	'[' '*' Var_name NUM ':' NUM ']'   { $$ = parser_for_range($4, $6, $3, &p_mul); }
|	'[' '/' Var_name NUM ':' NUM ']'   { return -2; } /* non valid espressions */
|	'[' '%' Var_name NUM ':' NUM ']'   { return -2; }
|	{ return -1; }
;

%%
/* EOF */
