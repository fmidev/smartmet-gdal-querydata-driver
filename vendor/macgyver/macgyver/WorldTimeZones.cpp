// ======================================================================
/*!
 * \brief Implementation of class WorldTimeZones
 *
 * Note: We use simple file read instead of memory mapping since the
 * BrainStorm server will maintain a singleton accessor anyway. Also,
 * data sizes are so small, that startup efficiency should not be
 * a problem.
 *
 */
// ======================================================================

#include "WorldTimeZones.h"

#include "Exception.h"
#include "StringConversion.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Read a position value at the given index
 */
// ----------------------------------------------------------------------

uint32_t read_pos(std::size_t thePos, char *theData)
{
  try
  {
    std::size_t offset = thePos * (sizeof(uint32_t) + sizeof(uint16_t));
    uint32_t tmp;
    memcpy(&tmp, theData + offset, sizeof(uint32_t));
    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Read an attribute value at the given index
 */
// ----------------------------------------------------------------------

uint16_t read_attr(std::size_t thePos, char *theData)
{
  try
  {
    std::size_t offset = thePos * (sizeof(uint32_t) + sizeof(uint16_t)) + sizeof(uint32_t);
    uint16_t tmp;
    memcpy(&tmp, theData + offset, sizeof(uint16_t));
    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

namespace Fmi
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

WorldTimeZones::~WorldTimeZones()
{
  delete[] itsData;
}
// ----------------------------------------------------------------------
/*!
 * \brief Establish timezone of lon/lat point
 *
 * Throws for unknown points.
 */
// ----------------------------------------------------------------------

const std::string &WorldTimeZones::zone_name(float lon, float lat) const
{
  try
  {
    if (lon < itsLon1 || lon > itsLon2 || lat < itsLat1 || lat > itsLat2)
      throw Fmi::Exception(BCP,
                           "Invalid lon-lat given to WorldTimeZones::zone_name: " +
                               Fmi::to_string(lon) + "," + Fmi::to_string(lat));

    // Calculate the index of the coordinate

    auto y = (lat - itsLat1) / (itsLat2 - itsLat1) * (itsHeight - 1);
    auto x = (lon - itsLon1) / (itsLon2 - itsLon1) * (itsWidth - 1);

    uint32_t i = lround(x);
    uint32_t j = lround(y);

    uint32_t pos = j + i * itsHeight;

    // Search the index from the data with binary search

    uint32_t lo = 0;
    uint32_t hi = itsSize;

    while (true)
    {
      if (lo == hi)
        break;

      uint32_t mid = (lo + hi) / 2 + 1;
      uint32_t midpos = read_pos(mid, itsData);

      if (midpos > pos)
        hi = mid - 1;
      else
        lo = mid;
    }
    uint16_t attr = read_attr(lo, itsData);

    if (attr <= 0 || static_cast<size_t>(attr) > itsZones.size())
      throw Fmi::Exception(BCP,
                           "Failed to find a timezone for coordinate " + Fmi::to_string(lon) + "," +
                               Fmi::to_string(lat));

    return itsZones[attr - 1];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

WorldTimeZones::WorldTimeZones(const std::string &theFile) : itsSize(0), itsData(nullptr)
{
  try
  {
    std::ifstream in(theFile.c_str());
    if (!in)
      throw Fmi::Exception(BCP, "Could not open '" + theFile + "' for reading");

    std::string token;
    in >> token;
    if (token != "SHAPEPACK")
      throw Fmi::Exception(BCP, "File '" + theFile + "' is not a shapepack file");

    int zonecount;
    in >> itsWidth >> itsHeight >> itsLon1 >> itsLat1 >> itsLon2 >> itsLat2 >> zonecount;
    // Skip the remaining line
    std::getline(in, token);

    if (!in.good())
      throw Fmi::Exception(BCP, "Invalid header in '" + theFile + "'");

    for (int i = 0; i < zonecount; i++)
    {
      std::getline(in, token);
      itsZones.push_back(token);
    }

    in.read(reinterpret_cast<char *>(&itsSize), sizeof(uint32_t));

    std::size_t bufsize = (sizeof(uint32_t) + sizeof(uint16_t)) * itsSize;

    itsData = new char[bufsize];
    if (itsData == nullptr)
      throw Fmi::Exception(BCP, "Failed to allocate memory for zone information");
    in.read(itsData, static_cast<long>(bufsize));
    if (in.bad())
      throw Fmi::Exception(BCP, "Reading timezone data failed");

    in.close();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Fmi

// ======================================================================
