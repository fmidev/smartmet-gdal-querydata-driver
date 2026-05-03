#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <chrono>
#include <random>
#include <thread>
#include <vector>

#include "LRUCache.h"

using namespace Fmi;

TEST_CASE("LRUCache basic operations", "[single-threaded]")
{
  LRUCache<int, 4> cache(100);

  SECTION("Put and get")
  {
    cache.put(1, std::make_shared<int>(10));
    cache.put(2, std::make_shared<int>(20));

    auto val1 = cache.get(1);
    REQUIRE(val1.has_value());
    REQUIRE(*val1.value() == 10);

    auto val2 = cache.get(2);
    REQUIRE(val2.has_value());
    REQUIRE(*val2.value() == 20);

    auto val3 = cache.get(3);
    REQUIRE_FALSE(val3.has_value());
  }

  SECTION("Eviction")
  {
    for (int i = 0; i < 150; ++i)
    {
      cache.put(i, std::make_shared<int>(i * 10));
    }

    auto stats = cache.getStats();
    REQUIRE(stats.evictions > 0);
    REQUIRE(cache.get(0) == std::nullopt);
    REQUIRE(cache.get(149).has_value());
  }

  SECTION("Update existing key")
  {
    cache.put(1, std::make_shared<int>(10));
    cache.put(1, std::make_shared<int>(20));

    auto val = cache.get(1);
    REQUIRE(val.has_value());
    REQUIRE(*val.value() == 20);

    auto stats = cache.getStats();
    REQUIRE(stats.inserts == 2);
  }
}

TEST_CASE("LRUCache concurrent operations", "[multi-threaded]")
{
  const int NUM_THREADS = 4;
  const int OPERATIONS_PER_THREAD = 10000;
  LRUCache<int, 16> cache(1000);

  SECTION("Concurrent inserts and reads")
  {
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i)
    {
      threads.emplace_back(
          [&cache]()
          {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 999);

            for (int j = 0; j < OPERATIONS_PER_THREAD; ++j)
            {
              int key = dis(gen);
              if (j % 2 == 0)
                cache.put(key, std::make_shared<int>(key));
              else
                cache.get(key);
            }
          });
    }

    for (auto& thread : threads)
    {
      thread.join();
    }

    auto stats = cache.getStats();
    REQUIRE(stats.inserts + stats.hits + stats.misses == NUM_THREADS * OPERATIONS_PER_THREAD);
  }

  SECTION("High contention scenario")
  {
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i)
    {
      threads.emplace_back(
          [&cache]()
          {
            for (int j = 0; j < OPERATIONS_PER_THREAD; ++j)
            {
              int key = j % 100;  // High contention on a small set of keys
              if (j % 2 == 0)
                cache.put(key, std::make_shared<int>(j));
              else
                cache.get(key);
            }
          });
    }

    for (auto& thread : threads)
    {
      thread.join();
    }

    auto stats = cache.getStats();
    REQUIRE(stats.inserts + stats.hits + stats.misses == NUM_THREADS * OPERATIONS_PER_THREAD);
  }
}
