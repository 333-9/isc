%{
#include <stdlib.h>
#include <stdio.h>

#include "range.h"


extern char *parser_input_str;
extern void  yyerror(int *, char *s);
int vars[25] = {0};
int prev_value = 0;
//|	'$' variable       { $$ = *((int *) $1); }


int yylex();
int ipow(int, int);


#define DIV_Z(i)  if (i == 0) { yyerror(NULL, "division by zero error"); return (-1); }
#define PAFR(a, b, c, d, e)  parser_assign_for_range(a, b, c, d, e)

%}

%parse-param { int *parser_ret }


%token EOL
%token NUM
%token Var_name
%token F_pow F_abs F_avg
%token F_min F_max F_count

%token O_lsh O_rsh O_pow
%token L_not L_and L_or L_xor
%token C_eq C_ne C_ge C_le '<' '>'

%token A_set A_add A_sub
%token A_mul A_div A_mod
%token A_and A_or  A_xor
%token A_ls  A_rs 


%right A_set A_add A_sub
%right A_mul A_div A_mod
%right A_and A_or  A_xor
%right A_ls  A_rs 
%right '?' ':'
%left C_eq C_ne C_ge C_le '<' '>'
%left L_and L_or L_xor
%left '+' '-'
%left '&' '^' '|'
%left O_lsh O_rsh
%left O_pow
%left '*' '/' '%'
%left L_not


%%


program:
	EOL    { return 0; }
|	expr EOL
	{
		prev_value = $1;
		if (parser_ret != NULL) *parser_ret = $1;
		return 0;
	}
;

variable:
	Var_name { $$ = (int) (vars + $1); }
|	Var_name NUM { if(($$ = (int) parser_get_num($2, $1)) == NULL) return 1; }
;

assignment:
	variable A_set expr   { $$ = *((int *) $1)   = $3; }
|	variable A_add expr   { $$ = *((int *) $1)  += $3; }
|	variable A_sub expr   { $$ = *((int *) $1)  -= $3; }
|	variable A_mul expr   { $$ = *((int *) $1)  *= $3; }
|	variable A_div expr   { DIV_Z(!$3);  $$ = *((int *) $1) /= $3; }
|	variable A_mod expr   { DIV_Z(!$3);  $$ = *((int *) $1) %= $3; }
|	variable A_and expr   { $$ = *((int *) $1)  &= $3; }
|	variable A_or  expr   { $$ = *((int *) $1)  ^= $3; }
|	variable A_xor expr   { $$ = *((int *) $1)  |= $3; }
|	variable A_ls  expr   { $$ = *((int *) $1) <<= $3; }
|	variable A_rs  expr   { $$ = *((int *) $1) >>= $3; }
;

range:
	F_count Var_name NUM ':' NUM  { $$ = 1 + ($5 - $3); }
|	F_avg Var_name NUM ':' NUM    { $$ = parser_for_range($3, $5, $2, &p_add) / (1 + ($5 - $3)); }
|	F_min Var_name NUM ':' NUM    { $$ = parser_for_range($3, $5, $2, &p_min); }
|	F_max Var_name NUM ':' NUM    { $$ = parser_for_range($3, $5, $2, &p_max); }
|	'~' Var_name NUM ':' NUM      { $$ = PAFR($3, $5, $2, &p_rand, 1); }
|	'+' Var_name NUM ':' NUM      { $$ = parser_for_range($3, $5, $2, &p_add); }
|	'-' Var_name NUM ':' NUM      { $$ = parser_for_range($3, $5, $2, &p_sub); }
|	'<' Var_name NUM ':' NUM      { $$ = parser_for_range($3, $5, $2, &p_min); }
|	'>' Var_name NUM ':' NUM      { $$ = parser_for_range($3, $5, $2, &p_max); }
|	'&' Var_name NUM ':' NUM      { $$ = parser_for_range($3, $5, $2, &p_and); }
|	'^' Var_name NUM ':' NUM      { $$ = parser_for_range($3, $5, $2, &p_xor); }
|	'|' Var_name NUM ':' NUM      { $$ = parser_for_range($3, $5, $2, &p_or ); }
|	'*' Var_name NUM ':' NUM      { $$ = parser_for_range($3, $5, $2, &p_mul); }
|	'/' Var_name NUM ':' NUM      { yyerror(NULL, "invalid operator"); return 1; }
|	'%' Var_name NUM ':' NUM      { yyerror(NULL, "invalid operator"); return 1; }
|	Var_name NUM ':' NUM A_set expr    { $$ = PAFR($2, $4, $1, &p_assign, $6); }
|	Var_name NUM ':' NUM A_add expr    { $$ = PAFR($2, $4, $1, &p_add, $6); }
|	Var_name NUM ':' NUM A_sub expr    { $$ = PAFR($2, $4, $1, &p_sub, $6); }
|	Var_name NUM ':' NUM A_mul expr    { $$ = PAFR($2, $4, $1, &p_mul, $6); }
|	Var_name NUM ':' NUM A_div expr    { DIV_Z($6);  $$ = PAFR($2, $4, $1, &p_div, $6); }
|	Var_name NUM ':' NUM A_mod expr    { DIV_Z($6);  $$ = PAFR($2, $4, $1, &p_mod, $6); }
|	Var_name NUM ':' NUM A_and expr    { $$ = PAFR($2, $4, $1, &p_and, $6); }
|	Var_name NUM ':' NUM A_or  expr    { $$ = PAFR($2, $4, $1, &p_or , $6); }
|	Var_name NUM ':' NUM A_xor expr    { $$ = PAFR($2, $4, $1, &p_xor, $6); }
|	Var_name NUM ':' NUM A_ls  expr    { $$ = PAFR($2, $4, $1, &p_ls , $6); }
|	Var_name NUM ':' NUM A_rs  expr    { $$ = PAFR($2, $4, $1, &p_rs , $6); }
;

val:
	NUM                { $$ = $1; }
|	'.'                { $$ = (parser_ret == NULL) ? 0 : *parser_ret; }
|	'_'                { $$ = prev_value; }
;

expr:
	val                { $$ = $1; }
|	expr '+' expr      { $$ = $1 + $3; }
|	expr '-' expr      { $$ = $1 - $3; }
|	expr '&' expr      { $$ = $1 & $3; }
|	expr '^' expr      { $$ = $1 ^ $3; }
|	expr '|' expr      { $$ = $1 | $3; }
|	expr '*' expr      { $$ = $1 * $3; }
|	expr '/' expr      { DIV_Z($3);  $$ = $1 / $3; }
|	expr '%' expr      { DIV_Z($3);  $$ = $1 % $3; }
|	expr O_lsh expr    { $$ = $1 << $3; }
|	expr O_rsh expr    { $$ = $1 >> $3; }
|	expr O_pow expr    { $$ = ($3 < 0) ? 0 : ipow($1 ,$3); }
|	expr '>' expr      { $$ = $1 > $3; }
|	expr '<' expr      { $$ = $1 < $3; }
|	expr C_eq expr     { $$ = $1 == $3; }
|	expr C_ne expr     { $$ = $1 != $3; }
|	expr C_ge expr     { $$ = $1 >= $3; }
|	expr C_le expr     { $$ = $1 <= $3; }
|	expr L_and expr    { $$ = $1 && $3; }
|	expr L_or  expr    { $$ = $1 || $3; }
|	expr L_xor expr    { $$ = ($1?1:0) ^ ($3?1:0); }
|	L_not expr         { $$ = !$2; }
|	expr '?' expr ':' expr  { $$ = $1 ? $3 : $5; }
|	'(' expr ')'            { $$ = $2; }
|	function                { $$ = $1; }
|	assignment              { $$ = $1; }
|	range                   { $$ = $1; }
;


function:
	F_pow '(' expr ',' expr ')'    { $$ = ($5 < 0)? 0 : ipow($3, $5); }
|	F_abs '(' expr ')'             { $$ = abs($3); }
|	F_avg '(' expr ',' expr ')'    { $$ = ($3 + $5) / 2; }
;


%%

int
ipow(int a, int b) {
	return (b < 1 ? 1 : a * ipow(a, --b));
};


/* EOF */
