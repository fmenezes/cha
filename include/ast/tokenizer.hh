#include <istream>

namespace ni {
namespace ast {
class location {
public:
  location(const std::string &file) : location(file, 1, 1, 1, 1){};
  location(const std::string &file, int line_begin, int column_begin,
           int line_end, int column_end)
      : file(file), line_begin(line_begin), column_begin(column_begin),
        line_end(line_end), column_end(column_end){};

  std::string file;
  int line_begin;
  int column_begin;
  int line_end;
  int column_end;

  std::string str() const {
    std::stringstream ss;

    ss << file << ":" << line_begin << ":" << column_begin;
    if (line_begin != line_end) {
      ss << "-" << line_end << ":" << column_end;
    } else if (column_begin != column_end) {
      ss << "-" << column_end;
    }

    return ss.str();
  }
};

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
  short lexemeBegin = -1;
  short lexemeForward = -1;
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
