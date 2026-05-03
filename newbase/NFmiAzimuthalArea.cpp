// ======================================================================
/*!
 * \file NFmiAzimuthalArea.cpp
 * \brief Implementation of class NFmiAzimuthalArea
 */
// ======================================================================
/*!
 * \class NFmiAzimuthalArea
 *
 * Abstract class for azimuthal projections.
 * Partially based on masterpiece "npolster.cpp" (Originaali 1.  3. 1996/Persa ja Kari)
 *
 * Azimuthal projections are based on refs.
 *
 *	-# Peter Richardus, Ron K.Adler: "Map Projections", North-Holland, 2nd printing 1974.
 *	-# "Navigation Facility Reference Manual" rev. 2.0 3/93
 *	-# "An Album of Map Projections" USGS Paper 1453
 *
 * SOME BASIC AZIMUTHAL PROJECTION CONCEPTS
 *
 * POLAR or NORMAL CASE means the central latitude is 90 degrees North,
 * that is the North Pole
 *
 * TANGENTIAL CASE means the true latitude is 90 degrees North, otherwise we talk
 * about NON-TANGENTIAL or SECANT CASE
 *
 *
 * NOTE:
 * Applying non-polar case (central latitude other than 90 degrees North) is
 * possible but not tested in this program. Applying both non-tangential projection
 * plane AND non-polar case together is NOT ALLOWED in this program! FMI defaults to
 * following values: central latitude 90 deg North, true latitude 60 deg North
 *
 *
 * The y axis lies along the central meridian (lon0), y increasing north;
 * The x axis is perpendicular to the y axis at (lon0,lat0), x increasing east.
 *
 * RECTANGULAR LOCAL COORDINATES are relative unitless cartesian XY-coordinates.
 * The range for these coordinates will be derived directly from the size of the
 * local rectangle used. The upper left corner XY-point and the opposite lower right
 * corner XY-point will define the LOCAL RECTANGLE.
 *
 * RECTANGULAR WORLD COORDINATES are cartesian XY-coordinates measured in meters
 * on "true world" map plane. The upper left corner XY-point and the opposite lower
 * right corner XY-point (in meters) will define the WORLD RECTANGLE.
 *
 * GEODETIC COORDINATES are latitude-longitude coordinates in degrees.
 *
 *
 * \see NFmiStereographicArea, NFmiGnomonicArea and NFmiEquidistArea
 *
 * INPUT PARAMETERS/ARGUMENT LISTS
 *
 * theBottomLeftLatLon = lower left corner point of the rectangle in GEODETIC
 *								 latitude-longitude coordinates
 *
 * theTopRightLatLon =	 upper right corner point of the rectangle in GEODETIC
 *								 latitude-longitude coordinates for
 *the
 *rectangle
 *
 * theCentralLongitude = central longitude (also called central meridian)
 *
 * theTopLeftXY = upper left corner of the rectangle in rectangular LOCAL coordinates
 *
 * theBottomRightXY = lower right corner of the rectangle in rectangular LOCAL coordinates
 *
 *
 * theCenterLatitude = See definition in ref [1]. For example, center latitude
 *					 for the north POLAR stereographic projection is always 90
 *degrees.
 *					 This is also called a "normal" projection case.
 *                   If the central latitude is other than 90 degrees the projection is NOT
 *					 a polar (or normal case of) projection.
 *
 * theTrueLatitude = latitude, where the Earth globe intersects the rectangular world
 *					 XY plane. For true angle 90 degrees North the intersecting
 *XY
 *plane
 *gently
 *					 touches the north pole. For details, see ref [2].
 *
 * theRadialRange = radius of the circle bounded by corner points forming a square.
 *                  Radius is assumed to be given in meters
 *
 */
// ======================================================================

#include "NFmiAzimuthalArea.h"
#include <macgyver/Hash.h>
#include <macgyver/Exception.h>
#include <cassert>
#include <cmath>
#include <iostream>

using namespace std;

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiAzimuthalArea::NFmiAzimuthalArea()
    : itsTopRightLatLon(),
      itsBottomLeftLatLon(),
      itsBottomLeftWorldXY(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect(),
      itsRadialRange(),
      itsCentralLongitude(),
      itsCentralLatitude(),
      itsTrueLatitude(),
      itsTrueLatScaleFactor()
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theAzimuthalArea The other area being copied
 */
// ----------------------------------------------------------------------

NFmiAzimuthalArea::NFmiAzimuthalArea(const NFmiAzimuthalArea &theAzimuthalArea)

    = default;

/*!
 * Constructor
 *
 * \param theCenterLatLon Undocumented
 * \param theRadialRangeInMeters Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
NFmiAzimuthalArea::NFmiAzimuthalArea(double theRadialRangeInMeters,
                                     const NFmiPoint &theCenterLatLon,
                                     const NFmiPoint &theTopLeftXY,
                                     const NFmiPoint &theBottomRightXY,
                                     bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsTopRightLatLon(),
      itsBottomLeftLatLon(),
      itsBottomLeftWorldXY(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect(),
      itsRadialRange(theRadialRangeInMeters),
      itsCentralLongitude(theCenterLatLon.X(), usePacificView),
      itsCentralLatitude(theCenterLatLon.Y()),
      itsTrueLatitude(90.),
      itsTrueLatScaleFactor(1.0)
{
}

// ----------------------------------------------------------------------
/*!
 * This constructor creates a XY world rectangle based on the given bottom
 * left and top right latlons.
 *
 * \param theBottomLeftLatLon The bottom left corner coordinates
 * \param theTopRightLatLon The top right corner coordinates
 * \param theCentralLongitude The central longitude of the projection
 * \param theTopLeftXY The top left view coordinates
 * \param theBottomRightXY The bottom right view coordinates
 * \param theCentralLatitude The projection center latitude
 * \param theTrueLatitude The tangential plane latitude
 */
// ----------------------------------------------------------------------

NFmiAzimuthalArea::NFmiAzimuthalArea(const NFmiPoint &theBottomLeftLatLon,
                                     const NFmiPoint &theTopRightLatLon,
                                     const double theCentralLongitude,
                                     const NFmiPoint &theTopLeftXY,
                                     const NFmiPoint &theBottomRightXY,
                                     const double theCentralLatitude,
                                     const double theTrueLatitude,
                                     bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsTopRightLatLon(theTopRightLatLon),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsBottomLeftWorldXY(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect(),
      itsRadialRange(0.),
      itsCentralLongitude(theCentralLongitude, usePacificView),
      itsCentralLatitude(theCentralLatitude),
      itsTrueLatitude(theTrueLatitude),
      itsTrueLatScaleFactor(0.0)
{
}

// ----------------------------------------------------------------------
/*!
 * This constructor creates a XY world rectangle based on the given bottom
 * left latlon and the given width and height of the world rectangle
 * in X and Y directions, respectively.
 *
 * \param theBottomLeftLatLon The bottom left corner coordinates
 * \param theCentralLongitude The central longitude of the projection
 * \param theTopLeftXY The top left view coordinates
 * \param theBottomRightXY The bottom right view coordinates
 * \param theCentralLatitude The projection center latitude
 * \param theTrueLatitude The tangential plane latitude
 */
// ----------------------------------------------------------------------

NFmiAzimuthalArea::NFmiAzimuthalArea(const NFmiPoint &theBottomLeftLatLon,
                                     const double theCentralLongitude,
                                     const NFmiPoint &theTopLeftXY,
                                     const NFmiPoint &theBottomRightXY,
                                     const double theCentralLatitude,
                                     const double theTrueLatitude,
                                     bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsTopRightLatLon(),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsBottomLeftWorldXY(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect(),
      itsRadialRange(0.),
      itsCentralLongitude(theCentralLongitude, usePacificView),
      itsCentralLatitude(theCentralLatitude),
      itsTrueLatitude(theTrueLatitude),
      itsTrueLatScaleFactor(0.0)
{
}

// ----------------------------------------------------------------------
/*!
 * This constructor creates a XY world square based on the radial range.
 * This range can be taken as the radius of the circle bounded by corner
 * points forming the square. Radius is assumed to be given in meters
 *
 * \param theRadialRange The redius of the view area in meters.
 * \param theCentralLongitude The central longitude of the projection
 * \param theTopLeftXY The top left view coordinates
 * \param theBottomRightXY The bottom right view coordinates
 * \param theCentralLatitude The projection center latitude
 * \param theTrueLatitude The tangential plane latitude
 */
// ----------------------------------------------------------------------

NFmiAzimuthalArea::NFmiAzimuthalArea(const double theRadialRange,
                                     const double theCentralLongitude,
                                     const NFmiPoint &theTopLeftXY,
                                     const NFmiPoint &theBottomRightXY,
                                     const double theCentralLatitude,
                                     const double theTrueLatitude,
                                     bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsTopRightLatLon(),
      itsBottomLeftLatLon(),
      itsBottomLeftWorldXY(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect(),
      itsRadialRange(theRadialRange),
      itsCentralLongitude(theCentralLongitude, usePacificView),
      itsCentralLatitude(theCentralLatitude),
      itsTrueLatitude(theTrueLatitude),
      itsTrueLatScaleFactor(0.0)
{
}

// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect Undocumented, unused
 */
// ----------------------------------------------------------------------

void NFmiAzimuthalArea::Init(bool /* fKeepWorldRect */)
{
  try
  {
    itsXScaleFactor = Width() / itsWorldRect.Width();
    itsYScaleFactor = Height() / itsWorldRect.Height();

    itsTrueLatScaleFactor =
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);

    itsTopRightLatLon = TopRightLatLon();
    itsBottomLeftLatLon = BottomLeftLatLon();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the geodetic latitude-longitude point corresponding the
 * rectangular LOCAL point theXYPoint. Point will be returned in degrees.
 *
 * \param theXYPoint The local xy-coordinates to be converted
 * \return The respective geodetic coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  try
  {
    double xWorld, yWorld;

    // Transform local xy-coordinates into world xy-coordinates (meters).

    xWorld = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
    yWorld = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;

    // Transform world xy-coordinates into geodetic coordinates.

    return WorldXYToLatLon(NFmiPoint(xWorld, yWorld));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the world XY coordinates corresponding the
 * rectangular LOCAL point theXYPoint.
 *
 * \param theXYPoint The local xy-coordinates to be converted
 * \return The respective world coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
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

// ----------------------------------------------------------------------
/*!
 * Returns the XY coordinates corresponding the
 * rectangular LOCAL point theWorldXYPoint.
 *
 * \param theWorldXYPoint The local world xy-coordinates to be converted
 * \return The respective XY coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::WorldXYToXY(const NFmiPoint &theWorldXYPoint) const
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
 * Transforms input world xy-coordinates into geodetic coordinates
 * (longitude,latitude) on globe. This proceeds in two steps:
 *
 *  -# transform world xy-coordinates into tangential world xy coordinates
 *  -# transform tangential world xy-coordinates into geodetic coordinates
 *
 * Some basic projection concepts:
 *
 *  - POLAR or NORMAL CASE means the central latitude is 90 degrees North,
 *    that is the North Pole
 *  - TANGENTIAL CASE means the true latitude is 90 degrees North,
 *    otherwise we talk about NON-TANGENTIAL or SECANT CASE
 *
 * \note
 * Applying non-polar case (central latitude other than 90 degrees North)
 * is possible but not tested in this program. Applying both non-tangential
 * projection plane -AND- non-polar case together is NOT ALLOWED in this program!
 * FMI defaults to following values: central latitude 90 deg North,
 * true latitude 60 deg North.
 *
 * \param theXYPoint The world xy-coordinates to be converted
 * \return The geodetic coordinates.
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::WorldXYToLatLon(const NFmiPoint &theXYPoint) const
{
  try
  {
    double lat0, lat, dlon, sinlat0, coslat0, coslat, delta;
    double sinDelta, cosDelta, B, cosB, xWorld, yWorld, xWorldTangential, yWorldTangential, xyDist;
    double dlonDeg, lonDeg, latDeg, trueLat, centralLat, centralLon;

    trueLat = itsTrueLatitude.Value();
    centralLat = itsCentralLatitude.Value();
    centralLon = itsCentralLongitude.Value();

    // See the second NOTE above
    if ((trueLat != 90.) && (centralLat != 90.))
      return NFmiPoint(kFloatMissing, kFloatMissing);

    // ----------------------------------------------------------------------------------------
    // STEP 1.
    // Transform world xy-coordinates (defined by itsTrueLatitude) onto the tangential xy-plane
    // ----------------------------------------------------------------------------------------

    DistanceFromPerspectivePointToCenterOfEarth();

    xWorld = theXYPoint.X();
    yWorld = theXYPoint.Y();

    xWorldTangential = 0.0;
    yWorldTangential = 0.0;

    if (trueLat == 90.)
    {
      // TANGENTIAL case

      xWorldTangential = xWorld;
      yWorldTangential = yWorld;
    }
    else
    {
      // NON-TANGENTIAL "secant" case

      // (D + kRearth)*xWorld/(D + kRearth*itsTrueLatitude.Sin());
      if (xWorld != 0.0)
        xWorldTangential = xWorld / itsTrueLatScaleFactor;

      // (D + kRearth)*yWorld/(D + kRearth*itsTrueLatitude.Sin());
      if (yWorld != 0.0)
        yWorldTangential = yWorld / itsTrueLatScaleFactor;
    }

    // ----------------------------------------------------------------------
    // STEP 2.
    // Transform tangential xy world coordinates into geodetic coordinates
    // ----------------------------------------------------------------------

    // XY distance measured between current point and the map center
    xyDist = sqrt(xWorldTangential * xWorldTangential + yWorldTangential * yWorldTangential);

    lat0 = FmiRad(centralLat);  // = Reference latitude = central latitude

    if (xyDist == 0.0)
      return NFmiPoint(centralLon, lat0);

    // Delta angle for the tangential plane depends on the tangential XY distance
    delta = CalcDelta(xyDist);

    if (centralLat == 90.)
    {
      // POLAR (=NORMAL) case

      // Compute absolute latitude

      // Fakta: lat = asin(cosdelta) = FmiRad(90) - delta
      lat = FmiRad(90) - delta;  // = asin(cosDelta);
      latDeg = FmiDeg(lat);

      // Compute longitude difference relative to central longitude

      if (yWorldTangential == 0.)
        dlonDeg = Sign(xWorldTangential) * 90.;
      else
      {
        // Fakta: dlon = asin((sinDelta*cosB)/cos(lat)) = atan(xWorldTangential/yWorldTangential)

        Sign(yWorldTangential) < 0
            ? dlonDeg =
                  Sign(xWorldTangential) * FmiDeg(atan(fabs(xWorldTangential / yWorldTangential)))
            : dlonDeg = Sign(xWorldTangential) *
                        (FmiDeg(atan(fabs(yWorldTangential / xWorldTangential))) + 90.);
      }

      lonDeg = NFmiLongitude(centralLon + dlonDeg, PacificView()).Value();
    }
    else
    {
      // HUOM! HUOM! Tätä "ei-polaaria" tapausta ei ole testattu!

      // NON-POLAR case

      // Azimuthal angle for the XY point ON THE TANGENTIAL MAP PLANE
      B = asin(yWorldTangential / xyDist);
      cosB = cos(B);

      sinlat0 = sin(lat0);
      coslat0 = cos(lat0);
      cosDelta = cos(delta);
      sinDelta = sin(delta);

      // Compute absolute latitude
      lat = asin(coslat0 * sinDelta * sin(B) + sinlat0 * cosDelta);
      coslat = cos(lat);
      latDeg = FmiDeg(lat);

      // Compute longitude difference relative to CentralLongitude
      dlon = asin((sinDelta * cosB) / coslat);
      dlonDeg = FmiDeg(dlon);

      // Quadrant check
      // VOI OLLA ETTEI TÄMÄ TARKASTELU OLE RIITTÄVÄ!
      if (xWorld > 0.0)
        lonDeg = NFmiLongitude(centralLon + dlonDeg, PacificView()).Value();
      else
        lonDeg = NFmiLongitude(centralLon - dlonDeg, PacificView()).Value();
    }

    return NFmiPoint(lonDeg, latDeg);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Transforms input geodetic coordinates (longitude,latitude) into world coordinates
 * (meters) on xy-plane cutting the Earth globe. This proceeds in two steps:
 *
 *  -# transform geodetic coordinates into TANGENTIAL world xy-coordinates
 *  -# transform tangential world xy-coordinates into final world xy coordinates
 *
 *
 * Some basic projection concepts:
 *
 *  - POLAR or NORMAL CASE means the central latitude is 90 degrees North, that
 *    is the North Pole
 *  - TANGENTIAL CASE means the true latitude is 90 degrees North, otherwise we talk
 *    about NON-TANGENTIAL or SECANT CASE
 *
 * \note
 * Aapplying non-polar case (central latitude other than 90 degrees North) is possible
 * but not tested in this program. Applying both non-tangential projection plane
 * -AND- non-polar case together is NOT ALLOWED in this program!
 *  FMI defaults to following values: central latitude 90 deg North, true
 * latitude 60 deg North.
 *
 * \param theLatLonPoint The geodetic coordinates to be converted
 * \return the world xy-coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    double k, lat0, lat, dlon, sinlat0, sinlat, coslat0, coslat, delta;
    double xWorldTangential, yWorldTangential;
    double xWorld, yWorld, trueLat, centralLat, centralLon;

    trueLat = itsTrueLatitude.Value();
    centralLat = itsCentralLatitude.Value();
    centralLon = itsCentralLongitude.Value();

    if ((trueLat != 90.) && (centralLat != 90.))
    {
      // Jos tullaan t�h�n, projektion parametrit eiv�t ole sallittuja, muuta centralLat arvoon
      // 90!!!!!!!
      assert(!((trueLat != 90.) && (centralLat != 90.)));
      return NFmiPoint::gMissingLatlon;
    }

    lat0 = FmiRad(centralLat);
    lat = FmiRad(theLatLonPoint.Y());
    dlon = FmiRad(theLatLonPoint.X() - centralLon);
    sinlat = sin(lat);
    coslat = cos(lat);

    DistanceFromPerspectivePointToCenterOfEarth();

    // ----------------------------------------------------------------------
    // STEP 1.
    // Transform geodetic coordinates into world coordinates on a xy-plane tangential to
    // the surface of Earth globe.
    // ----------------------------------------------------------------------

    if (centralLat == 90.)
    {
      // Polar (=normal) case, that is, lat0 = FmiRad(90 deg).
      // NOTE! This polar case COULD be computed just the way non-polar case is computed
      // by just assigning lat0 = FmiRad(90).
      // Howewer, since sin(lat0) == 1 and cos(lat0) == 0,
      // this is the reduced and somewhat faster way of computing the normal case:

      delta = sinlat;
      k = K(delta);
      if (k == kFloatMissing)
        return NFmiPoint::gMissingLatlon;

      // Fakta:  k*coslat*sin(dlon) = 2*kRearth*tan(0.5*(kPii/2 - lat))*sin(dlon)
      xWorldTangential = k * coslat * sin(dlon);
      // Fakta:  k*(-coslat*cos(dlon)) = -2*kRearth*tan(0.5*(kPii/2 - lat))*cos(dlon)
      yWorldTangential = k * (-coslat * cos(dlon));
    }
    else
    {
      // Non-polar case

      sinlat0 = sin(lat0);
      coslat0 = cos(lat0);

      delta = sinlat0 * sinlat + coslat0 * coslat * cos(dlon);
      k = K(delta);
      if (k == kFloatMissing)
        return NFmiPoint::gMissingLatlon;

      xWorldTangential = k * coslat * sin(dlon);
      yWorldTangential = k * (coslat0 * sinlat - sinlat0 * coslat * cos(dlon));
    }

    // ----------------------------------------------------------------------
    // STEP 2.
    // Transform tangential world xy-coordinates onto the xy-plane cutting Earth
    // globe. This cutting xy-plane is parallel to tangential xy-plane and is
    // defined by itsTrueLatitude
    // ----------------------------------------------------------------------

    xWorld = 0.0;
    yWorld = 0.0;

    if (trueLat == 90.)
    {
      // Cutting plane is already tangential to north pole - nothing to do.
      if (xWorldTangential != 0.0)
        xWorld = xWorldTangential;

      if (yWorldTangential != 0.0)
        yWorld = yWorldTangential;
    }
    else
    {
      // xWorldTangential*(D + kRearth*itsTrueLatitude.Sin())/(D + kRearth)
      if (xWorldTangential != 0.0)
        xWorld = xWorldTangential * itsTrueLatScaleFactor;

      // yWorldTangential*(D + kRearth*itsTrueLatitude.Sin())/(D + kRearth)
      if (yWorldTangential != 0.0)
        yWorld = yWorldTangential * itsTrueLatScaleFactor;
    }

    return NFmiPoint(xWorld, yWorld);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Transforms input geodetic coordinates (longitude,latitude) into local
 * (relative) coordinates on xy-plane.
 *
 * \param theLatLonPoint The geodetic coordinates to be converted.
 * \return The converted local xy-coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    double xLocal, yLocal;

    // Transform input geodetic coordinates into world coordinates (meters) on xy-plane.
    NFmiPoint latlon(FixLongitude(theLatLonPoint.X()), theLatLonPoint.Y());
    NFmiPoint xyWorld(LatLonToWorldXY(latlon));

    if (xyWorld == NFmiPoint::gMissingLatlon)
      return xyWorld;

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
 * Turns the input azimuth angle (in degrees) and radius (in meters)
 * into corresponding RELATIVE XY coordinates.
 *
 * \param theAzimuth The azimuth angle in degrees
 * \param theRadius The radius in meters
 * \return The respective relative xy-coordinates.
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::RadialXYPoint(double theAzimuth, double theRadius) const
{
  try
  {
    double startAngle;
    double radianAngle;

    // Start angle defaults to 0 deg. = North
    startAngle = 0.5 * kPii;

    radianAngle = startAngle - kPii * theAzimuth / 180.;

    return NFmiPoint(theRadius * cos(radianAngle), theRadius * sin(radianAngle));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Transforms input azimuth angle and radius into world coordinates on xy-plane.
 *
 * \param theAzimuth The azimuth angle
 * \param theRadius The radius
 * \return The respective world coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::LatLonToWorldXY(double theAzimuth, double theRadius) const
{
  try
  {
    return RadialXYPoint(theAzimuth, theRadius);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Transforms input azimuth angle and radius from world coordinates into
 * local (relative) coordinates on xy-plane.
 *
 * \param theAzimuth The azimuth angle
 * \param theRadius The radius
 * \return The respective local relative xy-coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::ToXY(double theAzimuth, double theRadius) const
{
  try
  {
    double xLocal, yLocal;

    // First, transform azimuth and radius into world coordinates (meters) on xy-plane.
    NFmiPoint xyWorld(LatLonToWorldXY(theAzimuth, theRadius));

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
 * Transforms input azimuth angle (in degrees) and radius (in meters)
 * from world coordinates into geodetic (latitude-longitude) coordinates.
 *
 * \param theAzimuth The azimuth angle in degrees
 * \param theRadius The radius in meters
 * \return The respective geodetic coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::ToLatLon(double theAzimuth, double theRadius) const
{
  try
  {
    return WorldXYToLatLon(RadialXYPoint(theAzimuth, theRadius));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *  Returns the geodetic center point
 *
 * \return The geodetic center point
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiAzimuthalArea::CurrentCenter() const
{
  try
  {
    return NFmiPoint(itsCentralLongitude.Value(), itsCentralLatitude.Value());
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
 * \param theArea The other area being copied
 * \return Referencee to the assigned value.
 * \todo Should protect from self assignment, since NFmiArea::operator=
 *       cannot be trusted here.
 */
// ----------------------------------------------------------------------

NFmiAzimuthalArea &NFmiAzimuthalArea::operator=(const NFmiAzimuthalArea &theArea)
{
  try
  {
    NFmiArea::operator=(theArea);

    itsBottomLeftLatLon = theArea.itsBottomLeftLatLon;
    itsTopRightLatLon = theArea.itsTopRightLatLon;
    itsCentralLongitude.SetValue(theArea.itsCentralLongitude.Value());
    itsCentralLatitude.SetValue(theArea.itsCentralLatitude.Value());
    itsTrueLatitude.SetValue(theArea.itsTrueLatitude.Value());
    itsXScaleFactor = theArea.itsXScaleFactor;
    itsYScaleFactor = theArea.itsYScaleFactor;
    itsBottomLeftWorldXY = theArea.itsBottomLeftWorldXY;
    itsWorldRect = theArea.itsWorldRect;
    itsRadialRange = theArea.itsRadialRange;
    itsTrueLatScaleFactor = theArea.itsTrueLatScaleFactor;

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
 * \param theArea The other area being compared to
 * \return True, if the areas are equivalent
 * \todo Investigate whether NFmiArea::operator== should also be called.
 */
// ----------------------------------------------------------------------

bool NFmiAzimuthalArea::operator==(const NFmiAzimuthalArea &theArea) const
{
  try
  {
    if ((itsBottomLeftLatLon == theArea.itsBottomLeftLatLon) &&
        (itsTopRightLatLon == theArea.itsTopRightLatLon) &&
        (itsCentralLongitude.Value() == theArea.itsCentralLongitude.Value()) &&
        (itsCentralLatitude.Value() == theArea.itsCentralLatitude.Value()) &&
        (itsTrueLatitude.Value() == theArea.itsTrueLatitude.Value()) &&
        (itsXScaleFactor == theArea.itsXScaleFactor) &&
        (itsYScaleFactor == theArea.itsYScaleFactor) &&
        (itsBottomLeftWorldXY == theArea.itsBottomLeftWorldXY) &&
        (itsWorldRect == theArea.itsWorldRect) && (itsRadialRange == theArea.itsRadialRange))
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
 * \param theArea The other area being compared to
 * \return True, if the areas are not equivalent
 */
// ----------------------------------------------------------------------

bool NFmiAzimuthalArea::operator!=(const NFmiAzimuthalArea &theArea) const
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
 * Equality comparison with an area derived from NFmiAzimuthalArea.
 *
 * \param theArea The other area being compared to
 * \return True, if the NFmiAzimuthalArea parts are equivalent
 * \todo Use static_cast instead of C-style cast
 */
// ----------------------------------------------------------------------

bool NFmiAzimuthalArea::operator==(const NFmiArea &theArea) const
{
  try
  {
    return *this == static_cast<const NFmiAzimuthalArea &>(theArea);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison with an area derived from NFmiAzimuthalArea.
 *
 * \param theArea The other area being compared to
 * \return True, if the NFmiAzimuthalArea parts are not equivalent
 */
// ----------------------------------------------------------------------

bool NFmiAzimuthalArea::operator!=(const NFmiArea &theArea) const
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
 * Write the projection to the given stream
 *
 * \param file The output stream to write to.
 * \return The output stream written to.
 */
// ----------------------------------------------------------------------

std::ostream &NFmiAzimuthalArea::Write(std::ostream &file) const
{
  try
  {
    NFmiArea::Write(file);

    file << itsBottomLeftLatLon << itsTopRightLatLon << itsCentralLongitude.Value() << endl
         << itsCentralLatitude.Value() << endl
         << itsTrueLatitude.Value() << endl;
    int oldPrec = file.precision();
    file.precision(15);

    // We trust everything to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 5)
    {
      file << itsRadialRange << " 0"
           << " 0" << endl;
      file << itsWorldRect << ' ';
    }
    else
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
 * Replace the projection with the one in the given input stream.
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiAzimuthalArea::Read(std::istream &file)
{
  try
  {
    double centralLatitude, trueLatitude, centralLongitude;

    NFmiArea::Read(file);

    file >> itsBottomLeftLatLon;
    file >> itsTopRightLatLon;
    PacificView(NFmiArea::IsPacificView(itsBottomLeftLatLon, itsTopRightLatLon));
    file >> centralLongitude;
    file >> centralLatitude;
    file >> trueLatitude;
    // We trust everything to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 5)
    {
      unsigned long dummy;
      file >> itsRadialRange >> dummy >> dummy;
    }

    itsCentralLongitude.SetValue(centralLongitude);
    itsCentralLatitude.SetValue(centralLatitude);
    itsTrueLatitude.SetValue(trueLatitude);

    file >> itsWorldRect;

    Init();

    return file;
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

std::size_t NFmiAzimuthalArea::HashValue() const
{
  try
  {
    std::size_t hash = NFmiArea::HashValue();
    Fmi::hash_combine(hash, itsTopRightLatLon.HashValue());
    Fmi::hash_combine(hash, itsBottomLeftLatLon.HashValue());
    Fmi::hash_combine(hash, itsBottomLeftWorldXY.HashValue());
    Fmi::hash_combine(hash, Fmi::hash_value(itsXScaleFactor));
    Fmi::hash_combine(hash, Fmi::hash_value(itsYScaleFactor));
    Fmi::hash_combine(hash, itsWorldRect.HashValue());
    Fmi::hash_combine(hash, Fmi::hash_value(itsRadialRange));
    Fmi::hash_combine(hash, itsCentralLongitude.HashValue());
    Fmi::hash_combine(hash, itsCentralLatitude.HashValue());
    Fmi::hash_combine(hash, itsTrueLatitude.HashValue());
    Fmi::hash_combine(hash, Fmi::hash_value(itsTrueLatScaleFactor));

    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
