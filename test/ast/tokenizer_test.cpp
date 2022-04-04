#include <sstream>

#include <gtest/gtest.h>

#include "ast/tokenizer.hh"

namespace ni {
namespace test {
namespace {

TEST(TokenizerTest, Success) {
  std::stringstream ss;

  ss << "fun main() int {\n";
  ss << "    ret 0\n";
  ss << "}\n";

  ni::ast::tokenizer t("file.ni", ss);


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

  EXPECT_EQ(t.scan(), false);
  EXPECT_EQ(t.next().kind, ni::ast::token_kind::end_of_file);
}

TEST(TokenizerTest, UnknownChar) {
  std::stringstream ss;

  ss << "fun main(%) int {\n";
  ss << "    ret 0\n";
  ss << "}\n";

  EXPECT_THROW({
    ni::ast::tokenizer t("file.ni", ss);
    while(t.scan()) {
      t.next();
    }
  }, ni::ast::tokenizer_error);
}

} // namespace
} // namespace test
} // namespace ni
