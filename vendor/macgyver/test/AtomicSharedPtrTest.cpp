// ======================================================================
/*!
 * \file
 * \brief Regression tests for AtomicSharedPtr.h
 */
// ======================================================================

#include "AtomicSharedPtr.h"
#include "Exception.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/test/included/unit_test.hpp>
#include <algorithm>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "AtomicSharedPtr tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

BOOST_AUTO_TEST_CASE(atomic_shared_ptr_constructor_1)
{
  std::shared_ptr<std::string> foo = std::make_shared<std::string>("not empty now");
  Fmi::AtomicSharedPtr<std::string> ptr(foo);
  BOOST_CHECK(bool(ptr.load()));
  BOOST_CHECK_EQUAL(*ptr.load(), "not empty now");
}

BOOST_AUTO_TEST_CASE(atomic_shared_ptr_test_1)
{
  Fmi::AtomicSharedPtr<std::string> ptr;
  std::shared_ptr<std::string> foo = std::make_shared<std::string>("not empty now");
  ptr.store(foo);
  BOOST_CHECK_EQUAL(ptr.load(), foo);
}

BOOST_AUTO_TEST_CASE(atomic_shared_ptr_test_2)
{
  Fmi::AtomicSharedPtr<std::string> ptr;
  std::shared_ptr<std::string> foo = std::make_shared<std::string>("not empty now");
  ptr.store(foo);
  BOOST_CHECK_EQUAL(ptr.load(), foo);
  std::shared_ptr<std::string> bar = std::make_shared<std::string>("not empty now either");
  ptr.store(bar);
  BOOST_CHECK_EQUAL(ptr.load(), bar);
  ptr.reset();
  std::shared_ptr<std::string> p2 = ptr.load();
  BOOST_CHECK(!p2);
}
