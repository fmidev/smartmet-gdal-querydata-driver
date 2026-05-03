#include "Cache.h"

#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <regression/tframe.h>
#include <ctime>
#include <iostream>
#include <list>
#include <string>

// FIXME: unfortunatelly no similar function to boost::filesystem::unique_pth is
//        present in std::filesystem. As result we have to use boost::filesystem in tests

using namespace std;
using namespace Fmi::Cache;

namespace fs = std::filesystem;

static std::filesystem::path* testpaths[3] = {nullptr};

namespace CacheTest
{
namespace
{
template <class T, class U, class V>
void insert_and_expire(T& theMap, U&& theKey, V&& theValue)
{
  theMap.insert(theKey, theValue, theKey);
  theMap.expire(theKey);
}
}  // namespace

struct custom_type
{
  unsigned int number;
  string text;
};

struct custom_comparator
{
  static std::size_t getSize(const custom_type& object) { return object.number; }
};

void testfilecache()
{
  fs::path testdir(*testpaths[0]);

  FileCache cache(testdir, 100);

  std::vector<std::pair<std::size_t, std::string> > input = {
      {0, "zero"},
      {1, "first"},
      {2, "second"},
      {500, "five hundred"},
      {std::numeric_limits<std::size_t>::max(), "max"}};

  for (auto& data : input)
  {
    cache.insert(data.first, data.second);
  }

  for (auto& data : input)
  {
    auto result = cache.find(data.first);
    if (!result)
    {
      TEST_FAILED("Did not find key '" + std::to_string((unsigned long long)data.first) +
                  "' from the file cache");
    }
    if (*result != data.second)
    {
      TEST_FAILED("Content mismatch with key '" + std::to_string((unsigned long long)data.first) +
                  "'. Should be '" + data.second + "', got instead '" + *result + "'");
    }
  }

  TEST_PASSED();
}

void testfilecacheorder()
{
  fs::path testdir(*testpaths[1]);

  FileCache cache(testdir, 100);

  std::vector<std::pair<std::size_t, std::string> > input = {
      {100, "zero"},
      {200, "first"},
      {999, "second"},
      {1202, "five hundred"},
  };

  std::vector<std::size_t> correct_order = {

      200,
      100,
      1202,
      999,

  };

  for (auto& data : input)
  {
    cache.insert(data.first, data.second);
  }

  auto unused = cache.find(100);
  unused = cache.find(1202);
  unused = cache.find(999);

  auto content = cache.getContent();

  for (int i = 0; i < 4; ++i)
  {
    if (content[i] != correct_order[i])
    {
      std::ostringstream os;
      os << "Cache content at index '" << i << "' is '" << content[i] << "', should be '"
         << correct_order[i] << "'" << std::endl;
      TEST_FAILED(os.str());
    }
  }

  TEST_PASSED();
}

void testfilecachesize()
{
  fs::path testdir(*testpaths[2]);

  FileCache cache(testdir, 8);

  std::vector<std::pair<std::size_t, std::string> > input = {{1, "1"}, {2, "12"}, {3, "123"}};

  std::vector<std::size_t> correct_order = {

      3, 1, 4

  };

  for (auto& data : input)
  {
    cache.insert(data.first, data.second);
  }

  auto unused = cache.find(1);

  cache.insert(4, "1234");

  auto content = cache.getContent();

  for (int i = 0; i < 3; ++i)
  {
    if (content.at(i) != correct_order.at(i))
    {
      std::ostringstream os;
      os << "Cache content at index '" << i << "' is '" << content[i] << "', should be '"
         << correct_order[i] << "'" << std::endl;
      TEST_FAILED(os.str());
    }
  }

  if (content.size() != 3)
    TEST_FAILED("Incorrect number of cache items, should be 3");

  std::size_t size = cache.getSize();
  if (size != 8)
    TEST_FAILED("Incorrect cache size, should be 8");

  TEST_PASSED();
}

void testcustomtype()
{
  Cache<int, custom_type, LRUEviction, int, StaticExpire, custom_comparator> thisCache(115);

  custom_type toInsert, toInsert2, toInsert3;
  toInsert.number = 10;
  toInsert.text = "Hello";
  toInsert2.number = 100;
  toInsert2.text = "HelloWorld";
  toInsert3.number = 6;
  toInsert3.text = "HelloAll";

  thisCache.insert(1, toInsert);
  thisCache.insert(2, toInsert2);

  auto test = thisCache.find(1);

  if (!test)
    TEST_FAILED("Did not find the inserted object.");

  test = thisCache.find(2);
  if (!test)
    TEST_FAILED("Did not find the inserted object.");

  thisCache.insert(3, toInsert3);

  test = thisCache.find(1);
  if (test)
    TEST_FAILED("Shouldn't have found the inserted object.");

  TEST_PASSED();
}

void testinstantexpire()
{
  Cache<int, string, LRUEviction, int, InstantExpire> thisCache(10, 1);

  std::list<int> onesTags = {1, 2};

  std::vector<int> twosTags = {2, 3};

  thisCache.insert(1, "moi", onesTags);
  thisCache.insert(2, "hei", onesTags);
  thisCache.insert(3, "heippa", twosTags);
  thisCache.insert(4, "terve", twosTags);
  thisCache.insert(5, "heihei", twosTags);

  thisCache.expire(1);
  boost::this_thread::sleep_for(boost::chrono::seconds(2));

  auto value1 = thisCache.find(1);
  auto value2 = thisCache.find(2);
  auto value3 = thisCache.find(3);

  if (value1)
    TEST_FAILED("Entry " + *thisCache.find(1) + " found in cache");

  if (value2)
    TEST_FAILED("Entry " + *thisCache.find(2) + " found in cache");

  if (!value3)
    TEST_FAILED("Entry 3 was not found in cache");

  TEST_PASSED();
}

void testtagless()
{
  Cache<int, string, FIFOEviction> thisCache(4);

  thisCache.insert(1, "moi");
  thisCache.insert(2, "hei");
  thisCache.insert(3, "heippa");
  thisCache.insert(4, "terve");
  thisCache.insert(5, "heihei");

  auto value1 = thisCache.find(1);

  if (value1)
    TEST_FAILED("Entry " + *thisCache.find(1) + " found in cache");

  TEST_PASSED();
}

void teststaticexpire()
{
  Cache<int, string, LRUEviction, int, StaticExpire> thisCache(10);

  int oneTag = 1;

  std::vector<int> twosTags = {2, 3};

  thisCache.insert(1, "moi", oneTag);
  thisCache.insert(2, "hei", oneTag);
  thisCache.insert(3, "heippa", twosTags);
  thisCache.insert(4, "terve", twosTags);
  thisCache.insert(5, "heihei", twosTags);

  thisCache.expire(2);

  auto value1 = thisCache.find(1);
  auto value2 = thisCache.find(2);
  auto value3 = thisCache.find(3);

  if (!value1)
    TEST_FAILED("Entry 1 not found in cache");

  if (!value2)
    TEST_FAILED("Entry 2 not found in cache");

  if (value3)
    TEST_FAILED("Entry 3 was found in cache");

  TEST_PASSED();
}

void testlru()
{
  Cache<int, string> thisCache(5);

  int oneTag = 1;

  std::vector<int> twosTags = {2};

  thisCache.insert(1, "eka", oneTag);
  thisCache.insert(2, "toka", oneTag);
  thisCache.insert(3, "kolmas", oneTag);
  thisCache.insert(4, "neljas", twosTags);
  thisCache.insert(5, "viides", twosTags);

  auto value = thisCache.find(4);
  value = thisCache.find(3);
  value = thisCache.find(2);
  value = thisCache.find(1);

  // This must remove the "5" entry from the cache since it is the last used
  thisCache.insert(6, "kuudes", twosTags);

  std::string expected = "neljas,kolmas,toka,eka,kuudes";

  if (expected == thisCache.getTextContent())
    TEST_PASSED();

  TEST_FAILED("Wrong cache content:\"" + thisCache.getTextContent() + "\", expected \"" + expected +
              "\"");
}

void testmru()
{
  Cache<int, string, MRUEviction, int> thisCache(5, 10000);

  int oneTag = 1;

  std::vector<int> twosTags = {2};

  thisCache.insert(1, "eka", oneTag);
  thisCache.insert(2, "toka", oneTag);
  thisCache.insert(3, "kolmas", oneTag);
  thisCache.insert(4, "neljas", twosTags);
  thisCache.insert(5, "viides", twosTags);

  //"4" is most recently used
  auto value = thisCache.find(4);

  // This must remove the "4" entry from the cache since it is the last used
  thisCache.insert(6, "kuudes", twosTags);

  auto return_value = thisCache.find(4);
  if (!return_value)
    TEST_PASSED();

  TEST_FAILED("Entry \"4\" is still in the cache");
}

void testsize()
{
  Cache<int, string> thisCache(3, 10000);

  int oneTag = 1;

  std::vector<int> twosTags = {2};

  thisCache.insert(1, "moi", oneTag);
  thisCache.insert(2, "hei", oneTag);
  thisCache.insert(3, "heippa", oneTag);
  thisCache.insert(4, "terve", twosTags);
  thisCache.insert(5, "heihei", twosTags);

  if (thisCache.size() != 3)
    TEST_FAILED("Wrong cache size: " + boost::lexical_cast<string>(thisCache.size()) +
                ", should be 3");

  TEST_PASSED();
}

void testregularexpiringcache()
{
  Cache<int, string, LRUEviction, int, InstantExpire> thisCache(10, 2);  // Expire in two second

  insert_and_expire(thisCache, 1, "moi");
  insert_and_expire(thisCache, 2, "hei");
  insert_and_expire(thisCache, 3, "heippa");
  insert_and_expire(thisCache, 4, "terve");
  insert_and_expire(thisCache, 5, "moikka");

  boost::this_thread::sleep_for(boost::chrono::seconds(3));

  insert_and_expire(thisCache, 6, "terve");
  insert_and_expire(thisCache, 7, "moikka");

  auto value = thisCache.find(1);
  value = thisCache.find(2);
  value = thisCache.find(3);
  value = thisCache.find(4);
  value = thisCache.find(5);

  std::string expected = "terve,moikka";

  if (expected == thisCache.getTextContent())
    TEST_PASSED();

  TEST_FAILED("Wrong cache content:\"" + thisCache.getTextContent() + "\", expected \"" + expected +
              "\"");
}

void testtwocaches()
{
  Cache<int, string, LRUEviction, int> thisCache(3, 10000);

  Cache<int, string, RandomEviction, int> anotherCache(5, 10000);

  int oneTag = 1;

  std::vector<int> twosTags = {2};

  thisCache.insert(1, "moi", oneTag);
  thisCache.insert(2, "hei", oneTag);
  thisCache.insert(3, "heippa", oneTag);
  thisCache.insert(4, "terve", twosTags);
  thisCache.insert(5, "heihei", twosTags);

  anotherCache.insert(1, "moi", oneTag);
  anotherCache.insert(2, "hei", oneTag);
  anotherCache.insert(3, "heippa", oneTag);
  anotherCache.insert(4, "terve", twosTags);
  anotherCache.insert(5, "heihei", twosTags);
  anotherCache.insert(6, "heihei", twosTags);

  if (thisCache.size() == 3 && anotherCache.size() == 5)
  {
    TEST_PASSED();
  }
  else
  {
    TEST_FAILED("Wrong cache sizes: " + boost::lexical_cast<string>(thisCache.size()) + ", " +
                boost::lexical_cast<string>(anotherCache.size()) + " should be 3 and 5");
  }
}

void testfifo()
{
  Cache<int, string, FIFOEviction, int> thisCache(5, 10000);

  std::list<int> onesTags = {1, 2};

  std::list<int> twosTags = {2, 3};

  thisCache.insert(1, "moi", onesTags);
  thisCache.insert(2, "hei", onesTags);
  thisCache.insert(3, "last", onesTags);
  thisCache.insert(4, "terve", twosTags);
  thisCache.insert(5, "heihei", twosTags);
  thisCache.insert(6, "heihei", twosTags);
  thisCache.insert(7, "heihei", twosTags);

  auto first_value = thisCache.find(1);
  auto second_value = thisCache.find(2);

  if (first_value)
    TEST_FAILED(*thisCache.find(1) + " should not have been found.");

  if (second_value)
    TEST_FAILED(*thisCache.find(2) + " should not have been found.");

  TEST_PASSED();
}

void testevictionvector()
{
  Cache<int, string, FIFOEviction> thisCache(3);

  std::vector<std::pair<int, string> > evictedItems;

  thisCache.insert(1, "moi", evictedItems);
  if (!evictedItems.empty())
  {
    TEST_FAILED("Case '1' : 'moi': Evicted items is not empty.");
  }
  thisCache.insert(2, "hei", evictedItems);
  if (!evictedItems.empty())
  {
    TEST_FAILED("Case '2' : 'hei': Evicted items is not empty.");
  }
  thisCache.insert(3, "data", evictedItems);
  if (!evictedItems.empty())
  {
    TEST_FAILED("Case '3' : 'data': Evicted items is not empty.");
  }

  thisCache.insert(4, "terve", evictedItems);
  if (evictedItems.size() != 1)
  {
    TEST_FAILED("Case '4' : 'terve': Evicted item vector not of size 1.");
  }
  if (evictedItems[0].first != 1 || evictedItems[0].second != "moi")
  {
    std::ostringstream os;
    os << "Incorrect item in eviction vector: '" << evictedItems[0].first << " : "
       << evictedItems[0].second << "'";
    TEST_FAILED(os.str());
  }

  TEST_PASSED();
}

void testcounters()
{
  Cache<int, string> thisCache(5);

  auto stats = thisCache.statistics();
  if (stats.size != 0)
    TEST_FAILED("Empty cache size should be zero");
  if (stats.inserts != 0)
    TEST_FAILED("Empty cache insert count should be zero");
  if (stats.hits != 0)
    TEST_FAILED("Empty cache hits should be zero");
  if (stats.misses != 0)
    TEST_FAILED("Empty cache misses should be zero");

  auto value = thisCache.find(1);
  stats = thisCache.statistics();
  if (stats.hits != 0)
    TEST_FAILED("Empty cache hits should be zero after one find call");
  if (stats.misses != 1)
    TEST_FAILED("Empty cache misses should be one after one find call");

  thisCache.insert(1, "one");
  thisCache.insert(2, "two");
  thisCache.insert(3, "three");
  thisCache.insert(4, "four");
  thisCache.insert(5, "five");

  stats = thisCache.statistics();
  if (stats.inserts != 5)
    TEST_FAILED("Cache insert could should be five after five inserts");

  value = thisCache.find(3);
  stats = thisCache.statistics();

  if (stats.hits != 1)
    TEST_FAILED("Cache hits should be one after one succesful find call");
  if (stats.misses != 1)
    TEST_FAILED("Cache misses should be one after one failed find call");

  value = thisCache.find(4);
  value = thisCache.find(-1);
  stats = thisCache.statistics();

  if (stats.hits != 2)
    TEST_FAILED("Cache hits should be two after two succesful find calls");
  if (stats.misses != 2)
    TEST_FAILED("Cache misses should be two after two failed find calls");

  TEST_PASSED();
}

class tests : public tframe::tests
{
  virtual const char* error_message_prefix() const { return "\n\t"; }
  void test(void)
  {
    TEST(testfilecache);
    TEST(testfilecacheorder);
    TEST(testfilecachesize);
    TEST(testcustomtype);
    TEST(testsize);
    TEST(testinstantexpire);
    TEST(teststaticexpire);
    TEST(testmru);
    TEST(testlru);
    TEST(testtwocaches);
    TEST(testfifo);
    TEST(testtagless);
    TEST(testregularexpiringcache);
    TEST(testevictionvector);
    TEST(testcounters);
  }
};
}  // namespace CacheTest

static void atexit_handler()
{
  for (unsigned int i = 0; i < sizeof(testpaths) / sizeof(*testpaths); i++)
  {
    if (testpaths[i] != nullptr)
    {
      std::filesystem::remove_all(*testpaths[i]);
      delete testpaths[i];
    }
  }
}

int main(void)
{
  using namespace std;
  cout << endl << "Cache" << endl << "=========" << endl;
  atexit(atexit_handler);  // Remove this if you need to debug test directory contents after test
  for (unsigned int i = 0; i < sizeof(testpaths) / sizeof(*testpaths); i++)
  {
    testpaths[i] = new std::filesystem::path(
        boost::filesystem::unique_path(std::filesystem::temp_directory_path().string() + "/" +
                                       "MacGyver_CacheTest_" + to_string(i) + "_%%%%%%%%").string());
    // cout << "Testpath " << i << " is " << *testpaths[i] << endl;
  }
  CacheTest::tests t;
  return t.run();
}
