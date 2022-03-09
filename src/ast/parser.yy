%{
#include <iostream>
#include <string>
#include <memory>
#include "ast/ast.hh"

using namespace std::string_literals;
%}

%require "3.2"
%language "c++"

%define api.value.type variant
%define api.token.constructor

%nterm <std::unique_ptr<ni::ast::NExpression>> expr;
%nterm <std::unique_ptr<ni::ast::NConstant>> const;
%nterm <std::unique_ptr<ni::ast::NStatement>> statement;
%nterm <std::unique_ptr<ni::ast::NFunctionDeclaration>> function;
%nterm <std::vector<std::unique_ptr<ni::ast::NStatement>>> statements;
%nterm <std::unique_ptr<ni::ast::NType>> typedef;
%nterm <std::vector<std::unique_ptr<ni::ast::NArgument>>> def_args;
%nterm <std::vector<std::unique_ptr<ni::ast::NExpression>>> call_args;
%nterm <std::vector<std::unique_ptr<ni::ast::NFunctionDeclaration>>> instructions;

%code {
# include "ast/parserdecl.h"
}

%param { ni::ast::Parser &p }

%token <std::string> CONST_INTEGER IDENTIFIER
%token VAR PLUS MINUS MULTIPLY OPENPAR CLOSEPAR EQUALS OPENCUR CLOSECUR FUN RET COMMA INT

%left PLUS MINUS
%left MULTIPLY

%start program

%%

program :
	instructions																	{ p.program.instructions = std::move($1); }
	;

instructions :
	function																		{ $$.push_back(std::move($1)); }
	| instructions function															{ $$ = std::move($1); $$.push_back(std::move($2)); }
	;

function :
	FUN IDENTIFIER OPENPAR CLOSEPAR OPENCUR statements CLOSECUR						{ $$ = std::make_unique<ni::ast::NFunctionDeclaration>($2, $6); }
	| FUN IDENTIFIER OPENPAR def_args CLOSEPAR OPENCUR statements CLOSECUR			{ $$ = std::make_unique<ni::ast::NFunctionDeclaration>($2, $4, $7); }
	| FUN IDENTIFIER OPENPAR CLOSEPAR typedef OPENCUR statements CLOSECUR			{ $$ = std::make_unique<ni::ast::NFunctionDeclaration>($2, $5, $7); }
	| FUN IDENTIFIER OPENPAR def_args CLOSEPAR typedef OPENCUR statements CLOSECUR	{ $$ = std::make_unique<ni::ast::NFunctionDeclaration>($2, $4, $6, $8); }
	;

def_args :
	IDENTIFIER typedef																{ $$.push_back(std::move(std::make_unique<ni::ast::NArgument>($1, $2))); }
	| def_args COMMA IDENTIFIER typedef												{ $$ = std::move($1); $$.push_back(std::move(std::make_unique<ni::ast::NArgument>($3, $4))); }
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
	VAR IDENTIFIER typedef															{ $$ = std::make_unique<ni::ast::NVariableDeclaration>($2, $3); }
	| IDENTIFIER EQUALS expr														{ $$ = std::make_unique<ni::ast::NVariableAssignment>($1, $3); }
	| expr																			{ $$ = std::move($1); }
	| RET expr																		{ $$ = std::make_unique<ni::ast::NFunctionReturn>($2); }
	| RET 																			{ $$ = std::make_unique<ni::ast::NFunctionReturn>(); }
	;

expr :
	const																			{ $$ = std::move($1); }
	| IDENTIFIER																	{ $$ = std::make_unique<ni::ast::NVariableLookup>($1); }
	| IDENTIFIER OPENPAR CLOSEPAR													{ $$ = std::make_unique<ni::ast::NFunctionCall>($1); }
	| IDENTIFIER OPENPAR call_args CLOSEPAR											{ $$ = std::make_unique<ni::ast::NFunctionCall>($1, $3); }
	| expr PLUS expr																{ $$ = std::make_unique<ni::ast::NBinaryOperation>("+"s, $1, $3); }
	| expr MINUS expr																{ $$ = std::make_unique<ni::ast::NBinaryOperation>("-"s, $1, $3); }
	| expr MULTIPLY expr															{ $$ = std::make_unique<ni::ast::NBinaryOperation>("*"s, $1, $3); }
	| OPENPAR expr CLOSEPAR															{ $$ = std::move($2); }
	;

typedef :
	INT																				{ $$ = std::make_unique<ni::ast::NInteger>(); }
	;

const :
	CONST_INTEGER																	{ $$ = std::make_unique<ni::ast::NConstantInteger>($1); }
	;

%%

void yy::parser::error (const std::string& m)
{
  std::cerr << m << '\n';
}
