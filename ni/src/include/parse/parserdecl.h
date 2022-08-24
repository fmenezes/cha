#pragma once

#include "generated/parser.tab.hh"
#include "ni/ast/ast.hh"
#include "ni/parse/syntax_parser.hh"

#define YY_DECL yy::parser::symbol_type yylex(ni::parse::syntax_parser &p)
YY_DECL;
