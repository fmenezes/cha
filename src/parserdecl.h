#pragma once

#include "nodes.hh"

#define YY_DECL yy::parser::symbol_type yylex(ni::NProgram &p)
YY_DECL;
