%{
#include <iostream>
#include <string>
#include "nodes.h"

int yylex(void);

void yyerror(char *);

using namespace std::string_literals;

%}

%union
{
	ni::Node *node;
	char *str;
	int token;
}

%token <str> INTEGER IDENTIFIER
%token <token> POWER VAR

%type <node> expr const statement

%left '+' '-'
%left '*' '/'
%precedence  '!'
%right POWER

%start program

%%

program :
	program statement '\n'	{ std::cout << $2->to_string() << std::endl; }
	|
	;

statement :
	VAR IDENTIFIER	{ $$ = new ni::NVariableDeclaration(std::string($2)); }
	| expr			{ $$ = $1; }
	;

expr :
	const				{ $$ = $1; }
	| expr '+' expr		{ $$ = new ni::NBinaryOperation("+"s, $1, $3); }
	| expr '-' expr		{ $$ = new ni::NBinaryOperation("-"s, $1, $3); }
	| expr '*' expr		{ $$ = new ni::NBinaryOperation("*"s, $1, $3); }
	| expr '/' expr		{ $$ = new ni::NBinaryOperation("/"s, $1, $3); }
	| '!' expr			{ $$ = new ni::NUnaryOperation("!"s, $2); }
	| expr POWER expr	{ $$ = new ni::NBinaryOperation("**"s, $1, $3); }
	| '(' expr ')'		{ $$ = $2; }
	;

const :
	INTEGER				{ $$ = new ni::NInteger(std::string($1)); }

%%

void yyerror(char *s)
{
	printf("%s\n", s);
}
