#include "Pool.h"
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <thread>
#include <future>

using namespace boost::unit_test;
using namespace std::string_literals;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Pool tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

BOOST_AUTO_TEST_CASE(simple_test_1)
{
  BOOST_TEST_MESSAGE("Simple test withput expanding pool");
  Fmi::Pool<Fmi::PoolInitType::Sequential, int> pool(2, 4);
  BOOST_CHECK_EQUAL(int(pool.size()), 2);
  BOOST_CHECK_EQUAL(int(pool.in_use()), 0);
  auto ptr = pool.get();
  BOOST_CHECK(ptr.get() != nullptr);
  BOOST_CHECK_EQUAL(int(pool.in_use()), 1);
  BOOST_CHECK_EQUAL(int(pool.size()), 2);
  ptr.reset();
  BOOST_CHECK_EQUAL(int(pool.in_use()), 0);
  BOOST_CHECK_EQUAL(int(pool.size()), 2);

  //pool.dumpInfo(std::cout);
}

BOOST_AUTO_TEST_CASE(simple_test_2)
{
  BOOST_TEST_MESSAGE("Simple test with expanding pool");
  Fmi::Pool<Fmi::PoolInitType::Sequential, int> pool(10, 20);

  BOOST_CHECK_EQUAL(int(pool.size()), 10);
  BOOST_CHECK_EQUAL(int(pool.in_use()), 0);

  std::vector<decltype(pool.get())> items;
  for (int i = 0; i < 15; i++)
    items.emplace_back(std::move(pool.get()));

  BOOST_CHECK_EQUAL(int(pool.in_use()), 15);
  BOOST_CHECK_EQUAL(int(pool.size()), 15);

  for (int i = 14; i >= 10; i--)
    items[i].reset();

  BOOST_CHECK_EQUAL(int(pool.in_use()), 10);
  BOOST_CHECK_EQUAL(int(pool.size()), 15);

  //pool.dumpInfo(std::cout);
}

BOOST_AUTO_TEST_CASE(factory_method_test)
{
  BOOST_TEST_MESSAGE("Factory method test");
  Fmi::Pool<Fmi::PoolInitType::Sequential, std::string, std::string> pool(
    [](const std::string& str) { return std::make_unique<std::string>(str); }, 2, 4, "hello"s);
  BOOST_CHECK_EQUAL(int(pool.size()), 2);
  BOOST_CHECK_EQUAL(int(pool.in_use()), 0);
  auto ptr = pool.get();
  BOOST_CHECK(ptr.get() != nullptr);
  BOOST_CHECK_EQUAL(int(pool.in_use()), 1);
  BOOST_CHECK_EQUAL(int(pool.size()), 2);
  BOOST_CHECK_EQUAL(*ptr.get(), "hello"s);
  ptr.reset();
  BOOST_CHECK_EQUAL(int(pool.in_use()), 0);
  BOOST_CHECK_EQUAL(int(pool.size()), 2);

  //pool.dumpInfo(std::cout);
}

namespace
{
  struct TestObj1
  {
    TestObj1(const std::string&) {}
    int value = 42;
  };

  struct TestObj2
  {
    TestObj2(const std::string&, int) {}
    int value = 42;
  };

  struct TestObj3
  {
    TestObj3() = default;

    void test() { std::this_thread::sleep_for(std::chrono::milliseconds(5)); }
  };
}

BOOST_AUTO_TEST_CASE(constructor_with_1_argument)
{
  BOOST_TEST_MESSAGE("Constructor with 1 argument test");
  Fmi::Pool<Fmi::PoolInitType::Sequential, TestObj1, std::string> pool(2, 4, "foobar"s);
  decltype(pool.get()) ptr = pool.get();
  BOOST_CHECK_EQUAL(ptr->value, 42);
}

BOOST_AUTO_TEST_CASE(constructor_with_2_arguments)
{
  BOOST_TEST_MESSAGE("Constructor with 2 arguments test");
  Fmi::Pool<Fmi::PoolInitType::Sequential, TestObj2, std::string, int> pool(2, 4, "foobar"s, 12);
  decltype(pool.get()) ptr = pool.get();
  BOOST_CHECK_EQUAL(ptr->value, 42);
}

BOOST_AUTO_TEST_CASE(parallel_use)
{
  BOOST_TEST_MESSAGE("Parallel use test");
  Fmi::Pool<Fmi::PoolInitType::Sequential, TestObj3> pool(5, 10);

  const int num_tasks = 50;
  const int iterations_per_task = 100;
  std::atomic<int> pass = 0;
  std::atomic<int> fail = 0;

  std::vector<std::shared_future<void>> tasks;
  for (int i = 0; i < num_tasks; i++)
  {
    tasks.emplace_back(std::async(std::launch::async, [&pool, &pass, &fail, iterations_per_task]() {
      for (int j = 0; j < iterations_per_task; j++)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        try
        {
          auto ptr = pool.get();
          ptr->test();
          pass++;
        }
        catch(...)
        {
          if (++fail == 1) // Report only first error
            std::cerr << Fmi::Exception::Trace(BCP, "Operation failed") << std::endl;
        }
      }
    }));
  }

  for (auto& task : tasks)
    task.wait();

  BOOST_REQUIRE_EQUAL(fail.load(), 0);
  BOOST_CHECK_EQUAL(pass.load(), num_tasks * iterations_per_task);
}

BOOST_AUTO_TEST_CASE(used_items_after_pool_destruction)
{
  BOOST_TEST_MESSAGE("Used object after pool destruction test");
  const auto getItem = []()
  {
    Fmi::Pool<Fmi::PoolInitType::Sequential, int> pool(2, 4);
    BOOST_CHECK_EQUAL(int(pool.size()), 2);
    return pool.get();
  };

  auto ptr1 = getItem(); // We hav now one item taken from pool but pool is gone
  ptr1.reset();
}

namespace
{
  struct TestObjThrow
  {
    TestObjThrow()
    {
      static std::atomic<int> count = 0;
      if ((++count % 5) == 0)
        throw Fmi::Exception(BCP, "TestObjThrow constructor exception");
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  };
}

BOOST_AUTO_TEST_CASE(parallel_initialization_with_exceptions)
{
  BOOST_TEST_MESSAGE("Parallel initialization with exceptions test");
  BOOST_CHECK(true); // To avoid "no checks" warning
  try
  {
    Fmi::Pool<Fmi::PoolInitType::Parallel, TestObjThrow> pool(10, 20);
    BOOST_FAIL("Exception expected but not thrown");
  }
  catch (const Fmi::Exception& e)
  {
    BOOST_TEST_MESSAGE("Caught expected exception: "s + e.what());
  }
}

BOOST_AUTO_TEST_CASE(sequential_initialization_with_exceptions)
{
  BOOST_TEST_MESSAGE("Sequential initialization with exceptions test");
  try
  {
    Fmi::Pool<Fmi::PoolInitType::Sequential, TestObjThrow> pool(10, 20);
    BOOST_FAIL("Exception expected but not thrown");
  }
  catch (const Fmi::Exception& e)
  {
    BOOST_CHECK(true);
  }
}
