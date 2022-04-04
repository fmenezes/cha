#include <regex>
#include <sstream>

#include "ast/tokenizer.hh"

bool isDigit(char c) { return c >= '0' && c <= '9'; }

bool isLetter(char c) { return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'; }

bool isAlpha(char c) { return isDigit(c) || isLetter(c); }

bool isFirstCharIdentifier(char c) {
  return c == '_' || c == '-' || isLetter(c);
}

bool isSecondCharIdentifier(char c) {
  return c == '_' || c == '-' || isAlpha(c);
}

bool isWhitespace(char c) { return c == ' ' || c == '\t' || c == '\r'; }

bool isNewLine(char c) { return c == '\n'; }

bool isSymbol(char c) {
  return c == '{' || c == '}' || c == '(' || c == ')' || c == ',';
}

bool isOperator(char c) { return c == '=' || c == '+' || c == '-' || c == '*'; }

bool isReservedWord(std::string w) {
  return w == "fun" || w == "int" || w == "ret" || w == "var";
}

bool ni::ast::tokenizer::scan() {
  if (this->_stream.eof()) {
    return false;
  }

  if (_c == -1) {
    _c = this->_stream.get();
  }

  while (!this->_stream.eof()) {
    if (isWhitespace(_c)) {
      do {
        this->_location.column_begin++;
        this->_location.column_end = this->_location.column_begin;

        _c = this->_stream.get();
      } while (isWhitespace(_c));
      continue;
    }

    if (isNewLine(_c)) {
      do {
        this->_location.line_begin++;
        this->_location.line_end = this->_location.line_begin;
        this->_location.column_begin = 1;
        this->_location.column_end = this->_location.column_begin;

        _c = this->_stream.get();
      } while (isNewLine(_c));
      continue;
    }

    if (isOperator(_c)) {
      this->_next = ni::ast::token(ni::ast::token_kind::op, std::string(1, _c),
                                   this->_location);
      _c = this->_stream.get();
      return true;
    }

    if (isSymbol(_c)) {
      this->_next = ni::ast::token(ni::ast::token_kind::symbol,
                                   std::string(1, _c), this->_location);
      _c = this->_stream.get();
      return true;
    }

    if (isDigit(_c)) {
      std::stringstream ss;
      do {
        ss << _c;
        this->_location.column_end++;
        _c = this->_stream.get();
      } while (isDigit(_c));
      this->_next = ni::ast::token(ni::ast::token_kind::number, ss.str(),
                                   this->_location);
      this->_location.column_begin = this->_location.column_end + 1;
      this->_location.column_end = this->_location.column_begin;
      return true;
    }

    if (isFirstCharIdentifier(_c)) {
      std::stringstream ss;
      do {
        ss << _c;
        this->_location.column_end++;
        _c = this->_stream.get();
      } while (isSecondCharIdentifier(_c));
      auto kind = ni::ast::token_kind::identifier;
      if (isReservedWord(ss.str())) {
        kind = ni::ast::token_kind::reserved_word;
      }
      this->_next = ni::ast::token(kind, ss.str(), this->_location);
      this->_location.column_begin = this->_location.column_end + 1;
      this->_location.column_end = this->_location.column_begin;
      return true;
    }

    throw std::runtime_error(_location.str() + " invalid char '" + _c + "' (" +
                             std::to_string((int)_c) + ")");
  }

  this->_location.column_begin++;
  this->_location.column_end = this->_location.column_begin;
  this->_next =
      ni::ast::token(ni::ast::token_kind::end_of_file, this->_location);
  return false;
}

ni::ast::token ni::ast::tokenizer::next() const { return this->_next; }
