%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "parser.h"
#include "log.h"

cha_ast_node_list *parsed_ast;

extern int yylex();
extern FILE *yyin;
const char *current_file;

int yyerror(const char *msg);
%}

%locations

%code{
# include "ast.h"

cha_ast_location convert_location(YYLTYPE start, YYLTYPE end);
}

%union {
  char *str;
  cha_ast_node* node;
  cha_ast_type* type;
  cha_ast_node_list* list;
}

%token KEYWORD_FUN OPEN_PAR CLOSE_PAR OPEN_CUR CLOSE_CUR COMMA KEYWORD_VAR EQUALS KEYWORD_RET ADD SUBTRACT MULTIPLY DIVIDE REFTYPE_INT8 REFTYPE_UINT8 REFTYPE_INT16 REFTYPE_UINT16 REFTYPE_INT32 REFTYPE_UINT32 REFTYPE_INT64 REFTYPE_UINT64 REFTYPE_INT REFTYPE_UINT REFTYPE_FLOAT16 REFTYPE_FLOAT32 REFTYPE_FLOAT64 REFTYPE_BOOL BOOL_TRUE BOOL_FALSE EQUALS_EQUALS NOT_EQUALS GREATER_THAN GREATER_THAN_OR_EQUALS LESS_THAN LESS_THAN_OR_EQUALS AND OR KEYWORD_CONST KEYWORD_IF KEYWORD_ELSE
%token <str> IDENTIFIER INTEGER UINTEGER FLOAT

%nterm <list> top_level block def_args call_args statements
%nterm <node> instruction const_definition function statement arg expr const_value
%nterm <type> reftype

%left ADD SUBTRACT
%left MULTIPLY DIVIDE

%start parse

%%

parse :
	top_level																		{ parsed_ast = $1; }
	;

top_level :
	instruction																		{ $$ = make_cha_ast_node_list($1); }
	| top_level instruction															{ $$ = $1; cha_ast_node_list_append($$, $2); }
	;

instruction :
	const_definition																{ $$ = $1; }
	| function																		{ $$ = $1; }
	;

const_definition :
	KEYWORD_CONST IDENTIFIER EQUALS const_value										{ $$ = make_cha_ast_node_constant_declaration(convert_location(@1, @4), $2, $4); }
	;

function :
	KEYWORD_FUN IDENTIFIER OPEN_PAR CLOSE_PAR block									{ $$ = make_cha_ast_node_function_declaration(convert_location(@1, @5), $2, NULL, NULL, $5); }
	| KEYWORD_FUN IDENTIFIER OPEN_PAR def_args CLOSE_PAR block						{ $$ = make_cha_ast_node_function_declaration(convert_location(@1, @6), $2, NULL, $4, $6); }
	| KEYWORD_FUN IDENTIFIER OPEN_PAR CLOSE_PAR reftype block						{ $$ = make_cha_ast_node_function_declaration(convert_location(@1, @6), $2, $5, NULL, $6); }
	| KEYWORD_FUN IDENTIFIER OPEN_PAR def_args CLOSE_PAR reftype block				{ $$ = make_cha_ast_node_function_declaration(convert_location(@1, @7), $2, $6, $4, $7); }
	;

block :
	OPEN_CUR statements CLOSE_CUR													{ $$ = $2; }
	| OPEN_CUR CLOSE_CUR															{ $$ = NULL; }
	;

arg :
	IDENTIFIER reftype																{ $$ = make_cha_ast_node_argument(convert_location(@1, @2), $1, $2); }
	;

def_args :
	arg																				{ $$ = make_cha_ast_node_list($1); }
	| def_args COMMA arg															{ $$ = $1; cha_ast_node_list_append($$, $3); }
	;

call_args :
	expr																			{ $$ = make_cha_ast_node_list($1); }
	| call_args COMMA expr															{ $$ = $1; cha_ast_node_list_append($$, $3); }
	;

statements :
	statement																		{ $$ = make_cha_ast_node_list($1); }
	| statements statement															{ $$ = $1; cha_ast_node_list_append($$, $2); }
	;

statement :
	KEYWORD_VAR IDENTIFIER reftype													{ $$ = make_cha_ast_node_variable_declaration(convert_location(@1, @3), $2, $3, NULL); }
	| KEYWORD_VAR IDENTIFIER reftype EQUALS expr									{ $$ = make_cha_ast_node_variable_declaration(convert_location(@1, @5), $2, $3, $5); }
	| IDENTIFIER EQUALS expr														{ $$ = make_cha_ast_node_variable_assignment(convert_location(@1, @3), $1, $3); }
	| expr																			{ $$ = $1; }
	| KEYWORD_RET expr																{ $$ = make_cha_ast_node_function_return(convert_location(@1, @2), $2); }
	| KEYWORD_RET 																	{ $$ = make_cha_ast_node_function_return(convert_location(@1, @1), NULL); }
	| KEYWORD_IF expr block															{ $$ = make_cha_ast_node_if(convert_location(@1, @3), $2, $3, NULL); }
	| KEYWORD_IF expr block KEYWORD_ELSE block										{ $$ = make_cha_ast_node_if(convert_location(@1, @5), $2, $3, $5); }
	;

expr :
	const_value																		{ $$ = $1; }
	| IDENTIFIER																	{ $$ = make_cha_ast_node_variable_lookup(convert_location(@1, @1), $1); }
	| IDENTIFIER OPEN_PAR CLOSE_PAR													{ $$ = make_cha_ast_node_function_call(convert_location(@1, @3), $1, NULL); }
	| IDENTIFIER OPEN_PAR call_args CLOSE_PAR										{ $$ = make_cha_ast_node_function_call(convert_location(@1, @4), $1, $3); }
	| expr ADD expr																	{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_ADD, $1, $3); }
	| expr SUBTRACT expr															{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_SUBTRACT, $1, $3); }
	| expr MULTIPLY expr															{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_MULTIPLY, $1, $3); }
	| expr DIVIDE expr																{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_DIVIDE, $1, $3); }
	| expr EQUALS_EQUALS expr														{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_EQUALS_EQUALS, $1, $3); }
	| expr NOT_EQUALS expr															{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_NOT_EQUALS, $1, $3); }
	| expr GREATER_THAN expr														{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_GREATER_THAN, $1, $3); }
	| expr GREATER_THAN_OR_EQUALS expr												{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_GREATER_THAN_OR_EQUALS, $1, $3); }
	| expr LESS_THAN expr															{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_LESS_THAN, $1, $3); }
	| expr LESS_THAN_OR_EQUALS expr													{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_LESS_THAN_OR_EQUALS, $1, $3); }
	| expr AND expr																	{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_AND, $1, $3); }
	| expr OR expr																	{ $$ = make_cha_ast_node_bin_op(convert_location(@1, @3), CHA_AST_OPERATOR_OR, $1, $3); }
	| OPEN_PAR expr CLOSE_PAR														{ $$ = $2; }
	;

reftype :
	REFTYPE_INT																		{ $$ = make_cha_ast_type_int(convert_location(@1, @1)); }
	| REFTYPE_UINT																	{ $$ = make_cha_ast_type_uint(convert_location(@1, @1)); }
	| REFTYPE_INT8																	{ $$ = make_cha_ast_type_int8(convert_location(@1, @1)); }
	| REFTYPE_UINT8																	{ $$ = make_cha_ast_type_uint8(convert_location(@1, @1)); }
	| REFTYPE_INT16																	{ $$ = make_cha_ast_type_int16(convert_location(@1, @1)); }
	| REFTYPE_UINT16																{ $$ = make_cha_ast_type_uint16(convert_location(@1, @1)); }
	| REFTYPE_INT32																	{ $$ = make_cha_ast_type_int32(convert_location(@1, @1)); }
	| REFTYPE_UINT32																{ $$ = make_cha_ast_type_uint32(convert_location(@1, @1)); }
	| REFTYPE_INT64																	{ $$ = make_cha_ast_type_int64(convert_location(@1, @1)); }
	| REFTYPE_UINT64																{ $$ = make_cha_ast_type_uint64(convert_location(@1, @1)); }
	| REFTYPE_FLOAT16																{ $$ = make_cha_ast_type_float16(convert_location(@1, @1)); }
	| REFTYPE_FLOAT32																{ $$ = make_cha_ast_type_float32(convert_location(@1, @1)); }
	| REFTYPE_FLOAT64																{ $$ = make_cha_ast_type_float64(convert_location(@1, @1)); }
	| REFTYPE_BOOL																	{ $$ = make_cha_ast_type_bool(convert_location(@1, @1)); }
	;

const_value :
	INTEGER																			{ $$ = make_cha_ast_node_constant_integer(convert_location(@1, @1), $1); }
	| UINTEGER																		{ $$ = make_cha_ast_node_constant_unsigned_integer(convert_location(@1, @1), $1); }
	| FLOAT																			{ $$ = make_cha_ast_node_constant_float(convert_location(@1, @1), $1); }
	| BOOL_TRUE																		{ $$ = make_cha_ast_node_constant_true(convert_location(@1, @1)); }
	| BOOL_FALSE																	{ $$ = make_cha_ast_node_constant_false(convert_location(@1, @1)); }
	;

%%

int yyerror(const char *msg) {
  cha_ast_location location;
  sprintf(location.file, "%s", current_file);
  location.line_begin = yylloc.first_line;
  location.column_begin = yylloc.first_column;
  location.line_end = yylloc.last_line;
  location.column_end = yylloc.last_column;
  
  log_validation_error(location, msg);
  return 1;
}

cha_ast_location convert_location(YYLTYPE start, YYLTYPE end) {
  cha_ast_location location;
  sprintf(location.file, "%s", current_file);
  location.line_begin = start.first_line;
  location.column_begin = start.first_column;
  location.line_end = end.last_line;
  location.column_end = end.last_column;
  return location;
}

int cha_parse(const char *file, cha_ast_node_list **out) {
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
