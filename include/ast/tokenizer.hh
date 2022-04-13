#include <istream>

#include "location.hh"

namespace ni {
namespace ast {
enum token_tag {
  unknown,
  end_of_file,
  equals,
  plus,
  minus,
  star,
  open_parentheses,
  close_parentheses,
  open_braces,
  close_braces,
  comma,
  fun,
  ret,
  var,
  type_int,
  identifier,
  number
};

class token {
public:
  token(token_tag tag, const location &location)
      : tag(tag), text(""), loc(location){};
  token(token_tag tag, const std::string &text, const location &location)
      : tag(tag), text(text), loc(location){};
  token_tag tag;
  std::string text;
  location loc;
};

class tokenizer_error : public std::runtime_error {
public:
  tokenizer_error(const std::string &message, const location &loc)
      : loc(loc), std::runtime_error(loc.str() + " " + message){};
  tokenizer_error(const char *message, const location &loc)
      : loc(loc), std::runtime_error(loc.str() + " " + message){};
  location loc;
};

class tokenizer {
public:
  tokenizer(const std::string &filename, std::istream *stream)
      : _stream(stream), _location(location(filename)),
        _next(token(token_tag::end_of_file, location(filename))){};
  token scan_next_token();
  token last_token() const { return _next; };

private:
  char c[1024] = "";
  std::streamsize c_len = 0;
  short lexeme_begin = -1;
  short lexeme_forward = -1;
  token _next;
  location _location;
  std::istream *_stream;
  bool read();
  bool eof();
  void peek();
  void step();
  void step_line();
  void step_begin();
  token set_token(token_tag tag);
  token set_identifier_token();
  std::string current_lexeme();
};
} // namespace ast
} // namespace ni
