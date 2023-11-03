%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "parser.h"
#include "log.h"

ni_ast_node_list *parsed_ast;

extern int yylex();
extern FILE *yyin;
const char *current_file;

int yyerror(const char *msg);
%}

%locations

%code{
# include "ast.h"
ni_ast_location convert_location(YYLTYPE start, YYLTYPE end);
}

%union {
  char *str;
  ni_ast_node* node;
  ni_ast_type* type;
  ni_ast_node_list* list;
}

%token FUN OPENPAR CLOSEPAR OPENCUR CLOSECUR COMMA VAR EQUALS RET PLUS MINUS MULTIPLY REFTYPE_INT8 REFTYPE_UINT8 REFTYPE_INT16 REFTYPE_UINT16 REFTYPE_INT32 REFTYPE_UINT32 REFTYPE_INT64 REFTYPE_UINT64 REFTYPE_INT128 REFTYPE_UINT128 REFTYPE_FLOAT32 REFTYPE_FLOAT64
%token <str> IDENTIFIER NUMBER FLOAT

%nterm <list> instructions block def_args call_args statements
%nterm <node> function statement arg expr const
%nterm <type> reftype

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
	FUN IDENTIFIER OPENPAR CLOSEPAR block											{ $$ = make_ni_ast_node_function_declaration(convert_location(@1, @5), $2, NULL, NULL, $5); free($2); }
	| FUN IDENTIFIER OPENPAR def_args CLOSEPAR block								{ $$ = make_ni_ast_node_function_declaration(convert_location(@1, @6), $2, NULL, $4, $6); free($2); }
	| FUN IDENTIFIER OPENPAR CLOSEPAR reftype block									{ $$ = make_ni_ast_node_function_declaration(convert_location(@1, @6), $2, $5, NULL, $6); free($2); }
	| FUN IDENTIFIER OPENPAR def_args CLOSEPAR reftype block						{ $$ = make_ni_ast_node_function_declaration(convert_location(@1, @7), $2, $6, $4, $7); free($2); }
	;

block :
	OPENCUR statements CLOSECUR														{ $$ = $2; }
	| OPENCUR CLOSECUR																{ $$ = NULL; }
	;

arg :
	IDENTIFIER reftype																{ $$ = make_ni_ast_node_argument(convert_location(@1, @2), $1, $2); free($1); }
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
	VAR IDENTIFIER reftype															{ $$ = make_ni_ast_node_variable_declaration(convert_location(@1, @3), $2, $3); free($2); }
	| IDENTIFIER EQUALS expr														{ $$ = make_ni_ast_node_variable_assignment(convert_location(@1, @3), $1, $3); free($1); }
	| expr																			{ $$ = $1; }
	| RET expr																		{ $$ = make_ni_ast_node_function_return(convert_location(@1, @2), $2); }
	| RET 																			{ $$ = make_ni_ast_node_function_return(convert_location(@1, @1), NULL); }
	;

expr :
	const																			{ $$ = $1; }
	| IDENTIFIER																	{ $$ = make_ni_ast_node_variable_lookup(convert_location(@1, @1), $1); free($1); }
	| IDENTIFIER OPENPAR CLOSEPAR													{ $$ = make_ni_ast_node_function_call(convert_location(@1, @3), $1, NULL); free($1); }
	| IDENTIFIER OPENPAR call_args CLOSEPAR											{ $$ = make_ni_ast_node_function_call(convert_location(@1, @4), $1, $3); free($1); }
	| expr PLUS expr																{ $$ = make_ni_ast_node_bin_op(convert_location(@1, @3), NI_AST_OPERATOR_PLUS, $1, $3); }
	| expr MINUS expr																{ $$ = make_ni_ast_node_bin_op(convert_location(@1, @3), NI_AST_OPERATOR_MINUS, $1, $3); }
	| expr MULTIPLY expr															{ $$ = make_ni_ast_node_bin_op(convert_location(@1, @3), NI_AST_OPERATOR_MULTIPLY, $1, $3); }
	| OPENPAR expr CLOSEPAR															{ $$ = $2; }
	;

reftype :
	REFTYPE_INT8																	{ $$ = make_ni_ast_type_int8(convert_location(@1, @1)); }
	| REFTYPE_UINT8																	{ $$ = make_ni_ast_type_uint8(convert_location(@1, @1)); }
	| REFTYPE_INT16																	{ $$ = make_ni_ast_type_int16(convert_location(@1, @1)); }
	| REFTYPE_UINT16																{ $$ = make_ni_ast_type_uint16(convert_location(@1, @1)); }
	| REFTYPE_INT32																	{ $$ = make_ni_ast_type_int32(convert_location(@1, @1)); }
	| REFTYPE_UINT32																{ $$ = make_ni_ast_type_uint32(convert_location(@1, @1)); }
	| REFTYPE_INT64																	{ $$ = make_ni_ast_type_int64(convert_location(@1, @1)); }
	| REFTYPE_UINT64																{ $$ = make_ni_ast_type_uint64(convert_location(@1, @1)); }
	| REFTYPE_INT128																{ $$ = make_ni_ast_type_int128(convert_location(@1, @1)); }
	| REFTYPE_UINT128																{ $$ = make_ni_ast_type_uint128(convert_location(@1, @1)); }
	| REFTYPE_FLOAT32																{ $$ = make_ni_ast_type_float32(convert_location(@1, @1)); }
	| REFTYPE_FLOAT64																{ $$ = make_ni_ast_type_float64(convert_location(@1, @1)); }
	;

const :
	NUMBER																			{ $$ = make_ni_ast_node_constant_number(convert_location(@1, @1), $1); free($1); }
	| FLOAT																			{ $$ = make_ni_ast_node_constant_float(convert_location(@1, @1), $1); free($1); }
	;

%%

int yyerror(const char *msg) {
  log_error(msg);
  return 1;
}

ni_ast_location convert_location(YYLTYPE start, YYLTYPE end) {
  ni_ast_location location;
  sprintf(location.file, "%s", current_file);
  location.line_begin = start.first_line;
  location.column_begin = start.first_column;
  location.line_end = end.last_line;
  location.column_end = end.last_column;
  return location;
}

int ni_parse(const char *file, ni_ast_node_list **out) {
  current_file = file;
  FILE *f = fopen(file, "r");
  if (f == NULL) {
    log_error("Could not open file %s\n", file);
    return 1;
  }

  yyin = f;
  int ret = yyparse();
  fclose(f);
  if (ret != 0) {
    return 1;
  }
  *out = parsed_ast;
  return 0;
}
