// ======================================================================
/*!
 * \file
 * \brief Regression tests for FastMath.StringConversion.h
 */
// ======================================================================

#include "FastMath.h"
#include <boost/test/included/unit_test.hpp>
#include <cmath>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fast math tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

BOOST_AUTO_TEST_CASE(fmifloor)
{
  BOOST_TEST_MESSAGE(" + Fmi::floor()");
  BOOST_CHECK_EQUAL(std::floor(0.1), Fmi::floor(0.1));
  BOOST_CHECK_EQUAL(std::floor(0.5), Fmi::floor(0.5));
  BOOST_CHECK_EQUAL(std::floor(1.5), Fmi::floor(1.5));
  BOOST_CHECK_EQUAL(std::floor(-0.1), Fmi::floor(-0.1));
  BOOST_CHECK_EQUAL(std::floor(-0.5), Fmi::floor(-0.5));
  BOOST_CHECK_EQUAL(std::floor(-1.5), Fmi::floor(-1.5));
}

BOOST_AUTO_TEST_CASE(fmiceil)
{
  BOOST_TEST_MESSAGE(" + Fmi::ceil()");
  BOOST_CHECK_EQUAL(std::ceil(0.1), Fmi::ceil(0.1));
  BOOST_CHECK_EQUAL(std::ceil(0.5), Fmi::ceil(0.5));
  BOOST_CHECK_EQUAL(std::ceil(1.5), Fmi::ceil(1.5));
  BOOST_CHECK_EQUAL(std::ceil(-0.1), Fmi::ceil(-0.1));
  BOOST_CHECK_EQUAL(std::ceil(-0.5), Fmi::ceil(-0.5));
  BOOST_CHECK_EQUAL(std::ceil(-1.5), Fmi::ceil(-1.5));
}
