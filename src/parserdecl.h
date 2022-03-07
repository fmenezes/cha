#pragma once

#include "ast.hh"

#define YY_DECL yy::parser::symbol_type yylex(ni::ast::NProgram &p)
YY_DECL;
