%{
#include "nodes.hh"
#include "parser.tab.hh"
#include "parserdecl.h"

%}

%option noyywrap nounput noinput batch

/* RegEx */
number [0-9]+
identifier [_a-zA-Z][_a-zA-Z0-9]*
%%

"var" {
		return yy::parser::make_VAR();
	}

"fun" {
		return yy::parser::make_FUN();
	}

"ret" {
		return yy::parser::make_RET();
	}

{identifier} {
		return yy::parser::make_IDENTIFIER(std::string(yytext, yyleng));
	}

{number} {
		return yy::parser::make_INTEGER(std::string(yytext, yyleng));
	}

"+" {
		return yy::parser::make_PLUS();
	}

"-" {
		return yy::parser::make_MINUS();
	}


"*" {
		return yy::parser::make_MULTIPLY();
	}

"=" {
		return yy::parser::make_EQUALS();
	}

"(" {
		return yy::parser::make_OPENPAR();
	}

")" {
		return yy::parser::make_CLOSEPAR();
	}

"{" {
		return yy::parser::make_OPENCUR();
	}

"}" {
		return yy::parser::make_CLOSECUR();
	}

"," {
		return yy::parser::make_COMMA();
	}

[ \t\r\n]+ ;

.	{ throw yy::parser::syntax_error("invalid character: " + std::string(yytext, yyleng)); }

%%
int ni::NProgram::parse(const std::string& f)
{
	if (!(yyin = fopen (f.c_str (), "r")))
	{
		std::cerr << "cannot open " << f << std::endl;
		exit (EXIT_FAILURE);
	}
	int r = this->parse();
	fclose(yyin);
	return r;
}
