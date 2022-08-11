#pragma once

#include <memory>
#include <string>

#include "ni/ast/ast.hh"

std::unique_ptr<ni::ast::program> parse(const std::string &filename);
std::string makeTempDir();
