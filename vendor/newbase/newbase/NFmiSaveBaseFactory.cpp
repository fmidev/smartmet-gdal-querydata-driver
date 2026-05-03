// ======================================================================
/*!
 * \file NFmiSaveBaseFactory.cpp
 * \brief Implementation of a factory for newbase objects
 */
// ======================================================================

#include "NFmiSaveBaseFactory.h"
#include "NFmiEquidistArea.h"
#include "NFmiGdalArea.h"
#include "NFmiGnomonicArea.h"
#include "NFmiGrid.h"
#include "NFmiLambertConformalConicArea.h"
#include "NFmiLambertEqualArea.h"
#include "NFmiLatLonArea.h"
#include "NFmiMercatorArea.h"
#include "NFmiQueryData.h"
#include "NFmiRotatedLatLonArea.h"
#include "NFmiStationBag.h"
#include "NFmiStereographicArea.h"
#include "NFmiVersion.h"
#include "NFmiWebMercatorArea.h"
#include "NFmiYKJArea.h"
#include <macgyver/Exception.h>
#include <macgyver/StringConversion.h>

// ----------------------------------------------------------------------
/*!
 * Returns a newbase object based on the given unique numeric ID.
 * The returned object is a void pointer, which makes this a rather
 * strange factory, but it will have to do until a complete redesign.
 *
 * \param classId The ID of the object to be created
 * \return Pointer to the new object
 * \todo Should return an std::shared_ptr
 */
// ----------------------------------------------------------------------

void *CreateSaveBase(unsigned int classId)
{
  try
  {
    switch (classId)
    {
      case kNFmiGrid:
        return static_cast<void *>(new NFmiGrid);

      case kNFmiLambertEqualArea:
        return static_cast<void *>(new NFmiLambertEqualArea);
      case kNFmiLatLonArea:
        return static_cast<void *>(new NFmiLatLonArea);
      case kNFmiRotatedLatLonArea:
        return static_cast<void *>(new NFmiRotatedLatLonArea);
      case kNFmiStereographicArea:
        return static_cast<void *>(new NFmiStereographicArea);
      case kNFmiYKJArea:
        return static_cast<void *>(new NFmiYKJArea);
      case kNFmiEquiDistArea:
        return static_cast<void *>(new NFmiEquidistArea);
      case kNFmiMercatorArea:
        return static_cast<void *>(new NFmiMercatorArea);
      case kNFmiWebMercatorArea:
        return static_cast<void *>(new NFmiWebMercatorArea);
      case kNFmiGnomonicArea:
        return static_cast<void *>(new NFmiGnomonicArea);
      case kNFmiLambertConformalConicArea:
        return static_cast<void *>(new NFmiLambertConformalConicArea);

      case kNFmiQueryData:
        return static_cast<void *>(new NFmiQueryData);
      case kNFmiQueryInfo:
        return static_cast<void *>(new NFmiQueryInfo);

      case kNFmiLocationBag:
        return static_cast<void *>(new NFmiLocationBag);
      case kNFmiStationBag:
        return static_cast<void *>(new NFmiStationBag);
#ifdef UNIX
#ifndef DISABLED_GDAL
      case kNFmiGdalArea:
        return static_cast<void *>(new NFmiGdalArea);
#endif
#endif

      default:
        throw Fmi::Exception(
            BCP,
            "Newbase: unable to create unknown class " + Fmi::to_string(classId));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
