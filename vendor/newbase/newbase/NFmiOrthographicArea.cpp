// ======================================================================
/*!
 * \file NFmiOrthographicArea.cpp
 * \brief Implementation of class NFmiOrthographicArea
 */
// ======================================================================
/*!
 * \class NFmiOrthographicArea
 *
 * Orthographic projection. Maps geodetic coordinates (in degrees) to rectangular
 * orthographic xy coordinates (in meters) and vice versa.
 *
 *	Projection is based on refs
 *
 *    -# Peter Richardus, Ron K.Adler: "Map Projections", North-Holland, 2nd printing 1974.
 *    -# "An Album of Map Projections" USGS Paper 1453
 *
 *
 *
 *
 * How to convert the geodetic coordinates into rectangular LOCAL orthographic coordinates ?
 *
 * \code
 *
 *	double lon0,lat0,lon1,lat1,centerLon,centerLat,zAzimuthAngle, radialRange;
 *
 *
 *  // Take Helsinki for central point.
 *
 *	centerLon = 25.0;
 *	centerLat = 60.0;
 *
 *  // Tilt angle: the angle of rotation on "screen plane", that is, about z axis.
 *  // Positive for clockwise rotations, negative for counterclockwise rotations.
 *  // Defaults to 0 degrees.
 *
 *  zAzimuthAngle = 0;
 *	radialRange = kRearth;
 *
 *
 *
 *  NFmiOrthographicArea *ortho = new NFmiOrthographicArea(NFmiPoint(centerLon, centerLat),
 *															radialRange,
 *zAzimuthAngle);
 *
 *
 *  -OR-
 *
 *	NFmiOrthographicArea *ortho = new NFmiOrthographicArea(NFmiPoint(24, 59),
 *														   NFmiPoint(26,
 *61),
 *														   zAzimuthAngle);
 *
 *
 *
 * NFmiPoint orthoXY =  ortho.ToXY(NFmiPoint(centerLon, centerLat));
 * \endcode
 *
 * How to convert the rectangular LOCAL orthographic coordinates back into geodetic coordinates ?
 *
 * \code
 * NFmiPoint orthoLatLon = ortho.ToLatLon(orthoXY);
 * \endcode
 */
// ======================================================================

#include "NFmiOrthographicArea.h"
#include <macgyver/Hash.h>
#include <fmt/format.h>
#include <macgyver/Exception.h>
#include <algorithm>
#include <cmath>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 * \todo Use initialization lists
 */
// ----------------------------------------------------------------------

NFmiOrthographicArea::NFmiOrthographicArea() : itsZoomFactor(), itsGlobeRadius(kRearth) {}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param thePerspectiveArea The object being copied
 * \todo Use intitialization lists
 */
// ----------------------------------------------------------------------

NFmiOrthographicArea::NFmiOrthographicArea(const NFmiOrthographicArea &theOrthographicArea)
    : NFmiAzimuthalArea(theOrthographicArea),
      itsAzimuthAngle(theOrthographicArea.itsAzimuthAngle),
      itsLat0(theOrthographicArea.itsLat0),
      itsLon0(theOrthographicArea.itsLon0),
      itsSinLat0(theOrthographicArea.itsSinLat0),
      itsCosLat0(theOrthographicArea.itsCosLat0),
      itsVisibilityTerm(kFloatMissing),
      itsZoomFactor(theOrthographicArea.itsZoomFactor),
      itsGlobeRadius(theOrthographicArea.itsGlobeRadius),
      itsCurrentLatlonPoint(NFmiPoint(kFloatMissing, kFloatMissing))
{
}

// ----------------------------------------------------------------------

NFmiOrthographicArea::NFmiOrthographicArea(const NFmiPoint &theCentralLatLonPoint,
                                           const double theRadialRange,
                                           const double theAzimuthAngle,
                                           const NFmiPoint &theTopLeftXY,
                                           const NFmiPoint &theBottomRightXY,
                                           bool usePacificView)

    : NFmiAzimuthalArea(theRadialRange,
                        theCentralLatLonPoint.X(),
                        theTopLeftXY,
                        theBottomRightXY,
                        theCentralLatLonPoint.Y(),
                        90.,
                        usePacificView),
      itsAzimuthAngle(theAzimuthAngle),
      itsZoomFactor(theRadialRange / kRearth)
{
  try
  {
    itsLat0 = FmiRad(CurrentCenter().Y());
    itsLon0 = FmiRad(CurrentCenter().X());
    itsSinLat0 = sin(itsLat0);
    itsCosLat0 = cos(itsLat0);

    itsGlobeRadius = kRearth;
    itsZoomFactor = theRadialRange / kRearth;
    itsAzimuthAngle = theAzimuthAngle;
    itsVisibilityTerm = kFloatMissing;
    itsCurrentLatlonPoint = NFmiPoint(kFloatMissing, kFloatMissing);

    // Purpose: to create a square bounding the circle of radius 'theRadialRange' (in meters)

    //  itsWorldRect = NFmiRect(NFmiPoint(-itsRadialRange,-itsRadialRange),
    // 						    NFmiPoint(itsRadialRange,itsRadialRange));

    NFmiOrthographicArea::Init();  // Em.  'itsWorldRect' -lause korvattu tällä Init():llä

    NFmiAzimuthalArea::Init(true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//  ----------------------------------------------------------------------

/*
        HUOM! NFmiOrthographicArea -PROJEKTIOSSA NURKKAPISTEIDEN MUUNNOS
        EI VÄLTTÄMÄTTÄ AINA OLE BIJEKTIO:
        ESIM. JOILLEKIN SUORAKULMAISEN KOORDINAATISTON XY-PISTEILLE EI LÖYDY
        VASTAAVIA MAANTIETEELLISIÄ LONLAT -PISTEITÄ (PISTEET OVAT "KARTTAPALLON" ULKOPUOLELLA)
*/

NFmiOrthographicArea::NFmiOrthographicArea(const NFmiPoint &theBottomLeftLatLon,
                                           const NFmiPoint &theTopRightLatLon,
                                           const double theAzimuthAngle,
                                           const NFmiPoint &theTopLeftXY,
                                           const NFmiPoint &theBottomRightXY,
                                           bool usePacificView)
    : NFmiAzimuthalArea(
          theBottomLeftLatLon,
          theTopRightLatLon,
          (theBottomLeftLatLon + theTopRightLatLon).X() / 2.,  // Alueen keskimeridiaani (lon)
          theTopLeftXY,
          theBottomRightXY,
          (theBottomLeftLatLon + theTopRightLatLon).Y() / 2.,  // Alueen keskileveys (lat)
          90.,
          usePacificView),
      itsAzimuthAngle(theAzimuthAngle),
      itsZoomFactor(1.0),
      itsGlobeRadius(kRearth)
{
  try
  {
    itsLat0 = FmiRad(CurrentCenter().Y());
    itsLon0 = FmiRad(CurrentCenter().X());
    itsSinLat0 = sin(itsLat0);
    itsCosLat0 = cos(itsLat0);

    // HUOM!
    // Tässä konstruktorissa on tarkoitus käyttää perinteisempää
    // "nurkkapistekonstruktoria"

    NFmiPoint bottomLeftWorldXY = LatLonToWorldXY(itsBottomLeftLatLon);
    NFmiPoint topRightWorldXY = LatLonToWorldXY(itsTopRightLatLon);

    // Lasketaan "keskipistearvot" CurrentCenter():lle MAAILMANKOORDINAATTIEN avulla
    // - *EI* latlon -keskileveys/korkeuskoordinaattien avulla
    // ==> saadaan suorakulmaisen maailmankoordinaattialueen keskipiste
    ///?????CurrentCenter(WorldXYToLatLon(0.5*(topRightWorldXY + bottomLeftWorldXY)));

    itsWorldRect = NFmiRect(bottomLeftWorldXY, topRightWorldXY);

    NFmiAzimuthalArea::Init(true);

    itsRadialRange = FmiMin(itsWorldRect.Width(), itsWorldRect.Height());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect Undocumented, unused
 */
// ----------------------------------------------------------------------

void NFmiOrthographicArea::Init(bool /* fKeepWorldRect */)
{
  try
  {
    double aspectRatio = Width() / Height();
    itsZoomFactor = 1.0;
    double width = itsZoomFactor * 2. * GlobeRadius();
    double height = aspectRatio * width;

    itsWorldRect = NFmiRect(-width / 2., -height / 2., width / 2., height / 2.);
    itsRadialRange = FmiMin(itsWorldRect.Width(), itsWorldRect.Height());

    itsXScaleFactor = Width() / itsWorldRect.Width();
    itsYScaleFactor = Height() / itsWorldRect.Height();

    itsTopRightLatLon = TopRightLatLon();
    itsBottomLeftLatLon = BottomLeftLatLon();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------

// bool IsVisiblePoint(const NFmiPoint & theLatLonPoint)
// {
//   if((itsCurrentLatlonPoint != theLatLonPoint) || (itsVisibilityTerm == kFloatMissing))
//   {
//     double lon = FmiRad(theLatLonPoint.X());
//     double lat = FmiRad(theLatLonPoint.Y());
//     double dlon = lon - itsLon0;
//     double coslat = cos(lat);
//     double sinlat = sin(lat);
//     double cosdlon = cos(dlon);
//
//     itsCurrentLatlonPoint = theLatLonPoint;
//     itsVisibilityTerm =  itsSinLat0*sinlat + itsCosLat0*coslat*cosdlon; // = cosz. See "An Album
//     of Map Projections"
//   }
//
//   if(itsVisibilityTerm < 0.)
//   {
//          // Not visible (since beyond the horizon). Reject the point.
//          return false;
//   }
//
//   return true;
// }

const NFmiPoint NFmiOrthographicArea::WorldXYToLatLon(const NFmiPoint &theXY) const
{
  try
  {
    // HUOM! Tämä metodi on kirjoittu tarkoituksella  NFmiAzimuthalArea -emoluokan metodin
    // WorldXYToLatLon()
    // päälle (override) koska menettely yksinkertaistaa ja nopeuttaa jonkin verran koodia

    double xUnrotated = theXY.X();
    double yUnrotated = theXY.Y();

    // Do we hit the globe at point 'theXY'?

    double xyDist2 = xUnrotated * xUnrotated + yUnrotated * yUnrotated;

    if (xyDist2 > kRearth * kRearth)
      return NFmiPoint(kFloatMissing, kFloatMissing);  // Not hit but lost in outer space ...

    if (xyDist2 == 0.)
      return CurrentCenter();

    double x = 0.;
    double y = 0.;

    if (itsAzimuthAngle != 0.)
    {
      // Rotate point (x, y) about z-axis in right-handed cartesian coordinate system
      // by the angle of 'itsAzimuthAngle' degrees

      double cosAng = cos(FmiRad(-itsAzimuthAngle));
      double sinAng = sin(FmiRad(-itsAzimuthAngle));

      x = (xUnrotated * cosAng - yUnrotated * sinAng) / GlobeRadius();  // x scaled to [0..1]
      y = (xUnrotated * sinAng + yUnrotated * cosAng) / GlobeRadius();  // y scaled to [0..1]
    }
    else
    {
      x = xUnrotated / GlobeRadius();  // x scaled to [0..1]
      y = yUnrotated / GlobeRadius();  // y scaled to [0..1]
    }

    // Finally, compute the longitude and latitude

    double xyDist = sqrt(xyDist2) / GlobeRadius();  // Distance scaled to [0..1]
    double c = asin(FmiMin(1., FmiMax(-1., xyDist)));
    double sinc = sin(c);
    double cosc = cos(c);

    double lon = itsLon0 + atan(x * sinc / (xyDist * itsCosLat0 * cosc - y * itsSinLat0 * sinc));
    double lat =
        asin(MunFmiMin(1, MunFmiMax(-1, cosc * itsSinLat0 + (y * sinc * itsCosLat0) / xyDist)));

    return NFmiPoint(FmiDeg(lon), FmiDeg(lat));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

const NFmiPoint NFmiOrthographicArea::LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    // HUOM! Tämä metodi on kirjoittu tarkoituksella  NFmiAzimuthalArea -emoluokan metodin
    // LatLonToWorldXY()
    // päälle (override) koska menettely yksinkertaistaa ja nopeuttaa jonkin verran koodia

    double lon = FmiRad(theLatLonPoint.X());
    double lat = FmiRad(theLatLonPoint.Y());
    double dlon = lon - itsLon0;
    double coslat = cos(lat);
    double sinlat = sin(lat);
    double cosdlon = cos(dlon);

    // See "An Album of Map Projections" ----------------->
    double cosz = itsSinLat0 * sinlat + itsCosLat0 * coslat * cosdlon;
    if (cosz < 0.)
    {
      // Reject the point.
      return NFmiPoint(kFloatMissing, kFloatMissing);
    }
    // See "An Album of Map Projections" <----------------

    double x = GlobeRadius() * coslat * sin(dlon);
    double y = GlobeRadius() * (itsCosLat0 * sinlat - itsSinLat0 * coslat * cosdlon);

    if (itsAzimuthAngle != 0.)
    {
      // Rotate point (x, y) about z-axis in right-handed cartesian coordinate system
      // by the angle of 'itsAzimuthAngle' degrees

      double cosAng = cos(FmiRad(-itsAzimuthAngle));
      double sinAng = sin(FmiRad(-itsAzimuthAngle));

      double xRotated = x * cosAng - y * sinAng;
      double yRotated = x * sinAng + y * cosAng;

      return NFmiPoint(xRotated, yRotated);
    }

    return NFmiPoint(x, y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

void NFmiOrthographicArea::ZoomFactor(double theZoomFactor)
{
  try
  {
    // In orthographics only: for "entire world view", use zoom factor value 1.

    itsZoomFactor = theZoomFactor;

    Init();  // Re-zooming requires re-initialization of everything associated with zooming
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

double NFmiOrthographicArea::ZoomFactor() const
{
  try
  {
    return itsZoomFactor;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

void NFmiOrthographicArea::GlobeRadius(double &theGlobeRadius)
{
  try
  {
    itsGlobeRadius = theGlobeRadius;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

void NFmiOrthographicArea::AzimuthAngle(double &theAzimuthAngle)
{
  try
  {
    itsAzimuthAngle = theAzimuthAngle;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiOrthographicArea::AzimuthAngle() const
{
  try
  {
    return itsAzimuthAngle;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

NFmiArea *NFmiOrthographicArea::NewArea(const NFmiPoint & /* theBottomLeftLatLon */,
                                        const NFmiPoint & /* theTopRightLatLon */,
                                        bool /* allowPacificFix */) const
{
  try
  {
    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return std::shared_ptr
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiOrthographicArea::Clone() const
{
  try
  {
    return new NFmiOrthographicArea(*this);
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
 * \param theArea The object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiOrthographicArea &NFmiOrthographicArea::operator=(const NFmiOrthographicArea &theArea)
{
  try
  {
    NFmiAzimuthalArea::operator=(theArea);
    return *this;
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
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiOrthographicArea::operator==(const NFmiOrthographicArea &theArea) const
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
 * \return True if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiOrthographicArea::operator!=(const NFmiOrthographicArea &theArea) const
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
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiOrthographicArea::operator==(const NFmiArea &theArea) const
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
 * \return True if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiOrthographicArea::operator!=(const NFmiArea &theArea) const
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

std::ostream &NFmiOrthographicArea::Write(std::ostream &file) const
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

std::istream &NFmiOrthographicArea::Read(std::istream &file)
{
  try
  {
    NFmiAzimuthalArea::Read(file);

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const std::string NFmiOrthographicArea::AreaStr() const
{
  try
  {
    std::ostringstream out;
    out << "orthographic," << itsCurrentLatlonPoint.X() << ',' << itsCurrentLatlonPoint.Y() << ','
        << itsGlobeRadius << ',' << itsAzimuthAngle << ':' << BottomLeftLatLon().X() << ','
        << BottomLeftLatLon().Y() << ',' << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
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
 * PROJCS["FMI_Orthographic",
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  PROJECTION["Orthographic"],
 *  PARAMETER["latitude_of_origin",lat_0],
 *  PARAMETER["central_meridian",lon_0],
 *  PARAMETER["false_easting",x_0],
 *  PARAMETER["false_northing",y_0],
 *  UNIT["Metre",1.0]]
 */
// ----------------------------------------------------------------------

const std::string NFmiOrthographicArea::WKT() const
{
  try
  {
    const char *fmt = R"(PROJCS["FMI_Orthographic",)"
                      R"(GEOGCS["FMI_Sphere",)"
                      R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",{:.0f},0]],)"
                      R"(PRIMEM["Greenwich",0],)"
                      R"(UNIT["Degree",0.0174532925199433]],)"
                      R"(PROJECTION["Orthographic"],)"
                      R"(PARAMETER["latitude_of_origin",{}],)"
                      R"(PARAMETER["central_meridian",{}],)"
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

std::size_t NFmiOrthographicArea::HashValue() const
{
  try
  {
    std::size_t hash = NFmiAzimuthalArea::HashValue();
    Fmi::hash_combine(hash, Fmi::hash_value(itsAzimuthAngle));
    Fmi::hash_combine(hash, Fmi::hash_value(itsGlobeRadius));
    Fmi::hash_combine(hash, itsCurrentLatlonPoint.HashValue());
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
