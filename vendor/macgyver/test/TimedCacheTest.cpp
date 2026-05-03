#include "TimedCache.h"

#include <boost/lexical_cast.hpp>
#include <regression/tframe.h>
#include <ctime>
#include <list>
#include <string>
#include <thread>

namespace CacheTest
{
void constructorDefault()
{
  Fmi::TimedCache::Cache<std::string, std::string> cache;
  if (cache.maxSize() != 10)
    TEST_FAILED("Default max size is not 10");

  if (cache.size() != 0)
    TEST_FAILED("Default size is not 0");

  TEST_PASSED();
}

void insertValues()
{
  std::list<std::string> valueList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  Fmi::TimedCache::Cache<std::string, std::string> cache;

  cache.insert("0", "0");
  if (cache.size() != 1)
    TEST_FAILED("After inserting one object to cache, size is not 1");

  for (auto item : valueList)
    cache.insert(item, item);
  if (cache.size() != valueList.size())
    TEST_FAILED("After inserting 10 objects to cache, size is not 10");

  for (auto item : valueList)
    cache.insert(item, item);
  if (cache.size() != valueList.size())
    TEST_FAILED("After inserting 10 objects and the dublicates of them to cache, size is not 10");

  TEST_PASSED();
}

void lruEviction()
{
  std::list<std::string> valueList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  Fmi::TimedCache::Cache<std::string, std::string> cache;

  for (auto item : valueList)
    cache.insert(item, item);

  // The call of find method arrange the objects in the cache so that "0" is the LRU object
  // to be evicted while inserting a new object.
  for (auto item : valueList)
    cache.find(item);
  cache.insert("10", "10");
  if (cache.find("0"))
    TEST_FAILED(
        "Object '0' should not be found from full cache after inserting a new object '10'.");

  TEST_PASSED();
}

void findAndInsert(Fmi::TimedCache::Cache<std::string, std::string>& cache,
                   const size_t& maxInsertions,
                   const size_t& uniqueRandomValues)
{
  for (size_t i = 0; i < maxInsertions; ++i)
  {
    auto nbr = std::lround(drand48() * uniqueRandomValues);
    std::this_thread::sleep_for(std::chrono::microseconds(nbr));
    const std::string key = boost::lexical_cast<std::string>(nbr);
    const std::string& value = key;

    if (not cache.find(key))
      cache.insert(key, value);
  }
}

void lruEvictionWithMultipleThreads()
{
  srand48(std::time(nullptr));
  const size_t maxInsertions = 10000;
  const size_t uniqueRandValues = 10;

  Fmi::TimedCache::Cache<std::string, std::string> cache;
  std::thread t1(findAndInsert, std::ref(cache), 1 * maxInsertions, 1 * uniqueRandValues);
  std::thread t2(findAndInsert, std::ref(cache), 2 * maxInsertions, 2 * uniqueRandValues);
  std::thread t3(findAndInsert, std::ref(cache), 3 * maxInsertions, 3 * uniqueRandValues);
  std::thread t4(findAndInsert, std::ref(cache), 4 * maxInsertions, 4 * uniqueRandValues);
  std::thread t5(findAndInsert, std::ref(cache), 5 * maxInsertions, 5 * uniqueRandValues);
  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();

  if (cache.size() != cache.maxSize())
    TEST_FAILED(
        "Cache is not full after multiple find and insert operations with multiple threads.");

  TEST_PASSED();
}

void constructorWithMaxSize()
{
  Fmi::TimedCache::Cache<std::string, std::string> cache(20);
  if (cache.maxSize() != 20)
    TEST_FAILED("Constructor with max size 20 does not result a cache of size 20.");

  if (cache.size() != 0)
    TEST_FAILED("Constructor with max size 20 does not result an empty cache.");

  TEST_PASSED();
}

void constructorWithMaxSizeAndEvictionTime()
{
  Fmi::TimedCache::Cache<std::string, std::string> cache(20, std::chrono::seconds(1));
  if (cache.maxSize() != 20)
    TEST_FAILED(
        "Constructor with max size 20 and eviction time 1 does not result a cache of size 20.");

  if (cache.size() != 0)
    TEST_FAILED("Constructor with max size 20 and eviction time 1 does not result an empty cache.");

  TEST_PASSED();
}

void timeEviction()
{
  std::list<std::string> valueList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  Fmi::TimedCache::Cache<std::string, std::string> cache(10, std::chrono::seconds(1));

  cache.insert(valueList.front(), valueList.front());
  std::this_thread::sleep_for(std::chrono::microseconds(2100000));
  if (cache.find(valueList.front()))
    TEST_FAILED("Time eviction failed: one object stored");

  for (auto item : valueList)
    cache.insert(item, item);
  std::this_thread::sleep_for(std::chrono::microseconds(2100000));
  for (auto item : valueList)
  {
    if (cache.find(item))
      TEST_FAILED("Time eviction failed: all objects should have been too old to be returned.");
  }

  for (auto item : valueList)
    cache.insert(item, item);
  std::this_thread::sleep_for(std::chrono::microseconds(2100000));
  cache.insert("10", "10");
  if (cache.size() != 1)
    TEST_FAILED(
        "Time eviction failed: all other object should have been cleaned from the cache while "
        "inserting a new object.");

  TEST_PASSED();
}

void customTimeEviction()
{
  std::list<std::string> valueList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  Fmi::TimedCache::Cache<std::string, std::string> cache(10, std::chrono::seconds(1));

  cache.insert(valueList.front(), valueList.front());
  cache.insert(valueList.back(), valueList.back(), std::chrono::seconds(3));
  std::this_thread::sleep_for(std::chrono::microseconds(2100000));
  if (cache.find(valueList.front()))
    TEST_FAILED(
        "Time eviction failed: global eviction time does not work with custom time eviction.");

  if (not cache.find(valueList.back()))
    TEST_FAILED(
        "Time eviction failed: custom eviction time does not override the global time value.");

  TEST_PASSED();
}

void cacheStatisticsInsertSuccess()
{
  std::list<std::string> valueList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  Fmi::TimedCache::Cache<std::string, std::string> cache(10);

  for (auto item : valueList)
    cache.insert(item, item);

  auto statistics = cache.getCacheStatistics();

  auto now = std::chrono::high_resolution_clock::now();
  auto timeLimitHigh = now + std::chrono::seconds(1);
  auto timeLimitLow = now - std::chrono::seconds(1);
  if (statistics.getConstructionTime() < timeLimitLow or
      timeLimitHigh < statistics.getConstructionTime())
    TEST_FAILED("CacheStatistics does not create construction time correctly.");

  if (statistics.getHits() != 0)
    TEST_FAILED("CacheStatistics does not count hits correctly.");

  if (statistics.getMisses() != 0)
    TEST_FAILED("CacheStatistics does not count misses correctly.");

  if (statistics.getEvictions() != 0)
    TEST_FAILED("CacheStatistics does not count evictions correctly.");

  if (statistics.getInsertFailures() != 0)
    TEST_FAILED("CacheStatistics does not count missed insertions correctly.");

  if (statistics.getInsertSuccesses() != valueList.size())
    TEST_FAILED("CacheStatistics does not count succeed insertions correctly.");

  TEST_PASSED();
}

void cacheStatisticsMisses()
{
  std::list<std::string> valueList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  Fmi::TimedCache::Cache<std::string, std::string> cache(10);

  for (auto item : valueList)
  {
    cache.find(item);
  }

  auto statistics = cache.getCacheStatistics();
  if (statistics.getHits() != 0)
    TEST_FAILED("CacheStatistics does not count hits correctly.");

  if (statistics.getMisses() != valueList.size())
    TEST_FAILED("CacheStatistics does not count misses correctly.");

  if (statistics.getEvictions() != 0)
    TEST_FAILED("CacheStatistics does not count evictions correctly.");

  if (statistics.getInsertFailures() != 0)
    TEST_FAILED("CacheStatistics does not count missed insertions correctly.");

  if (statistics.getInsertSuccesses() != 0)
    TEST_FAILED("CacheStatistics does not count succeed insertions correctly.");

  TEST_PASSED();
}

void cacheStatisticsHits()
{
  std::list<std::string> valueList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  Fmi::TimedCache::Cache<std::string, std::string> cache(10);

  for (auto item : valueList)
  {
    cache.insert(item, item);
    cache.find(item);
  }

  auto statistics = cache.getCacheStatistics();
  if (statistics.getHits() != valueList.size())
    TEST_FAILED("CacheStatistics does not count hits correctly.");

  if (statistics.getMisses() != 0)
    TEST_FAILED("CacheStatistics does not count misses correctly.");

  if (statistics.getEvictions() != 0)
    TEST_FAILED("CacheStatistics does not count evictions correctly.");

  if (statistics.getInsertFailures() != 0)
    TEST_FAILED("CacheStatistics does not count missed insertions correctly.");

  if (statistics.getInsertSuccesses() != valueList.size())
    TEST_FAILED("CacheStatistics does not count succeed insertions correctly.");

  TEST_PASSED();
}

void cacheStatisticsEvictions()
{
  std::list<std::string> valueList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  Fmi::TimedCache::Cache<std::string, std::string> cache(10);

  for (auto item : valueList)
  {
    cache.insert(item, item);
  }

  cache.insert("10", "10");

  auto statistics = cache.getCacheStatistics();
  if (statistics.getEvictions() != 1)
    TEST_FAILED("CacheStatistics does not count evictions correctly (part 1).");

  std::this_thread::sleep_for(std::chrono::microseconds(2100000));

  cache.insert(valueList.front(), valueList.front());

  statistics = cache.getCacheStatistics();
  if (statistics.getHits() != 0)
    TEST_FAILED("CacheStatistics does not count hits correctly.");

  if (statistics.getMisses() != 0)
    TEST_FAILED("CacheStatistics does not count misses correctly.");

  if (statistics.getEvictions() != 2)
    TEST_FAILED("CacheStatistics does not count evictions correctly (part 2).");

  if (statistics.getInsertFailures() != 0)
    TEST_FAILED("CacheStatistics does not count missed insertions correctly.");

  if (statistics.getInsertSuccesses() != valueList.size() + 2)
    TEST_FAILED("CacheStatistics does not count succeed insertions correctly.");

  TEST_PASSED();
}

class tests : public tframe::tests
{
  virtual const char* error_message_prefix() const { return "\n\t"; }
  void test(void)
  {
    TEST(constructorDefault);
    TEST(insertValues);
    TEST(lruEviction);
    TEST(lruEvictionWithMultipleThreads);

    TEST(constructorWithMaxSize);
    TEST(constructorWithMaxSizeAndEvictionTime);
    TEST(timeEviction);
    TEST(customTimeEviction);

    TEST(cacheStatisticsInsertSuccess);
    TEST(cacheStatisticsMisses);
    TEST(cacheStatisticsHits);
    TEST(cacheStatisticsEvictions);
  }
};

}  // namespace CacheTest

int main(void)
{
  using namespace std;
  std::cout << endl << "TimedCache::Cache" << endl << "=================" << endl;
  CacheTest::tests t;
  return t.run();
}
