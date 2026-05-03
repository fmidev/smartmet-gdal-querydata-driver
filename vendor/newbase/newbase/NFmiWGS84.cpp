#include "NFmiWGS84.h"
#include <fmt/format.h>
#include <gis/SpatialReference.h>
#include <macgyver/Exception.h>
#include <memory>

// ----------------------------------------------------------------------
/*!
 * \brief Implementation details
 */
// ----------------------------------------------------------------------

class NFmiWGS84Impl
{
 public:
#ifdef NEW_NFMIAREA
  NFmiWGS84Impl() : sr("WGS84") {}
#else
  NFmiWGS84Impl() : sr(fmt::format("+proj=longlat +R={} +wktext +no_defs +type=crs", kRearth)) {}
#endif

  const Fmi::SpatialReference& SpatialReference() const { return sr; }

 private:
  Fmi::SpatialReference sr;
};

const Fmi::SpatialReference& NFmiWGS84::SpatialReference()
{
  try
  {
    static NFmiWGS84Impl impl;  // thread safe init in C++11
    return impl.SpatialReference();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
