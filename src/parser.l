%{
#include "nodes.h"
#include "parser.tab.h"


void yyerror(char *);
%}


/* RegEx */
number [0-9]+
identifier [_a-zA-Z][_a-zA-Z0-9]*


%%

"var" {
		return VAR;
	}

{identifier} {
		yylval.str = new std::string(yytext, yyleng);
		return IDENTIFIER;
	}

{number} {
		yylval.str = new std::string(yytext, yyleng);
		return INTEGER;
	}

[-+()=*] {
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
