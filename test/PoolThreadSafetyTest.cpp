#include "Pool.h"
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Pool Thread Safety tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

namespace
{
  struct TestItem
  {
    std::atomic<int> access_count{0};
    int value = 42;
    
    void use()
    {
      access_count++;
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  };
}

// Test 1: Stress test with rapid pool destruction while items are in use
BOOST_AUTO_TEST_CASE(stress_test_pool_destruction_with_active_items)
{
  BOOST_TEST_MESSAGE("Stress test: Pool destruction with active items");
  
  const int iterations = 100;
  std::atomic<int> errors{0};
  
  for (int i = 0; i < iterations; i++)
  {
    try
    {
      auto pool = std::make_unique<Fmi::Pool<Fmi::PoolInitType::Sequential, TestItem>>(3, 10);
      
      // Get multiple items
      std::vector<decltype(pool->get())> items;
      for (int j = 0; j < 5; j++)
      {
        items.emplace_back(pool->get());
      }
      
      // Use items in separate threads
      std::vector<std::thread> threads;
      for (auto& item : items)
      {
        threads.emplace_back([&item]() {
          if (item)
          {
            item->use();
          }
        });
      }
      
      // Destroy pool while threads are still running
      pool.reset();
      
      // Wait for threads
      for (auto& t : threads)
      {
        if (t.joinable())
          t.join();
      }
      
      // Items should still be valid after pool destruction
      for (auto& item : items)
      {
        if (item && item->access_count == 0)
        {
          errors++;
          break;
        }
      }
    }
    catch (const std::exception& e)
    {
      std::cerr << "Exception in iteration " << i << ": " << e.what() << std::endl;
      errors++;
    }
  }
  
  BOOST_CHECK_EQUAL(errors.load(), 0);
}

// Test 2: Concurrent get/release operations during pool destruction
BOOST_AUTO_TEST_CASE(concurrent_operations_during_destruction)
{
  BOOST_TEST_MESSAGE("Concurrent get/release during pool destruction");
  
  std::atomic<bool> stop_flag{false};
  std::atomic<int> errors{0};
  
  auto pool = std::make_shared<Fmi::Pool<Fmi::PoolInitType::Sequential, TestItem>>(5, 10);
  
  // Worker threads that continuously get and release items
  std::vector<std::thread> workers;
  for (int i = 0; i < 4; i++)
  {
    workers.emplace_back([&stop_flag, &errors, pool_copy = pool]() {
      while (!stop_flag.load(std::memory_order_acquire))
      {
        try
        {
          auto item = pool_copy->get(Fmi::TimeDuration(0, 0, 0, 100)); // 100ms timeout
          if (item)
          {
            item->use();
          }
        }
        catch (const Fmi::Exception& e)
        {
          // Timeout exceptions are expected
          if (std::string(e.what()).find("Timeout") == std::string::npos)
          {
            std::cerr << "Unexpected exception: " << e.what() << std::endl;
            errors++;
          }
        }
        catch (const std::exception& e)
        {
          std::cerr << "Unexpected std exception: " << e.what() << std::endl;
          errors++;
        }
      }
    });
  }
  
  // Let workers run for a bit
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  
  // Destroy pool while workers are active
  pool.reset();
  
  // Stop workers
  stop_flag.store(true, std::memory_order_release);
  
  for (auto& t : workers)
  {
    if (t.joinable())
      t.join();
  }
  
  BOOST_CHECK_EQUAL(errors.load(), 0);
}

// Test 3: Ensure items held after pool destruction are properly cleaned up
BOOST_AUTO_TEST_CASE(items_cleanup_after_pool_destruction)
{
  BOOST_TEST_MESSAGE("Items cleanup after pool destruction");
  
  std::vector<std::unique_ptr<Fmi::Pool<Fmi::PoolInitType::Sequential, TestItem>::Ptr>> saved_items;
  
  {
    Fmi::Pool<Fmi::PoolInitType::Sequential, TestItem> pool(3, 5);
    
    // Get items and save them
    for (int i = 0; i < 4; i++)
    {
      saved_items.emplace_back(
        std::make_unique<decltype(pool.get())>(pool.get())
      );
    }
    
    BOOST_CHECK_EQUAL(int(pool.in_use()), 4);
  } // Pool destroyed here
  
  // Items should still be valid
  for (auto& item_ptr : saved_items)
  {
    BOOST_REQUIRE(item_ptr != nullptr);
    BOOST_REQUIRE(item_ptr->get() != nullptr);
    BOOST_CHECK_EQUAL((*item_ptr)->value, 42);
  }
  
  // Explicitly reset items
  saved_items.clear();
  
  BOOST_CHECK(true); // If we got here without crashes, test passed
}

// Test 4: Race between release() and destructor
BOOST_AUTO_TEST_CASE(race_between_release_and_destructor)
{
  BOOST_TEST_MESSAGE("Race between release() and destructor");
  
  const int iterations = 200;
  std::atomic<int> errors{0};
  
  for (int i = 0; i < iterations; i++)
  {
    try
    {
      auto pool = std::make_shared<Fmi::Pool<Fmi::PoolInitType::Sequential, TestItem>>(2, 5);
      
      // Get items
      auto item1 = pool->get();
      auto item2 = pool->get();
      
      // Thread that will release items
      std::thread releaser([item1 = std::move(item1), item2 = std::move(item2)]() mutable {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        item1.reset();
        item2.reset();
      });
      
      // Destroy pool almost immediately
      std::this_thread::sleep_for(std::chrono::microseconds(5));
      pool.reset();
      
      releaser.join();
    }
    catch (const std::exception& e)
    {
      std::cerr << "Exception in iteration " << i << ": " << e.what() << std::endl;
      errors++;
    }
  }
  
  BOOST_CHECK_EQUAL(errors.load(), 0);
}

// Test 5: Multiple threads acquiring and releasing with pool at max size
BOOST_AUTO_TEST_CASE(high_contention_at_max_size)
{
  BOOST_TEST_MESSAGE("High contention at max pool size");
  
  const int pool_size = 5;
  const int num_threads = 20;
  const int iterations_per_thread = 50;
  
  Fmi::Pool<Fmi::PoolInitType::Sequential, TestItem> pool(pool_size, pool_size);
  
  std::atomic<int> successes{0};
  std::atomic<int> timeouts{0};
  std::atomic<int> errors{0};
  
  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; i++)
  {
    threads.emplace_back([&pool, &successes, &timeouts, &errors, iterations_per_thread]() {
      for (int j = 0; j < iterations_per_thread; j++)
      {
        try
        {
          auto item = pool.get(Fmi::TimeDuration(0, 0, 0, 50)); // 50ms timeout
          item->use();
          successes++;
        }
        catch (const Fmi::Exception& e)
        {
          if (std::string(e.what()).find("Timeout") != std::string::npos)
          {
            timeouts++;
          }
          else
          {
            std::cerr << "Unexpected exception: " << e.what() << std::endl;
            errors++;
          }
        }
        catch (const std::exception& e)
        {
          std::cerr << "Unexpected std exception: " << e.what() << std::endl;
          errors++;
        }
      }
    });
  }
  
  for (auto& t : threads)
  {
    t.join();
  }
  
  BOOST_TEST_MESSAGE("Successes: " << successes.load() << ", Timeouts: " << timeouts.load());
  BOOST_CHECK_EQUAL(errors.load(), 0);
  BOOST_CHECK_EQUAL(successes.load() + timeouts.load(), num_threads * iterations_per_thread);
}

// Test 6: Verify no data races in parallel initialization
BOOST_AUTO_TEST_CASE(parallel_initialization_thread_safety)
{
  BOOST_TEST_MESSAGE("Parallel initialization thread safety");
  
  const int iterations = 50;
  std::atomic<int> errors{0};
  
  for (int i = 0; i < iterations; i++)
  {
    try
    {
      Fmi::Pool<Fmi::PoolInitType::Parallel, TestItem> pool(10, 20);
      BOOST_CHECK_EQUAL(int(pool.size()), 10);
      BOOST_CHECK_EQUAL(int(pool.in_use()), 0);
    }
    catch (const std::exception& e)
    {
      std::cerr << "Exception in iteration " << i << ": " << e.what() << std::endl;
      errors++;
    }
  }
  
  BOOST_CHECK_EQUAL(errors.load(), 0);
}

// Test 7: Stress test rapid item acquisition and release
BOOST_AUTO_TEST_CASE(rapid_acquire_release_stress)
{
  BOOST_TEST_MESSAGE("Rapid acquire/release stress test");
  
  Fmi::Pool<Fmi::PoolInitType::Sequential, TestItem> pool(10, 20);
  
  const int num_threads = 10;
  const int iterations = 500;
  std::atomic<int> errors{0};
  
  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; i++)
  {
    threads.emplace_back([&pool, &errors, iterations]() {
      for (int j = 0; j < iterations; j++)
      {
        try
        {
          auto item = pool.get();
          // Immediately release by going out of scope
        }
        catch (const std::exception& e)
        {
          std::cerr << "Exception: " << e.what() << std::endl;
          errors++;
          break;
        }
      }
    });
  }
  
  for (auto& t : threads)
  {
    t.join();
  }
  
  BOOST_CHECK_EQUAL(errors.load(), 0);
  BOOST_CHECK_EQUAL(int(pool.in_use()), 0);
}
