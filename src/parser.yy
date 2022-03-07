%{
#include <iostream>
#include <string>
#include <memory>
#include "nodes.hh"

using namespace std::string_literals;

extern ni::NProgram *program;
%}

%require "3.2"
%language "c++"

%define api.value.type variant
%define api.token.constructor

%nterm <std::unique_ptr<ni::NExpression>> expr;
%nterm <std::unique_ptr<ni::NConstant>> const;
%nterm <std::unique_ptr<ni::NStatement>> statement;
%nterm <std::unique_ptr<ni::NFunctionDeclaration>> function;
%nterm <std::vector<std::unique_ptr<ni::NStatement>>> statements;
%nterm <std::unique_ptr<ni::NType>> typedef;
%nterm <std::vector<std::unique_ptr<ni::NArgument>>> def_args;
%nterm <std::vector<std::unique_ptr<ni::NExpression>>> call_args;

%code {
# include "parserdecl.h"
}

%param { ni::NProgram& p }

%token <std::string> CONST_INTEGER IDENTIFIER
%token VAR PLUS MINUS MULTIPLY OPENPAR CLOSEPAR EQUALS OPENCUR CLOSECUR FUN RET COMMA INT

%left PLUS MINUS
%left MULTIPLY

%start program

%%

program :
	function { p.instructions.push_back(std::move($1)); }
	| program function { p.instructions.push_back(std::move($2)); }
	;

function :
	FUN IDENTIFIER OPENPAR CLOSEPAR OPENCUR statements CLOSECUR	{ $$ = std::make_unique<ni::NFunctionDeclaration>($2, $6); }
	| FUN IDENTIFIER OPENPAR def_args CLOSEPAR OPENCUR statements CLOSECUR	{ $$ = std::make_unique<ni::NFunctionDeclaration>($2, $4, $7); }
	;

def_args :
	IDENTIFIER typedef { $$.push_back(std::move(std::make_unique<ni::NArgument>($1, $2))); }
	| def_args COMMA IDENTIFIER typedef { $$ = std::move($1); $$.push_back(std::move(std::make_unique<ni::NArgument>($3, $4))); }
	;

call_args :
	expr { $$.push_back(std::move($1)); }
	| call_args COMMA expr { $$ = std::move($1); $$.push_back(std::move($3)); }
	;

statements :
	statement	{ $$.push_back(std::move($1)); }
	| statements statement	{ $$ = std::move($1); $$.push_back(std::move($2)); }
	;

statement :
	VAR IDENTIFIER typedef		{ $$ = std::make_unique<ni::NVariableDeclaration>($2, $3); }
	| IDENTIFIER EQUALS expr	{ $$ = std::make_unique<ni::NVariableAssignment>($1, $3); }
	| expr						{ $$ = std::move($1); }
	| RET expr					{ $$ = std::make_unique<ni::NFunctionReturn>($2); }
	;

expr :
	const					{ $$ = std::move($1); }
	| IDENTIFIER			{ $$ = std::make_unique<ni::NVariableLookup>($1); }
	| IDENTIFIER OPENPAR CLOSEPAR	{ $$ = std::make_unique<ni::NFunctionCall>($1); }
	| IDENTIFIER OPENPAR call_args CLOSEPAR	{ $$ = std::make_unique<ni::NFunctionCall>($1, $3); }
	| expr PLUS expr		{ $$ = std::make_unique<ni::NBinaryOperation>("+"s, $1, $3); }
	| expr MINUS expr		{ $$ = std::make_unique<ni::NBinaryOperation>("-"s, $1, $3); }
	| expr MULTIPLY expr	{ $$ = std::make_unique<ni::NBinaryOperation>("*"s, $1, $3); }
	| OPENPAR expr CLOSEPAR	{ $$ = std::move($2); }
	;

typedef :
	INT						{ $$ = std::make_unique<ni::NInteger>(); }
	;

const :
	CONST_INTEGER			{ $$ = std::make_unique<ni::NConstantInteger>($1); }
	;

%%

void yy::parser::error (const std::string& m)
{
  std::cerr << m << '\n';
}
