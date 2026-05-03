// ======================================================================
/*!
 * \brief Tagged multi-strategy caching in multithreaded environment
 */
// ======================================================================
#pragma once

#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/functional/hash.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <utility>

#include "CacheStats.h"
#include "DateTime.h"

namespace Fmi
{
namespace Cache
{
using MutexType = boost::mutex;
using Lock = boost::lock_guard<MutexType>;

const CacheStats EMPTY_CACHE_STATS;

// ----------------------------------------------------------------------
/*!
 * \brief Object which is stored in the cache
 */
// ----------------------------------------------------------------------

template <class KeyType, class ValueType, class TagSetType>
struct CacheObject
{
  CacheObject(const KeyType& theKey,
              const ValueType& theValue,
              const TagSetType& theSet,
              std::size_t theSize)
      : itsKey(theKey), itsValue(theValue), itsTagSet(theSet), itsSize(theSize)
  {
  }

  KeyType itsKey;

  ValueType itsValue;

  TagSetType itsTagSet;

  std::size_t itsHits = 0;

  std::size_t itsSize = 0;
};

template <class KeyType, class ValueType, class TagSetType>
struct CacheReportingObject
{
  CacheReportingObject(const KeyType& theKey,
                       const ValueType& theValue,
                       const TagSetType& theSet,
                       std::size_t theHits,
                       std::size_t theSize)
      : itsKey(theKey), itsValue(theValue), itsTagSet(theSet), itsHits(theHits), itsSize(theSize)
  {
  }

  KeyType itsKey;

  ValueType itsValue;

  TagSetType itsTagSet;

  std::size_t itsHits = 0;

  std::size_t itsSize = 0;
};

// ----------------------------------------------------------------------
/*!
 * \brief Eviction types which determine how cache is evicted. Eviction
 * can be either object or size based.
 */
// ----------------------------------------------------------------------

template <class ValueType>
struct TrivialSizeFunction
{
  static std::size_t getSize(const ValueType& /* theValue */) { return 1; }
};

template <class TagSetType, class TagMapType>
void perform_tag_eviction(const TagSetType& tags, TagMapType& inputTagMap)
{
  for (auto tagit = tags.begin(); tagit != tags.end(); ++tagit)
  {
    auto mapit = inputTagMap.find(*tagit);

    if (mapit != inputTagMap.end())
    {
      // Decrease ownership count
      mapit->second.second--;

      if (mapit->second.second == 0)
      {
        inputTagMap.erase(mapit);
      }
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Eviction policies determine how cache is cleaned upon reaching the maximum size.
 */
// ----------------------------------------------------------------------

// Remove the least used object from the cache
template <class MapType,
          class TagMapType,
          class KeyType,
          class ValueType,
          class TagSetType,
          class SizeFunction>
struct LRUEviction
{
  static void onEvict(MapType& inputMap,
                      TagMapType& inputTagMap,
                      std::size_t& currentSize,
                      std::size_t maxSize)
  {
    while (currentSize > maxSize)
    {
      std::size_t valueSize = inputMap.right.front().first.itsSize;
      TagSetType valueTags = inputMap.right.front().first.itsTagSet;

      perform_tag_eviction<TagSetType, TagMapType>(valueTags, inputTagMap);

      inputMap.right.pop_front();
      currentSize -= valueSize;
    }
  }

  static void onEvict(MapType& inputMap,
                      TagMapType& inputTagMap,
                      std::size_t& currentSize,
                      std::size_t maxSize,
                      std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    while (currentSize > maxSize)
    {
      auto& item = inputMap.right.front().first;
      std::size_t valueSize = item.itsSize;
      TagSetType valueTags = item.itsTagSet;

      perform_tag_eviction<TagSetType, TagMapType>(valueTags, inputTagMap);

      evictedItems.push_back(std::make_pair(item.itsKey, item.itsValue));

      inputMap.right.pop_front();
      currentSize -= valueSize;
    }
  }

  static bool onInsert(MapType& inputMap,
                       TagMapType& inputTagMap,
                       const KeyType& key,
                       const ValueType& value,
                       const TagSetType& tags,
                       std::size_t& currentSize,
                       std::size_t maxSize)
  {
    std::size_t amountToInsert = SizeFunction::getSize(value);

    if (amountToInsert >= maxSize)
    {
      // User attempted to insert something that is bigger than the entire cache, return fail
      return false;
    }

    currentSize += amountToInsert;

    // Evict until within the maxSize limit
    onEvict(inputMap, inputTagMap, currentSize, maxSize);

    inputMap.insert(typename MapType::value_type(
        key, CacheObject<KeyType, ValueType, TagSetType>(key, value, tags, amountToInsert)));

    return true;
  }

  static bool onInsert(MapType& inputMap,
                       TagMapType& inputTagMap,
                       const KeyType& key,
                       const ValueType& value,
                       const TagSetType& tags,
                       std::size_t& currentSize,
                       std::size_t maxSize,
                       std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    std::size_t amountToInsert = SizeFunction::getSize(value);

    if (amountToInsert >= maxSize)
    {
      // User attempted to insert something that is bigger than the entire cache, return fail
      return false;
    }

    currentSize += amountToInsert;

    // Evict until within the maxSize limit
    onEvict(inputMap, inputTagMap, currentSize, maxSize, evictedItems);

    inputMap.insert(typename MapType::value_type(
        key, CacheObject<KeyType, ValueType, TagSetType>(key, value, tags, amountToInsert)));

    return true;
  }

  static void onAccess(MapType& inputMap, typename MapType::left_iterator& it)
  {
    inputMap.right.relocate(inputMap.right.end(), inputMap.project_right(it));
  }
};

// Remove the most used object from the cache
template <class MapType,
          class TagMapType,
          class KeyType,
          class ValueType,
          class TagSetType,
          class SizeFunction>
struct MRUEviction
{
  static void onEvict(MapType& inputMap,
                      TagMapType& inputTagMap,
                      std::size_t& currentSize,
                      std::size_t maxSize)
  {
    while (currentSize > maxSize)
    {
      std::size_t valueSize = inputMap.right.back().first.itsSize;
      TagSetType valueTags = inputMap.right.back().first.itsTagSet;

      perform_tag_eviction<TagSetType, TagMapType>(valueTags, inputTagMap);

      inputMap.right.pop_back();
      currentSize -= valueSize;
    }
  }

  static void onEvict(MapType& inputMap,
                      TagMapType& inputTagMap,
                      std::size_t& currentSize,
                      std::size_t maxSize,
                      std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    while (currentSize > maxSize)
    {
      auto& item = inputMap.right.back().first;
      std::size_t valueSize = item.itsSize;
      TagSetType valueTags = item.itsTagSet;

      evictedItems.push_back(std::make_pair(item.itsKey, item.itsValue));

      perform_tag_eviction<TagSetType, TagMapType>(valueTags, inputTagMap);

      inputMap.right.pop_back();
      currentSize -= valueSize;
    }
  }

  static bool onInsert(MapType& inputMap,
                       TagMapType& inputTagMap,
                       const KeyType& key,
                       const ValueType& value,
                       const TagSetType& tags,
                       std::size_t& currentSize,
                       std::size_t maxSize)
  {
    std::size_t amountToInsert = SizeFunction::getSize(value);

    if (amountToInsert >= maxSize)
    {
      // User attempted to insert something that is bigger than the entire cache, return fail
      return false;
    }

    currentSize += amountToInsert;

    onEvict(inputMap, inputTagMap, currentSize, maxSize);

    inputMap.insert(typename MapType::value_type(
        key, CacheObject<KeyType, ValueType, TagSetType>(key, value, tags, amountToInsert)));

    return true;
  }

  static bool onInsert(MapType& inputMap,
                       TagMapType& inputTagMap,
                       const KeyType& key,
                       const ValueType& value,
                       const TagSetType& tags,
                       std::size_t& currentSize,
                       std::size_t maxSize,
                       std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    std::size_t amountToInsert = SizeFunction::getSize(value);

    if (amountToInsert >= maxSize)
    {
      // User attempted to insert something that is bigger than the entire cache, return fail
      return false;
    }

    currentSize += amountToInsert;

    onEvict(inputMap, inputTagMap, currentSize, maxSize, evictedItems);

    inputMap.insert(typename MapType::value_type(
        key, CacheObject<KeyType, ValueType, TagSetType>(key, value, tags, amountToInsert)));

    return true;
  }

  static void onAccess(MapType& inputMap, typename MapType::left_iterator& it)
  {
    inputMap.right.relocate(inputMap.right.end(), inputMap.project_right(it));
  }
};

// Remove randomly chosen object from the cache
template <class MapType,
          class TagMapType,
          class KeyType,
          class ValueType,
          class TagSetType,
          class SizeFunction>
struct RandomEviction
{
  static void onEvict(MapType& inputMap,
                      TagMapType& inputTagMap,
                      std::size_t& currentSize,
                      std::size_t maxSize)
  {
    static std::random_device dev;
    static std::mt19937 generator(dev());

    while (currentSize > maxSize)
    {
      std::size_t mapSize = inputMap.size();
      std::uniform_int_distribution<> dist(0, mapSize - 1);
      std::size_t randomInteger = dist(generator);
      auto iterator = inputMap.right.begin();
      std::advance(iterator, randomInteger);

      std::size_t valueSize = iterator->first.itsSize;
      TagSetType valueTags = iterator->first.itsTagSet;

      perform_tag_eviction<TagSetType, TagMapType>(valueTags, inputTagMap);

      inputMap.right.erase(iterator);
      currentSize -= valueSize;
    }
  }

  static void onEvict(MapType& inputMap,
                      TagMapType& inputTagMap,
                      std::size_t& currentSize,
                      std::size_t maxSize,
                      std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    static std::random_device dev;
    static std::mt19937 generator(dev());

    while (currentSize > maxSize)
    {
      std::size_t mapSize = inputMap.size();
      std::uniform_int_distribution<> dist(0, mapSize - 1);
      std::size_t randomInteger = dist(generator);
      auto iterator = inputMap.right.begin();
      std::advance(iterator, randomInteger);

      auto& item = iterator->first;
      std::size_t valueSize = item.itsSize;
      TagSetType valueTags = item.itsTagSet;

      perform_tag_eviction<TagSetType, TagMapType>(valueTags, inputTagMap);

      evictedItems.push_back(std::make_pair(item.itsKey, item.itsValue));

      inputMap.right.erase(iterator);
      currentSize -= valueSize;
    }
  }

  static bool onInsert(MapType& inputMap,
                       TagMapType& inputTagMap,
                       const KeyType& key,
                       const ValueType& value,
                       const TagSetType& tags,
                       std::size_t& currentSize,
                       std::size_t maxSize)
  {
    std::size_t amountToInsert = SizeFunction::getSize(value);

    if (amountToInsert >= maxSize)
    {
      // User attempted to insert something that is bigger than the entire cache, return fail
      return false;
    }

    currentSize += amountToInsert;

    // Evict until within the maxSize limit
    onEvict(inputMap, inputTagMap, currentSize, maxSize);

    inputMap.insert(typename MapType::value_type(
        key, CacheObject<KeyType, ValueType, TagSetType>(key, value, tags, amountToInsert)));

    return true;
  }

  static bool onInsert(MapType& inputMap,
                       TagMapType& inputTagMap,
                       const KeyType& key,
                       const ValueType& value,
                       const TagSetType& tags,
                       std::size_t& currentSize,
                       std::size_t maxSize,
                       std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    std::size_t amountToInsert = SizeFunction::getSize(value);

    if (amountToInsert >= maxSize)
    {
      // User attempted to insert something that is bigger than the entire cache, return fail
      return false;
    }

    currentSize += amountToInsert;

    // Evict until within the maxSize limit
    onEvict(inputMap, inputTagMap, currentSize, maxSize, evictedItems);

    inputMap.insert(typename MapType::value_type(
        key, CacheObject<KeyType, ValueType, TagSetType>(key, value, tags, amountToInsert)));

    return true;
  }

  static void onAccess(MapType& /* inputMap */, typename MapType::left_iterator& /* it */)
  {
    // No-op
  }
};

// First in first out removal policy
template <class MapType,
          class TagMapType,
          class KeyType,
          class ValueType,
          class TagSetType,
          class SizeFunction>
struct FIFOEviction
{
  static void onEvict(MapType& inputMap,
                      TagMapType& inputTagMap,
                      std::size_t& currentSize,
                      std::size_t maxSize)
  {
    while (currentSize > maxSize)
    {
      std::size_t valueSize = inputMap.right.front().first.itsSize;
      TagSetType valueTags = inputMap.right.front().first.itsTagSet;

      perform_tag_eviction<TagSetType, TagMapType>(valueTags, inputTagMap);

      inputMap.right.pop_front();
      currentSize -= valueSize;
    }
  }

  static void onEvict(MapType& inputMap,
                      TagMapType& inputTagMap,
                      std::size_t& currentSize,
                      std::size_t maxSize,
                      std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    while (currentSize > maxSize)
    {
      auto& item = inputMap.right.front().first;
      std::size_t valueSize = item.itsSize;
      TagSetType valueTags = item.itsTagSet;

      perform_tag_eviction<TagSetType, TagMapType>(valueTags, inputTagMap);

      evictedItems.push_back(std::make_pair(item.itsKey, item.itsValue));

      inputMap.right.pop_front();
      currentSize -= valueSize;
    }
  }

  static bool onInsert(MapType& inputMap,
                       TagMapType& inputTagMap,
                       const KeyType& key,
                       const ValueType& value,
                       const TagSetType& tags,
                       std::size_t& currentSize,
                       std::size_t maxSize)
  {
    std::size_t amountToInsert = SizeFunction::getSize(value);

    if (amountToInsert >= maxSize)
    {
      // User attempted to insert something that is bigger than the entire cache, return fail
      return false;
    }

    currentSize += amountToInsert;

    // Evict until within the maxSize limit
    onEvict(inputMap, inputTagMap, currentSize, maxSize);

    inputMap.insert(typename MapType::value_type(
        key, CacheObject<KeyType, ValueType, TagSetType>(key, value, tags, amountToInsert)));

    return true;
  }

  static bool onInsert(MapType& inputMap,
                       TagMapType& inputTagMap,
                       const KeyType& key,
                       const ValueType& value,
                       const TagSetType& tags,
                       std::size_t& currentSize,
                       std::size_t maxSize,
                       std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    std::size_t amountToInsert = SizeFunction::getSize(value);

    if (amountToInsert >= maxSize)
    {
      // User attempted to insert something that is bigger than the entire cache, return fail
      return false;
    }

    currentSize += amountToInsert;

    // Evict until within the maxSize limit
    onEvict(inputMap, inputTagMap, currentSize, maxSize, evictedItems);

    inputMap.insert(typename MapType::value_type(
        key, CacheObject<KeyType, ValueType, TagSetType>(key, value, tags, amountToInsert)));

    return true;
  }

  static void onAccess(MapType& /* inputMap */, typename MapType::left_iterator& /* it */)
  {
    // No-op
  }
};

// First in last out removal policy
template <class MapType,
          class TagMapType,
          class KeyType,
          class ValueType,
          class TagSetType,
          class SizeFunction>
struct FILOEviction
{
  static void onEvict(MapType& inputMap,
                      TagMapType& inputTagMap,
                      std::size_t& currentSize,
                      std::size_t maxSize)
  {
    while (currentSize > maxSize)
    {
      std::size_t valueSize = inputMap.right.back().first.itsSize;
      TagSetType valueTags = inputMap.right.back().first.itsTagSet;

      perform_tag_eviction<TagSetType, TagMapType>(valueTags, inputTagMap);

      inputMap.right.pop_back();
      currentSize -= valueSize;
    }
  }

  static void onEvict(MapType& inputMap,
                      TagMapType& inputTagMap,
                      std::size_t& currentSize,
                      std::size_t maxSize,
                      std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    while (currentSize > maxSize)
    {
      auto& item = inputMap.right.back().first;
      std::size_t valueSize = item.itsSize;
      TagSetType valueTags = item.itsTagSet;

      perform_tag_eviction<TagSetType, TagMapType>(valueTags, inputTagMap);

      evictedItems.push_back(std::make_pair(item.itsKey, item.itsValue));

      inputMap.right.pop_back();
      currentSize -= valueSize;
    }
  }

  static bool onInsert(MapType& inputMap,
                       TagMapType& inputTagMap,
                       const KeyType& key,
                       const ValueType& value,
                       const TagSetType& tags,
                       std::size_t& currentSize,
                       std::size_t maxSize)
  {
    std::size_t amountToInsert = SizeFunction::getSize(value);

    if (amountToInsert >= maxSize)
    {
      // User attempted to insert something that is bigger than the entire cache, return fail
      return false;
    }

    currentSize += amountToInsert;

    // Evict until within the maxSize limit
    onEvict(inputMap, inputTagMap, currentSize, maxSize);

    inputMap.insert(typename MapType::value_type(
        key, CacheObject<KeyType, ValueType, TagSetType>(key, value, tags, amountToInsert)));

    return true;
  }

  static bool onInsert(MapType& inputMap,
                       TagMapType& inputTagMap,
                       const KeyType& key,
                       const ValueType& value,
                       const TagSetType& tags,
                       std::size_t& currentSize,
                       std::size_t maxSize,
                       std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    std::size_t amountToInsert = SizeFunction::getSize(value);

    if (amountToInsert >= maxSize)
    {
      // User attempted to insert something that is bigger than the entire cache, return fail
      return false;
    }

    currentSize += amountToInsert;

    // Evict until within the maxSize limit
    onEvict(inputMap, inputTagMap, currentSize, maxSize, evictedItems);

    inputMap.insert(typename MapType::value_type(
        key, CacheObject<KeyType, ValueType, TagSetType>(key, value, tags, amountToInsert)));

    return true;
  }

  static void onAccess(MapType& /* inputMap */, typename MapType::left_iterator& /* it */)
  {
    // No-op
  }
};

// ----------------------------------------------------------------------
/*!
 * \brief Tag expiration policies. theTagTime is expiration time stamp for the given tag
 */
// ----------------------------------------------------------------------

// Tag expiration does not depend on expiration age
struct StaticExpire
{
  static bool isExpired(const std::time_t& theTagTime, long timeConstant)
  {
    (void)timeConstant;
    // Max value means tag is valid
    return (theTagTime != std::numeric_limits<std::time_t>::max());
  }

  // All expired tags are deleted
  static bool toDelete(const std::time_t& theTagTime, long timeConstant)
  {
    (void)timeConstant;
    return (theTagTime != std::numeric_limits<std::time_t>::max());
  }
};

// Tag expires instantly after expiration age is reached
struct InstantExpire
{
  static bool isExpired(const std::time_t& theTagTime, long timeConstant)
  {
    // Max value means tag is valid
    if (theTagTime == std::numeric_limits<std::time_t>::max())
    {
      return false;
    }

    std::time_t now = std::time(nullptr);

    return ((now - theTagTime) > timeConstant);
  }

  // Expired tags older than timeConstant are deleted
  static bool toDelete(const std::time_t& theTagTime, long timeConstant)
  {
    std::time_t now = std::time(nullptr);

    return ((now - theTagTime) > timeConstant);
  }
};

struct CoinFlipExpire
{
  static bool isExpired(const std::time_t& theTagTime, long timeConstant)
  {
    static std::random_device dev;
    static std::mt19937 generator(dev());
    static std::uniform_int_distribution<> dist(0, 1);

    // Max value means tag is valid
    if (theTagTime == std::numeric_limits<std::time_t>::max())
    {
      return false;
    }

    std::time_t now = std::time(nullptr);

    if ((now - theTagTime) > timeConstant)
    {
      int chance = dist(generator);
      return (chance == 0);
    }
    return false;
  }

  // Expired tags older than 2*timeConstant are deleted
  static bool toDelete(const std::time_t& theTagTime, long timeConstant)
  {
    std::time_t now = std::time(nullptr);

    return ((now - theTagTime) > 2 * timeConstant);
  }
};

// Linearly time-dependent expiration
struct LinearTimeExpire
{
  static bool isExpired(const std::time_t& theTagTime, long timeConstant)
  {
    static std::random_device dev;
    static std::mt19937 generator(dev());
    static std::uniform_real_distribution<> dist(0.0, 1.0);

    // Max value means tag is valid
    if (theTagTime == std::numeric_limits<std::time_t>::max())
    {
      return false;
    }

    std::time_t now = std::time(nullptr);
    long tagAge = now - theTagTime;

    assert(timeConstant != 0);

    double expirationProbability = double(tagAge) / double(timeConstant);
    double chance = dist(generator);
    return (chance < expirationProbability);
  }

  // Expired tags with >100% removal probability are deleted
  static bool toDelete(const std::time_t& theTagTime, long timeConstant)
  {
    std::time_t now = std::time(nullptr);

    return ((now - theTagTime) > timeConstant);
  }
};

// Sigmoidal time-dependent expiration
struct SigmoidTimeExpire
{
  static bool isExpired(const std::time_t& theTagTime, long timeConstant)
  {
    static std::random_device dev;
    static std::mt19937 generator(dev());
    static std::uniform_real_distribution<> dist(0.0, 1.0);
    // Max value means tag is valid
    if (theTagTime == std::numeric_limits<std::time_t>::max())
    {
      return false;
    }

    std::time_t now = std::time(nullptr);
    long tagAge = now - theTagTime;

    double expirationProbability =
        1.0 /
        (1.0 + std::exp(-0.02 * static_cast<double>(tagAge) + static_cast<double>(timeConstant)));
    double chance = dist(generator);
    if (chance < expirationProbability)
    {
      return true;
    }
    return false;
  }

  // Expired tags are removed when their return probability is less than 1%
  static bool toDelete(const std::time_t& theTagTime, long timeConstant)
  {
    static double eliminationProbability = 0.99;
    static double eliminationAge =
        (std::log(eliminationProbability / (1.0 - eliminationProbability)) + 0.02) /
        double(timeConstant);
    std::time_t now = std::time(nullptr);

    return (static_cast<double>((now - theTagTime)) > eliminationAge);
  }
};

// ----------------------------------------------------------------------
/*!
 * \brief Cache object supporting tags and various eviction and expiration policies
 */
// ----------------------------------------------------------------------

template <class KeyType,
          class ValueType,
          template <class, class, class, class, class, class> class EvictionPolicy = LRUEviction,
          class TagType = int,
          class ExpirationPolicy = StaticExpire,
          class SizeFunc = TrivialSizeFunction<ValueType> >
class Cache
{
 public:
  using TagSetType = std::set<TagType>;
  using CacheObjectType = CacheObject<KeyType, ValueType, TagSetType>;
  using CacheReportingObjectType = CacheReportingObject<KeyType, ValueType, TagSetType>;

  using MapType = boost::bimaps::bimap<
      boost::bimaps::unordered_set_of<KeyType, boost::hash<KeyType>, std::equal_to<KeyType> >,
      boost::bimaps::list_of<CacheObjectType> >;

  using LeftIteratorType = typename MapType::left_iterator;
  using RightIteratorType = typename MapType::right_iterator;

  using TagMapType = std::map<TagType, std::pair<std::time_t, std::size_t> >;
  using ItemVector = std::vector<std::pair<KeyType, ValueType> >;

  // Default constructor eases the use as data member
  Cache() = default;

  Cache(const Cache& other) = delete;
  Cache(Cache&& other) = delete;
  Cache& operator=(const Cache& other) = delete;
  Cache& operator=(Cache&& other) = delete;

  Cache(std::size_t maxSize, long timeConstant = 0)
      : itsMaxSize(maxSize), itsTimeConstant(timeConstant)
  {
    //		itsMap.bucket_size(itsMaxSize);
  }

  // ----------------------------------------------------------------------
  /*!
   * \brief Get cache statistics
   */
  // ----------------------------------------------------------------------
  CacheStats statistics() const
  {
    Lock lock(itsMutex);
    return CacheStats(itsStartTime, itsMaxSize, itsSize, itsInsertCount, itsHitCount, itsMissCount);
  }

  // Insert with a list of tags
  template <class ListType>
  bool insert(const KeyType& key, const ValueType& value, const ListType& tags)
  {
    Lock wlock(itsMutex);

    bool result;

    LeftIteratorType it = itsMap.left.find(key);

    if (it == itsMap.left.end())
    {
      // Value not in cache

      // Make tagSet
      TagSetType tagSet;
      for (const auto& tag : tags)
        tagSet.insert(tag);

      // Perform insertion
      result =
          EvictionPolicy<MapType, TagMapType, KeyType, ValueType, TagSetType, SizeFunc>::onInsert(
              itsMap, itsTagMap, key, value, tagSet, itsSize, itsMaxSize);

      if (result)
      {
        ++itsInsertCount;

        // Successful insertion
        for (const auto& tag : tags)
        {
          // Check and update tags
          // Check if tag exists in tag map, if not, insert default  value
          auto tagIt = itsTagMap.find(tag);
          if (tagIt == itsTagMap.end())
          {
            itsTagMap.insert(typename TagMapType::value_type(
                tag, std::make_pair(std::numeric_limits<std::time_t>::max(), 1)));
          }
          else
          {
            // Check if user is reusing an expired tag. If so, update the tag
            if (ExpirationPolicy::toDelete(tagIt->second.first, itsTimeConstant))
            {
              tagIt->second.first = std::numeric_limits<std::time_t>::max();
            }

            // Increase the usage counter
            tagIt->second.second++;
          }
        }
      }
    }

    else
    {
      // Value already in cache, not inserted
      result = false;
    }

    return result;
  }

  // Insert with a list of tags, return evicted items
  template <class ListType>
  bool insert(const KeyType& key,
              const ValueType& value,
              const ListType& tags,
              std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    evictedItems.clear();

    Lock wlock(itsMutex);

    bool result;

    LeftIteratorType it = itsMap.left.find(key);

    if (it == itsMap.left.end())
    {
      // Value not in cache

      // Make tagSet
      TagSetType tagSet;
      for (const auto& tag : tags)
        tagSet.insert(tag);

      // Perform insertion
      result =
          EvictionPolicy<MapType, TagMapType, KeyType, ValueType, TagSetType, SizeFunc>::onInsert(
              itsMap, itsTagMap, key, value, tagSet, itsSize, itsMaxSize, evictedItems);

      if (result)
      {
        // Successful insertion

        ++itsInsertCount;

        for (const auto& tag : tags)
        {
          // Check and update tags
          // Check if tag exists in tag map, if not, insert default  value
          auto tagIt = itsTagMap.find(tag);
          if (tagIt == itsTagMap.end())
          {
            itsTagMap.insert(typename TagMapType::value_type(
                tag, std::make_pair(std::numeric_limits<std::time_t>::max(), 1)));
          }
          else
          {
            // Check if user is reusing an expired tag. If so, update the tag
            if (ExpirationPolicy::toDelete(tagIt->second.first, itsTimeConstant))
            {
              tagIt->second.first = std::numeric_limits<std::time_t>::max();
            }

            // Increase the usage counter
            tagIt->second.second++;
          }
        }
      }
    }

    else
    {
      // Value already in cache, not inserted
      result = false;
    }

    return result;
  }

  // Insert with a single tag
  bool insert(const KeyType& key, const ValueType& value, const TagType& tag)
  {
    Lock wlock(itsMutex);

    bool result;

    LeftIteratorType it = itsMap.left.find(key);

    if (it == itsMap.left.end())
    {
      // Value not in cache

      TagSetType tagSet;
      tagSet.insert(tag);

      // Perform insertion
      result =
          EvictionPolicy<MapType, TagMapType, KeyType, ValueType, TagSetType, SizeFunc>::onInsert(
              itsMap, itsTagMap, key, value, tagSet, itsSize, itsMaxSize);

      if (result)
      {
        // Successful insertion

        ++itsInsertCount;

        // Check if tag exists in tag map, if not, insert default  value
        auto tagIt = itsTagMap.find(tag);
        if (tagIt == itsTagMap.end())
        {
          itsTagMap.insert(typename TagMapType::value_type(
              tag, std::make_pair(std::numeric_limits<std::time_t>::max(), 1)));
        }
        else
        {
          // Check if user is reusing an expired tag. If so, update the tag
          if (ExpirationPolicy::toDelete(tagIt->second.first, itsTimeConstant))
          {
            tagIt->second.first = std::numeric_limits<std::time_t>::max();
          }

          // Increase the usage counter
          tagIt->second.second++;
        }
      }
    }

    else
    {
      // Value already in cache, not inserted
      result = false;
    }

    return result;
  }

  // Insert with a single tag, returns evicted items
  bool insert(const KeyType& key,
              const ValueType& value,
              const TagType& tag,
              std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    evictedItems.clear();

    Lock wlock(itsMutex);

    bool result;

    LeftIteratorType it = itsMap.left.find(key);

    if (it == itsMap.left.end())
    {
      // Value not in cache

      TagSetType tagSet;
      tagSet.insert(tag);

      // Perform insertion
      result =
          EvictionPolicy<MapType, TagMapType, KeyType, ValueType, TagSetType, SizeFunc>::onInsert(
              itsMap, itsTagMap, key, value, tagSet, itsSize, itsMaxSize, evictedItems);

      if (result)
      {
        // Successful insertion

        ++itsInsertCount;

        // Check if tag exists in tag map, if not, insert default  value
        auto tagIt = itsTagMap.find(tag);
        if (tagIt == itsTagMap.end())
        {
          itsTagMap.insert(typename TagMapType::value_type(
              tag, std::make_pair(std::numeric_limits<std::time_t>::max(), 1)));
        }
        else
        {
          // Check if user is reusing an expired tag. If so, update the tag
          if (ExpirationPolicy::toDelete(tagIt->second.first, itsTimeConstant))
          {
            tagIt->second.first = std::numeric_limits<std::time_t>::max();
          }

          // Increase the usage counter
          tagIt->second.second++;
        }
      }
    }

    else
    {
      // Value already in cache, not inserted
      result = false;
    }

    return result;
  }

  // Tagless insert for simple use
  bool insert(const KeyType& key, const ValueType& value)
  {
    Lock lock(itsMutex);

    bool result;

    LeftIteratorType it = itsMap.left.find(key);

    if (it == itsMap.left.end())
    {
      // Value not in cache
      // Perform insertion
      result =
          EvictionPolicy<MapType, TagMapType, KeyType, ValueType, TagSetType, SizeFunc>::onInsert(
              itsMap, itsTagMap, key, value, TagSetType(), itsSize, itsMaxSize);

      if (result)
        ++itsInsertCount;
    }

    else
    {
      // Value already in cache, not inserted
      result = false;
    }

    return result;
  }

  // Tagless insert for simple use, returns evicted items
  bool insert(const KeyType& key,
              const ValueType& value,
              std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    evictedItems.clear();

    Lock lock(itsMutex);

    bool result;

    LeftIteratorType it = itsMap.left.find(key);

    if (it == itsMap.left.end())
    {
      // Value not in cache
      // Perform insertion
      result =
          EvictionPolicy<MapType, TagMapType, KeyType, ValueType, TagSetType, SizeFunc>::onInsert(
              itsMap, itsTagMap, key, value, TagSetType(), itsSize, itsMaxSize, evictedItems);
      if (result)
        ++itsInsertCount;
    }

    else
    {
      // Value already in cache, not inserted
      result = false;
    }

    return result;
  }

  // Find value from cache
  std::optional<ValueType> find(const KeyType& key)
  {
    Lock lock(itsMutex);
    LeftIteratorType it = itsMap.left.find(key);

    if (it != itsMap.left.end())
    {
      // Check if any of the tags in the requested key have been phased out
      auto& tagSet = it->second.itsTagSet;
      for (auto tagIt = tagSet.begin(); tagIt != tagSet.end(); ++tagIt)
      {
        auto tagMapIt = itsTagMap.find(*tagIt);

        if (tagMapIt == itsTagMap.end())
        {
          // Tag has expired and tag map has been cleaned. Remove from cache
          itsMap.left.erase(it);
          ++itsMissCount;
          return {};
        }

        // If one of the tags is expired, remove object from cache
        if (ExpirationPolicy::isExpired(tagMapIt->second.first, itsTimeConstant))
        {
          // This tag expired, erase the object from cache and return empty
          itsMap.left.erase(it);
          ++itsMissCount;
          return {};
        }
      }

      EvictionPolicy<MapType, TagMapType, KeyType, ValueType, TagSetType, SizeFunc>::onAccess(
          itsMap, it);

      ++itsHitCount;

      // Update hit count for this entry
      ++it->second.itsHits;

      return it->second.itsValue;
    }

    ++itsMissCount;

    return {};
  }

  // Find value from cache and return also its hits
  std::optional<ValueType> find(const KeyType& key, std::size_t& hits)
  {
    Lock mapLock(itsMutex);
    LeftIteratorType it = itsMap.left.find(key);

    if (it != itsMap.left.end())
    {
      // Check if any of the tags in the requested key have been phased out
      auto& tagSet = it->second.itsTagSet;
      for (auto tagIt = tagSet.begin(); tagIt != tagSet.end(); ++tagIt)
      {
        auto tagMapIt = itsTagMap.find(*tagIt);

        if (tagMapIt == itsTagMap.end())
        {
          // Tag has expired and tag map has been cleaned. Remove from cache
          itsMap.left.erase(it);
          ++itsMissCount;
          return {};
        }

        // If one of the tags is expired, remove object from cache

        if (ExpirationPolicy::isExpired(tagMapIt->second.first, itsTimeConstant))
        {
          // This tag expired, erase the object from cache and return empty
          itsMap.left.erase(it);
          ++itsMissCount;
          return {};
        }
      }

      EvictionPolicy<MapType, TagMapType, KeyType, ValueType, TagSetType, SizeFunc>::onAccess(
          itsMap, it);

      ++itsHitCount;

      // Update hit count for this entry

      hits = ++it->second.itsHits;

      return it->second.itsValue;
    }

    ++itsMissCount;
    return {};
  }

  void expire(const TagType& tagToExpire, const std::time_t& expirationTime = std::time(nullptr))
  {
    Lock wlock(itsMutex);
    // Set data containing the given tag as expired
    auto tagIt = itsTagMap.find(tagToExpire);
    if (tagIt == itsTagMap.end())
    {
      // Trying to expire nonexisting tag
      return;
    }
    else
    {
      tagIt->second.first = expirationTime;
    }

    // If tag map has grown large, clean it up
    if (itsTagMap.size() >= itsMaxSize)
    {
      clearExpiredTags();
    }
  }

  void clear()
  {
    Lock lock(itsMutex);
    itsMap.clear();
    itsTagMap.clear();
    itsSize = 0;
  }

  // Resize cache
  void resize(std::size_t newMaxSize)
  {
    Lock lock(itsMutex);

    // Evict until the cache fits into new size
    itsMaxSize = newMaxSize;
    EvictionPolicy<MapType, TagMapType, KeyType, ValueType, TagSetType, SizeFunc>::onEvict(
        itsMap, itsTagMap, itsSize, itsMaxSize);
  }

  // Resize cache, return evicted items
  void resize(std::size_t newMaxSize, std::vector<std::pair<KeyType, ValueType> >& evictedItems)
  {
    evictedItems.clear();

    Lock lock(itsMutex);

    // Evict until the cache fits into new size
    itsMaxSize = newMaxSize;
    EvictionPolicy<MapType, TagMapType, KeyType, ValueType, TagSetType, SizeFunc>::onEvict(
        itsMap, itsTagMap, itsSize, itsMaxSize, evictedItems);
  }

  std::size_t size() const
  {
    Lock lock(itsMutex);
    return itsSize;
  }

  std::size_t maxSize() const
  {
    Lock lock(itsMutex);
    return itsMaxSize;
  }

  std::list<CacheReportingObjectType> getContent() const
  {
    Lock lock(itsMutex);
    std::list<CacheReportingObjectType> result;
    for (auto it = itsMap.right.begin(); it != itsMap.right.end(); ++it)
    {
      result.push_back(CacheReportingObjectType(it->second,
                                                it->first.itsValue,
                                                it->first.itsTagSet,
                                                it->first.itsHits,
                                                it->first.itsSize));
    }
    return result;
  }

  std::string getTextContent() const
  {
    std::stringstream output;
    Lock lock(itsMutex);
    auto n = 0UL;

    for (auto it = itsMap.right.begin(); it != itsMap.right.end(); ++it, ++n)
    {
      if (n > 0)
        output << ',';
      output << it->first.itsValue;
    }

    return output.str();
  }

 private:
  // Clear expired tags from the tag map
  void clearExpiredTags()
  {
    for (auto tagIt = itsTagMap.begin(); tagIt != itsTagMap.end();)
    {
      if (ExpirationPolicy::toDelete(tagIt->second.first, itsTimeConstant))
      {
        itsTagMap.erase(tagIt++);
      }
      else
      {
        ++tagIt;
      }
    }
  }

  MapType itsMap;
  TagMapType itsTagMap;

  mutable MutexType itsMutex;

  std::size_t itsSize = 0;
  std::size_t itsMaxSize = 0;

  std::size_t itsInsertCount = 0;
  std::size_t itsMissCount = 0;
  std::size_t itsHitCount = 0;

  const DateTime itsStartTime = Fmi::SecondClock::universal_time();

  long itsTimeConstant = 0;
};

// Size_t parser for the FileCache
bool parse_size_t(const std::string& input, std::size_t& result);

// ----------------------------------------------------------------------
/*!
 * \brief File Cache object for filesystem-backed caching
 */
// ----------------------------------------------------------------------
struct FileCacheStruct
{
  FileCacheStruct(const std::filesystem::path& thePath, std::size_t theSize)
      : path(thePath), fileSize(theSize)
  {
  }

  std::filesystem::path path;
  std::size_t fileSize;
};

class FileCache
{
  using MutexType = boost::shared_mutex;
  using ReadLock = boost::shared_lock<MutexType>;
  using WriteLock = boost::unique_lock<MutexType>;
  using UpgradeReadLock = boost::upgrade_lock<MutexType>;
  using UpgradeWriteLock = boost::upgrade_to_unique_lock<MutexType>;

  using MapType = boost::bimaps::bimap<boost::bimaps::unordered_set_of<std::size_t,
                                                                       boost::hash<std::size_t>,
                                                                       std::equal_to<std::size_t> >,
                                       boost::bimaps::list_of<FileCacheStruct> >;

 public:
  // ----------------------------------------------------------------------
  /*!
   * \brief Constructor
   * This attempts to validate the cache directory for permission failures etc.
   */
  // ----------------------------------------------------------------------
  FileCache(const std::filesystem::path& directory, std::size_t maxSize);

  FileCache(const FileCache& other) = delete;
  FileCache(FileCache&& other) = delete;
  FileCache& operator=(const FileCache& other) = delete;
  FileCache& operator=(FileCache&& other) = delete;

  // ----------------------------------------------------------------------
  /*!
   * \brief Find entry from the cache
   */
  // ----------------------------------------------------------------------

  std::optional<std::string> find(std::size_t key);

  // ----------------------------------------------------------------------
  /*!
   * \brief Insert an entry into the cache.
   * If performCleanup is false, no cleanup is performed and if the entry
   * does not fit into the cache failure is returned.
   */
  // ----------------------------------------------------------------------

  bool insert(std::size_t key, const std::string& value, bool performCleanup = true);

  // ----------------------------------------------------------------------
  /*!
   * \brief Get keys from the cache
   */
  // ----------------------------------------------------------------------

  std::vector<std::size_t> getContent() const;

  // ----------------------------------------------------------------------
  /*!
   * \brief Get cache size in bytes
   */
  // ----------------------------------------------------------------------

  std::size_t getSize() const;

  // ----------------------------------------------------------------------
  /*!
   * \brief Do manual cleanup of the cache
   */
  // ----------------------------------------------------------------------

  bool clean(std::size_t spaceNeeded);

  // ----------------------------------------------------------------------
  /*!
   * \brief Get cache statistics
   */
  // ----------------------------------------------------------------------

  CacheStats statistics() const;

 private:
  // ----------------------------------------------------------------------
  /*!
   * \brief Performs cleanup
   */
  // ----------------------------------------------------------------------

  bool performCleanup(std::size_t space_needed);

  // ----------------------------------------------------------------------
  /*!
   * \brief Recurses through the cache directory structure and updates the cache map accordingly
   */
  // ----------------------------------------------------------------------

  void update();

  // ----------------------------------------------------------------------
  /*!
   * \brief Writes a file to disk
   */
  // ----------------------------------------------------------------------

  bool writeFile(const std::filesystem::path& theDir,
                 const std::string& fileName,
                 const std::string& theValue) const;

  // ----------------------------------------------------------------------
  /*!
   * \brief Checks that entry can be written to disk
   */
  // ----------------------------------------------------------------------

  bool checkForDiskSpace(const std::filesystem::path& thePath,
                         const std::string& theValue,
                         bool doCleanup);

  // ----------------------------------------------------------------------
  /*!
   * \brief Gets subdirectory and filename from hash value
   */
  // ----------------------------------------------------------------------

  std::pair<std::string, std::string> getFileDirAndName(std::size_t hashValue) const;

  // ----------------------------------------------------------------------
  /*!
   * \brief Gets hash value from subdirectory and filename
   */
  // ----------------------------------------------------------------------

  bool getKey(const std::string& directory, const std::string& filename, std::size_t& key) const;

  std::size_t itsSize = 0;

  std::size_t itsMaxSize = 0;

  std::size_t itsInsertCount = 0;
  std::size_t itsMissCount = 0;
  std::size_t itsHitCount = 0;

  const DateTime itsStartTime = Fmi::SecondClock::universal_time();

  std::filesystem::path itsDirectory;

  MapType itsContentMap;

  mutable MutexType itsMutex;
};

}  // namespace Cache

}  // namespace Fmi
