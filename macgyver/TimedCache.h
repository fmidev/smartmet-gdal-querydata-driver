#pragma once

#include "Exception.h"
#include <boost/thread.hpp>
#include <chrono>
#include <map>
#include <string>

namespace Fmi
{
namespace TimedCache
{
using MutexType = boost::mutex;
using Lock = boost::lock_guard<MutexType>;

class CacheStatistics
{
 public:
  using SizeType = std::size_t;
  using ClockType = std::chrono::high_resolution_clock;
  using TimeType = std::chrono::time_point<ClockType>;

  CacheStatistics()
      : mConstructionTime(ClockType::now()),
        mHits(0),
        mMisses(0),
        mEvictions(0),
        mInsertFailures(0),
        mInsertSuccesses(0)
  {
  }

  void hit() { mHits++; }
  void miss() { mMisses++; }
  void insertFail() { mInsertFailures++; }
  void insertSuccess() { mInsertSuccesses++; }
  void eviction() { mEvictions++; }

  TimeType getConstructionTime() const { return mConstructionTime; }
  SizeType getHits() const { return mHits; }
  SizeType getMisses() const { return mMisses; }
  SizeType getEvictions() const { return mEvictions; }
  SizeType getInsertFailures() const { return mInsertFailures; }
  SizeType getInsertSuccesses() const { return mInsertSuccesses; }

 private:
  TimeType mConstructionTime;
  SizeType mHits;
  SizeType mMisses;
  SizeType mEvictions;
  SizeType mInsertFailures;
  SizeType mInsertSuccesses;
};

/** Cache with LRU and time eviction.

  Cache discards the least recently used (LRU) items first.
  Cache discards time expired items if constructed with
  a positive time eviction duration as seconds.

  User can override the expiration duration by giving
  custom eviction value for each or selected cache items
  while inserting them into the cache.
 */
template <typename KeyType, typename ValueType>
class [[deprecated]] Cache
{
  using SizeType = std::size_t;
  using DurationType = std::chrono::seconds;
  using ClockType = std::chrono::high_resolution_clock;
  using TimeType = std::chrono::time_point<ClockType>;
  using TimeValuePair = std::pair<TimeType, ValueType>;
  using KeyTimeValuePair = std::pair<KeyType, TimeValuePair>;
  using KeyTimeValueList = std::list<KeyTimeValuePair>;
  using KeyTimeValueListIt = decltype(KeyTimeValueList().begin());
  using KeyKeyTimeValueListItPair = std::pair<KeyType, KeyTimeValueListIt>;
  using KeyTimeValueMap = std::map<KeyType, KeyTimeValueListIt>;

 public:
  // Constructor with LRU eviction and fixed cache size 10.
  Cache() : mMaxSize(10), mDuration(0) {}

  // Constructor with LRU eviction and user defined cache size.
  Cache(const std::size_t& maxSize) : mMaxSize(maxSize), mDuration(0) {}

  // Constructor with LRU eviction and time eviction and user defined cache size.
  Cache(const std::size_t& maxSize, const std::chrono::seconds& duration)
      : mMaxSize(maxSize), mDuration(duration)
  {
  }

  /** Insert a key-value-pair with a custom time eviction duration.
   *  The value of duration attribute overrides the duration of constructor.
   *  @param key The key is needed when finding the value from cache with the find method.
   *  @param value An object to be stored to the cache.
   *  @retval true The key-value insertion success.
   *  @retval false The key-value insertion failed (the key has a valid value in the cache)
   */
  bool insert(const KeyType& key, const ValueType& value, const std::chrono::seconds& duration)
  {
    Lock lock(mMutex);

    if (mMaxSize == 0)
      return false;

    TimeType now = ClockType::now();
    auto mapIt = mKeyTimeValueMap.find(key);
    if (mapIt != mKeyTimeValueMap.end())
    {
      // Remove the object if expired, otherwise it is valid.
      auto listIt = mapIt->second;
      if (now > listIt->second.first)
      {
        mCacheStatistics.eviction();
        mKeyTimeValueList.erase(listIt);
        mKeyTimeValueMap.erase(mapIt);
      }
      else
      {
        // Cache object is still valid.
        mCacheStatistics.insertFail();
        return false;
      }
    }

    // Object is not in the cache.
    // Make space if full and remove expired objects.
    if (mKeyTimeValueList.size() >= mMaxSize)
    {
      auto listIt = mKeyTimeValueList.rbegin();
      auto listEndIt = mKeyTimeValueList.rend();
      while (
          (mKeyTimeValueList.size() >= mMaxSize) or
          (mDuration > DurationType(0) and (listIt != listEndIt) and (now > listIt->second.first)))
      {
        mapIt = mKeyTimeValueMap.find(listIt->first);
        if (mapIt != mKeyTimeValueMap.end())
        {
          mCacheStatistics.eviction();
          mKeyTimeValueList.pop_back();
          mapIt = mKeyTimeValueMap.erase(mapIt);
        }

        listIt = mKeyTimeValueList.rbegin();
        listEndIt = mKeyTimeValueList.rend();
      }
    }

    if (mKeyTimeValueList.size() >= mMaxSize)
      throw Fmi::Exception(BCP, "Object cache is still full after cleaning");

    mCacheStatistics.insertSuccess();
    TimeType evictionTime = now + duration;
    mKeyTimeValueList.push_front(KeyTimeValuePair(key, TimeValuePair(evictionTime, value)));
    mKeyTimeValueMap.insert(KeyKeyTimeValueListItPair(key, mKeyTimeValueList.begin()));

    return true;
  }

  /** Insert a key-value-pair
   *  @param key The key is needed when finding the value from cache with the find method.
   *  @param value An object to be stored to the cache.
   *  @retval true The key-value insertion success.
   *  @retval false The key-value insertion failed (the key has a valid value in the cache)
   */
  bool insert(const KeyType& key, const ValueType& value) { return insert(key, value, mDuration); }

  /** Finds object with specific key.
   *  @param key Key of the object to search for.
   *  @return Non empty std::optional value if the object is found.
   */
  std::optional<ValueType> find(const KeyType& key) const
  {
    Lock lock(mMutex);

    if (mMaxSize == 0)
    {
      mCacheStatistics.miss();
      return std::optional<ValueType>();
    }

    auto mapIt = mKeyTimeValueMap.find(key);
    if (mapIt == mKeyTimeValueMap.end())
    {
      mCacheStatistics.miss();
      return std::optional<ValueType>();
    }

    TimeType now = ClockType::now();
    auto listIt = mapIt->second;

    if (mDuration > DurationType(0))
    {
      // Remove the object from the cache if expired
      if (now > listIt->second.first)
      {
        mKeyTimeValueList.erase(listIt);
        mKeyTimeValueMap.erase(mapIt);

        mCacheStatistics.eviction();
        mCacheStatistics.miss();
        return std::optional<ValueType>();
      }
    }

    // Move the object to the begin of the list (LRU).
    mKeyTimeValueList.splice(mKeyTimeValueList.begin(), mKeyTimeValueList, listIt);
    listIt = mKeyTimeValueList.begin();
    if (listIt->first != key)
      throw Fmi::Exception(BCP, "Mixed keys after a splice of cache list");

    // Update the reference of object to the map.
    mapIt = mKeyTimeValueMap.find(listIt->first);
    if (mapIt == mKeyTimeValueMap.end())
      throw Fmi::Exception(BCP, "Map object not found after a splice of cache list");
    mapIt->second = listIt;

    mCacheStatistics.hit();
    return std::optional<ValueType>(listIt->second.second);
  }

  std::size_t size() const { return mKeyTimeValueMap.size(); }

  std::size_t maxSize() const { return mMaxSize; }

  CacheStatistics getCacheStatistics() const
  {
    Lock lock(mMutex);
    return mCacheStatistics;
  }

 private:
  Cache(const Cache&) = delete;
  Cache& operator=(const Cache&) = delete;

  mutable KeyTimeValueMap mKeyTimeValueMap;
  mutable KeyTimeValueList mKeyTimeValueList;
  mutable MutexType mMutex;

  SizeType mMaxSize;

  DurationType mDuration;

  mutable CacheStatistics mCacheStatistics;
};
}  // namespace TimedCache
}  // namespace Fmi
