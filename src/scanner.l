%{
#include <string.h>

int yycolumn = 1;

#define YYSTYPE char*
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
	yylloc.last_column = yycolumn + yyleng; \
    yycolumn += yyleng;

#include "parser.tab.h"
%}

%option noyywrap yylineno

/* RegEx */
number -?[0-9]+|0[xX][0-9A-Fa-f]+
float -?[0-9]+\.[0-9]+
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

"byte" {
	return REFTYPE_BYTE;
}

"sbyte" {
	return REFTYPE_SBYTE;
}

"short" {
	return REFTYPE_SHORT;
}

"ushort" {
	return REFTYPE_USHORT;
}

"int" {
	return REFTYPE_INT;
}

"uint" {
	return REFTYPE_UINT;
}

"long" {
	return REFTYPE_LONG;
}

"ulong" {
	return REFTYPE_ULONG;
}

"large" {
	return REFTYPE_LARGE;
}

"ularge" {
	return REFTYPE_ULARGE;
}

"double" {
	return REFTYPE_DOUBLE;
}

"float" {
	return REFTYPE_FLOAT;
}

"sfloat" {
	return REFTYPE_SFLOAT;
}

{identifier} {
	yylval = strdup(yytext);
	return IDENTIFIER;
}

{number} {
	yylval = strdup(yytext);
	return NUMBER;
}

{float} {
	yylval = strdup(yytext);
	return FLOAT;
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

[ \t\r]+ { /* ignore whitespace */ }

[\n]+ { yycolumn = 1; }

%%
