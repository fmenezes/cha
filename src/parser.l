%{
#include "nodes.h"
#include "parser.tab.h"

void yyerror(char *);
%}

%%

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
