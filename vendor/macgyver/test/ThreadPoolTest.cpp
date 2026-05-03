#include "ThreadPool.h"
#include <atomic>
#include <iostream>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fmi::ThreadPool tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  std::setlocale(LC_ALL, "C");
  return nullptr;
}

BOOST_AUTO_TEST_CASE(test_thread_pool_1)
{
  std::atomic<int> count = 0;
  Fmi::ThreadPool::ThreadPool pool(4, 6);
  pool.setGracefulShutdown(true);
  pool.start();

  for (int i = 0; i < 10; ++i)
  {
    bool inserted = false;
    do
    {
        inserted = pool.schedule(
            [&count]() {
                boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
                count++;
            });
        //std::cout << "i=" << i << " inserted=" << inserted << std::endl;
        if (!inserted)
        {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
        }
    } while (!inserted);
  }

  pool.join();
  pool.shutdown();
  BOOST_CHECK_EQUAL(count, 10);
}
