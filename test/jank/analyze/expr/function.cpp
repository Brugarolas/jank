#include <doctest/doctest.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::analyze::expr
{
  TEST_CASE("Empty")
  {
    read::lex::processor l_prc{ "(fn* [])" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context anal_ctx;
    processor anal_prc{ rt_ctx };

    auto const expr(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
    auto const *fn_expr(boost::get<function<expression>>(&expr.data));
    CHECK(fn_expr != nullptr);
    CHECK(fn_expr->local_frame.locals.empty());
    CHECK(fn_expr->body.body.empty());
  }

  TEST_CASE("Parameters")
  {
    runtime::context rt_ctx;
    context anal_ctx;
    processor anal_prc{ rt_ctx };

    SUBCASE("With local references")
    {
      read::lex::processor l_prc{ "(fn* [a b] a b)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      auto const expr(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
      auto const *fn_expr(boost::get<function<expression>>(&expr.data));
      CHECK(fn_expr != nullptr);
      CHECK(fn_expr->local_frame.locals.size() == 2);
      CHECK(fn_expr->body.body.size() == 2);
    }

    SUBCASE("Missing - nothing else")
    {
      read::lex::processor l_prc{ "(fn*)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
    }

    SUBCASE("Missing - with body")
    {
      read::lex::processor l_prc{ "(fn* 1)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
    }

    SUBCASE("Not all symbols")
    {
      read::lex::processor l_prc{ "(fn* [a 1 {}])" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
    }

    SUBCASE("Qualified symbol")
    {
      read::lex::processor l_prc{ "(fn* [foo.bar/a])" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
    }

    SUBCASE("Too many")
    {
      read::lex::processor l_prc{ "(fn* [a b c d e f g h i j k])" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
    }
  }
}
