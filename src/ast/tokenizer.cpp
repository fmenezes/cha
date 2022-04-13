#include <istream>
#include <regex>
#include <sstream>

#include "ast/tokenizer.hh"

bool is_number(const std::string &str) {
  return std::regex_match(str, std::regex("[0-9]+"));
}

bool is_identifier(const std::string &str) {
  return std::regex_match(str, std::regex("[a-zA-Z_][a-zA-Z0-9_-]*"));
  ;
}

bool is_whitespace(const std::string &str) {
  return std::regex_match(str, std::regex("[ \t\r]+"));
  ;
}

bool is_newline(const std::string &str) {
  return std::regex_match(str, std::regex("\n+"));
  ;
}

ni::ast::token_tag reserved_word(const std::string &str) {
  if (str == "fun") {
    return ni::ast::token_tag::fun;
  }

  if (str == "var") {
    return ni::ast::token_tag::var;
  }

  if (str == "ret") {
    return ni::ast::token_tag::ret;
  }

  if (str == "int") {
    return ni::ast::token_tag::type_int;
  }

  return ni::ast::token_tag::identifier;
}

ni::ast::token_tag symbol_tag(char c) {
  switch (c) {
  case '=':
    return ni::ast::token_tag::equals;
  case '+':
    return ni::ast::token_tag::plus;
  case '-':
    return ni::ast::token_tag::minus;
  case '*':
    return ni::ast::token_tag::star;
  case '(':
    return ni::ast::token_tag::open_parentheses;
  case ')':
    return ni::ast::token_tag::close_parentheses;
  case '{':
    return ni::ast::token_tag::open_braces;
  case '}':
    return ni::ast::token_tag::close_braces;
  case ',':
    return ni::ast::token_tag::comma;
  default:
    return ni::ast::token_tag::unknown;
  }
}

bool ni::ast::tokenizer::read() {
  if (lexeme_begin > 0 && lexeme_begin < c_len) {
    auto s = lexeme_forward - lexeme_begin;
    memmove(&c, &c[lexeme_begin], s);
    _stream->read(&c[s], sizeof(c) - s);
    c_len = _stream->gcount() + s;
    lexeme_begin = 0;
    lexeme_forward = s;
    return ((c_len - s) > 0);
  } else {
    _stream->read(c, sizeof(c));
    c_len = _stream->gcount();
    lexeme_begin = 0;
    lexeme_forward = 1;
    return (c_len > 0);
  }
}

std::string ni::ast::tokenizer::current_lexeme() {
  return std::string(&c[lexeme_begin], lexeme_forward - lexeme_begin);
}

void ni::ast::tokenizer::step_begin() {
  _location.line_begin = _location.line_end;
  _location.column_begin = _location.column_end;
  lexeme_begin = (lexeme_forward - 1);
}

ni::ast::token ni::ast::tokenizer::set_token(ni::ast::token_tag tag) {
  step();
  _next = ni::ast::token(
      tag, std::string(&c[lexeme_begin], lexeme_forward - lexeme_begin - 1),
      _location);
  step_begin();
  return _next;
}

ni::ast::token ni::ast::tokenizer::set_identifier_token() {
  step();
  auto word = std::string(&c[lexeme_begin], lexeme_forward - lexeme_begin - 1);
  auto tag = reserved_word(word);
  _next = ni::ast::token(tag, word, _location);
  step_begin();
  return _next;
}

void ni::ast::tokenizer::peek() { lexeme_forward++; }

void ni::ast::tokenizer::step() {
  _location.column_end += (lexeme_forward - lexeme_begin - 1);
}

bool ni::ast::tokenizer::eof() {
  if (lexeme_begin == -1 || lexeme_forward >= c_len) {
    return !read();
  }

  return false;
}

void ni::ast::tokenizer::step_line() {
  _location.line_end++;
  _location.column_end = 1;
}

ni::ast::token ni::ast::tokenizer::scan_next_token() {
  while (!eof()) {
    if (is_whitespace(current_lexeme())) {
      do {
        peek();
      } while ((is_whitespace(current_lexeme())));
      step();
      step_begin();
      continue;
    }

    if (is_newline(current_lexeme())) {
      do {
        peek();
        step_line();
      } while ((is_newline(current_lexeme())));
      step_begin();
      continue;
    }

    auto s = symbol_tag(c[lexeme_begin]);
    if (s != ni::ast::token_tag::unknown) {
      peek();
      return set_token(s);
    }

    if (is_number(current_lexeme())) {
      do {
        peek();
      } while (is_number(current_lexeme()));
      return set_token(ni::ast::token_tag::number);
    }

    if (is_identifier(current_lexeme())) {
      do {
        peek();
      } while (is_identifier(current_lexeme()));
      return set_identifier_token();
    }

    peek();
    return set_token(ni::ast::token_tag::unknown);
  }

  return set_token(ni::ast::token_tag::end_of_file);
}
