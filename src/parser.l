%{
#include "nodes.h"
#include "parser.tab.h"

void yyerror(char *);
%}

%%

"var" {
		return VAR;
	}

[_a-zA-Z][_a-zA-Z0-9]* {
		yylval.str = yytext;
		return IDENTIFIER;
	}

[0-9]+ {
		yylval.str = yytext;
		return INTEGER;
	}

"**" {
		return POWER;
	}

[-+()=*/!] {
		return *yytext;
	}

"\n" {
		return *yytext;
	}

[ \t]+ ;

.	{ yyerror("invalid character"); }

%%

int yywrap() {
	return 1;
}
