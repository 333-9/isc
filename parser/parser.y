%{
#include <stdlib.h>
#include <stdio.h>

#include "parser/range.h"

char *parser_input_str;

extern void  yyerror(int *, char *s);
   
int vars[25] = {0};


int yylex();


#define DIV_Z(i)  if (i == 0) { yyerror(NULL, "division by zero error"); return (-1); }
#define PAFR(a, b, c, d, e)  parser_assign_for_range(a, b, c, d, e)

%}

%parse-param { int *parser_ret }

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


program: '$'             { return 0; }
|	expr '$'         { if (parser_ret != NULL) *parser_ret = $1; return 0; }
|	/* NOP */        { return -1; }
;

variable:
	Var_name { $$ = (int) (vars + $1); }
|	Var_name NUM { if(($$ = (int) parser_get_num($2, $1)) == NULL) return -1; }
;

assignment: variable '=' expr  { $$ = *((int *) $1)   = $3; }
|	variable AO_add expr   { $$ = *((int *) $1)  += $3; }
|	variable AO_sub expr   { $$ = *((int *) $1)  -= $3; }
|	variable AO_mul expr   { $$ = *((int *) $1)  *= $3; }
|	variable AO_div expr   { DIV_Z(!$3);  $$ = *((int *) $1) /= $3; }
|	variable AO_mod expr   { DIV_Z(!$3);  $$ = *((int *) $1) %= $3; }
|	variable AO_and expr   { $$ = *((int *) $1)  &= $3; }
|	variable AO_or  expr   { $$ = *((int *) $1)  ^= $3; }
|	variable AO_xor expr   { $$ = *((int *) $1)  |= $3; }
|	variable AO_ls  expr   { $$ = *((int *) $1) <<= $3; }
|	variable AO_rs  expr   { $$ = *((int *) $1) >>= $3; }
;

range:  '[' Var_name NUM ':' NUM '~' ']'  { $$ = PAFR($3, $5, $2, &p_rand, $7); }
|	'[' '+' Var_name NUM ':' NUM ']'  { $$ = parser_for_range($4, $6, $3, &p_add); }
|	'[' '-' Var_name NUM ':' NUM ']'  { $$ = parser_for_range($4, $6, $3, &p_sub); }
|	'[' '>' Var_name NUM ':' NUM ']'  { $$ = parser_for_range($4, $6, $3, &p_max); }
|	'[' '<' Var_name NUM ':' NUM ']'  { $$ = parser_for_range($4, $6, $3, &p_min); }
|	'[' '&' Var_name NUM ':' NUM ']'  { $$ = parser_for_range($4, $6, $3, &p_and); }
|	'[' '^' Var_name NUM ':' NUM ']'  { $$ = parser_for_range($4, $6, $3, &p_xor); }
|	'[' '|' Var_name NUM ':' NUM ']'  { $$ = parser_for_range($4, $6, $3, &p_or ); }
|	'[' '*' Var_name NUM ':' NUM ']'  { $$ = parser_for_range($4, $6, $3, &p_mul); }
|	'[' '/' Var_name NUM ':' NUM ']'  { yyerror(NULL, "invalid operator"); return -2; }
|	'[' '%' Var_name NUM ':' NUM ']'  { yyerror(NULL, "invalid operator"); return -2; }
|	'[' Var_name NUM ':' NUM '=' expr ']'     { $$ = PAFR($3, $5, $2, &p_assign, $7); }
|	'[' Var_name NUM ':' NUM AO_add expr ']'  { $$ = PAFR($3, $5, $2, &p_add, $7); }
|	'[' Var_name NUM ':' NUM AO_sub expr ']'  { $$ = PAFR($3, $5, $2, &p_sub, $7); }
|	'[' Var_name NUM ':' NUM AO_mul expr ']'  { $$ = PAFR($3, $5, $2, &p_mul, $7); }
|	'[' Var_name NUM ':' NUM AO_div expr ']'  { DIV_Z($7);  $$ = PAFR($3, $5, $2, &p_div, $7); }
|	'[' Var_name NUM ':' NUM AO_mod expr ']'  { DIV_Z($7);  $$ = PAFR($3, $5, $2, &p_mod, $7); }
|	'[' Var_name NUM ':' NUM AO_and expr ']'  { $$ = PAFR($3, $5, $2, &p_and, $7); }
|	'[' Var_name NUM ':' NUM AO_or  expr ']'  { $$ = PAFR($3, $5, $2, &p_or , $7); }
|	'[' Var_name NUM ':' NUM AO_xor expr ']'  { $$ = PAFR($3, $5, $2, &p_xor, $7); }
|	'[' Var_name NUM ':' NUM AO_ls  expr ']'  { $$ = PAFR($3, $5, $2, &p_ls , $7); }
|	'[' Var_name NUM ':' NUM AO_rs  expr ']'  { $$ = PAFR($3, $5, $2, &p_rs , $7); }
;

expr: NUM { $$ = $1; }
|	expr '+' expr      { $$ = $1 + $3; }
|	expr '-' expr      { $$ = $1 - $3; }
|	expr '*' expr      { $$ = $1 * $3; }
|	expr '&' expr      { $$ = $1 & $3; }
|	expr '^' expr      { $$ = $1 ^ $3; }
|	expr '|' expr      { $$ = $1 | $3; }
|	expr L_Shift expr  { $$ = $1 << $3; }
|	expr R_Shift expr  { $$ = $1 >> $3; }
|	expr '%' expr      { DIV_Z($3);  $$ = $1 % $3; }
|	expr '/' expr      { DIV_Z($3);  $$ = $1 / $3; }
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
|	variable                { $$ = *((int *) $1); }
|	assignment              { $$ = $1; }
|	range                   { $$ = $1; }
;


%%
/* EOF */
