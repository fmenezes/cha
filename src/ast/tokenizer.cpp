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

bool is_newline(char c) { return c == '\n'; }

bool is_symbol(char c) {
  return c == '{' || c == '}' || c == '(' || c == ')' || c == ',';
}

bool is_operator(char c) {
  return c == '=' || c == '+' || c == '-' || c == '*';
}

bool is_reserved_word(std::string w) {
  return w == "fun" || w == "int" || w == "ret" || w == "var";
}

ni::ast::token ni::ast::tokenizer::scan_next_token() {
  if (_stream->eof()) {
    return _next;
  }

  if (_c == -1) {
    _c = _stream->get();
  }

  while (!_stream->eof()) {
    if (is_whitespace(_c)) {
      do {
        _location.column_end++;

        _c = _stream->get();
      } while (is_whitespace(_c));
      _location.column_begin = _location.column_end;
      continue;
    }

    if (is_newline(_c)) {
      _location.column_end = 1;
      do {
        _location.line_end++;
        _c = _stream->get();
      } while (is_newline(_c));
      _location.line_begin = _location.line_end;
      _location.column_begin = _location.column_end;
      continue;
    }

    if (is_operator(_c)) {
      _location.column_end++;
      _next = ni::ast::token(ni::ast::token_kind::op, std::string(1, _c),
                             _location);
      _c = _stream->get();
      _location.column_begin = _location.column_end;
      return _next;
    }

    if (is_symbol(_c)) {
      _location.column_end++;
      _next = ni::ast::token(ni::ast::token_kind::symbol, std::string(1, _c),
                             _location);
      _c = _stream->get();
      _location.column_begin = _location.column_end;
      return _next;
    }

    if (is_digit(_c)) {
      std::stringstream ss;
      do {
        ss << _c;
        _location.column_end++;
        _c = _stream->get();
      } while (is_digit(_c));
      _next = ni::ast::token(ni::ast::token_kind::number, ss.str(), _location);
      _location.column_begin = _location.column_end;
      return _next;
    }

    if (is_first_char_identifier(_c)) {
      std::stringstream ss;
      do {
        ss << _c;
        _location.column_end++;
        _c = _stream->get();
      } while (is_second_char_identifier(_c));
      auto kind = ni::ast::token_kind::identifier;
      if (is_reserved_word(ss.str())) {
        kind = ni::ast::token_kind::reserved_word;
      }
      _next = ni::ast::token(kind, ss.str(), _location);
      _location.column_begin = _location.column_end;
      return _next;
    }

    throw ni::ast::tokenizer_error(std::string("invalid char '") + _c + "'",
                                   _location);
  }

  _location.column_begin++;
  _location.column_end = _location.column_begin;
  _next = ni::ast::token(ni::ast::token_kind::end_of_file, _location);
  return _next;
}
