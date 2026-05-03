// ======================================================================
/*!
 * \file NFmiEquidistArea.cpp
 * \brief Implementation of class NFmiEquidistArea
 */
// ======================================================================
/*!
 * \class NFmiEquidistArea
 *
 * Equidist projection. Maps geodetic coordinates (in degrees) to rectangular
 * equidist xy coordinates (in meters) and vice versa.
 *
 *	Projection is based on refs
 *
 *	-# Peter Richardus, Ron K.Adler: "Map Projections", North-Holland, 2nd printing 1974.
 *  -# Navigation Facility Reference Manual rev. 2.0 3/93
 *  -# "An Album of Map Projections" USGS Paper 1453
 *
 * Note: the y axis lies along the central meridian (lon0), y increasing north;
 * the x axis is perpendicular to the y axis at (lon0,lat0), x increasing east.
 *
 * \see NFmiAzimuthalArea, NFmiGnomonicArea and NFmiStereographicArea
 *
 *
 * Currently there exists no "true" latitude for equidistant projection.
 * This is because the xy-projection plane is assumed to be tangential to
 * the Earth surface in any case, so true latitude is always 90 degrees.
 *
 * How to convert the geodetic coordinates into rectangular LOCAL polar
 * equidist coordinates ?
 *
 * \code
 *	CRect rekti;
 *	GetClientRect(rekti);
 *
 *	double lon0,lat0,lon1,lat1,centralLon,centralLat;
 *
 *	lon0 = ...;
 *	lat0 = ...;
 *
 *	lon1 = ...;
 *	lat1 = ...;
 *
 *  True longitude for Helsinki.
 *	centralLon = 25.0;
 *
 *  True latitude for POLAR equidist projection.
 *	centralLat = 90.0;
 *
 *
 *	NFmiPoint bottomLeftLatLon2(lon0,lat0);
 *	NFmiPoint topRightLatLon2(lon1,lat1);
 *
 *	NFmiEquidistArea equidist(bottomLeftLatLon2
 *		                     ,topRightLatLon2
 *							 ,centralLon
 *							 ,NFmiPoint(0.,0.)
 *							 ,NFmiPoint(rekti.right,rekti.bottom)
 *							 ,centralLat);
 *
 *	NFmiPoint equidistXY =  equidist.ToXY(bottomLeftLatLon2);
 * \endcode
 *
 * How to convert the rectangular LOCAL equidist coordinates back
 * into geodetic coordinates ?
 *
 * \code
 * NFmiPoint equidistLatLon = equidist.ToLatLon(equidistXY);
 * \endcode
 */
// ======================================================================

#include "NFmiEquidistArea.h"
#include <macgyver/Exception.h>
#include <algorithm>

#include <fmt/format.h>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theEquidistArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiEquidistArea::NFmiEquidistArea(const NFmiEquidistArea &theEquidistArea)

    = default;

/*!
 * Constructor
 *
 * \param theCenterLatLon Undocumented
 * \param theRadialRangeInMeters Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
NFmiEquidistArea::NFmiEquidistArea(double theRadialRangeInMeters,
                                   const NFmiPoint &theCenterLatLon,
                                   const NFmiPoint &theTopLeftXY,
                                   const NFmiPoint &theBottomRightXY)
    : NFmiAzimuthalArea(theRadialRangeInMeters, theCenterLatLon, theTopLeftXY, theBottomRightXY)
{
  try
  {
    // Muodostaa projektioalueen rajaamalla "world-xy"-tasossa 'theRadialRangeInMeters'-säteiselle
    // ympyrälle
    // "bounding-boxin", jonka keskipisteenä on maantiet. piste 'theCenterLatLon'

    itsTrueLatScaleFactor =
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);

    NFmiPoint centerWorldXY = LatLonToWorldXY(NFmiPoint(theCenterLatLon.X(), theCenterLatLon.Y()));
    itsBottomLeftWorldXY = NFmiPoint(centerWorldXY.X() - theRadialRangeInMeters,
                                     centerWorldXY.Y() - theRadialRangeInMeters);
    itsWorldRect = NFmiRect(
        itsBottomLeftWorldXY,
        itsBottomLeftWorldXY + NFmiPoint(2 * theRadialRangeInMeters, 2 * theRadialRangeInMeters));

    Init(true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theOrientation Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCentralLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiEquidistArea::NFmiEquidistArea(const NFmiPoint &theBottomLeftLatLon,
                                   const NFmiPoint &theTopRightLatLon,
                                   const double theOrientation,
                                   const NFmiPoint &theTopLeftXY,
                                   const NFmiPoint &theBottomRightXY,
                                   const double theCentralLatitude,
                                   bool usePacificView)
    : NFmiAzimuthalArea(theBottomLeftLatLon,
                        theTopRightLatLon,
                        theOrientation,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCentralLatitude,
                        90.0,
                        usePacificView)
{
  try
  {
    Init();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theWidthInMeters Undocumented
 * \param theHeightInMeters Undocumented
 * \param theOrientation Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCentralLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiEquidistArea::NFmiEquidistArea(const NFmiPoint &theBottomLeftLatLon,
                                   const double theWidthInMeters,
                                   const double theHeightInMeters,
                                   const double theOrientation,
                                   const NFmiPoint &theTopLeftXY,
                                   const NFmiPoint &theBottomRightXY,
                                   const double theCentralLatitude)
    : NFmiAzimuthalArea(theBottomLeftLatLon,
                        theOrientation,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCentralLatitude,
                        90.0)
{
  try
  {
    itsTrueLatScaleFactor =
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);

    itsBottomLeftWorldXY =
        LatLonToWorldXY(NFmiPoint(theBottomLeftLatLon.X(), theBottomLeftLatLon.Y()));
    itsWorldRect = NFmiRect(itsBottomLeftWorldXY,
                            itsBottomLeftWorldXY + NFmiPoint(theWidthInMeters, theHeightInMeters));

    Init(true);

    // 28.8.2001/Marko&Esa itsWorldRect on laskettu sellaisilla argumenteilla tässä,
    // mitkä eivät ole dataosia, joten sitä ei saa laskea Init:issä uudestaan
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theRadialRange Undocumented
 * \param theOrientation Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCentralLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiEquidistArea::NFmiEquidistArea(const double theRadialRange,
                                   const double theOrientation,
                                   const NFmiPoint &theTopLeftXY,
                                   const NFmiPoint &theBottomRightXY,
                                   const double theCentralLatitude)
    : NFmiAzimuthalArea(
          theRadialRange, theOrientation, theTopLeftXY, theBottomRightXY, theCentralLatitude, 90.0)
{
  try
  {
    // Purpose: to create a square bounding the circle of radius theRadialRange

    itsTrueLatScaleFactor =
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2000/EL

    itsWorldRect = NFmiRect(NFmiPoint(-itsRadialRange, -itsRadialRange),
                            NFmiPoint(itsRadialRange, itsRadialRange));

    Init(true);

    // 28.8.2001/Marko&Esa itsWorldRect on laskettu sellaisilla argumenteilla
    // tässä, mitkä eivät ole dataosia, joten sitä ei saa laskea Init:issä uudestaan
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param delta Undocumented
 * \return Undocumented
 *
 * \todo Should handle the case when acosDelta =~ 0 (almost equitorial)
 */
// ----------------------------------------------------------------------

double NFmiEquidistArea::K(const double delta) const
{
  try
  {
    double d = std::max(-1.0, std::min(delta, 1.0));

    // See ref [3] p. 228
    if (sin(d) == 0.0 || d == 1)
      return kRearth;

    // return kRearth*d/sin(delta); // 27.7.98/EL Replaced this one ...
    // ... with these ones

    double acosDelta = acos(d);
    if (acosDelta == 0)
      return kRearth;

    return kRearth * acosDelta / sin(acosDelta);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param xyDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiEquidistArea::CalcDelta(const double xyDistance) const
{
  try
  {
    // Calculates the delta angle for azimuthal equidistant projection.
    // See details in ref. [1] pp. 67-68; arc delta = arc OP = OP' = xyDistance
    return xyDistance / kRearth;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always 0.0
 */
// ----------------------------------------------------------------------

double NFmiEquidistArea::DistanceFromPerspectivePointToCenterOfEarth() const
{
  try
  {
    return 0.0;  // This is a non-perspective projection
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 * \todo Should return an std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiEquidistArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                    const NFmiPoint &theTopRightLatLon,
                                    bool allowPacificFix) const
{
  try
  {
    if (allowPacificFix)
    {
      PacificPointFixerData fixedPointData =
          NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
      return new NFmiEquidistArea(fixedPointData.itsBottomLeftLatlon,
                                  fixedPointData.itsTopRightLatlon,
                                  itsCentralLongitude.Value(),
                                  TopLeft(),
                                  BottomRight(),
                                  itsCentralLatitude.Value(),
                                  fixedPointData.fIsPacific);
    }

    return new NFmiEquidistArea(theBottomLeftLatLon,
                                theTopRightLatLon,
                                itsCentralLongitude.Value(),
                                TopLeft(),
                                BottomRight(),
                                itsCentralLatitude.Value(),
                                PacificView());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiEquidistArea::Clone() const
{
  try
  {
    return new NFmiEquidistArea(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theArea The other object being copied
 * \return The object assigned to
 * \todo Since NFmiAzimulthalArea::operator= is unknown, should protect
 *       from self assignment
 */
// ----------------------------------------------------------------------

NFmiEquidistArea &NFmiEquidistArea::operator=(const NFmiEquidistArea &theArea) = default;

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The object being compared to
 * \return True, if the objects are equivalent
 */
// ----------------------------------------------------------------------

bool NFmiEquidistArea::operator==(const NFmiEquidistArea &theArea) const
{
  try
  {
    return NFmiAzimuthalArea::operator==(static_cast<const NFmiAzimuthalArea &>(theArea));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The object being compared to
 * \return True, if the objects are not equivalent
 */
// ----------------------------------------------------------------------

bool NFmiEquidistArea::operator!=(const NFmiEquidistArea &theArea) const
{
  try
  {
    return !(*this == theArea);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The object being compared to
 * \return True, if the objects are equivalent
 */
// ----------------------------------------------------------------------

bool NFmiEquidistArea::operator==(const NFmiArea &theArea) const
{
  try
  {
    return NFmiAzimuthalArea::operator==(theArea);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The object being compared to
 * \return True, if the objects are not equivalent
 */
// ----------------------------------------------------------------------

bool NFmiEquidistArea::operator!=(const NFmiArea &theArea) const
{
  try
  {
    return !(*this == theArea);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiEquidistArea::Init(bool fKeepWorldRect)
{
  try
  {
    if (!fKeepWorldRect)
    {
      itsTrueLatScaleFactor =
          (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
          (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);

      itsWorldRect =
          NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));
    }

    NFmiAzimuthalArea::Init(fKeepWorldRect);

    const char *fmt =
        "+proj=aeqd +lat_0={} +lon_0={} +x_0=0 +y_0=0 +R={} +units=m +wktext +no_defs +type=crs";

    itsProjStr = fmt::format(fmt::runtime(fmt), CentralLatitude(), CentralLongitude(), kRearth);
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const std::string NFmiEquidistArea::AreaStr() const
{
  try
  {
    // Todo: use cppformat instead
    std::ostringstream out;
    out << "equidist," << CentralLongitude() << ',' << CentralLatitude() << ':'
        << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ',' << TopRightLatLon().X()
        << ',' << TopRightLatLon().Y();
    return out.str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return Well Known Text representation of the GCS
 * PROJCS["FMI_Azimuthal_Equidistant",
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  PROJECTION["Azimuthal_Equidistant"],
 *  PARAMETER["latitude_of_center",lat_0],
 *  PARAMETER["longitude_of_center",lon_0],
 *  PARAMETER["false_easting",x_0],
 *  PARAMETER["false_northing",y_0],
 *  UNIT["Metre",1.0]]
 */
// ----------------------------------------------------------------------

const std::string NFmiEquidistArea::WKT() const
{
  try
  {
    const char *fmt = R"(PROJCS["FMI_Azimuthal_Equidistant",)"
                      R"(GEOGCS["FMI_Sphere",)"
                      R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",{:.0f},0]],)"
                      R"(PRIMEM["Greenwich",0],)"
                      R"(UNIT["Degree",0.0174532925199433]],)"
                      R"(PROJECTION["Azimuthal_Equidistant"],)"
                      R"(PARAMETER["latitude_of_center",{}],)"
                      R"(PARAMETER["longitude_of_center",{}],)"
                      R"(UNIT["Metre",1.0]])";
    return fmt::format(fmt::runtime(fmt), kRearth, itsCentralLatitude.Value(), itsCentralLongitude.Value());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Hash value
 */
// ----------------------------------------------------------------------

std::size_t NFmiEquidistArea::HashValue() const
{
  try
  {
    std::size_t hash = NFmiAzimuthalArea::HashValue();
    // no private members
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
