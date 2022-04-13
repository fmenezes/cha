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
} // namespace ast
} // namespace ni
