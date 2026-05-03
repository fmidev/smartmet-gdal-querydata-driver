// ======================================================================
/*!
 * \file
 * \brief Regression tests for StaticCleanup
 * */
// ======================================================================

#include "StaticCleanup.h"
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fmi::StaticCleanup";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  std::setlocale(LC_ALL, "C");
  return nullptr;
}

namespace
{
    std::list<std::string> list_1;
    std::list<std::string> list_2;
    Fmi::StaticCleanup cleanup_1([]() { list_1.clear(); });
    Fmi::StaticCleanup cleanup_2([]() { list_2.clear(); });
}

BOOST_AUTO_TEST_CASE(test_staticcleanup)
{
    BOOST_TEST_MESSAGE("Testing StaticCleanup");

    BOOST_CHECK(list_1.empty());
    BOOST_CHECK(list_2.empty());

    list_1.push_back("foo");
    list_2.push_back("bar");

    {
        Fmi::StaticCleanup::AtExit atexit;
        {
            Fmi::StaticCleanup::AtExit atexit;
            BOOST_CHECK(!list_1.empty());
            BOOST_CHECK(!list_2.empty());
        }
        BOOST_CHECK(!list_1.empty());
        BOOST_CHECK(!list_2.empty());
    }

    BOOST_CHECK(list_1.empty());
    BOOST_CHECK(list_2.empty());
}
