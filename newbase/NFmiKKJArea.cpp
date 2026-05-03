// ======================================================================
/*
 * \file NFmiKKJArea.cpp
 * \brief Implementation of class NFmiKKJArea
 */
// ======================================================================
/*!
 * \class NFmiKKJArea
 *
 *
 * Implements transformation from rectangular PKJ or YKJ coordinates into geodetic
 * coordinates and vice versa for the Finnish National Grid Coordinate System KKJ
 * (KartastoKoordinaattiJärjestelmä). For details see e.g. Matti Ollikainen in:
 * Geodeettinen Laitos Tiedote 8, 1993.
 *
 * The rectangular PKJ (PerusKoordinaattiJärjestelmä) coordinates are located on KKJ
 * zones 1-4 with easting and northing in meters.
 *
 * The rectangular YKJ (YhtenäisKoordinaattiJärjestelmä) coordinates are aligned to KKJ
 * zone 3 only (central meridian 27 degrees) with easting and northing in meters.
 *
 * Projection used is Gauss-Krueger projection with Hayford's International Reference ellipsoid.
 *
 * INHERETANCE:
 *
 * NOTES:
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
 * \see NFmiPKJArea and NFmiYKJArea
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
 * theKKJArea = KKJArea object to be copied
 *
 * NOTES
 *
 * The first constructor is meant to be used when both of the geodetic corner points
 * are known, whereas the second constructor is used when the upper right geodetic
 * corner point is unknown but the metric dimensions of the world rectangle are known.
 *
 */
// ======================================================================

#include "NFmiKKJArea.h"
#include <macgyver/Hash.h>
#include <macgyver/Exception.h>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiKKJArea::NFmiKKJArea()
    : NFmiArea(),
      itsTopRightLatLon(),
      itsBottomLeftLatLon(),
      a(),
      p(),
      b(),
      e(),
      dn(),
      dn2(),
      dn3(),
      dn4(),
      a1(),
      h1(),
      h2(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
{
  try
  {
    SetupGaussKruger();
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
 * \param theKKJArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiKKJArea::NFmiKKJArea(const NFmiKKJArea &theKKJArea)
    : NFmiArea(theKKJArea),
      itsTopRightLatLon(theKKJArea.itsTopRightLatLon),
      itsBottomLeftLatLon(theKKJArea.itsBottomLeftLatLon),
      a(),
      p(),
      b(),
      e(),
      dn(),
      dn2(),
      dn3(),
      dn4(),
      a1(),
      h1(),
      h2(),
      itsXScaleFactor(theKKJArea.itsXScaleFactor),
      itsYScaleFactor(theKKJArea.itsYScaleFactor),
      itsWorldRect(theKKJArea.itsWorldRect)
{
  try
  {
    SetupGaussKruger();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiKKJArea::NFmiKKJArea(const NFmiPoint &theBottomLeftLatLon,
                         const NFmiPoint &theTopRightLatLon,
                         const NFmiPoint &theTopLeftXY,
                         const NFmiPoint &theBottomRightXY,
                         bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsTopRightLatLon(theTopRightLatLon),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      a(),
      p(),
      b(),
      e(),
      dn(),
      dn2(),
      dn3(),
      dn4(),
      a1(),
      h1(),
      h2(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
{
  try
  {
    SetupGaussKruger();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*!
 * Constructor
 *
 * \param theRadialRangeInMeters Undocumented
 * \param theCenterLatLon Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
NFmiKKJArea::NFmiKKJArea(double /* theRadialRangeInMeters */,
                         const NFmiPoint & /* theCenterLatLon */,
                         const NFmiPoint &theTopLeftXY,
                         const NFmiPoint &theBottomRightXY)

    : NFmiArea(theTopLeftXY, theBottomRightXY),
      itsTopRightLatLon(),
      itsBottomLeftLatLon(),
      a(),
      p(),
      b(),
      e(),
      dn(),
      dn2(),
      dn3(),
      dn4(),
      a1(),
      h1(),
      h2(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
{
  try
  {
    SetupGaussKruger();
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
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiKKJArea::NFmiKKJArea(const NFmiPoint &theBottomLeftLatLon,
                         const NFmiPoint &theTopLeftXY,
                         const NFmiPoint &theBottomRightXY)
    : NFmiArea(theTopLeftXY, theBottomRightXY),
      itsTopRightLatLon(),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      a(),
      p(),
      b(),
      e(),
      dn(),
      dn2(),
      dn3(),
      dn4(),
      a1(),
      h1(),
      h2(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
{
  try
  {
    SetupGaussKruger();
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
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiKKJArea::NFmiKKJArea(const NFmiPoint &theTopLeftXY, const NFmiPoint &theBottomRightXY)
    : NFmiArea(theTopLeftXY, theBottomRightXY),
      itsTopRightLatLon(),
      itsBottomLeftLatLon(),
      a(),
      p(),
      b(),
      e(),
      dn(),
      dn2(),
      dn3(),
      dn4(),
      a1(),
      h1(),
      h2(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
{
  try
  {
    SetupGaussKruger();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Initializes parameters for Gauss-Kruger projection.
 */
// ----------------------------------------------------------------------

void NFmiKKJArea::SetupGaussKruger()
{
  try
  {
    // Set up Gauss-Krueger projection parameters
    a = 6378388.;                         // Major radius
    p = 1. / 297.;                        // Flatness
    b = (1. - p) * a;                     // Minor radius
    e = sqrt((a * a - b * b) / (a * a));  // Eccentricity

    dn = (a - b) / (a + b);
    dn2 = dn * dn;
    dn3 = dn * dn * dn;
    dn4 = dn * dn * dn * dn;

    a1 = a / (1. + dn) * (1. + dn2 / 4. + dn4 / 64.);

    h1[0] = dn / 2. - 2. * dn2 / 3. + 5. * dn3 / 16. + 41. * dn4 / 180.;
    h1[1] = 13. * dn2 / 48. - 3. * dn3 / 5. + 557. * dn4 / 1440.;
    h1[2] = 61. * dn3 / 240. - 103. * dn4 / 140.;
    h1[3] = 49561. * dn4 / 161280.;

    h2[0] = dn / 2. - (2. / 3.) * dn2 + 37. * dn3 / 96. - dn4 / 360.;
    h2[1] = dn2 / 48. + dn3 / 15. - 437. * dn4 / 1440.;
    h2[2] = 17. * dn3 / 480. - 37. * dn4 / 840.;
    h2[3] = 4397. * dn4 / 161280.;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theA Undocumented
 * \param theP Undocumented
 */
// ----------------------------------------------------------------------

void NFmiKKJArea::SetupAnyEllipsoid(double theA, double theP)
{
  try
  {
    a = theA;
    p = theP;

    // Set up any ellipsoid parameters
    // a = 6378388.;	 // Major radius specific to Hayford
    // p = 1./297.;	 // Flatness  specific to Hayford
    b = (1. - p) * a;                     // Minor radius
    e = sqrt((a * a - b * b) / (a * a));  // Eccentricity

    dn = (a - b) / (a + b);
    dn2 = dn * dn;
    dn3 = dn * dn * dn;
    dn4 = dn * dn * dn * dn;

    a1 = a / (1. + dn) * (1. + dn2 / 4. + dn4 / 64.);

    h1[0] = dn / 2. - 2. * dn2 / 3. + 5. * dn3 / 16. + 41. * dn4 / 180.;
    h1[1] = 13. * dn2 / 48. - 3. * dn3 / 5. + 557. * dn4 / 1440.;
    h1[2] = 61. * dn3 / 240. - 103. * dn4 / 140.;
    h1[3] = 49561. * dn4 / 161280.;

    h2[0] = dn / 2. - (2. / 3.) * dn2 + 37. * dn3 / 96. - dn4 / 360.;
    h2[1] = dn2 / 48. + dn3 / 15. - 437. * dn4 / 1440.;
    h2[2] = 17. * dn3 / 480. - 37. * dn4 / 840.;
    h2[3] = 4397. * dn4 / 161280.;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Works as ToLatLon() but the input rectangular parameter theXYPoint is
 * expected to be given in rectangular world coordinates (meters) instead of
 * local coordinates.
 *
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiKKJArea::WorldXYToLatLon(const NFmiPoint &theXYPoint) const
{
  try
  {
    double xeast, ynorth, zone, xeastForCentralMeridian, ksi, eta, ksi1, eta1, lambda, q, temp, q2,
        d, testv, lat, lon, dind;
    int ind;

    // Transform world xy-coordinates into geodetic coordinates.
    xeast = theXYPoint.X();   // Easting
    ynorth = theXYPoint.Y();  // Northing

    zone = double(ZoneNumberByEasting(xeast));

    xeastForCentralMeridian = 1000000. * (6. + zone) - 5500000.;
    ksi = ynorth / a1;
    eta = (xeast - xeastForCentralMeridian) / a1;
    ksi1 = 2. * ksi;
    eta1 = 2. * eta;

    for (ind = 1, dind = 1.0; ind <= 4; ind++, dind += 1.0)
    {
      ksi = ksi - h2[ind - 1] * sin(dind * ksi1) * cosh(dind * eta1);
      eta = eta - h2[ind - 1] * cos(dind * ksi1) * sinh(dind * eta1);
    }

    lambda = atan(sinh(eta) / cos(ksi));
    q = Arsinh(cos(lambda) * tan(ksi));
    temp = sin(atan(sinh(1.0040685 * q)));

    do
    {
      q2 = Artanh(e * temp);
      d = sinh(q + e * q2);
      d = d / sqrt(1. + d * d);
      testv = fabs(d - temp);
      temp = d;
    } while (testv >= 1.0E-11);

    lat = Deg(asin(d));
    lon = 18. + zone * 3.0 + Deg(lambda);

    return NFmiPoint(lon, lat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the geodetic latitude-longitude point corresponding the
 * rectangular local point theXYPoint. Point will be returned in degrees.
 *
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiKKJArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  try
  {
    double xeast, ynorth;

    // Transform local xy-coordinates into world xy-coordinates (meters).
    xeast = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;    // Easting
    ynorth = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;  // Northing

    // Transform world xy-coordinates into geodetic coordinates.
    return WorldXYToLatLon(NFmiPoint(xeast, ynorth));
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
 * \todo Remove the method as unnecessary duplicate
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiKKJArea::ToWorldXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    return LatLonToWorldXY(theLatLonPoint);
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

const NFmiPoint NFmiKKJArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
{
  try
  {
    double xeast = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
    double ynorth = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;

    return NFmiPoint(xeast, ynorth);
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

const NFmiPoint NFmiKKJArea::WorldXYToXY(const NFmiPoint &theWorldXYPoint) const
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
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiKKJArea::LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    // Transforms input geodetic coordinates (longitude,latitude) into world coordinates
    // (meters) for the Finnish National Grid Coordinate System (KKJ).

    double lat, lon, fii, d, zone, xeastForCentralMeridian, lambda, xeast, ynorth, q, ksi, ksi1,
        eta, eta1, dind;
    int ind;

    lat = theLatLonPoint.Y();
    lon = theLatLonPoint.X();

    fii = Rad(lat);
    d = sin(fii);

    zone = double(ZoneNumberByLongitude(lon));
    xeastForCentralMeridian = 1000000.0 * (6. + zone) - 5500000.;
    lambda = Rad(lon - zone * 3. - 18.);

    q = sinh(Arsinh(tan(fii)) - e * Artanh(e * d));
    ksi = atan(q / cos(lambda));
    ksi1 = 2. * ksi;
    eta = Artanh(sin(lambda) / sqrt(1. + q * q));
    eta1 = 2. * eta;

    for (ind = 1, dind = 1.0; ind <= 4; ind++, dind += 1.0)
    {
      ksi = ksi + h1[ind - 1] * sin(dind * ksi1) * cosh(dind * eta1);
      eta = eta + h1[ind - 1] * cos(dind * ksi1) * sinh(dind * eta1);
    }

    ynorth = a1 * ksi;                           // Northing
    xeast = a1 * eta + xeastForCentralMeridian;  // Easting

    return NFmiPoint(xeast, ynorth);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the local rectangular XY-point corresponding the input geodetic
 * latitude-longitude point theLatLonPoint.
 *
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiKKJArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    double xLocal, yLocal;

    // Transform input geodetic coordinates into world coordinates (meters).
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
 * Assignment operator
 *
 * \param theArea The other object being copied
 * \return The object assigned to
 * \bug Should protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiKKJArea &NFmiKKJArea::operator=(const NFmiKKJArea &theArea)
{
  try
  {
    NFmiArea::operator=(theArea);

    itsBottomLeftLatLon = theArea.itsBottomLeftLatLon;
    itsTopRightLatLon = theArea.itsTopRightLatLon;
    itsXScaleFactor = theArea.itsXScaleFactor;
    itsYScaleFactor = theArea.itsYScaleFactor;
    itsWorldRect = theArea.itsWorldRect;

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
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiKKJArea::operator==(const NFmiKKJArea &theArea) const
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
 * Inequality comparison
 *
 * \param theArea The object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiKKJArea::operator!=(const NFmiKKJArea &theArea) const
{
  try
  {
    return (!(*this == theArea));
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
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiKKJArea::operator==(const NFmiArea &theArea) const
{
  try
  {
    return *this == *(static_cast<const NFmiKKJArea *>(&theArea));
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
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiKKJArea::operator!=(const NFmiArea &theArea) const
{
  try
  {
    return (!(*this == theArea));
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

std::ostream &NFmiKKJArea::Write(std::ostream &file) const
{
  try
  {
    NFmiArea::Write(file);

    file << itsBottomLeftLatLon;
    file << itsTopRightLatLon;

    // Dummies to replace old removed variables
    file << "0 0\n0 0\n";

    file << itsXScaleFactor << " " << itsYScaleFactor << std::endl;
    file << itsWorldRect;

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

std::istream &NFmiKKJArea::Read(std::istream &file)
{
  try
  {
    NFmiArea::Read(file);

    double dummy;

    file >> itsBottomLeftLatLon;
    file >> itsTopRightLatLon;
    PacificView(NFmiArea::IsPacificView(itsBottomLeftLatLon, itsTopRightLatLon));

    file >> dummy >> dummy >> dummy >> dummy;  // old removed variables

    file >> itsXScaleFactor >> itsYScaleFactor;
    file >> itsWorldRect;

    Init(true);

    return file;
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

void NFmiKKJArea::Init(bool fKeepWorldRect)
{
  try
  {
    SetupGaussKruger();
    NFmiArea::Init(fKeepWorldRect);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const std::string NFmiKKJArea::AreaStr() const
{
  try
  {
    std::ostringstream out;
    out << "kkj:" << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ','
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
 *
 * The value cannot be defined without multiple zones, so we throw instead.
 */
// ----------------------------------------------------------------------

const std::string NFmiKKJArea::WKT() const
{
  try
  {
    throw Fmi::Exception(BCP, "WKT not available for generic KKJ projections");
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

std::size_t NFmiKKJArea::HashValue() const
{
  try
  {
    std::size_t hash = NFmiArea::HashValue();
    Fmi::hash_combine(hash, itsTopRightLatLon.HashValue());
    Fmi::hash_combine(hash, itsBottomLeftLatLon.HashValue());
    // no need to handle a, p, b, e, dn, dn2, dn3, dn4, a1, h1 or h2 here
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
