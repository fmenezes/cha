%{
#include "ast/ast.hh"
#include "ast/parser.hh"
#include "parser.tab.hh"
#include "ast/parserdecl.h"

# define YY_USER_ACTION  loc.columns (yyleng);
%}

%option noyywrap nounput noinput batch

/* RegEx */
number -?[0-9]+
identifier [_a-zA-Z][_a-zA-Z0-9]*
%%
%{
  yy::location& loc = p.location;
  loc.step();
%}
"var" {
		return yy::parser::make_VAR(loc);
	}

"fun" {
		return yy::parser::make_FUN(loc);
	}

"ret" {
		return yy::parser::make_RET(loc);
	}

"int" {
		return yy::parser::make_INT(loc);
	}

{identifier} {
		return yy::parser::make_IDENTIFIER(std::string(yytext, yyleng), loc);
	}

{number} {
		return yy::parser::make_CONST_INTEGER(std::string(yytext, yyleng), loc);
	}

"+" {
		return yy::parser::make_PLUS(loc);
	}

"-" {
		return yy::parser::make_MINUS(loc);
	}


"*" {
		return yy::parser::make_MULTIPLY(loc);
	}

"=" {
		return yy::parser::make_EQUALS(loc);
	}

"(" {
		return yy::parser::make_OPENPAR(loc);
	}

")" {
		return yy::parser::make_CLOSEPAR(loc);
	}

"{" {
		return yy::parser::make_OPENCUR(loc);
	}

"}" {
		return yy::parser::make_CLOSECUR(loc);
	}

"," {
		return yy::parser::make_COMMA(loc);
	}

[ \t\r]+ { loc.step(); }

[\n]+ { loc.lines(yyleng); loc.step(); }

.	{ throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext, yyleng)); }

<<EOF>>    { return yy::parser::make_YYEOF (loc); }

%%
