#include "DirectoryMonitor.h"
#include "DateTime.h"
#include <boost/chrono.hpp>
#include <boost/scope_exit.hpp>
#include <boost/thread.hpp>
#include <regression/tframe.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>

namespace fs = std::filesystem;

namespace microsec_clock = Fmi::date_time::MicrosecClock;

namespace DirectoryMonitorTests
{
void stopping_test()
{
  std::atomic<bool> started(false);
  std::mutex m;
  std::condition_variable c;
  Fmi::DirectoryMonitor monitor;
  monitor.watch(
      ".",
      [](Fmi::DirectoryMonitor::Watcher,
         const fs::path&,
         const boost::regex,
         const Fmi::DirectoryMonitor::Status&) {},
      [](Fmi::DirectoryMonitor::Watcher, const fs::path&, const boost::regex&, const std::string&) {
      },
      10);
  const Fmi::DateTime start = microsec_clock::universal_time();
  std::thread t(
      [&monitor, &started, &m, &c]()
      {
	std::unique_lock<std::mutex> lock(m);
        started = true;
        c.notify_all();
	lock.unlock();
        monitor.run();
      });

  {
    std::unique_lock<std::mutex> lock(m);
    if (not started)
    {
      c.wait_for(lock, std::chrono::seconds(5), [&started]() -> bool { return started; });
    }
  }

  // const Fmi::DateTime t1 = microsec_clock::universal_time();
  // std::cout << t1 - start << std::endl;
  monitor.stop();
  t.join();

  const Fmi::DateTime done = microsec_clock::universal_time();
  if ((done - start).total_milliseconds() > 500)
  {
    std::ostringstream tmp;
    tmp << (done - start);
    TEST_FAILED("Stopping Fmi::DirectoryMonitor takes too long (" + tmp.str() + ")");
  }

  TEST_PASSED();
}

void interruption_test()
{
  std::atomic<bool> started(false);
  std::mutex m;
  std::condition_variable c;
  Fmi::DirectoryMonitor monitor;
  monitor.watch(
      ".",
      [](Fmi::DirectoryMonitor::Watcher,
         const fs::path&,
         const boost::regex,
         const Fmi::DirectoryMonitor::Status&) {},
      [](Fmi::DirectoryMonitor::Watcher, const fs::path&, const boost::regex&, const std::string&) {
      },
      10);
  boost::thread task(
      [&monitor, &started, &m, &c]()
      {
	std::unique_lock<std::mutex> lock(m);
        started = true;
        c.notify_all();
	lock.unlock();
        monitor.run();
      });

  BOOST_SCOPE_EXIT(&monitor, &task)
  {
    monitor.stop();
    task.join();
  }
  BOOST_SCOPE_EXIT_END;

  {
    std::unique_lock<std::mutex> lock(m);
    if (not started)
    {
      c.wait_for(lock, std::chrono::seconds(5), [&started]() -> bool { return started; });
    }
  }

  const Fmi::DateTime t1 = microsec_clock::universal_time();
  // std::cout << t1 - start << std::endl;
  task.interrupt();
  bool joined = task.try_join_for(boost::chrono::milliseconds(1000));
  if (joined)
  {
    const Fmi::DateTime t2 = microsec_clock::universal_time();
    const auto dt = 1.0e-6 * (t2-t1).total_microseconds();
    if (dt > 0.1)
    {
        std::cout << "\nStopping time dt seconds after interruption" << std::endl;
    }
  }
  else
  {
    const Fmi::DateTime t2 = microsec_clock::universal_time();
    const auto dt = 1.0e-6 * (t2-t1).total_microseconds();
    std::cout << "\nWait time " << dt << " seconds is too long" << std::endl;
    TEST_FAILED("Interrupting request did not stop directory monitor");
  }

  TEST_PASSED();
}

void wait_until_ready_test_1()
{
  Fmi::DateTime t2, t3;

  do {
    Fmi::DirectoryMonitor monitor;
    monitor.watch(
		  ".",
		  [](Fmi::DirectoryMonitor::Watcher,
		     const fs::path&,
		     const boost::regex,
		     const Fmi::DirectoryMonitor::Status&) {},
		  [](Fmi::DirectoryMonitor::Watcher, const fs::path&, const boost::regex&, const std::string&) {
		  },
		  10);

    boost::thread task([&monitor]() { monitor.run(); });
    t2 = microsec_clock::universal_time();

    BOOST_SCOPE_EXIT(&monitor, &task)
      {
	monitor.stop();
	task.join();
      }
    BOOST_SCOPE_EXIT_END;

    bool ok = monitor.wait_until_ready();
    if (not ok)
      {
	TEST_FAILED("Waiting for first scan returned false");
      }
  } while (false);

  // Timing could be extremly unreliable especially in virtual machines
  // (at least under Virtual Box)
  t3 = microsec_clock::universal_time();
  const auto dt = (t3 - t2).total_milliseconds();
  if (dt > 750) {
    TEST_FAILED("Waiting for test end took " + std::to_string(dt) + " millisec > 750");
  }

  TEST_PASSED();
}

void wait_until_ready_test_2()
{
  static Fmi::DateTime t1;

  do {
    Fmi::DirectoryMonitor monitor;
    monitor.watch(
		  ".",
		  [](Fmi::DirectoryMonitor::Watcher,
		     const fs::path&,
		     const boost::regex,
		     const Fmi::DirectoryMonitor::Status&) {},
		  [](Fmi::DirectoryMonitor::Watcher, const fs::path&,
		     const boost::regex&, const std::string&) {
		  },
		  10);

    boost::thread task([&monitor]() { monitor.run(); });

    BOOST_SCOPE_EXIT(&monitor, &task)
      {
	monitor.stop();
	task.join();
      }
    BOOST_SCOPE_EXIT_END;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    t1 = microsec_clock::universal_time();

    bool ok = monitor.wait_until_ready();
    if (!ok)
      {
	TEST_FAILED("Monitor already ended. Should have returned false");
      }
  } while (false);

  const auto t2 = microsec_clock::universal_time();
  const auto dt = (t2 - t1).total_milliseconds();
  if (dt > 250) {
    TEST_FAILED("Waiting for test end took " + std::to_string(dt) + " millisec > 250");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

// The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(stopping_test);
    TEST(interruption_test);
    TEST(wait_until_ready_test_1);
    TEST(wait_until_ready_test_2);
  }

};  // class tests

}  // namespace DirectoryMonitorTests

int main(void)
{
  std::cout << std::endl
            << "DirectoryMonitor tester" << std::endl
            << "=======================" << std::endl;
  DirectoryMonitorTests::tests t;
  return t.run();
}
