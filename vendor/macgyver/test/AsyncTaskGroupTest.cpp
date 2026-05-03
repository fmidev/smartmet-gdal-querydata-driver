// ======================================================================
/*!
 * \file
 * \brief Regression tests for Fmi::AsyncTaskGroup
 */
// ======================================================================

#include "AsyncTaskGroup.h"
#include "DateTime.h"
#include "Exception.h"
#include "TypeName.h"
#include "DateTime.h"
#include <regression/tframe.h>
#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <thread>


namespace microsec_clock = Fmi::date_time::MicrosecClock;

namespace AsyncTaskGroupTest
{
// Verify that there is no corruption when wait() is not called
void not_waiting_for_result()
{
  std::unique_ptr<std::string> test(new std::string);
  std::unique_ptr<Fmi::AsyncTaskGroup> tg(new Fmi::AsyncTaskGroup);
  tg->add("not waiting for result",
          [&]()
          {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            *test = "foobar";
          });
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  tg.reset();
  test.reset();
  TEST_PASSED();
}

void empty_group()
{
  Fmi::AsyncTaskGroup tg;
  tg.wait();
  TEST_PASSED();
}

void single_task()
{
  std::atomic<int> test(0);
  Fmi::AsyncTaskGroup tg;
  tg.add("", [&]() { test++; });
  tg.wait();
  if (test != 1)
  {
    TEST_FAILED("Not expected result");
  }
  TEST_PASSED();
}

void several_tasks()
{
  std::atomic<int> test(0);
  Fmi::AsyncTaskGroup tg;
  for (int i = 0; i < 10; i++)
  {
    tg.add("", [&]() { test++; });
  }
  tg.wait();
  if (test != 10)
  {
    TEST_FAILED("Not expected result");
  }
  TEST_PASSED();
}

struct TestException
{
};

void task_throws_not_std_exception()
{
  std::string exception_name;
  std::string task_name;
  Fmi::AsyncTaskGroup tg;
  tg.on_task_error(
      [&](const std::string& name)
      {
        exception_name = Fmi::current_exception_type();
        task_name = name;
      });
  tg.add("foobar", []() { throw TestException(); });
  tg.wait();
  if (exception_name != "AsyncTaskGroupTest::TestException")
  {
    TEST_FAILED("Did not get exception name");
  }
  if (task_name != "foobar")
  {
    TEST_FAILED("Did not get task name");
  }
  TEST_PASSED();
}

void large_number_of_tasks()
{
  bool failed = false;
  std::atomic<int> test(0);
  Fmi::AsyncTaskGroup tg(10);
  for (int i = 0; i < 100; i++)
  {
    tg.add("test",
           [&]()
           {
             std::this_thread::sleep_for(std::chrono::milliseconds(10));
             test++;
           });
  }
  if (not failed and tg.get_num_active_tasks() > 10)
  {
    TEST_FAILED("Too many parallel tasks");
    failed = true;
  }
  tg.wait();
  if (test != 100)
  {
    TEST_FAILED("Not all tasks executed");
  }
  TEST_PASSED();
}

void test_interrupting_1()
{
  std::unique_ptr<Fmi::AsyncTaskGroup> tg(new Fmi::AsyncTaskGroup(10));
  for (int i = 0; i < 5; i++)
  {
    tg->add("test", []() { boost::this_thread::sleep_for(boost::chrono::milliseconds(1000)); });
  }

  const Fmi::DateTime t1 = microsec_clock::universal_time();
  tg->stop();
  tg->wait();
  const Fmi::DateTime t2 = microsec_clock::universal_time();
  tg.reset();
  const Fmi::DateTime t3 = microsec_clock::universal_time();
  // std::cout << '\n' << (t2-t1) << std::endl;
  // std::cout << '\n' << (t3-t1) << std::endl;
  if ((t2 - t1).total_milliseconds() > 50)
  {
    TEST_FAILED("Did not stop in time");
  }
  if ((t3 - t1).total_milliseconds() > 50)
  {
    TEST_FAILED("Stuck in destructor for too long time");
  }
  TEST_PASSED();
}

void try_adding_task_after_stop_request()
{
  std::atomic<int> counter(0);
  std::unique_ptr<Fmi::AsyncTaskGroup> tg(new Fmi::AsyncTaskGroup(10));
  tg->add("task 1", [&counter]() { counter++; });
  tg->wait();
  tg->stop();
  tg->add("task ", [&counter]() { counter++; });
  tg->wait();
  switch (int(counter))
  {
    case 0:
      TEST_FAILED("No task executed");
      break;
    case 1:
      break;
    case 2:
      TEST_FAILED("Task added after stop should not be executed");
      break;
    default:
      TEST_FAILED("Unexpected value of counter");
      break;
  }
  TEST_PASSED();
}

void test_stop_on_error_1()
{
  std::atomic<int> counter(0);
  const auto task_1 = [&]()
  {
    for (int i = 0; i < 10; i++)
    {
      Fmi::AsyncTask::interruption_point();
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    counter++;
  };

  const auto task_2 = []() { throw std::runtime_error("An error"); };

  std::unique_ptr<Fmi::AsyncTaskGroup> tg(new Fmi::AsyncTaskGroup(10));
  tg->stop_on_error(true);
  tg->add("task 1a", task_1);
  tg->add("task 1b", task_1);
  tg->add("task_2", task_2);

  const Fmi::DateTime t1 = microsec_clock::universal_time();

  try
  {
    tg->wait();
    TEST_FAILED("Excepted exception not thrown");
  }
  catch (const tframe::failed&)
  {
    throw;
  }
  catch (const Fmi::Exception&)
  {
  }

  const Fmi::DateTime t2 = microsec_clock::universal_time();
  if ((t2 - t1).total_milliseconds() > 30)
  {
    TEST_FAILED("Did not stop in time");
  }

  if (counter != 0)
  {
    TEST_FAILED("Tasks not stopped as expected");
  }

  TEST_PASSED();
}

void test_stop_on_error_2()
{
  std::string tmp;
  std::string* tmp_ptr = &tmp;

  const auto task_1 = [&]()
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    *tmp_ptr = "foo";
  };

  const auto task_2 = []() { throw std::runtime_error("An error"); };

  std::unique_ptr<Fmi::AsyncTaskGroup> tg(new Fmi::AsyncTaskGroup(10));
  tg->stop_on_error(true);
  tg->add("task 1", task_1);
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  tg->add("task_2", task_2);

  try
  {
    tg->wait();
    TEST_FAILED("Excepted exception not thrown");
  }
  catch (const tframe::failed&)
  {
    throw;
  }
  catch (const Fmi::Exception&)
  {
  }

  tmp_ptr = nullptr;

  tg->wait();

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * The actual test suite
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  virtual const char* error_message_prefix() const { return "\n\t"; }

  void test(void)
  {
    TEST(not_waiting_for_result);
    TEST(empty_group);
    TEST(single_task);
    TEST(several_tasks);
    TEST(task_throws_not_std_exception);
    TEST(large_number_of_tasks);
    TEST(test_interrupting_1);
    TEST(try_adding_task_after_stop_request);
    TEST(test_stop_on_error_1);
    TEST(test_stop_on_error_2);
  }
};

}  // namespace AsyncTaskGroupTest

int main(void)
{
  std::cout << std::endl
            << "AsyncTaskGroup tester" << std::endl
            << "=====================" << std::endl;
  AsyncTaskGroupTest::tests t;
  return t.run();
}
