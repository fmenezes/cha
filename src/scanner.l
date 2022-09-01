%{
#include <string.h>
#define YYSTYPE char *
#include "parser.tab.h"
%}

%option noyywrap

/* RegEx */
number -?[0-9]+
identifier [_a-zA-Z][_a-zA-Z0-9]{0,31}
%%
"var" {
		return VAR;
	}

"fun" {
		return FUN;
	}

"ret" {
		return RET;
	}

"int" {
		return INT;
	}

{identifier} {
		yylval = strdup(yytext);
		return IDENTIFIER;
	}

{number} {
		yylval = strdup(yytext);
		return NUMBER;
	}

"+" {
		return PLUS;
	}

"-" {
		return MINUS;
	}


"*" {
		return MULTIPLY;
	}

"=" {
		return EQUALS;
	}

"(" {
		return OPENPAR;
	}

")" {
		return CLOSEPAR;
	}

"{" {
		return OPENCUR;
	}

"}" {
		return CLOSECUR;
	}

"," {
		return COMMA;
	}

[ \t\r\n]+ { /* ignore whitespace */ }

%%
