%{
#include <stdio.h>
#include <string.h>
#include <math.h>

int yylex(void);

void yyerror(char *);

int lineNum = 1;
%}

%token INTEGER POWER

%union
{
	int val;
}

%type <val> expr INTEGER

%left '+' '-'
%left '*' '/'
%precedence  '!'
%right POWER

%start program

%%

program :
	program expr '\n' { printf("%d\n", $2); }
	|
	;

expr :
	INTEGER			  { $$ = $1; }
	| expr '+' expr   { $$ = $1 + $3; }
	| expr '-' expr   { $$ = $1 - $3; }
	| expr '*' expr   { $$ = $1 * $3; }
	| expr '/' expr   { $$ = $1 / $3; }
	| '!' expr		  { $$ = ~$2; }
	| expr POWER expr { $$ = (int)pow($1, $3); }
	| '(' expr ')'    { $$ = $2; }
	;

%%

void yyerror(char *s)
{
	printf("%s\n", s);
}

int main()
{
	yyparse();
	return 0;
}
