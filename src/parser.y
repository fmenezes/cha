%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ni/ast.h"
#include "log.h"

ni_ast_node_list *parsed_ast;

extern int yylex();
int yyerror(const char *msg);
%}

%code{
# include "ni/ast.h"
}

%union {
  char *str;
  ni_ast_node* node;
  ni_ast_node_list* list;
}

%token FUN OPENPAR CLOSEPAR OPENCUR CLOSECUR COMMA VAR EQUALS RET PLUS MINUS MULTIPLY INT
%token <str> IDENTIFIER NUMBER

%nterm <list> instructions block def_args call_args statements
%nterm <node> function statement arg expr typedef const


%left PLUS MINUS
%left MULTIPLY

%start parse

%%

parse :
	instructions																	{ parsed_ast = $1; }
	;

instructions :
	function																		{ $$ = make_ni_ast_node_list($1); }
	| instructions function															{ $$ = $1; ni_ast_node_list_append($$, $2); }
	;

function :
	FUN IDENTIFIER OPENPAR CLOSEPAR block											{ $$ = make_ni_ast_node_function_declaration((ni_ast_location){}, $2, NULL, NULL, $5); }
	| FUN IDENTIFIER OPENPAR def_args CLOSEPAR block								{ $$ = make_ni_ast_node_function_declaration((ni_ast_location){}, $2, NULL, $4, $6); }
	| FUN IDENTIFIER OPENPAR CLOSEPAR typedef block									{ $$ = make_ni_ast_node_function_declaration((ni_ast_location){}, $2, $5, NULL, $6); }
	| FUN IDENTIFIER OPENPAR def_args CLOSEPAR typedef block						{ $$ = make_ni_ast_node_function_declaration((ni_ast_location){}, $2, $6, $4, $7); }
	;

block :
	OPENCUR statements CLOSECUR														{ $$ = $2; }
	| OPENCUR CLOSECUR																{ $$ = NULL; }
	;

arg :
	IDENTIFIER typedef																{ $$ = make_ni_ast_node_argument((ni_ast_location){}, $1, $2); }
	;

def_args :
	arg																				{ $$ = make_ni_ast_node_list($1); }
	| def_args COMMA arg															{ $$ = $1; ni_ast_node_list_append($$, $3); }
	;

call_args :
	expr																			{ $$ = make_ni_ast_node_list($1); }
	| call_args COMMA expr															{ $$ = $1; ni_ast_node_list_append($$, $3); }
	;

statements :
	statement																		{ $$ = make_ni_ast_node_list($1); }
	| statements statement															{ $$ = $1; ni_ast_node_list_append($$, $2); }
	;

statement :
	VAR IDENTIFIER typedef															{ $$ = make_ni_ast_node_variable_declaration((ni_ast_location){}, $2, $3); }
	| IDENTIFIER EQUALS expr														{ $$ = make_ni_ast_node_variable_assignment((ni_ast_location){}, $1, $3); }
	| expr																			{ $$ = $1; }
	| RET expr																		{ $$ = make_ni_ast_node_function_return((ni_ast_location){}, $2); }
	| RET 																			{ $$ = make_ni_ast_node_function_return((ni_ast_location){}, NULL); }
	;

expr :
	const																			{ $$ = $1; }
	| IDENTIFIER																	{ $$ = make_ni_ast_node_variable_lookup((ni_ast_location){}, $1); }
	| IDENTIFIER OPENPAR CLOSEPAR													{ $$ = make_ni_ast_node_function_call((ni_ast_location){}, $1, NULL); }
	| IDENTIFIER OPENPAR call_args CLOSEPAR											{ $$ = make_ni_ast_node_function_call((ni_ast_location){}, $1, $3); }
	| expr PLUS expr																{ $$ = make_ni_ast_node_bin_op((ni_ast_location){}, NI_AST_OPERATOR_PLUS, $1, $3); }
	| expr MINUS expr																{ $$ = make_ni_ast_node_bin_op((ni_ast_location){}, NI_AST_OPERATOR_MINUS, $1, $3); }
	| expr MULTIPLY expr															{ $$ = make_ni_ast_node_bin_op((ni_ast_location){}, NI_AST_OPERATOR_MULTIPLY, $1, $3); }
	| OPENPAR expr CLOSEPAR															{ $$ = $2; }
	;

typedef :
	INT																				{ $$ = make_ni_ast_node_int_type((ni_ast_location){}); }
	;

const :
	NUMBER																			{ $$ = make_ni_ast_node_int_const((ni_ast_location){}, $1); }
	;

%%

int yyerror(const char *msg) {
  log_error(msg);
  return 1;
}
