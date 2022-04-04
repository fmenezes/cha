#include <regex>
#include <sstream>

#include "ast/tokenizer.hh"

bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool is_letter(char c) { return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'; }

bool is_alpha(char c) { return is_digit(c) || is_letter(c); }

bool is_first_char_identifier(char c) {
  return c == '_' || c == '-' || is_letter(c);
}

bool is_second_char_identifier(char c) {
  return c == '_' || c == '-' || is_alpha(c);
}

bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\r'; }

bool is_newLine(char c) { return c == '\n'; }

bool is_symbol(char c) {
  return c == '{' || c == '}' || c == '(' || c == ')' || c == ',';
}

bool is_operator(char c) {
  return c == '=' || c == '+' || c == '-' || c == '*';
}

bool is_reserved_word(std::string w) {
  return w == "fun" || w == "int" || w == "ret" || w == "var";
}

bool ni::ast::tokenizer::scan() {
  if (this->_stream->eof()) {
    return false;
  }

  if (_c == -1) {
    _c = this->_stream->get();
  }

  while (!this->_stream->eof()) {
    if (is_whitespace(_c)) {
      do {
        this->_location.column_begin++;
        this->_location.column_end = this->_location.column_begin;

        _c = this->_stream->get();
      } while (is_whitespace(_c));
      continue;
    }

    if (is_newLine(_c)) {
      do {
        this->_location.line_begin++;
        this->_location.line_end = this->_location.line_begin;
        this->_location.column_begin = 1;
        this->_location.column_end = this->_location.column_begin;

        _c = this->_stream->get();
      } while (is_newLine(_c));
      continue;
    }

    if (is_operator(_c)) {
      this->_next = ni::ast::token(ni::ast::token_kind::op, std::string(1, _c),
                                   this->_location);
      _c = this->_stream->get();
      return true;
    }

    if (is_symbol(_c)) {
      this->_next = ni::ast::token(ni::ast::token_kind::symbol,
                                   std::string(1, _c), this->_location);
      _c = this->_stream->get();
      return true;
    }

    if (is_digit(_c)) {
      std::stringstream ss;
      do {
        ss << _c;
        this->_location.column_end++;
        _c = this->_stream->get();
      } while (is_digit(_c));
      this->_next = ni::ast::token(ni::ast::token_kind::number, ss.str(),
                                   this->_location);
      this->_location.column_begin = this->_location.column_end + 1;
      this->_location.column_end = this->_location.column_begin;
      return true;
    }

    if (is_first_char_identifier(_c)) {
      std::stringstream ss;
      do {
        ss << _c;
        this->_location.column_end++;
        _c = this->_stream->get();
      } while (is_second_char_identifier(_c));
      auto kind = ni::ast::token_kind::identifier;
      if (is_reserved_word(ss.str())) {
        kind = ni::ast::token_kind::reserved_word;
      }
      this->_next = ni::ast::token(kind, ss.str(), this->_location);
      this->_location.column_begin = this->_location.column_end + 1;
      this->_location.column_end = this->_location.column_begin;
      return true;
    }

    throw ni::ast::tokenizer_error(std::string("invalid char '") + _c + "'",
                                   _location);
  }

  this->_location.column_begin++;
  this->_location.column_end = this->_location.column_begin;
  this->_next =
      ni::ast::token(ni::ast::token_kind::end_of_file, this->_location);
  return false;
}

ni::ast::token ni::ast::tokenizer::next() const { return this->_next; }
