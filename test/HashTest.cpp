// ======================================================================
/*!
 * \file
 * \brief Regression tests for Hash.h
 */
// ======================================================================

#include "Hash.h"
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Hash tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

BOOST_AUTO_TEST_CASE(hash_value)
{
  BOOST_TEST_MESSAGE(" + Fmi::hash_value()");

  // We excpect all results to be roughly 64 bits in size and greatly different
  BOOST_CHECK_EQUAL(5650585516096373764UL, Fmi::hash_value(false));
  BOOST_CHECK_EQUAL(5650585516096373764UL, Fmi::hash_value(0));
  BOOST_CHECK_EQUAL(11755695917061617349UL, Fmi::hash_value(true));
  BOOST_CHECK_EQUAL(11755695917061617349UL, Fmi::hash_value(1));
  BOOST_CHECK_EQUAL(14907161335529820101UL, Fmi::hash_value(2));
}

BOOST_AUTO_TEST_CASE(array_hash)
{
  BOOST_TEST_MESSAGE(" + Fmi::hash_value()");

  std::array<int, 3> arr = {1, 2, 3};
  std::size_t hash1 = 54241748134;
  Fmi::hash_merge(hash1, arr[0], arr[1], arr[2]);
  std::size_t hash2 = Fmi::hash_value(arr);
  BOOST_CHECK_EQUAL(hash1, hash2);
}

BOOST_AUTO_TEST_CASE(hash_combine)
{
  BOOST_TEST_MESSAGE(" + Fmi::hash_combine()");

  std::size_t hash = 0;
  Fmi::hash_combine(hash, Fmi::hash_value(false));
  BOOST_CHECK_EQUAL(17566711995980968168UL, hash);

  hash = 0;
  Fmi::hash_combine(hash, Fmi::hash_value(true));
  BOOST_CHECK_EQUAL(924864021926404432UL, hash);
}

BOOST_AUTO_TEST_CASE(hash_merge)
{
  BOOST_TEST_MESSAGE(" + Fmi::hash_merge()");

  std::size_t hash = 0;
  Fmi::hash_merge(hash, false);
  BOOST_CHECK_EQUAL(17566711995980968168UL, hash);

  hash = 0;
  Fmi::hash_merge(hash, true);
  BOOST_CHECK_EQUAL(924864021926404432UL, hash);
}

BOOST_AUTO_TEST_CASE(variadic_hash_template_1)
{
  BOOST_TEST_MESSAGE(" + Fmi::hash variadic template function (1/3");
  const std::string str = "Hello, World!";
  const std::vector<double> vec = {1.0, 2.0, 3.0};
  const int64_t i = 42;

  std::size_t hash1 = Fmi::hash_value(str);
  Fmi::hash_combine(hash1, Fmi::hash_value(vec));
  Fmi::hash_combine(hash1, Fmi::hash_value(i));

  std::size_t hash2 = Fmi::hash(str, vec, i);
  BOOST_CHECK_EQUAL(hash1, hash2);

  // Must check also that the variadic template works with a single argument
  BOOST_CHECK_EQUAL(Fmi::hash(str), Fmi::hash_value(str));
}

BOOST_AUTO_TEST_CASE(variadic_hash_template_2)
{
  BOOST_TEST_MESSAGE(" + Fmi::hash variadic template function (2/3)");
  const std::string str = "Hello, World!";
  const std::vector<double> vec = {1.0, 2.0, 3.0};
  const int64_t i = 42;

  std::size_t hash1 = Fmi::hash_value(str);
  Fmi::hash_combine(hash1, Fmi::hash_value(vec));
  Fmi::hash_combine(hash1, Fmi::hash_value(i));

  // Case when one of hashes is already calculated (e.g. returned of method call of some class)
  std::size_t hash2 = Fmi::hash(str, Fmi::HashValue(Fmi::hash_value(vec)), i);
  BOOST_CHECK_EQUAL(hash1, hash2);
}

BOOST_AUTO_TEST_CASE(variadic_hash_template_3)
{
  BOOST_TEST_MESSAGE(" + Fmi::hash variadic template function (3/3)");
  const std::string str = "Hello, World!";
  const std::vector<double> vec = {1.0, 2.0, 3.0};
  const int64_t i = 42;
  const std::optional<int> opt_i1;
  const std::optional<int> opt_i2 = 100;

  std::size_t hash1 = Fmi::hash_value(str);
  std::size_t hash2 = hash1;
  Fmi::hash_combine(hash1, Fmi::hash_value(vec));
  Fmi::hash_combine(hash1, Fmi::hash_value(i));
  Fmi::hash_combine(hash1, Fmi::hash_value(opt_i1));
  Fmi::hash_combine(hash1, Fmi::hash_value(opt_i2));

  // Case when one of hashes is already calculated (e.g. returned of method call of some class)
  Fmi::hash_merge(hash2, vec, i, opt_i1, opt_i2);
  BOOST_CHECK_EQUAL(hash1, hash2);
}

namespace
{
struct TestHashable1
{
  std::string name;
  int value;

  TestHashable1(const std::string& n, int v) : name(n), value(v) {}

  std::size_t HashValue() const { return Fmi::hash(name, value); }
};

struct TestHashable2
{
  std::string name;
  int value;

  TestHashable2(const std::string& n, int v) : name(n), value(v) {}

  std::size_t hashValue() const { return Fmi::hash(name, value); }

  std::size_t hash_value() const
  {
    abort();
    // This method should not be called, it is here to test
    // that hash_value() is not used and also does not cause ambiguity
  }
};

struct TestHashable3
{
  std::string name;
  int value;

  TestHashable3(const std::string& n, int v) : name(n), value(v) {}

  std::size_t hash_value() const { return Fmi::hash(name, value); }
};
}  // namespace

BOOST_AUTO_TEST_CASE(hashable)
{
  BOOST_TEST_MESSAGE(" + Fmi::hash_value for hashable objects (have method HashValue)");

  const std::string name1 = "Test";
  const int value1 = 42;
  TestHashable1 obj1(name1, value1);
  TestHashable2 obj2(name1, value1);
  TestHashable3 obj3(name1, value1);

  std::size_t hash1 = Fmi::hash(name1, value1);
  std::size_t hash2 = Fmi::hash_value(obj1);
  std::size_t hash3 = Fmi::hash_value(obj2);
  std::size_t hash4 = Fmi::hash_value(obj3);

  BOOST_CHECK_EQUAL(hash1, hash2);
  BOOST_CHECK_EQUAL(hash1, hash3);
  BOOST_CHECK_EQUAL(hash1, hash4);
}
