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

enum token_kind { op, reserved_word, symbol, identifier, number, end_of_file };

class token {
public:
  token(token_kind kind, const location &location)
      : kind(kind), text(""), location(location){};
  token(token_kind kind, const std::string &text, const location &location)
      : kind(kind), text(text), location(location){};
  token_kind kind;
  std::string text;
  location location;
};

class tokenizer_error : public std::runtime_error {
public:
  tokenizer_error(const std::string &message, const location &location)
      : location(location), std::runtime_error(message){};
  location location;
};

class tokenizer {
public:
  tokenizer(const std::string &filename, std::istream &stream)
      : _stream(stream), _location(location(filename)),
        _next(token(token_kind::end_of_file, _location)){};
  bool scan();
  token next() const;

private:
  char _c = -1;
  token _next;
  location _location;
  std::istream &_stream;
};
} // namespace ast
} // namespace ni
