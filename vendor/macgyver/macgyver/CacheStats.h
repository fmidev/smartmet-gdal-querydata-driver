// ======================================================================
/*!
 * \brief Interface of class CacheStats
 */
// ======================================================================

#pragma once

#include "DateTime.h"
#include <map>

namespace Fmi
{
namespace Cache
{
struct CacheStats
{
  CacheStats() = default;
  CacheStats(const DateTime& t) : starttime(t) {}
  CacheStats(const DateTime& t,
             std::size_t maxsize_,
             std::size_t size_,
             std::size_t inserts_,
             std::size_t hits_,
             std::size_t misses_)
      : starttime(t),
        maxsize(maxsize_),
        size(size_),
        inserts(inserts_),
        hits(hits_),
        misses(misses_)
  {
  }

  DateTime starttime = Fmi::SecondClock::universal_time();
  std::size_t maxsize = 0;
  std::size_t size = 0;
  std::size_t inserts = 0;
  std::size_t hits = 0;
  std::size_t misses = 0;
};

using CacheStatistics = std::map<std::string, CacheStats>;

}  // namespace Cache
}  // namespace Fmi
