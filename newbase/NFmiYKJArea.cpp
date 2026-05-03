// ======================================================================
/*!
 * \file NFmiYKJArea.cpp
 * \brief Implementation of class NFmiYKJArea
 */
// ======================================================================
/*!
 * \class NFmiYKJArea
 *
 * Implements transformation from rectangular YKJ (YhtenäisKoordinaattiJärjestelmä)
 * coordinates into geodetic coordinates and vice versa for the Finnish National Grid
 * Coordinate System KKJ (KartastoKoordinaattiJärjestelmä).
 *
 * The rectangular YKJ coordinates are aligned to KKJ zone 3 only (central meridian 27
 * degrees) with easting and northing in meters.
 *
 * Projection used is Gauss-Krueger projection with Hayford's International Reference ellipsoid.
 *
 * RECTANGULAR LOCAL COORDINATES are relative unitless cartesian XY-coordinates.
 * The range for these coordinates will be derived directly from the size of the
 * local rectangle used. The upper left corner XY-point and the opposite lower right
 * corner XY-point will define the LOCAL RECTANGLE.
 *
 * RECTANGULAR WORLD COORDINATES are cartesian XY-coordinates measured in meters
 * on "true world terrain". The upper left corner XY-point and the opposite lower right
 * corner XY-point (in meters) will define the WORLD RECTANGLE.
 *
 * GEODETIC COORDINATES are latitude-longitude coordinates in degrees.
 *
 * \see NFmiKKJArea and NFmiPKJArea
 *
 *
 * INPUT PARAMETERS/ARGUMENT LISTS
 *
 *	theBottomLeftLatLon = lower left corner point of the rectangle in GEODETIC
 *								 latitude-longitude coordinates
 *
 * theTopRightLatLon =	 upper right corner point of the rectangle in GEODETIC
 *								 latitude-longitude coordinates for
 *the
 *rectangle
 *
 * theTopLeftXY = upper left corner of the rectangle in rectangular LOCAL coordinates
 *
 * theBottomRightXY = lower right corner of the rectangle in rectangular LOCAL coordinates
 *
 * theYKJArea = YKJArea object to be copied
 *
 *
 * The first constructor is meant to be used when both of the geodetic corner points
 * are known, whereas the second constructor is used when the upper right geodetic
 * corner point is unknown but the metric dimensions of the world rectangle are known.
 *
 * EXAMPLES
 *
 * How to convert the rectangular WORLD YKJ coordinates (in meters) into geodetic
 * coordinates ?
 *
 * \code
 *	NFmiPoint bottomLeftLatLon = ....;
 * 	NFmiYKJArea yykoojii(bottomLeftLatLon, 512000., 1300000.);
 *  NFmiPoint YKJLatLon, YKJxy;
 * \endcode
 *
 * Finally, this is how to convert rectangular WORLD YKJ coordinates into
 * geodetic coordinates:
 *
 * \code
 * YKJLatLon = yykoojii.WorldXYToLatLon(YKJxy);
 * \endcode
 *
 *
 * How to convert the geodetic coordinates back into rectangular WORLD YKJ
 * coordinates (in meters) ?
 *
 * \code
 * YKJxy = yykoojii.LatLonToWorldXY(YKJLatLon);
 * \endcode
 *
 * How to convert the rectangular LOCAL YKJ coordinates into geodetic coordinates ?
 *
 * \code
 * NFmiPoint YKJxy = ...;
 * YKJLatLon = yykoojii.ToLatLon(YKJxy);
 * \endcode
 *
 * How to convert the geodetic coordinates back into rectangular LOCAL
 * YKJ coordinates ?
 *
 * \code
 * YKJxy = yykoojii.ToXY(YKJLatLon);
 * \endcode
 *
 */
// ======================================================================

#include "NFmiYKJArea.h"
#include <fmt/format.h>
#include <macgyver/Exception.h>
#include <cmath>
#include <cstdlib>
#include <string>

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiYKJArea::NFmiYKJArea() : NFmiKKJArea() {}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theYKJArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiYKJArea::NFmiYKJArea(const NFmiYKJArea &theYKJArea) = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiYKJArea::NFmiYKJArea(const NFmiPoint &theBottomLeftLatLon,
                         const NFmiPoint &theTopRightLatLon,
                         const NFmiPoint &theTopLeftXY,
                         const NFmiPoint &theBottomRightXY,
                         bool usePacificView)
    : NFmiKKJArea(
          theBottomLeftLatLon, theTopRightLatLon, theTopLeftXY, theBottomRightXY, usePacificView)
{
  try
  {
    itsWorldRect =
        NFmiRect(CornerWorldXY(NFmiPoint(theBottomLeftLatLon.X(), theBottomLeftLatLon.Y())),
                 CornerWorldXY(NFmiPoint(theTopRightLatLon.X(), theTopRightLatLon.Y())));
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
 * \param theRadialRangeInMeters Undocumented
 * \param theCenterLatLon Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiYKJArea::NFmiYKJArea(double theRadialRangeInMeters,
                         const NFmiPoint &theCenterLatLon,
                         const NFmiPoint &theTopLeftXY,
                         const NFmiPoint &theBottomRightXY)
    : NFmiKKJArea(theRadialRangeInMeters, theCenterLatLon, theTopLeftXY, theBottomRightXY)
{
  try
  {
    // Muodostaa projektioalueen rajaamalla "world-xy"-tasossa 'theRadialRangeInMeters'-säteiselle
    // ympyrälle
    // "bounding-boxin", jonka keskipisteenä on maantiet. piste 'theCenterLatLon'

    NFmiPoint centerWorldXY = LatLonToWorldXY(NFmiPoint(theCenterLatLon.X(), theCenterLatLon.Y()));
    NFmiPoint bottomLeftWorldXY = NFmiPoint(centerWorldXY.X() - theRadialRangeInMeters,
                                            centerWorldXY.Y() - theRadialRangeInMeters);
    itsWorldRect = NFmiRect(
        bottomLeftWorldXY,
        bottomLeftWorldXY + NFmiPoint(2 * theRadialRangeInMeters, 2 * theRadialRangeInMeters));

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
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiYKJArea::NFmiYKJArea(const NFmiPoint &theBottomLeftLatLon,
                         const double theWidthInMeters,
                         const double theHeightInMeters,
                         const NFmiPoint &theTopLeftXY,
                         const NFmiPoint &theBottomRightXY)
    : NFmiKKJArea(theBottomLeftLatLon, theTopLeftXY, theBottomRightXY)
{
  try
  {
    itsWorldRect =
        NFmiRect(CornerWorldXY(NFmiPoint(theBottomLeftLatLon.X(), theBottomLeftLatLon.Y())),
                 CornerWorldXY(NFmiPoint(theBottomLeftLatLon.X(), theBottomLeftLatLon.Y())) +
                     NFmiPoint(theWidthInMeters, theHeightInMeters));

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
 * \param theBottomLeftWorldXY Undocumented (in meters)
 * \param theTopRightWorldXY Undocumented (in meters)
 * \param isWorldXYInMeters Undocumented, unused
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiYKJArea::NFmiYKJArea(const NFmiPoint &theBottomLeftWorldXY,
                         const NFmiPoint &theTopRightWorldXY,
                         const bool & /* isWorldXYInMeters */,
                         const NFmiPoint &theTopLeftXY,
                         const NFmiPoint &theBottomRightXY)
    : NFmiKKJArea(theTopLeftXY, theBottomRightXY)
{
  try
  {
    itsWorldRect = NFmiRect(theBottomLeftWorldXY, theTopRightWorldXY);

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
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiYKJArea::Init(bool fKeepWorldRect)
{
  try
  {
    if (!fKeepWorldRect)
      itsWorldRect = NFmiRect(CornerWorldXY(itsBottomLeftLatLon), CornerWorldXY(itsTopRightLatLon));

    itsXScaleFactor = Width() / itsWorldRect.Width();
    itsYScaleFactor = Height() / itsWorldRect.Height();

    itsTopRightLatLon = TopRightLatLon();
    itsBottomLeftLatLon = BottomLeftLatLon();

    NFmiKKJArea::Init(fKeepWorldRect);

    itsProjStr = "EPSG:2393";
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(2393);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the rectangular world point in meters corresponding the input
 * geodetic latitude-longitude point theLatLonPoint.
 *
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiYKJArea::CornerWorldXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    // Calculate world coordinates (meters) for YKJ rectangle corner point.
    return LatLonToWorldXY(theLatLonPoint);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the KKJ zone number for YKJ. By definition, zone number for YKJ is
 * always 3.
 *
 * \param lon Undocumented
 * \result The zone number
 */
// ----------------------------------------------------------------------

int NFmiYKJArea::ZoneNumberByLongitude(const double /* lon */) const
{
  try
  {
    // For YKJ, the zone number is a constant.
    return 3;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the KKJ zone number for YKJ. By definition, zone number for YKJ is
 * always 3.
 *
 * \param easting Undocumented, unused
 * \result The zone number
 */
// ----------------------------------------------------------------------

int NFmiYKJArea::ZoneNumberByEasting(const double /* easting */) const
{
  try
  {
    // For YKJ, the zone number is a constant.
    return 3;
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
 * \result Undocumented
 * \todo Return std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiYKJArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                               const NFmiPoint &theTopRightLatLon,
                               bool allowPacificFix) const
{
  try
  {
    if (allowPacificFix)
    {
      PacificPointFixerData fixedPointData =
          NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
      return new NFmiYKJArea(fixedPointData.itsBottomLeftLatlon,
                             fixedPointData.itsTopRightLatlon,
                             TopLeft(),
                             BottomRight(),
                             fixedPointData.fIsPacific);
    }

    return new NFmiYKJArea(
        theBottomLeftLatLon, theTopRightLatLon, TopLeft(), BottomRight(), PacificView());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \result Undocumented
 * \todo Return std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiYKJArea::Clone() const
{
  try
  {
    return new NFmiYKJArea(*this);
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
 * \todo Since NFmiKKJArea is unknown, self assignment should be protected
 */
// ----------------------------------------------------------------------

NFmiYKJArea &NFmiYKJArea::operator=(const NFmiYKJArea &theArea) = default;

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are equivalent
 */
// ----------------------------------------------------------------------

bool NFmiYKJArea::operator==(const NFmiYKJArea &theArea) const
{
  try
  {
    return NFmiKKJArea::operator==(static_cast<const NFmiKKJArea &>(theArea));
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

bool NFmiYKJArea::operator!=(const NFmiYKJArea &theArea) const
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
 * \return True, if the objects are equivalent
 */
// ----------------------------------------------------------------------

bool NFmiYKJArea::operator==(const NFmiArea &theArea) const
{
  try
  {
    return NFmiKKJArea::operator==(static_cast<const NFmiArea &>(theArea));
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

bool NFmiYKJArea::operator!=(const NFmiArea &theArea) const
{
  try
  {
    return NFmiKKJArea::operator!=(static_cast<const NFmiArea &>(theArea));
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

std::ostream &NFmiYKJArea::Write(std::ostream &file) const
{
  try
  {
    NFmiKKJArea::Write(file);
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
 * \return file The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiYKJArea::Read(std::istream &file)
{
  try
  {
    NFmiKKJArea::Read(file);
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const std::string NFmiYKJArea::AreaStr() const
{
  try
  {
    std::ostringstream out;
    out << "ykj:" << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ','
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
 * PROJCS["KKJ / Finland Uniform Coordinate System",
 *  GEOGCS["KKJ",
 *         DATUM["Kartastokoordinaattijarjestelma", SPHEROID["International 1924",6378388,297]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["degree",0.0174532925199433]],
 *  PROJECTION["Transverse_Mercator"],
 *  PARAMETER["latitude_of_origin",0],
 *  PARAMETER["central_meridian",27],
 *  PARAMETER["scale_factor",1],
 *  PARAMETER["false_easting",3500000],
 *  PARAMETER["false_northing",0],
 *  UNIT["metre",1]]
 */
// ----------------------------------------------------------------------

const std::string NFmiYKJArea::WKT() const
{
  try
  {
    const char *fmt =
        R"(PROJCS["KKJ / Finland Uniform Coordinate System",)"
        R"(GEOGCS["KKJ",)"
        R"(DATUM["Kartastokoordinaattijarjestelma", SPHEROID["International 1924",6378388,297]],)"
        R"(PRIMEM["Greenwich",0],)"
        R"(UNIT["degree",0.0174532925199433]],)"
        R"(PROJECTION["Transverse_Mercator"],)"
        R"(PARAMETER["latitude_of_origin",0],)"
        R"(PARAMETER["central_meridian",27],)"
        R"(PARAMETER["scale_factor",1],)"
        R"(PARAMETER["false_easting",3500000],)"
        R"(PARAMETER["false_northing",0],)"
        R"(UNIT["metre",1]])";
    return fmt;
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

std::size_t NFmiYKJArea::HashValue() const
{
  try
  {
    std::size_t hash = NFmiKKJArea::HashValue();
    // no private members
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
