#include "Join.h"
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fmi::join tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  std::setlocale(LC_ALL, "C");
  return nullptr;
}

BOOST_AUTO_TEST_CASE(join_vector_of_strings)
{
  BOOST_TEST_MESSAGE("Fmi::join std::vector<std::string>");

  std::vector<std::string> empty_vector = {};
  BOOST_CHECK_EQUAL(Fmi::join(empty_vector), "");

  std::vector<std::string> one_element = {"foo"};
  BOOST_CHECK_EQUAL(Fmi::join(one_element), "foo");

  std::vector<std::string> two_elements = {"foo", "bar"};
  BOOST_CHECK_EQUAL(Fmi::join(two_elements), "foo,bar");
}

BOOST_AUTO_TEST_CASE(join_vector_of_ints)
{
  BOOST_TEST_MESSAGE("Fmi::join std::vector<int>");

  std::vector<int> empty_vector = {};
  BOOST_CHECK_EQUAL(Fmi::join(empty_vector), "");

  std::vector<int> one_element = {1};
  BOOST_CHECK_EQUAL(Fmi::join(one_element), "1");

  std::vector<int> two_elements = {2, 1};
  BOOST_CHECK_EQUAL(Fmi::join(two_elements), "2,1");
}

BOOST_AUTO_TEST_CASE(join_with_separator)
{
  BOOST_TEST_MESSAGE("Fmi::join std::vector<std::string> with '; '");

  std::vector<std::string> empty_vector = {};
  BOOST_CHECK_EQUAL(Fmi::join(empty_vector, "; "), "");

  std::vector<std::string> one_element = {"foo"};
  BOOST_CHECK_EQUAL(Fmi::join(one_element, "; "), "foo");

  std::vector<std::string> two_elements = {"foo", "bar"};
  BOOST_CHECK_EQUAL(Fmi::join(two_elements, "; "), "foo; bar");
}

BOOST_AUTO_TEST_CASE(join_vector_of_chars)
{
  BOOST_TEST_MESSAGE("Fmi::join std::vector<char*>");

  std::vector<const char*> values = {"foo", "bar", "etc"};
  BOOST_CHECK_EQUAL(Fmi::join(values), "foo,bar,etc");
}

BOOST_AUTO_TEST_CASE(join_set_of_strings)
{
  BOOST_TEST_MESSAGE("Fmi::join std::set<std::string>");
  std::set<std::string> values = {"foo", "bar", "etc"};
  BOOST_CHECK_EQUAL(Fmi::join(values), "bar,etc,foo");
}

BOOST_AUTO_TEST_CASE(join_field_from_struct)
{
  BOOST_TEST_MESSAGE("Fmi::join std::vector<struct.field>");

  struct Foo
  {
    Foo(const std::string& name, int value) : m_name(name), m_value(value) {}
    std::string m_name;
    int m_value;
  };

  std::vector<Foo> values = {{"Jack", 1}, {"Jones", 2}};
  BOOST_CHECK_EQUAL(Fmi::join(values, [](const auto& value) { return value.m_name; }),
                    "Jack,Jones");
  BOOST_CHECK_EQUAL(Fmi::join(values, [](const auto& value) { return value.m_value; }), "1,2");
}
