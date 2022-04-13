%{
#include <iostream>
#include <string>
#include <memory>
#include "ast/ast.hh"
#include "ast/parser.hh"

using namespace std::string_literals;
%}

%require "3.2"
%language "c++"

%locations

%define api.value.type variant
%define api.token.constructor

%nterm <std::unique_ptr<ni::ast::expression>> expr;
%nterm <std::unique_ptr<ni::ast::constant>> const;
%nterm <std::unique_ptr<ni::ast::statement>> statement;
%nterm <std::unique_ptr<ni::ast::function_declaration>> function;
%nterm <std::unique_ptr<ni::ast::block>> block;
%nterm <std::vector<std::unique_ptr<ni::ast::statement>>> statements;
%nterm <std::unique_ptr<ni::ast::node_type>> typedef;
%nterm <std::vector<std::unique_ptr<ni::ast::argument>>> def_args;
%nterm <std::vector<std::unique_ptr<ni::ast::expression>>> call_args;
%nterm <std::vector<std::unique_ptr<ni::ast::function_declaration>>> instructions;
%nterm <std::unique_ptr<ni::ast::program>> program;

%code {
# include "ast/ast.hh"
# include "ast/parser.hh"
# include "ast/parserdecl.h"
}

%param { ni::ast::parser &p }

%token <std::string> CONST_INTEGER IDENTIFIER
%token VAR PLUS MINUS MULTIPLY OPENPAR CLOSEPAR EQUALS OPENCUR CLOSECUR FUN RET COMMA INT
%token YYEOF 0

%left PLUS MINUS
%left MULTIPLY

%start parse

%%

parse :
	program																			{ p.prg = std::move($1); }
	;

program :
	instructions																	{ $$ = std::make_unique<ni::ast::program>($1, yy::location($1.front()->location.begin, $1.back()->location.end)); }
	;

instructions :
	function																		{ $$.push_back(std::move($1)); }
	| instructions function															{ $$ = std::move($1); $$.push_back(std::move($2)); }
	;

function :
	FUN IDENTIFIER OPENPAR CLOSEPAR block											{ $$ = std::make_unique<ni::ast::function_declaration>($2, $5, yy::location(@1.begin, @5.end)); }
	| FUN IDENTIFIER OPENPAR def_args CLOSEPAR block								{ $$ = std::make_unique<ni::ast::function_declaration>($2, $4, $6, yy::location(@1.begin, @6.end)); }
	| FUN IDENTIFIER OPENPAR CLOSEPAR typedef block									{ $$ = std::make_unique<ni::ast::function_declaration>($2, $5, $6, yy::location(@1.begin, @6.end)); }
	| FUN IDENTIFIER OPENPAR def_args CLOSEPAR typedef block						{ $$ = std::make_unique<ni::ast::function_declaration>($2, $4, $6, $7, yy::location(@1.begin, @7.end)); }
	;

block :
	OPENCUR statements CLOSECUR														{ $$ = std::make_unique<ni::ast::block>($2, yy::location(@1.begin, @3.end)); }
	| OPENCUR CLOSECUR																{ std::vector<std::unique_ptr<ni::ast::statement>> e;
																					  $$ = std::make_unique<ni::ast::block>(e, yy::location(@1.begin, @2.end)); }
	;

def_args :
	IDENTIFIER typedef																{ $$.push_back(std::move(std::make_unique<ni::ast::argument>($1, $2, yy::location(@1.begin, @2.end)))); }
	| def_args COMMA IDENTIFIER typedef												{ $$ = std::move($1); $$.push_back(std::move(std::make_unique<ni::ast::argument>($3, $4, yy::location(@3.begin, @4.end)))); }
	;

call_args :
	expr																			{ $$.push_back(std::move($1)); }
	| call_args COMMA expr															{ $$ = std::move($1); $$.push_back(std::move($3)); }
	;

statements :
	statement																		{ $$.push_back(std::move($1)); }
	| statements statement															{ $$ = std::move($1); $$.push_back(std::move($2)); }
	;

statement :
	VAR IDENTIFIER typedef															{ $$ = std::make_unique<ni::ast::variable_declaration>($2, $3, yy::location(@1.begin, @3.end)); }
	| IDENTIFIER EQUALS expr														{ $$ = std::make_unique<ni::ast::variable_assignment>($1, $3, yy::location(@1.begin, @3.end)); }
	| expr																			{ $$ = std::move($1); }
	| RET expr																		{ $$ = std::make_unique<ni::ast::function_return>($2, yy::location(@1.begin, @2.end)); }
	| RET 																			{ $$ = std::make_unique<ni::ast::function_return>(@1); }
	;

expr :
	const																			{ $$ = std::move($1); }
	| IDENTIFIER																	{ $$ = std::make_unique<ni::ast::variable_lookup>($1, @1); }
	| IDENTIFIER OPENPAR CLOSEPAR													{ $$ = std::make_unique<ni::ast::function_call>($1, yy::location(@1.begin, @3.end)); }
	| IDENTIFIER OPENPAR call_args CLOSEPAR											{ $$ = std::make_unique<ni::ast::function_call>($1, $3, yy::location(@1.begin, @4.end)); }
	| expr PLUS expr																{ $$ = std::make_unique<ni::ast::binary_operation>("+"s, $1, $3, yy::location(@1.begin, @3.end)); }
	| expr MINUS expr																{ $$ = std::make_unique<ni::ast::binary_operation>("-"s, $1, $3, yy::location(@1.begin, @3.end)); }
	| expr MULTIPLY expr															{ $$ = std::make_unique<ni::ast::binary_operation>("*"s, $1, $3, yy::location(@1.begin, @3.end)); }
	| OPENPAR expr CLOSEPAR															{ $$ = std::move($2); }
	;

typedef :
	INT																				{ $$ = std::make_unique<ni::ast::integer>(@1); }
	;

const :
	CONST_INTEGER																	{ $$ = std::make_unique<ni::ast::constant_integer>($1, @1); }
	;

%%

void yy::parser::error (const location_type& loc, const std::string& m)
{
  throw yy::parser::syntax_error(loc, "invalid syntax: " + m);
}
