#include <iostream>
#include <sstream>

#include <gtest/gtest.h>

#include "ast/tokenizer.hh"

namespace ni {
namespace test {
namespace {

TEST(TokenizerTest, Success) {
  std::string s;

  s = "fun main() int {\n";
  s += "    ret 0\n";
  s += "}\n";

  std::istringstream ss(s);

  ni::ast::tokenizer t("file.ni", &ss);

  auto next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::fun);
  EXPECT_EQ(next.loc.line_begin, 1);
  EXPECT_EQ(next.loc.line_end, 1);
  EXPECT_EQ(next.loc.column_begin, 1);
  EXPECT_EQ(next.loc.column_end, 4);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::identifier);
  EXPECT_EQ(next.text, "main");
  EXPECT_EQ(next.loc.line_begin, 1);
  EXPECT_EQ(next.loc.line_end, 1);
  EXPECT_EQ(next.loc.column_begin, 5);
  EXPECT_EQ(next.loc.column_end, 9);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::open_parentheses);
  EXPECT_EQ(next.loc.line_begin, 1);
  EXPECT_EQ(next.loc.line_end, 1);
  EXPECT_EQ(next.loc.column_begin, 9);
  EXPECT_EQ(next.loc.column_end, 10);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::close_parentheses);
  EXPECT_EQ(next.loc.line_begin, 1);
  EXPECT_EQ(next.loc.line_end, 1);
  EXPECT_EQ(next.loc.column_begin, 10);
  EXPECT_EQ(next.loc.column_end, 11);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::type_int);
  EXPECT_EQ(next.loc.line_begin, 1);
  EXPECT_EQ(next.loc.line_end, 1);
  EXPECT_EQ(next.loc.column_begin, 12);
  EXPECT_EQ(next.loc.column_end, 15);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::open_braces);
  EXPECT_EQ(next.loc.line_begin, 1);
  EXPECT_EQ(next.loc.line_end, 1);
  EXPECT_EQ(next.loc.column_begin, 16);
  EXPECT_EQ(next.loc.column_end, 17);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::ret);
  EXPECT_EQ(next.loc.line_begin, 2);
  EXPECT_EQ(next.loc.line_end, 2);
  EXPECT_EQ(next.loc.column_begin, 5);
  EXPECT_EQ(next.loc.column_end, 8);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::number);
  EXPECT_EQ(next.text, "0");
  EXPECT_EQ(next.loc.line_begin, 2);
  EXPECT_EQ(next.loc.line_end, 2);
  EXPECT_EQ(next.loc.column_begin, 9);
  EXPECT_EQ(next.loc.column_end, 10);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::close_braces);
  EXPECT_EQ(next.loc.line_begin, 3);
  EXPECT_EQ(next.loc.line_end, 3);
  EXPECT_EQ(next.loc.column_begin, 1);
  EXPECT_EQ(next.loc.column_end, 2);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::end_of_file);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::end_of_file);
}

TEST(TokenizerTest, UnknownChar) {
  std::string s;

  s = "fun main(%) int {\n";
  s += "    ret 0\n";
  s += "}\n";

  std::istringstream ss(s);
  ni::ast::tokenizer t("file.ni", &ss);

 auto next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::fun);
  EXPECT_EQ(next.loc.line_begin, 1);
  EXPECT_EQ(next.loc.line_end, 1);
  EXPECT_EQ(next.loc.column_begin, 1);
  EXPECT_EQ(next.loc.column_end, 4);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::identifier);
  EXPECT_EQ(next.text, "main");
  EXPECT_EQ(next.loc.line_begin, 1);
  EXPECT_EQ(next.loc.line_end, 1);
  EXPECT_EQ(next.loc.column_begin, 5);
  EXPECT_EQ(next.loc.column_end, 9);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::open_parentheses);
  EXPECT_EQ(next.loc.line_begin, 1);
  EXPECT_EQ(next.loc.line_end, 1);
  EXPECT_EQ(next.loc.column_begin, 9);
  EXPECT_EQ(next.loc.column_end, 10);

  next = t.scan_next_token();
  EXPECT_EQ(next.tag, ni::ast::token_tag::unknown);
  EXPECT_EQ(next.text, "%");
  EXPECT_EQ(next.loc.line_begin, 1);
  EXPECT_EQ(next.loc.line_end, 1);
  EXPECT_EQ(next.loc.column_begin, 10);
  EXPECT_EQ(next.loc.column_end, 11);
}

} // namespace
} // namespace test
} // namespace ni
