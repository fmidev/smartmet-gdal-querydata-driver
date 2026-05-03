// ======================================================================
/*!
 * \file
 * \brief Regression tests for Fmi::AsyncTask
 */
// ======================================================================

#include "AsyncTask.h"
#include "DateTime.h"
#include "TypeName.h"
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <regression/tframe.h>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

namespace microsec_clock = Fmi::date_time::MicrosecClock;

namespace AsyncTaskTest
{
struct TestException
{
};

// Verify that there is no corruption when wait() is not called
void not_waiting_for_result()
{
  std::atomic<int> test(0);

  std::unique_ptr<Fmi::AsyncTask>
    task(new Fmi::AsyncTask("not waiting for result",
			    [&]()
			    {
			      boost::this_thread::sleep_for(boost::chrono::seconds(5));
			      test = 1;
			    }));
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  Fmi::DateTime t1 = microsec_clock::universal_time();
  task.reset();
  Fmi::DateTime t2 = microsec_clock::universal_time();
  if ((t2 - t1).total_milliseconds() > 200)
  {
    // std::cout << "\n" << (t2 - t1) << std::endl;
    TEST_FAILED("Task not stopped as expected when wait for completion missing");
  }
  if (test != 0)
  {
    TEST_FAILED("Variable not upFmi::Dated as expected by task");
  }
  TEST_PASSED();
}

void not_waiting_for_result_2()
{
  std::unique_ptr<Fmi::AsyncTask> task(
      new Fmi::AsyncTask("not waiting for result", [&]() { throw TestException(); }));
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  task.reset();
  TEST_PASSED();
}

void single_task()
{
  std::atomic<int> test(0);
  Fmi::AsyncTask task("", [&]() { test++; });
  task.wait();
  if (test != 1)
  {
    TEST_FAILED("Not expected result");
  }
  if (task.get_status() != Fmi::AsyncTask::ok)
  {
    TEST_FAILED("Not expected test status");
  }
  TEST_PASSED();
}

void task_throws_not_std_exception()
{
  bool got_expected_exception = false;
  std::string exception_name;
  std::string task_name;
  Fmi::AsyncTask task("foobar", []() { throw TestException(); });
  try
  {
    task.wait();
    TEST_FAILED("Expection not thrown");
  }
  catch (const TestException&)
  {
    got_expected_exception = true;
  }
  catch (const tframe::failed&)
  {
    throw;
  }
  catch (...)
  {
    TEST_FAILED("Unexcpected exception of type '" + Fmi::current_exception_type() + "' thrown");
  }
  if (!got_expected_exception)
  {
    TEST_FAILED("Did not get expected exception TestException from AsyncTask");
  }
  if (task.get_status() != Fmi::AsyncTask::failed)
  {
    TEST_FAILED("Not expected test status");
  }
  TEST_PASSED();
}

void test_cancel_task()
{
  Fmi::DateTime tm1, tm2, tm3;

  std::ostringstream debug_info;

  try {
    Fmi::DateTime t1 = microsec_clock::universal_time();
    Fmi::AsyncTask
      task("foobar",
	   [&]()
	   {
	     try {
	       tm1 = microsec_clock::universal_time();
	       boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
	       tm2 = microsec_clock::universal_time();
	     } catch (const boost::thread_interrupted&) {
	       tm3 = microsec_clock::universal_time();
	       throw;
	     }
	   });

    Fmi::DateTime t2 = microsec_clock::universal_time();

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    Fmi::DateTime t3 = microsec_clock::universal_time();
    task.cancel();
    task.wait();
    Fmi::DateTime t4 = microsec_clock::universal_time();

    debug_info << "started            : " << t1 << '\n'
	       << "task created       : " << t2 << '\n'
	       << "task started       : " << tm1 << '\n'
	       << "task ended         : " << tm2 << " (should not happen)\n"
	       << "got interrupted    : " << tm3 << '\n'
	       << "about to interrupt : " << t3 << '\n'
	       << "done               : " << t4 << '\n'
	       << "dt                 : " << (t4-t3) << std::endl;

    if (task.get_status() != Fmi::AsyncTask::interrupted)
      {
	TEST_FAILED("Not expected test status");
      }

    const auto dt = (t2 - t1).total_milliseconds();
    if (dt > 100)
      {
	TEST_FAILED("Canceling task took too long time (" + std::to_string(dt) + "milliseconds)");
      }

  }
  catch (const tframe::failed&)
  {
    std::cout << std::endl << debug_info.str();
    throw;
  }
  TEST_PASSED();
}

void test_notify()
{
  Fmi::DateTime tm1, tm2, tm3;
  std::mutex m;
  std::condition_variable cond;
  bool ended = false;
  const Fmi::DateTime t1 = microsec_clock::universal_time();
  Fmi::AsyncTask task(
      "foobar",
      [&tm1, &tm2]()
      {
	tm1 = microsec_clock::universal_time();
	std::this_thread::sleep_for(std::chrono::milliseconds(5));
	tm2 = microsec_clock::universal_time();
      },
      [&cond, &m, &ended, &tm3]()
      {
	std::unique_lock<std::mutex> lock(m);
	ended = true;
	cond.notify_all();
	tm3 = microsec_clock::universal_time();
      });
  const Fmi::DateTime t2 = microsec_clock::universal_time();

  std::unique_lock<std::mutex> lock(m);
  const auto got_notification =
    cond.wait_for(lock,
		   std::chrono::milliseconds(5000),
		  [&task]()
		  {
		    return task.ended();
		  });

  const Fmi::DateTime t3 = microsec_clock::universal_time();

  std::ostringstream debug_info;
  debug_info << "--------------- Test notify ----------------\n";
  debug_info << "started            : " << t1 << '\n';
  debug_info << "task created       : " << t2-t1 << '\n';
  debug_info << "timed wait done    : " << t3-t1 << " (" << t3-t2 << '\n';
  debug_info << "task started       : " << tm1-t1 << '\n';
  debug_info << "task ended         : " << tm2-t1 << '\n';
  debug_info << "notification sent  : " << t3-t1 << '\n';

  if (!got_notification)
  {
    std::cout << std::endl << debug_info.str() << std::endl;
    TEST_FAILED("Did not get notification about task end");
    task.cancel();
  }
  lock.unlock();

  TEST_PASSED();
}

// Stopping already finished task should not influence the status
void stop_attempt_after_task_end()
{
  std::mutex m;
  std::condition_variable cond;
  Fmi::AsyncTask task(
      "foobar",
      []()
      {
        /* Do nothing */
      },
      [&cond]() { cond.notify_all(); });

  std::unique_lock<std::mutex> lock(m);
  if (!cond.wait_for(lock, std::chrono::milliseconds(100), [&task]() { return task.ended(); }))
  {
    TEST_FAILED("Did not get notification about task end");
    task.cancel();
  }
  lock.unlock();

  task.cancel();

  task.wait();
  if (task.get_status() != Fmi::AsyncTask::ok)
  {
    TEST_FAILED("Not expected test status");
  }

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
    Fmi::AsyncTask::silent = true;
    //Fmi::AsyncTask::log_time = true;

    TEST(not_waiting_for_result);
    TEST(not_waiting_for_result_2);
    TEST(single_task);
    TEST(task_throws_not_std_exception);
    TEST(test_cancel_task);
    TEST(test_notify);
    TEST(stop_attempt_after_task_end);
  }
};

}  // namespace AsyncTaskTest

int main(void)
{
  std::cout << std::endl << "AsyncTask tester" << std::endl << "=====================" << std::endl;
  AsyncTaskTest::tests t;
  return t.run();
}
