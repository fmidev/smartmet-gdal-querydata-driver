#include "WorkerPool.h"
#include "Exception.h"
#include <chrono>
#include <condition_variable>
#include <thread>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "WorkerPoolTest";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

namespace {

    struct SlowAdd
    {
        SlowAdd() : a(0), b(0) {}

        int add(int a, int b)
        {
            // Intentional side effect
            this->a = a;
            this->b = b;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return this->a + this->b;
        }

        int a, b;
    };

    struct Test2
    {
        Test2(int x) : x(x) {}

        int test(int a)
        {
            return a + x;
        }

        int x;
    };

    struct Test3
    {
        static int num_failed;
        std::condition_variable cond;
        std::mutex m;
        bool canceled;

        Test3() : canceled(false) {}

        void test()
        {
            std::unique_lock<std::mutex> lock(m);
            if (cond.wait_for(lock, std::chrono::seconds(5)) == std::cv_status::timeout) {
                num_failed++;
            }
        }

        void cancel()
        {
            std::unique_lock<std::mutex> lock(m);
            canceled = true;
            cond.notify_one();
        }
    };

    int Test3::num_failed = 0;
}

BOOST_AUTO_TEST_CASE(simple)
{
    BOOST_TEST_MESSAGE("+ Test getting one object from pool");
    Fmi::WorkerPool<SlowAdd> pool(10, 10);
    BOOST_CHECK_EQUAL(pool.reserve()->add(2, 3), 5);
}

BOOST_AUTO_TEST_CASE(simple_no_default_constructor)
{
    BOOST_TEST_MESSAGE("+ Test getting one object from pool (no default constructor)");
    Fmi::WorkerPool<Test2> pool( []() { return std::make_shared<Test2>(5); }, 10, 10);
    BOOST_CHECK_EQUAL(pool.reserve()->test(3), 8);
}

BOOST_AUTO_TEST_CASE(shutdown)
{
    BOOST_TEST_MESSAGE("+ Test attempt getting object from pool after shutdown request");
    Fmi::WorkerPool<SlowAdd> pool(10, 10);
    BOOST_CHECK_EQUAL(pool.reserve()->add(2, 3), 5);
    pool.shutdown();
    BOOST_CHECK_THROW(pool.reserve(), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(parallel_1)
{
    BOOST_TEST_MESSAGE("+ Test parallel requests of using objects from pool (object count limit not reached)");
    std::atomic<int> num_errors(0);
    std::atomic<int> num_tests(0);
    Fmi::WorkerPool<SlowAdd> pool(10, 10);

    const auto thread_proc = [&](int a0, int b0)
    {
        for (int i = 0; i < 10; i++) {
            int a = a0 + 23*i;
            int b = b0 + 17*i;
            num_tests++;
            if (pool.reserve()->add(a, b) != a + b) {
                num_errors++;
            }
        }
    };

    std::thread t1([&]() { thread_proc(1000, 1234); });
    std::thread t2([&]() { thread_proc(2000, 5432); });
    std::thread t3([&]() { thread_proc(2222, 1111); });

    t1.join();
    t2.join();
    t3.join();

    BOOST_CHECK_EQUAL(num_errors, 0);
}

BOOST_AUTO_TEST_CASE(parallel_2)
{
    BOOST_TEST_MESSAGE("+ Test parallel requests of using objects from pool (object count limit reached)");
    std::atomic<int> num_errors(0);
    std::atomic<int> num_tests(0);
    Fmi::WorkerPool<SlowAdd> pool(2, 2);

    const auto thread_proc = [&](int a0, int b0)
    {
        for (int i = 0; i < 10; i++) {
            int a = a0 + 23*i;
            int b = b0 + 17*i;
            num_tests++;
            if (pool.reserve()->add(a, b) != a + b) {
                num_errors++;
            }
        }
    };

    std::thread t1([&]() { thread_proc(1000, 1234); });
    std::thread t2([&]() { thread_proc(2000, 5432); });
    std::thread t3([&]() { thread_proc(2222, 1111); });
    std::thread t4([&]() { thread_proc(2111, 2333); });

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    BOOST_CHECK_EQUAL(num_errors, 0);
    BOOST_CHECK_EQUAL(int(pool.get_max_reached_pool_size()), 2);
}

BOOST_AUTO_TEST_CASE(test_canceling_operation)
{
    BOOST_TEST_MESSAGE("+ Test canceling operation");
    Fmi::WorkerPool<Test3> pool(10, 10);
    std::thread t1([&pool]() { pool.reserve()->test(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    pool.cancel();
    t1.join();
    pool.shutdown();
    BOOST_REQUIRE_EQUAL(Test3::num_failed, 0);
}

#if 0
// Tests that failure to release object causes SIGABRT [test disabled]
BOOST_AUTO_TEST_CASE(failure_to_release_object)
{
    static std::shared_ptr<SlowAdd> dummy;
    Fmi::WorkerPool<SlowAdd> pool(2, 2);
    dummy = pool.reserve();
}
#endif

#if 0
BOOST_AUTO_TEST_CASE(nocompile_1)
{
    Fmi::WorkerPool<SlowAdd> pool(10, 10);
    pool.cancel();
}
#endif
