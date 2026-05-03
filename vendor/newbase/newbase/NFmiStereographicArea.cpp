// ======================================================================
/*!
 * \file NFmiStereographicArea.cpp
 * \brief Implementation of class NFmiStereographicArea
 */
// ======================================================================
/*!
 * \class NFmiStereographicArea
 *
 * Stereographic projection. Maps geodetic coordinates (in degrees) to rectangular
 * stereographic xy coordinates (in meters) and vice versa.
 *
 *	Projection is based on refs
 *
 *    -# Peter Richardus, Ron K.Adler: "Map Projections", North-Holland, 2nd printing 1974.
 *    -# Navigation Facility Reference Manual rev. 2.0 3/93
 *    -# "An Album of Map Projections" USGS Paper 1453
 *
 * \see NFmiGnomonicArea and NFmiEquidistArea
 *
 *
 * How to convert the geodetic coordinates into rectangular LOCAL polar stereographic coordinates ?
 *
 * \code
 *	CRect rekti;
 *	GetClientRect(rekti);
 *
 *	double lon0,lat0,lon1,lat1,centerLon,centerLat,trueLat;
 *
 *	lon0 = ...;
 *	lat0 = ...;
 *
 *	lon1 = ...;
 *	lat1 = ...;
 *
 * // Central longitude for Helsinki.
 *	centerLon = 25.0;
 *
 * // Central latitude for POLAR stereographic projection.
 *	centerLat = 90.0;
 *
 * // True latitude for Helsinki
 *	trueLat = 60.0;
 *
 *
 *	NFmiPoint bottomLeftLatLon2(lon0,lat0);
 *	NFmiPoint topRightLatLon2(lon1,lat1);
 *
 *	NFmistereographicArea stereo(bottomLeftLatLon2,
 *		                         topRightLatLon2,
 *								 centerLon,
 *							     NFmiPoint(0.,0.),
 *								 NFmiPoint(rekti.right,rekti.bottom),
 *								 centralLat,
 *								 trueLat);
 *
 *	NFmiPoint stereoXY =  stereo.ToXY(bottomLeftLatLon2);
 * \endcode
 *
 * How to convert the rectangular LOCAL stereographic coordinates back into geodetic coordinates ?
 *
 * \code
 * NFmiPoint stereoLatLon = stereo.ToLatLon(stereoXY);
 * \endcode
 */
// ======================================================================

#include "NFmiStereographicArea.h"
#include <fmt/format.h>
#include <macgyver/Exception.h>
#include <cmath>

using namespace std;

namespace
{
const char *proj_fmt =
    "+proj=stere +lat_0={} +lat_ts={} +lon_0={} +R={} +units=m +wktext +no_defs +type=crs";
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiStereographicArea::NFmiStereographicArea() = default;
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theStereographicArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiStereographicArea::NFmiStereographicArea(const NFmiStereographicArea &theStereographicArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theCentralLongitude Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCenterLatitude Undocumented
 * \param theTrueLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiStereographicArea::NFmiStereographicArea(const NFmiPoint &theBottomLeftLatLon,
                                             const NFmiPoint &theTopRightLatLon,
                                             const double theCentralLongitude,
                                             const NFmiPoint &theTopLeftXY,
                                             const NFmiPoint &theBottomRightXY,
                                             const double theCenterLatitude,
                                             const double theTrueLatitude,
                                             bool usePacificView)
    : NFmiAzimuthalArea(theBottomLeftLatLon,
                        theTopRightLatLon,
                        theCentralLongitude,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCenterLatitude,
                        theTrueLatitude,
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

/*!
 * Constructor
 *
 * \param theCenterLatLon Undocumented
 * \param theRadialRangeInMeters Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
NFmiStereographicArea::NFmiStereographicArea(double theRadialRangeInMeters,
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
 * \param theWidthInMeters Undocumented
 * \param theHeightInMeters Undocumented
 * \param theCentralLongitude Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCentralLatitude Undocumented
 * \param theTrueLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiStereographicArea::NFmiStereographicArea(const NFmiPoint &theBottomLeftLatLon,
                                             const double theWidthInMeters,
                                             const double theHeightInMeters,
                                             const double theCentralLongitude,
                                             const NFmiPoint &theTopLeftXY,
                                             const NFmiPoint &theBottomRightXY,
                                             const double theCentralLatitude,
                                             const double theTrueLatitude)
    : NFmiAzimuthalArea(theBottomLeftLatLon,
                        theCentralLongitude,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCentralLatitude,
                        theTrueLatitude)
{
  try
  {
    itsTrueLatScaleFactor =
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2001/EL

    itsBottomLeftWorldXY = LatLonToWorldXY(theBottomLeftLatLon);
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
 * \param theCentralLongitude Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCentralLatitude Undocumented
 * \param theTrueLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiStereographicArea::NFmiStereographicArea(const double theRadialRange,
                                             const double theCentralLongitude,
                                             const NFmiPoint &theTopLeftXY,
                                             const NFmiPoint &theBottomRightXY,
                                             const double theCentralLatitude,
                                             const double theTrueLatitude)
    : NFmiAzimuthalArea(theRadialRange,
                        theCentralLongitude,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCentralLatitude,
                        theTrueLatitude)
{
  try
  {
    // Purpose: to create a square bounding the circle of radius theRadialRange

    itsTrueLatScaleFactor =
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2001/EL

    itsWorldRect = NFmiRect(NFmiPoint(-itsRadialRange, -itsRadialRange),
                            NFmiPoint(itsRadialRange, itsRadialRange));
    NFmiAzimuthalArea::Init(true);

    itsProjStr = fmt::format(fmt::runtime(proj_fmt),
                             itsCentralLatitude.Value(),
                             itsTrueLatitude.Value(),
                             itsCentralLongitude.Value(),
                             kRearth);
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);

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
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStereographicArea::Init(bool fKeepWorldRect)
{
  try
  {
    if (!fKeepWorldRect)
    {
      itsTrueLatScaleFactor =
          (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
          (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2001/EL

      itsWorldRect =
          NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));
    }
    NFmiAzimuthalArea::Init();

    itsProjStr = fmt::format(fmt::runtime(proj_fmt),
                             itsCentralLatitude.Value(),
                             itsTrueLatitude.Value(),
                             itsCentralLongitude.Value(),
                             kRearth);
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param delta Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

double NFmiStereographicArea::K(const double delta) const
{
  try
  {
    double D;

    D = DistanceFromPerspectivePointToCenterOfEarth();
    if ((D + (kRearth * delta)) != 0)
      return kRearth * (D + kRearth) / (D + (kRearth * delta));
    else
      return kFloatMissing;
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

double NFmiStereographicArea::CalcDelta(const double xyDistance) const
{
  try
  {
    // Calculates the delta angle for stereographic projection.
    // See details in ref. [2] p. 13.

    return 2.0 * atan(xyDistance / (2.0 * kRearth));
    // 11.5.98/EL: delta is always computed for tangential plane only --> itsTrueLatitude.Sin() == 1
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiStereographicArea::DistanceFromPerspectivePointToCenterOfEarth() const
{
  try
  {
    // Distance (in world-coordinate meters) for stereographic projection.
    // See details in ref. [2] p. 13.

    return kRearth;
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

NFmiArea *NFmiStereographicArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                         const NFmiPoint &theTopRightLatLon,
                                         bool allowPacificFix) const
{
  try
  {
    if (allowPacificFix)
    {
      PacificPointFixerData fixedPointData =
          NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
      return new NFmiStereographicArea(fixedPointData.itsBottomLeftLatlon,
                                       fixedPointData.itsTopRightLatlon,
                                       itsCentralLongitude.Value(),
                                       TopLeft(),
                                       BottomRight(),
                                       itsCentralLatitude.Value(),
                                       itsTrueLatitude.Value(),
                                       fixedPointData.fIsPacific);
    }
    else
      return new NFmiStereographicArea(theBottomLeftLatLon,
                                       theTopRightLatLon,
                                       itsCentralLongitude.Value(),
                                       TopLeft(),
                                       BottomRight(),
                                       itsCentralLatitude.Value(),
                                       itsTrueLatitude.Value(),
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

NFmiArea *NFmiStereographicArea::Clone() const
{
  try
  {
    return new NFmiStereographicArea(*this);
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
 * \todo Since NFmiAzimuthalArea::operator= is unknown, must protect
 *       from self assignment
 */
// ----------------------------------------------------------------------

NFmiStereographicArea &NFmiStereographicArea::operator=(const NFmiStereographicArea &theArea) =
    default;

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiStereographicArea::operator==(const NFmiStereographicArea &theArea) const
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
 * \param theArea The other object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiStereographicArea::operator!=(const NFmiStereographicArea &theArea) const
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
 * \param theArea The other object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiStereographicArea::operator==(const NFmiArea &theArea) const
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
 * \param theArea The other object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiStereographicArea::operator!=(const NFmiArea &theArea) const
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
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiStereographicArea::Write(std::ostream &file) const
{
  try
  {
    NFmiAzimuthalArea::Write(file);
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiStereographicArea::Read(std::istream &file)
{
  try
  {
    NFmiAzimuthalArea::Read(file);
    itsProjStr = fmt::format(fmt::runtime(proj_fmt),
                             itsCentralLatitude.Value(),
                             itsTrueLatitude.Value(),
                             itsCentralLongitude.Value(),
                             kRearth);
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiArea *NFmiStereographicArea::CreateNewArea(const NFmiRect &theRect) const
{
  try
  {
    NFmiPoint bottomLeft(ToLatLon(theRect.BottomLeft()));
    NFmiPoint topRight(ToLatLon(theRect.TopRight()));
    NFmiArea *area = new NFmiStereographicArea(
        bottomLeft, topRight, itsCentralLongitude.Value(), TopLeft(), BottomRight());
    return area;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const std::string NFmiStereographicArea::AreaStr() const
{
  try
  {
    std::ostringstream out;
    out << "stereographic," << CentralLongitude() << ',' << CentralLatitude() << ','
        << itsTrueLatitude.Value() << ':' << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y()
        << ',' << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
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
 *
 * Geneerinen:
 *
 * PROJCS["FMI_Stereographic",
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  PROJECTION["Stereographic"],
 *  PARAMETER["latitude_of_origin",lat_0],
 *  PARAMETER["central_meridian",lon_0],
 *  PARAMETER["false_easting",x_0],
 *  PARAMETER["false_northing",y_0],
 *  UNIT["Metre",1.0]]
 *
 * Polaaristereograafinen:
 *
 * PROJCS["FMI_Polar_Stereographic",
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  PROJECTION["Polar_Stereographic"],
 *  PARAMETER["latitude_of_origin",lat_ts],
 *  PARAMETER["central_meridian",lon],
 *  PARAMETER["false_easting",x_0],
 *  PARAMETER["false_northing",y_0],
 *  UNIT["Metre",1.0]]
 */
// ----------------------------------------------------------------------

const std::string NFmiStereographicArea::WKT() const
{
  try
  {
    if (itsCentralLatitude.Value() != 90)
    {
      const char *fmt = R"(PROJCS["FMI_Stereographic",)"
                        R"(GEOGCS["FMI_Sphere",)"
                        R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",{:.0f},0]],)"
                        R"(PRIMEM["Greenwich",0],)"
                        R"(UNIT["Degree",0.0174532925199433]],)"
                        R"(PROJECTION["Stereographic"],)"
                        R"(PARAMETER["latitude_of_origin",{}],)"
                        R"(PARAMETER["central_meridian",{}],)"
                        R"(UNIT["Metre",1.0]])";

      return fmt::format(fmt::runtime(fmt), kRearth, itsCentralLatitude.Value(), itsCentralLongitude.Value());
    }

    const char *fmt = R"(PROJCS["FMI_Polar_Stereographic",)"
                      R"(GEOGCS["FMI_Sphere",)"
                      R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",{:.0f},0]],)"
                      R"(PRIMEM["Greenwich",0],)"
                      R"(UNIT["Degree",0.0174532925199433]],)"
                      R"(PROJECTION["Polar_Stereographic"],)"
                      R"(PARAMETER["latitude_of_origin",{}],)"
                      R"(PARAMETER["central_meridian",{}],)"
                      R"(UNIT["Metre",1.0]])";

    return fmt::format(fmt::runtime(fmt), kRearth, itsTrueLatitude.Value(), itsCentralLongitude.Value());
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

std::size_t NFmiStereographicArea::HashValue() const
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
