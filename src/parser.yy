%{
#include <iostream>
#include <string>
#include "nodes.hh"

using namespace std::string_literals;

extern ni::NProgram *program;
%}

%require "3.2"
%language "c++"

%define api.value.type variant
%define api.token.constructor

%nterm <ni::Node*> expr;
%nterm <ni::Node*> const;
%nterm <ni::Node*> statement;
%nterm <std::vector<ni::Node*>> statements;

%code {
# include "parserdecl.h"
}

%param { ni::NProgram& p }

%token <std::string> INTEGER IDENTIFIER
%token VAR NEWLINE PLUS MINUS MULTIPLY OPENPAR CLOSEPAR EQUALS

%left PLUS MINUS
%left MULTIPLY

%start program

%%

program :
	statements { p.value = $1; }
	|
	;


statements :
	statement NEWLINE	{ $$.push_back($1); }
	| statements statement NEWLINE	{ $$ = $1; $$.push_back($2); }
	;

statement :
	VAR IDENTIFIER				{ $$ = new ni::NVariableDeclaration($2); }
	| IDENTIFIER EQUALS expr	{ $$ = new ni::NVariableAssignment($1, $3); }
	| expr						{ $$ = $1; }
	;

expr :
	const					{ $$ = $1; }
	| IDENTIFIER			{ $$ = new ni::NVariableLookup($1); }
	| expr PLUS expr		{ $$ = new ni::NBinaryOperation("+"s, $1, $3); }
	| expr MINUS expr		{ $$ = new ni::NBinaryOperation("-"s, $1, $3); }
	| expr MULTIPLY expr	{ $$ = new ni::NBinaryOperation("*"s, $1, $3); }
	| OPENPAR expr CLOSEPAR	{ $$ = $2; }
	;

const :
	INTEGER				{ $$ = new ni::NInteger($1); }

%%

void yy::parser::error (const std::string& m)
{
  std::cerr << m << '\n';
}
