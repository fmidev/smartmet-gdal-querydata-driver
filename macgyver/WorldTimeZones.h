// ======================================================================
/*!
 * \brief Interface of class WorldTimeZones
 *
 * Constructed objects are immutable and hence thread safe
 */
// ======================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Fmi
{
class WorldTimeZones
{
 public:
  WorldTimeZones(const std::string& file);
  ~WorldTimeZones();

  const std::string& zone_name(float lon, float lat) const;
  const std::vector<std::string> zones() const { return itsZones; }

 private:
  unsigned int itsWidth;
  unsigned int itsHeight;
  float itsLon1;
  float itsLat1;
  float itsLon2;
  float itsLat2;

  std::vector<std::string> itsZones;

  uint32_t itsSize;
  char* itsData;

};  // class WorldTimeZones
}  // namespace Fmi

// ======================================================================
