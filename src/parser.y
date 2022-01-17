%{
#include <iostream>
#include <string>
#include "nodes.h"

int yylex(void);

void yyerror(char *);

using namespace std::string_literals;

extern ni::NProgram *program;

%}

%union
{
	ni::Node *node;
	ni::NStatementList *statementlist;
	std::string *str;
	int token;
}

%token <str> INTEGER IDENTIFIER
%token <token> VAR

%type <node> expr const statement 
%type <statementlist> statements

%left '+' '-'
%left '*' '/'

%start program

%%

program :
	statements { program = new ni::NProgram($1); }
	|
	;


statements :
	statement '\n'	{ $$ = new ni::NStatementList($1); }
	| statements statement '\n'	{ $1->statements.push_back($2); }
	;

statement :
	VAR IDENTIFIER			{ $$ = new ni::NVariableDeclaration(*$2); }
	| IDENTIFIER '=' expr	{ $$ = new ni::NVariableAssignment(*$1, $3); }
	| expr					{ $$ = $1; }
	;

expr :
	const				{ $$ = $1; }
	| IDENTIFIER		{ $$ = new ni::NVariableLookup(*$1); }
	| expr '+' expr		{ $$ = new ni::NBinaryOperation("+"s, $1, $3); }
	| expr '-' expr		{ $$ = new ni::NBinaryOperation("-"s, $1, $3); }
	| expr '*' expr		{ $$ = new ni::NBinaryOperation("*"s, $1, $3); }
	| expr '/' expr		{ $$ = new ni::NBinaryOperation("/"s, $1, $3); }
	| '(' expr ')'		{ $$ = $2; }
	;

const :
	INTEGER				{ $$ = new ni::NInteger(*$1); }

%%

void yyerror(char *s)
{
	printf("%s\n", s);
}


