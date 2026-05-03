#include "NFmiLambertConformalConicArea.h"
#include "NFmiStringTools.h"
#include <macgyver/Hash.h>
#include <fmt/format.h>
#include <macgyver/Exception.h>
#include <cmath>

using namespace std;

// For some reason Boost.Constants does not define quart_pi so we'll just use our own definitions
namespace
{
const double pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
const double half_pi = pi / 2;
const double quart_pi = pi / 4;
}  // namespace

// ----------------------------------------------------------------------
/*!
 * Constructor
 */
// ----------------------------------------------------------------------

NFmiLambertConformalConicArea::NFmiLambertConformalConicArea(const NFmiPoint &theBottomLeftLatLon,
                                                             const NFmiPoint &theTopRightLatLon,
                                                             double theCentralLongitude,
                                                             double theCentralLatitude,
                                                             double theTrueLatitude1,
                                                             double theTrueLatitude2,
                                                             double theRadius,
                                                             bool usePacificView,
                                                             const NFmiPoint &theTopLeftXY,
                                                             const NFmiPoint &theBottomRightXY)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsCentralLongitude(theCentralLongitude),
      itsCentralLatitude(theCentralLatitude),
      itsTrueLatitude1(theTrueLatitude1),
      itsTrueLatitude2(theTrueLatitude2),
      itsRadius(theRadius)
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
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLambertConformalConicArea::Init(bool fKeepWorldRect)
{
  try
  {
    const double lat1 = FmiRad(itsTrueLatitude1);
    const double lat2 = FmiRad(itsTrueLatitude2);
    const double lat0 = FmiRad(itsCentralLatitude);

    if (itsTrueLatitude1 != itsTrueLatitude2)
      itsN = log(cos(lat1) / cos(lat2)) / log(tan(quart_pi + lat2 / 2) / tan(quart_pi + lat1 / 2));
    else
      itsN = sin(lat1);

    itsF = cos(lat1) * pow(tan(quart_pi + lat1 / 2), itsN) / itsN;
    itsRho0 = itsRadius * itsF / pow(tan(quart_pi + lat0 / 2), itsN);

    if (!fKeepWorldRect)
      itsWorldRect =
          NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));

    itsXScaleFactor = Width() / itsWorldRect.Width();
    itsYScaleFactor = Height() / itsWorldRect.Height();

    const char *fmt =
        "+proj=lcc +lat_1={} +lat_2={} +lat_0={} +lon_0={} +x_0=0 +y_0=0 +R={} +units=m +wktext "
        "+no_defs +type=crs";
    itsProjStr = fmt::format(fmt::runtime(fmt),
                             itsTrueLatitude1,
                             itsTrueLatitude2,
                             itsCentralLatitude,
                             itsCentralLongitude,
                             itsRadius);
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);
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

NFmiArea *NFmiLambertConformalConicArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                                 const NFmiPoint &theTopRightLatLon,
                                                 bool allowPacificFix) const
{
  try
  {
    if (allowPacificFix)
    {
      PacificPointFixerData fixedPointData =
          NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
      return new NFmiLambertConformalConicArea(fixedPointData.itsBottomLeftLatlon,
                                               fixedPointData.itsTopRightLatlon,
                                               itsCentralLongitude,
                                               itsCentralLatitude,
                                               itsTrueLatitude1,
                                               itsTrueLatitude2,
                                               itsRadius,
                                               fixedPointData.fIsPacific,
                                               TopLeft(),
                                               BottomRight());
    }

    return new NFmiLambertConformalConicArea(theBottomLeftLatLon,
                                             theTopRightLatLon,
                                             itsCentralLongitude,
                                             itsCentralLatitude,
                                             itsTrueLatitude1,
                                             itsTrueLatitude2,
                                             itsRadius,
                                             PacificView(),
                                             TopLeft(),
                                             BottomRight());
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

NFmiArea *NFmiLambertConformalConicArea::Clone() const
{
  try
  {
    return new NFmiLambertConformalConicArea(*this);
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

bool NFmiLambertConformalConicArea::operator==(const NFmiLambertConformalConicArea &theArea) const
{
  try
  {
    if ((itsBottomLeftLatLon == theArea.itsBottomLeftLatLon) &&
        (itsTopRightLatLon == theArea.itsTopRightLatLon) &&
        (itsCentralLongitude == theArea.itsCentralLongitude) &&
        (itsCentralLatitude == theArea.itsCentralLatitude) &&
        (itsTrueLatitude1 == theArea.itsTrueLatitude1) &&
        (itsTrueLatitude2 == theArea.itsTrueLatitude2) && (itsRadius == theArea.itsRadius) &&
        (itsWorldRect == theArea.itsWorldRect))
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
 * Inequality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiLambertConformalConicArea::operator!=(const NFmiLambertConformalConicArea &theArea) const
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

bool NFmiLambertConformalConicArea::operator==(const NFmiArea &theArea) const
{
  try
  {
    return *this == static_cast<const NFmiLambertConformalConicArea &>(theArea);
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

bool NFmiLambertConformalConicArea::operator!=(const NFmiArea &theArea) const
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

std::ostream &NFmiLambertConformalConicArea::Write(std::ostream &file) const
{
  try
  {
    NFmiArea::Write(file);

    file << itsBottomLeftLatLon << itsTopRightLatLon << endl
         << itsCentralLongitude << ' ' << itsCentralLatitude << endl
         << itsTrueLatitude1 << ' ' << itsTrueLatitude2 << endl
         << itsRadius << endl;

    int oldPrec = file.precision();
    file.precision(15);
    file << itsWorldRect << endl;

    file.precision(oldPrec);

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

std::istream &NFmiLambertConformalConicArea::Read(std::istream &file)
{
  try
  {
    NFmiArea::Read(file);

    file >> itsBottomLeftLatLon >> itsTopRightLatLon;
    PacificView(NFmiArea::IsPacificView(itsBottomLeftLatLon, itsTopRightLatLon));
    file >> itsCentralLongitude >> itsCentralLatitude >> itsTrueLatitude1 >> itsTrueLatitude2 >>
        itsRadius >> itsWorldRect;
    Init();

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiArea *NFmiLambertConformalConicArea::CreateNewArea(const NFmiRect &theRect) const
{
  try
  {
    NFmiPoint bottomLeft(ToLatLon(theRect.BottomLeft()));
    NFmiPoint topRight(ToLatLon(theRect.TopRight()));
    NFmiArea *area = new NFmiLambertConformalConicArea(bottomLeft,
                                                       topRight,
                                                       itsCentralLongitude,
                                                       itsCentralLatitude,
                                                       itsTrueLatitude1,
                                                       itsTrueLatitude2,
                                                       itsRadius,
                                                       false,
                                                       TopLeft(),
                                                       BottomRight());
    return area;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const std::string NFmiLambertConformalConicArea::AreaStr() const
{
  try
  {
    std::ostringstream out;
    out << "lcc," << itsCentralLongitude << ',' << itsCentralLatitude << ',' << itsTrueLatitude1;
    if (itsTrueLatitude2 != itsTrueLatitude2 || itsRadius != kRearth)
      out << ',' << itsTrueLatitude2 << ',' << itsRadius;
    out << ':' << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ','
        << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
    return out.str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

const std::string NFmiLambertConformalConicArea::WKT() const
{
  try
  {
    const char *fmt = R"(PROJCS["FMI_LambertConic",)"
                      R"(GEOGCS["Unknown",)"
                      R"(DATUM["Unknown",SPHEROID["Sphere",{:.0f},0]],)"
                      R"(PRIMEM["Greenwich",0],)"
                      R"(UNIT["Degree",0.0174532925199433]],)"
                      R"(PROJECTION["Lambert_Conformal_Conic_2SP"],)"
                      R"(PARAMETER["latitude_of_origin",{}],)"
                      R"(PARAMETER["central_meridian",{}],)"
                      R"(PARAMETER["standard_parallel_1",{}],)"
                      R"(PARAMETER["standard_parallel_2",{}],)"
                      R"(UNIT["Metre",1.0]])";
    return fmt::format(fmt::runtime(fmt),
                       itsRadius,
                       itsCentralLatitude,
                       itsCentralLongitude,
                       itsTrueLatitude1,
                       itsTrueLatitude2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiPoint NFmiLambertConformalConicArea::LatLonToWorldXY(
    const NFmiPoint &theLatLonPoint) const
{
  try
  {
    const double lat = FmiRad(theLatLonPoint.Y());

    const double theta = itsN * FmiRad(theLatLonPoint.X() - itsCentralLongitude);
    const double rho = itsRadius * itsF * pow(tan(quart_pi + lat / 2), -itsN);

    const double x = rho * sin(theta);
    const double y = itsRho0 - rho * cos(theta);

    return NFmiPoint(x, y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiPoint NFmiLambertConformalConicArea::WorldXYToLatLon(const NFmiPoint &theXYPoint) const
{
  try
  {
    double lambda = 0;
    double phi = 0;

    double x = theXYPoint.X();
    double y = itsRho0 - theXYPoint.Y();

    double rho = std::hypot(x, y);

    if (rho == 0)
    {
      // Either pole
      phi = (itsN > 0 ? half_pi : -half_pi);
    }
    else
    {
      if (itsN < 0)
      {
        rho = -rho;
        x = -x;
        y = -y;
      }
      phi = 2 * atan(pow(itsRadius * itsF / rho, 1 / itsN)) - half_pi;
      lambda = atan2(x, y) / itsN + FmiRad(itsCentralLongitude);
    }

    return NFmiPoint(FmiDeg(lambda), FmiDeg(phi));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiPoint NFmiLambertConformalConicArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
{
  try
  {
    double xWorld = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
    double yWorld = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;

    return NFmiPoint(xWorld, yWorld);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiPoint NFmiLambertConformalConicArea::WorldXYToXY(const NFmiPoint &theWorldXYPoint) const
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

const NFmiPoint NFmiLambertConformalConicArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  try
  {
    // Transform local xy-coordinates into world xy-coordinates (meters).

    double xWorld = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
    double yWorld = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;

    // Transform world xy-coordinates into geodetic coordinates.

    return WorldXYToLatLon(NFmiPoint(xWorld, yWorld));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiPoint NFmiLambertConformalConicArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    double xLocal, yLocal;

    // Transform input geodetic coordinates into world coordinates (meters) on xy-plane.
    NFmiPoint latlon(FixLongitude(theLatLonPoint.X()), theLatLonPoint.Y());
    NFmiPoint xyWorld(LatLonToWorldXY(latlon));

    if (xyWorld == NFmiPoint::gMissingLatlon)
    {
      return xyWorld;
    }

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
 * \brief Hash value
 */
// ----------------------------------------------------------------------

std::size_t NFmiLambertConformalConicArea::HashValue() const
{
  try
  {
    std::size_t hash = NFmiArea::HashValue();
    Fmi::hash_combine(hash, itsBottomLeftLatLon.HashValue());
    Fmi::hash_combine(hash, itsTopRightLatLon.HashValue());
    Fmi::hash_combine(hash, Fmi::hash_value(itsCentralLongitude));
    Fmi::hash_combine(hash, Fmi::hash_value(itsCentralLatitude));
    Fmi::hash_combine(hash, Fmi::hash_value(itsTrueLatitude1));
    Fmi::hash_combine(hash, Fmi::hash_value(itsTrueLatitude2));
    Fmi::hash_combine(hash, Fmi::hash_value(itsRadius));
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ======================================================================
