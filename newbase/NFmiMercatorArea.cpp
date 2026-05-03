// ======================================================================
/*!
 * \file NFmiMercatorArea.cpp
 * \brief Implementation of class NFmiMercatorArea
 */
// ======================================================================
/*!
 * \class NFmiMercatorArea
 *
 * Provides for equidistance cylindrical projection. Maps geodetic
 * coordinates (in degrees) to rectangular mercator xy coordinates
 * (in meters) and vice versa.
 *
 * Projection is based on refs
 *  -# [2] Navigation Facility Reference Manual rev. 2.0 3/93
 *  -# [3] "An Album of Map Projections" USGS Paper 1453
 *
 * RECTANGULAR LOCAL COORDINATES are relative unitless cartesian
 * XY-coordinates. The range for these coordinates will be derived
 * directly from the size of the local rectangle used. The upper left
 * corner XY-point and the opposite lower right corner XY-point will
 * define the LOCAL RECTANGLE.
 *
 * RECTANGULAR WORLD COORDINATES are cartesian XY-coordinates measured
 * in meters on "true world terrain". The upper left corner XY-point
 * and the opposite lower right corner XY-point (in meters) will define
 * the WORLD RECTANGLE.
 *
 * GEODETIC COORDINATES are latitude-longitude coordinates in degrees.
 *
 * \note
 * The y axis lies along the central meridian (CURRENTLY ALWAYS ZERO),
 * y increasing north.
 * The x axis lies along the Equator and is perpendicular to the y axis,
 * increasing east.
 *
 * The example below shows how to convert the geodetic coordinates (deg)
 * into METRIC rectangular WORLD mercator coordinates (m)?
 *
 * \code
 *	double lon0 = ...;
 *	double lat0 = ...;
 *
 *	double lon1 = ...;
 *	double lat1 = ...;
 *
 *	NFmiPoint bottomLeftLatLon(lon0,lat0);
 *	NFmiPoint topRightLatLon(lon1,lat1);
 *
 *	NFmiMercatorArea mercator(bottomLeftLatLon, topRightLatLon);
 *	NFmiPoint mercatorWorldXY =  mercator.LatLonToWorldXY(bottomLeftLatLon);
 * \endcode
 *
 * To convert the METRIC mercator WORLD coordinates back into geodetic coordinates
 * one can use
 *
 * \code
 * NFmiPoint mercatorLatLon = mercator.ToLatLon(mercatorWorldXY);
 * \endcode
 *
 * To convert the geodetic coordinates into rectangular LOCAL mercator coordinates use
 *
 * \code
 *	double lon0 = ...;
 *	double lat0 = ...;
 *
 *	double lon1 = ...;
 *	double lat1 = ...;
 *
 *	NFmiPoint bottomLeftLatLon(lon0,lat0);
 *	NFmiPoint topRightLatLon(lon1,lat1);
 *
 *	NFmiMercatorArea mercator(bottomLeftLatLon, topRightLatLon);
 *	NFmiPoint mercatorXY =  mercator.ToXY(bottomLeftLatLon);
 * \endcode
 *
 * To convert the rectangular LOCAL mercator coordinates back into
 * geodetic coordinates use
 *
 * \code
 * NFmiPoint marcatorLatLon = marcator.ToLatLon(marcatorXY);
 * \endcode
 *
 *
 * \see NFmiLatLonArea and NFmiArea
 *
 */
// ======================================================================

#include "NFmiMercatorArea.h"
#include <macgyver/Hash.h>
#include <fmt/format.h>
#include <macgyver/Exception.h>
#include <limits>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiMercatorArea::NFmiMercatorArea()
    : NFmiArea(),
      itsBottomLeftLatLon(),
      itsTopRightLatLon(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiMercatorArea::NFmiMercatorArea(const NFmiPoint& theBottomLeftLatLon,
                                   const NFmiPoint& theTopRightLatLon,
                                   const NFmiPoint& theTopLeftXY,
                                   const NFmiPoint& theBottomRightXY,
                                   bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
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
 * Copy constructor
 *
 * \param theLatLonArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiMercatorArea::NFmiMercatorArea(const NFmiMercatorArea& theLatLonArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea* NFmiMercatorArea::Clone() const
{
  try
  {
    return new NFmiMercatorArea(*this);
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

void NFmiMercatorArea::Init(bool fKeepWorldRect)
{
  try
  {
    // Sopimus:
    // alueen oikean ylänurkan longitudin on oltava suurempi kuin vasemman alanurkan longitudin.
    if (itsTopRightLatLon.X() < itsBottomLeftLatLon.X())
    {
      // Em. sopimus ei toteudu ==> tehdään täysi kierros lännestä itään ==> oikean ylänurkan
      // longitudi on nyt suurempi kuin vasemman alanurkan longitudi
      itsTopRightLatLon += NFmiPoint(360., 0.);
    }

    if (!fKeepWorldRect)
      itsWorldRect =
          NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon),
                   LatLonToWorldXY(itsTopRightLatLon));  // 28.8.2001/Marko&Esa Lisätty laskuihin.

    itsXScaleFactor = Width() / itsWorldRect.Width();
    itsYScaleFactor = Height() / itsWorldRect.Height();

    NFmiArea::Init(fKeepWorldRect);

    const char* fmt = "+proj=merc +R={} +wktext +over +no_defs +type=crs";
    itsProjStr = fmt::format(fmt::runtime(fmt), kRearth);
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiMercatorArea::LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const
{
  try
  {
    // Computes the metric world xy coordinates from the input geodetic coordinates

    // Limit Y-values to prevent infinity

    double y = std::max(std::min(theLatLonPoint.Y(), 89.9999), -89.9999);

    return NFmiPoint(kRearth * FmiRad(theLatLonPoint.X()),
                     kRearth * log(tan(FmiRad(45. + 0.5 * y))));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiMercatorArea::ToXY(const NFmiPoint& theLatLonPoint) const
{
  try
  {
    // Transforms input geodetic coordinates (longitude,latitude) into local (relative)
    // coordinates on xy-plane.
    double xLocal, yLocal;

    // Transform input geodetic coordinates into world coordinates (meters) on xy-plane.
    NFmiPoint latlon(FixLongitude(theLatLonPoint.X()), theLatLonPoint.Y());
    NFmiPoint xyWorld(LatLonToWorldXY(latlon));

    // Finally, transform world xy-coordinates into local xy-coordinates
    xLocal = Left() + itsXScaleFactor * (xyWorld.X() - itsWorldRect.Left());
    yLocal = Top() + itsYScaleFactor * (itsWorldRect.Bottom() - xyWorld.Y());

    return NFmiPoint(xLocal, yLocal);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiMercatorArea::WorldXYToLatLon(const NFmiPoint& theXYPoint) const
{
  try
  {
    // Computes the geodetic coordinates (in degrees) from the input (metric) world xy coordinates

    double worldY = theXYPoint.Y();
    double lon = NFmiLongitude(FmiDeg(theXYPoint.X() / kRearth), PacificView()).Value();
    double lat = FmiDeg(2.0 * atan(exp(worldY / kRearth)) - 0.5 * kPii);

    return NFmiPoint(lon, lat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiMercatorArea::XYToWorldXY(const NFmiPoint& theXYPoint) const
{
  try
  {
    // Transform local xy-coordinates into world xy-coordinates (meters).
    double xWorld = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
    double yWorld = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;

    return NFmiPoint(xWorld, yWorld);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theWorldXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiMercatorArea::WorldXYToXY(const NFmiPoint& theWorldXYPoint) const
{
  try
  {
    double x = itsXScaleFactor * (theWorldXYPoint.X() - itsWorldRect.Left()) + Left();
    double y = Top() - itsYScaleFactor * (theWorldXYPoint.Y() - itsWorldRect.Bottom());
    return NFmiPoint(x, y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiMercatorArea::ToLatLon(const NFmiPoint& theXYPoint) const
{
  try
  {
    // Transforms input local xy-coordinates into geodetic coordinates
    // (longitude,latitude) on globe.
    return WorldXYToLatLon(XYToWorldXY(theXYPoint));
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

double NFmiMercatorArea::XScale() const
{
  try
  {
    return 1. / itsXScaleFactor;
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

double NFmiMercatorArea::YScale() const
{
  try
  {
    return 1. / itsYScaleFactor;
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

NFmiArea* NFmiMercatorArea::NewArea(const NFmiPoint& theBottomLeftLatLon,
                                    const NFmiPoint& theTopRightLatLon,
                                    bool allowPacificFix) const
{
  try
  {
    if (allowPacificFix)
    {
      PacificPointFixerData fixedPointData =
          NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
      return new NFmiMercatorArea(fixedPointData.itsBottomLeftLatlon,
                                  fixedPointData.itsTopRightLatlon,
                                  TopLeft(),
                                  BottomRight(),
                                  fixedPointData.fIsPacific);
    }
    else
      return new NFmiMercatorArea(
          theBottomLeftLatLon, theTopRightLatLon, TopLeft(), BottomRight(), PacificView());
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

std::ostream& NFmiMercatorArea::Write(std::ostream& file) const
{
  try
  {
    NFmiArea::Write(file);
    file << itsBottomLeftLatLon;
    file << itsTopRightLatLon;

    // Dummies to replace old removed variables
    file << "0 0\n0 0\n";

    file << itsXScaleFactor << " ";
    file << itsYScaleFactor << std::endl;
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

std::istream& NFmiMercatorArea::Read(std::istream& file)
{
  try
  {
    double dummy;

    NFmiArea::Read(file);
    file >> itsBottomLeftLatLon;
    file >> itsTopRightLatLon;
    PacificView(NFmiArea::IsPacificView(itsBottomLeftLatLon, itsTopRightLatLon));

    file >> dummy >> dummy >> dummy >> dummy;  // old removed variables

    file >> itsXScaleFactor;
    file >> itsYScaleFactor;

    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));

    Init();

    // 28.8.2001/Marko&Esa Tätä kutsuttaessa kaikki dataosat päivittyvät,
    // jolloin voidaan esim. editoida tiedostoissa olevia areoiden projektio
    // parametreja siten (tätä ei voitu tehdä ennen koska skaalauskertoimet eivät päivittyneet)

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const std::string NFmiMercatorArea::AreaStr() const
{
  try
  {
    std::ostringstream out;
    out << "mercator:" << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ','
        << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
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
 * PROJCS["FMI_Transverse_Mercator",
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  PROJECTION["Transverse_Mercator"],
 *  PARAMETER["latitude_of_origin",lat_0],
 *  PARAMETER["central_meridian",lon_0],
 *  PARAMETER["false_easting",x_0],
 *  PARAMETER["false_northing",y_0],
 *  UNIT["Metre",1.0]]
 *
 * Note: The default lat_0 and lon_0 values of zero are fine,
 * so we do not need to output them.
 */
// ----------------------------------------------------------------------

const std::string NFmiMercatorArea::WKT() const
{
  try
  {
    const char* fmt = R"(PROJCS["FMI_Transverse_Mercator",)"
                      R"(GEOGCS["FMI_Sphere",)"
                      R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",{:.0f},0]],)"
                      R"(PRIMEM["Greenwich",0],)"
                      R"(UNIT["Degree",0.0174532925199433]],)"
                      R"(PROJECTION["Transverse_Mercator"],)"
                      R"(UNIT["Metre",1.0]])";
    return fmt::format(fmt::runtime(fmt), kRearth);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison with an NFmiMercatorArea.
 *
 * \param theArea The other area being compared to
 * \return True, if the NFmiMercatorArea parts are equivalent
 * \todo Use static_cast instead of C-style cast
 */
// ----------------------------------------------------------------------

bool NFmiMercatorArea::operator==(const NFmiArea& theArea) const
{
  try
  {
    return *this == static_cast<const NFmiMercatorArea&>(theArea);
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
 * \param theArea The other area being compared to
 * \return True, if the areas are equivalent
 * \todo Investigate whether NFmiArea::operator== should also be called.
 */
// ----------------------------------------------------------------------

bool NFmiMercatorArea::operator==(const NFmiMercatorArea& theArea) const
{
  try
  {
    if ((itsBottomLeftLatLon == theArea.itsBottomLeftLatLon) &&
        (itsTopRightLatLon == theArea.itsTopRightLatLon) &&
        (itsXScaleFactor == theArea.itsXScaleFactor) &&
        (itsYScaleFactor == theArea.itsYScaleFactor) && (itsWorldRect == theArea.itsWorldRect))
      return true;

    return false;
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

std::size_t NFmiMercatorArea::HashValue() const
{
  try
  {
    std::size_t hash = NFmiArea::HashValue();
    Fmi::hash_combine(hash, itsBottomLeftLatLon.HashValue());
    Fmi::hash_combine(hash, itsTopRightLatLon.HashValue());
    Fmi::hash_combine(hash, Fmi::hash_value(itsXScaleFactor));
    Fmi::hash_combine(hash, Fmi::hash_value(itsYScaleFactor));
    Fmi::hash_combine(hash, itsWorldRect.HashValue());
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
