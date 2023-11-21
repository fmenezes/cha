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
integer [+-]?[0-9]+|0[xX][0-9A-Fa-f]+
uinteger ([0-9]+|0[xX][0-9A-Fa-f]+)u
float [+-]?[0-9]+\.[0-9]+
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

"int8" {
	return REFTYPE_INT8;
}

"uint8" {
	return REFTYPE_UINT8;
}

"byte" { // type alias for uint8
	return REFTYPE_UINT8;
}

"int16" {
	return REFTYPE_INT16;
}

"uint16" {
	return REFTYPE_UINT16;
}

"int32" {
	return REFTYPE_INT32;
}

"uint32" {
	return REFTYPE_UINT32;
}

"int" {
	return REFTYPE_INT;
}

"uint" {
	return REFTYPE_UINT;
}

"int64" {
	return REFTYPE_INT64;
}

"uint64" {
	return REFTYPE_UINT64;
}

"int128" {
	return REFTYPE_INT128;
}

"uint128" {
	return REFTYPE_UINT128;
}

"float16" {
	return REFTYPE_FLOAT16;
}

"float32" {
	return REFTYPE_FLOAT32;
}

"float64" {
	return REFTYPE_FLOAT64;
}

"bool" {
	return REFTYPE_BOOL;
}

"true" {
	return BOOL_TRUE;
}

"false" {
	return BOOL_FALSE;
}

"+" {
	return ADD;
}

"-" {
	return SUBTRACT;
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

"==" {
	return EQUALS_EQUALS;
}

"!=" {
	return NOT_EQUALS;
}

">" {
	return GREATER_THAN;
}

">=" {
	return GREATER_THAN_OR_EQUALS;
}

"<" {
	return LESS_THAN;
}

"<=" {
	return LESS_THAN_OR_EQUALS;
}

"&&" {
	return AND;
}

"||" {
	return OR;
}

"const" {
	return KEYWORD_CONST;
}

[ \t\r]+ { /* ignore whitespace */ }

[\n]+ { yycolumn = 1; }

{integer} { // regex matchers should be last
	yylval = strdup(yytext);
	return INTEGER;
}

{uinteger} {
	yylval = strdup(yytext);
	return UINTEGER;
}

{float} {
	yylval = strdup(yytext);
	return FLOAT;
}

{identifier} {
	yylval = strdup(yytext);
	return IDENTIFIER;
}

%%
