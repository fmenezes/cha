#pragma once

#include "ast/ast.hh"
#include "generated/parser.tab.hh"

#define YY_DECL yy::parser::symbol_type yylex(ni::ast::parser &p)
YY_DECL;
