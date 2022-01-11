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

%token <str> INTEGER
%token <token> POWER

%type <node> expr const

%left '+' '-'
%left '*' '/'
%precedence  '!'
%right POWER

%start program

%%

program :
	program expr '\n'	{ std::cout << $2->to_string() << std::endl; }
	|
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
