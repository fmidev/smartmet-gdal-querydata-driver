#include "Optional.h"
#include <sstream>
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Optional tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  std::setlocale(LC_ALL, "C");
  return nullptr;
}

BOOST_AUTO_TEST_CASE(write_to_stream_1)
{
  std::ostringstream out;
  std::optional<int> opt_int = 42;
  std::optional<int> empty_opt_int = std::nullopt;

  out << opt_int;
  BOOST_CHECK_EQUAL(out.str(), " 42");

  out.str("");
  out << empty_opt_int;
  BOOST_CHECK_EQUAL(out.str(), "--");
}

BOOST_AUTO_TEST_CASE(read_from_stream_1)
{
  std::istringstream s1(" 42 X");
  std::optional<int> opt_int;
  s1 >> opt_int;
  BOOST_CHECK(bool(s1));
  char c = s1.peek();
  BOOST_CHECK_EQUAL(c, ' ');
  BOOST_CHECK(bool(opt_int));
  BOOST_CHECK_EQUAL(*opt_int, 42);
}
