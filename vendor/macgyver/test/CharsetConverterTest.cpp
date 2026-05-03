// ======================================================================
/*!
 * \file
 * \brief Regression tests for CharsetConverter.h
 */
// ======================================================================

#include "CharsetConverter.h"
#include "Exception.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/test/included/unit_test.hpp>
#include <algorithm>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "String charset conversions tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

BOOST_AUTO_TEST_CASE(latin1_utf8_test_1)
{
  Fmi::CharsetConverter conv("latin1", "UTF-8");
  BOOST_CHECK_EQUAL("ä", conv.convert("\344"));

  std::string src, out, exp;
  for (int i = 0; i < 16384; i++)
  {
    src += "\344";
    exp += "ä";
  }
  out = conv.convert(src);
  BOOST_CHECK(out == exp);
}

BOOST_AUTO_TEST_CASE(latin7_utf8_test_1)
{
  Fmi::CharsetConverter conv1("latin7", "UTF-8");
  Fmi::CharsetConverter conv2("UTF-8", "latin7");

  std::string s1 = "\315ie\354elis";
  std::string s2 = "Ķieģelis";
  BOOST_CHECK_EQUAL(s2, conv1.convert(s1));
  BOOST_CHECK_EQUAL(s1, conv2.convert(s2));
}
