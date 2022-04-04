#include <sstream>

#include <gtest/gtest.h>

#include "ast/tokenizer.hh"

namespace ni {
namespace test {
namespace {

TEST(TokenizerTest, Success) {
  std::cout << "Debug Success\n";

  std::stringstream ss;

  ss << "fun main() int {\n";
  ss << "    ret 0\n";
  ss << "}\n";

  ni::ast::tokenizer t("file.ni", &ss);

  std::cout << "Debug 2 Success\n";

  EXPECT_EQ(t.scan(), true);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::reserved_word);
  EXPECT_EQ(t.next().text, "fun");

  EXPECT_EQ(t.scan(), true);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::identifier);
  EXPECT_EQ(t.next().text, "main");

  EXPECT_EQ(t.scan(), true);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::symbol);
  EXPECT_EQ(t.next().text, "(");

  EXPECT_EQ(t.scan(), true);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::symbol);
  EXPECT_EQ(t.next().text, ")");

  EXPECT_EQ(t.scan(), true);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::reserved_word);
  EXPECT_EQ(t.next().text, "int");

  EXPECT_EQ(t.scan(), true);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::symbol);
  EXPECT_EQ(t.next().text, "{");

  EXPECT_EQ(t.scan(), true);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::reserved_word);
  EXPECT_EQ(t.next().text, "ret");

  EXPECT_EQ(t.scan(), true);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::number);
  EXPECT_EQ(t.next().text, "0");

  EXPECT_EQ(t.scan(), true);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::symbol);
  EXPECT_EQ(t.next().text, "}");

  std::cout << "Debug 3 Success\n";

  EXPECT_EQ(t.scan(), false);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::end_of_file);

  std::cout << "Debug 4 Success\n";
}

TEST(TokenizerTest, UnknownChar) {
  std::cout << "Debug UnknownChar\n";

  std::stringstream ss;

  ss << "fun main(%) int {\n";
  ss << "    ret 0\n";
  ss << "}\n";

  std::cout << "Debug 2 UnknownChar\n";
  EXPECT_THROW({
    std::cout << "Debug 3 UnknownChar\n";
    ni::ast::tokenizer t("file.ni", &ss);
    std::cout << "Debug 4 UnknownChar\n";
    while(t.scan()) {
      std::cout << "Debug 5 UnknownChar\n";
      t.next();
    }
    std::cout << "Debug 6 UnknownChar\n";
  }, ni::ast::tokenizer_error);
  std::cout << "Debug 7 UnknownChar\n";
}

} // namespace
} // namespace test
} // namespace ni
