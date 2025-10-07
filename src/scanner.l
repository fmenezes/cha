%{
#include <string.h>

int yycolumn = 1;
extern const char *current_file;

#define YYSTYPE char*
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
	yylloc.last_column = yycolumn + yyleng; \
    yycolumn += yyleng;

#include "parser.tab.hpp"
%}

%option noyywrap yylineno

/* RegEx */
integer (0[xX][0-9A-Fa-f]+|0|[1-9][0-9]*)
uinteger (0[xX][0-9A-Fa-f]+|0|[1-9][0-9]*)u
float (([0-9]*\.[0-9]+)|([0-9]+\.))([eE][+-]?[0-9]+)?
identifier [_a-zA-Z][_a-zA-Z0-9]{0,31}
%%
"var" {
	return KEYWORD_VAR;
}

"fun" {
	return KEYWORD_FUN;
}

"ret" {
	return KEYWORD_RET;
}

"int8" {
	return KEYWORD_INT8;
}

"uint8" {
	return KEYWORD_UINT8;
}

"byte" { // type alias for uint8
	return KEYWORD_UINT8;
}

"int16" {
	return KEYWORD_INT16;
}

"uint16" {
	return KEYWORD_UINT16;
}

"int32" {
	return KEYWORD_INT32;
}

"uint32" {
	return KEYWORD_UINT32;
}

"int" {
	return KEYWORD_INT;
}

"uint" {
	return KEYWORD_UINT;
}

"int64" {
	return KEYWORD_INT64;
}

"uint64" {
	return KEYWORD_UINT64;
}

"float16" {
	return KEYWORD_FLOAT16;
}

"float32" {
	return KEYWORD_FLOAT32;
}

"float64" {
	return KEYWORD_FLOAT64;
}

"bool" {
	return KEYWORD_BOOL;
}

"true" {
	return BOOL_TRUE;
}

"false" {
	return BOOL_FALSE;
}

"+" {
	return PLUS;
}

"-" {
	return MINUS;
}

"*" {
	return STAR;
}

"/" {
	return SLASH;
}

"=" {
	return EQUALS;
}

"(" {
	return OPEN_PAR;
}

")" {
	return CLOSE_PAR;
}

"{" {
	return OPEN_CUR;
}

"}" {
	return CLOSE_CUR;
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

"!" {
	return EXCLAMATION;
}

"const" {
	return KEYWORD_CONST;
}

"if" {
	return KEYWORD_IF;
}

"else" {
	return KEYWORD_ELSE;
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

. {
	// Unknown character - report error and exit
	fprintf(stderr, "Error: %s:%d:%d: unexpected character '%c'\n", 
	        (current_file ? current_file : ""), yylineno, yycolumn, yytext[0]);
	exit(1);
}

%%
