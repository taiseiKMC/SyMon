#include "../src/io_operators.hh"
#include "common_types.hh"
#include "non_symbolic_number_constraint.hh"
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(NonSymbolicNumberConstraintTests)
  BOOST_AUTO_TEST_SUITE(LexicalCastTests)
    using boost::lexical_cast;

    BOOST_AUTO_TEST_CASE(variable) {
      std::string str = "x12";
      using type = NonSymbolic::NumberExpression<double>;
      type result = lexical_cast<type>(str);
      BOOST_CHECK_EQUAL(std::get<VariableID>(result.child), 12);
      BOOST_CHECK_EQUAL(result.kind, NonSymbolic::NumberExpressionKind::ATOM);
    }

    BOOST_AUTO_TEST_CASE(constantInt) {
      std::string str = "123";
      using type = NonSymbolic::NumberExpression<int>;
      type result = lexical_cast<type>(str);
      BOOST_CHECK_EQUAL(std::get<int>(result.child), 123);
      BOOST_CHECK_EQUAL(result.kind, NonSymbolic::NumberExpressionKind::CONSTANT);
    }

    BOOST_AUTO_TEST_CASE(constantDouble) {
      std::string str = "1.25";
      using type = NonSymbolic::NumberExpression<double>;
      type result = lexical_cast<type>(str);
      BOOST_CHECK_EQUAL(std::get<double>(result.child), 1.25);
      BOOST_CHECK_EQUAL(result.kind, NonSymbolic::NumberExpressionKind::CONSTANT);
    }

    BOOST_AUTO_TEST_CASE(operators) {
      std::string str = "1.5 + x1 - 10";
      using type = NonSymbolic::NumberExpression<double>;
      type result = lexical_cast<type>(str);
      using T = std::array<std::shared_ptr<type>, 2>;
      BOOST_CHECK(std::holds_alternative<T>(result.child));
      BOOST_CHECK_EQUAL(result.kind, NonSymbolic::NumberExpressionKind::MINUS);
      auto &[t0, e2] = std::get<T>(result.child);
      auto &[e0, e1] = std::get<T>(t0->child);
      BOOST_CHECK_EQUAL(t0->kind, NonSymbolic::NumberExpressionKind::PLUS);
      BOOST_CHECK_EQUAL(std::get<double>(e0->child), 1.5);
      BOOST_CHECK_EQUAL(e0->kind, NonSymbolic::NumberExpressionKind::CONSTANT);
      BOOST_CHECK_EQUAL(std::get<VariableID>(e1->child), 1);
      BOOST_CHECK_EQUAL(e1->kind, NonSymbolic::NumberExpressionKind::ATOM);
      BOOST_CHECK_EQUAL(std::get<double>(e2->child), 10);
      BOOST_CHECK_EQUAL(e2->kind, NonSymbolic::NumberExpressionKind::CONSTANT);
    }

    BOOST_AUTO_TEST_CASE(varEqConstant) {
      std::string str = "x0 == 0.5";
      using type = NonSymbolic::NumberConstraint<double>;
      type result = lexical_cast<type>(str);
      BOOST_CHECK_EQUAL(result.kind, NonSymbolic::NumberComparatorKind::EQ);
      BOOST_CHECK_EQUAL(std::get<VariableID>(result.children[0].child), 0);
      BOOST_CHECK_EQUAL(std::get<double>(result.children[1].child), 0.5);
    }

    BOOST_AUTO_TEST_CASE(varLtConstant) {
      std::string str = "0 < x1";
      using type = NonSymbolic::NumberConstraint<double>;
      type result = lexical_cast<type>(str);
      BOOST_CHECK_EQUAL(result.kind, NonSymbolic::NumberComparatorKind::LT);
      BOOST_CHECK_EQUAL(std::get<double>(result.children[0].child), 0);
      BOOST_CHECK_EQUAL(std::get<VariableID>(result.children[1].child), 1);
    }

    BOOST_AUTO_TEST_CASE(multipleConstraints) {
      std::string str = "{0 == x0, x1 != x2, 3 + x0 < x1 - 4}";
      using type = std::vector<NonSymbolic::NumberConstraint<double>>;
      type result = lexical_cast<type>(str);
      BOOST_CHECK_EQUAL(result.size(), 3);

      // 0 == x0
      BOOST_CHECK_EQUAL(result[0].kind, NonSymbolic::NumberComparatorKind::EQ);
      BOOST_CHECK_EQUAL(std::get<double>(result[0].children[0].child), 0);
      BOOST_CHECK_EQUAL(std::get<VariableID>(result[0].children[1].child), 0);

      // x1 != x2
      BOOST_CHECK_EQUAL(result[1].kind, NonSymbolic::NumberComparatorKind::NE);
      BOOST_CHECK_EQUAL(std::get<VariableID>(result[1].children[0].child), 1);
      BOOST_CHECK_EQUAL(std::get<VariableID>(result[1].children[1].child), 2);

      BOOST_CHECK_EQUAL(result[2].kind, NonSymbolic::NumberComparatorKind::LT);
      // Left: 3 + x0
      {
        using T = std::array<std::shared_ptr<NonSymbolic::NumberExpression<double>>, 2>;
        BOOST_CHECK(std::holds_alternative<T>(result[2].children[0].child));
        auto &[e0, e1] = std::get<T>(result[2].children[0].child);
        BOOST_CHECK_EQUAL(e0->kind, NonSymbolic::NumberExpressionKind::CONSTANT);
        BOOST_CHECK_EQUAL(std::get<double>(e0->child), 3);
        BOOST_CHECK_EQUAL(e1->kind, NonSymbolic::NumberExpressionKind::ATOM);
        BOOST_CHECK_EQUAL(std::get<VariableID>(e1->child), 0);
      }
      // Right: x1 - 4
      {
        using T = std::array<std::shared_ptr<NonSymbolic::NumberExpression<double>>, 2>;
        BOOST_CHECK(std::holds_alternative<T>(result[2].children[1].child));
        auto &[e0, e1] = std::get<T>(result[2].children[1].child);
        BOOST_CHECK_EQUAL(e0->kind, NonSymbolic::NumberExpressionKind::ATOM);
        BOOST_CHECK_EQUAL(std::get<VariableID>(e0->child), 1);
        BOOST_CHECK_EQUAL(e1->kind, NonSymbolic::NumberExpressionKind::CONSTANT);
        BOOST_CHECK_EQUAL(std::get<double>(e1->child), 4);
      }
    }

    BOOST_AUTO_TEST_SUITE_END() // LexicalCastTests
    BOOST_AUTO_TEST_SUITE_END() // NonSymbolicStringConstraintTests
