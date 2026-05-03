#include "NFmiNearTreeLocation.h"
#include "NFmiGlobals.h"
#include "NFmiLocation.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * \brief Default constructor is needed for NFmiNearTree
 */
// ----------------------------------------------------------------------

NFmiNearTreeLocation::NFmiNearTreeLocation() = default;
// ----------------------------------------------------------------------
/*!
 * \brief Construct unit circle location from latlon information
 */
// ----------------------------------------------------------------------

NFmiNearTreeLocation::NFmiNearTreeLocation(const NFmiLocation& theLocation, std::size_t theIndex)
    : itsX(), itsY(), itsZ(), itsIndex(theIndex)
{
  try
  {
    double lon = FmiRad(theLocation.GetLongitude());
    double lat = FmiRad(theLocation.GetLatitude());
    itsX = cos(lat) * cos(lon);
    itsY = cos(lat) * sin(lon);
    itsZ = sin(lat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
