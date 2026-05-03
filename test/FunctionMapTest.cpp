// ======================================================================
/*!
 * \file
 * \brief Regression tests for FunctionMap.h
 */
// ======================================================================

#include "FunctionMap.h"
#include "DebugTools.h"
#include <functional>
#include <iostream>
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fmi::FunctionMap tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

BOOST_AUTO_TEST_CASE(test_with_std_function)
{
    BOOST_TEST_MESSAGE("Testing with std::function");
    Fmi::FunctionMap<std::string, const std::string&> W;
    W.add("foo",
        [](const std::string& x) -> std::string
        {
            return "foo: '" + x + "'";
        });
    W.add("bar",
        [](const std::string& x) -> std::string
        {
            return "bar: '" + x + "'";
        });
    std::string result;
    BOOST_REQUIRE_NO_THROW(result = W("foo", "hello"));
    BOOST_CHECK_EQUAL(result, std::string("foo: 'hello'"));

    BOOST_REQUIRE_NO_THROW(result = W("bar", "world"));
    BOOST_CHECK_EQUAL(result, std::string("bar: 'world'"));

    // No function defined for name (should throw)
    BOOST_REQUIRE_THROW(W("baz", "bad"), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(test_with_std_function_and_vector)
{
    BOOST_TEST_MESSAGE("Testing with exact names");
    Fmi::FunctionMap<std::string, const std::string&> W;

    W.add("foo", [](const std::string& x) -> std::string { return "foo: '" + x + "'"; })
     .add("bar", [](const std::string& x) -> std::string { return "bar: '" + x + "'"; })
     .add("baz", [](const std::string& x) -> std::string { return "baz: '" + x + "'"; });

    std::string result;
    BOOST_REQUIRE_NO_THROW(result = W("foo", "hello"));
    BOOST_CHECK_EQUAL(result, std::string("foo: 'hello'"));
    BOOST_REQUIRE_NO_THROW(result = W("bar", "world"));
    BOOST_CHECK_EQUAL(result, std::string("bar: 'world'"));
}

BOOST_AUTO_TEST_CASE(test_with_std_function_and_vector_of_names)
{
    BOOST_TEST_MESSAGE("Testing with vector of names");
    Fmi::FunctionMap<std::string, const std::string&> W;

    W.add(
        {"foo", "bar", "baz"},
        [](const std::string& x) -> std::string { return "foo: '" + x + "'"; });
    std::string result;
    BOOST_REQUIRE_NO_THROW(result = W("foo", "hello"));
    BOOST_CHECK_EQUAL(result, std::string("foo: 'hello'"));
    BOOST_REQUIRE_NO_THROW(result = W("bar", "world"));
    BOOST_CHECK_EQUAL(result, std::string("foo: 'world'"));
    BOOST_REQUIRE_NO_THROW(result = W("baz", "world"));
    BOOST_CHECK_EQUAL(result, std::string("foo: 'world'"));
}

BOOST_AUTO_TEST_CASE(test_regex_entry)
{
    BOOST_TEST_MESSAGE("Testing with exact names and boost::regex");
    Fmi::FunctionMap<std::string, const std::string&> W;

    W.add("foo", [](const std::string& x) -> std::string { return "foo: '" + x + "'"; })
     .add("bar", [](const std::string& x) -> std::string { return "bar: '" + x + "'"; })
     .add("baz", [](const std::string& x) -> std::string { return "baz: '" + x + "'"; })
     .add("Regex test", boost::regex("FOO\\(([^\\)]+)\\)"),
          [](const std::vector<std::string>& args, const std::string& x) -> std::string
          {
            return "regex: '" + args.at(0) + "'";
          });

    std::string result;
    BOOST_REQUIRE_NO_THROW(result = W("foo", "hello"));
    BOOST_CHECK_EQUAL(result, std::string("foo: 'hello'"));
    BOOST_REQUIRE_NO_THROW(result = W("bar", "world"));
    BOOST_CHECK_EQUAL(result, std::string("bar: 'world'"));
    BOOST_REQUIRE_NO_THROW(result = W("baz", "world"));
    BOOST_CHECK_EQUAL(result, std::string("baz: 'world'"));
    BOOST_REQUIRE_NO_THROW(result = W("FOO(abc)", "ignored"));
    BOOST_CHECK_EQUAL(result, std::string("regex: 'abc'"));
}