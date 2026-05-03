// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiGeoTools
 */
// ======================================================================
/*!
 * \namespace NFmiGeoTools
 *
 * \brief Various geographic utility functions
 */
// ======================================================================

#include "NFmiGeoTools.h"
#include "NFmiGlobals.h"
#include <macgyver/Exception.h>
#include <algorithm>
#include <cmath>

namespace NFmiGeoTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Cartesian 2D distance
 *
 * \param theX1 The x-coordinate of point 1
 * \param theY1 The y-coordinate of point 1
 * \param theX2 The x-coordinate of point 2
 * \param theY2 The y-coordinate of point 2
 * \return The distance between the points
 */
// ----------------------------------------------------------------------

double Distance(double theX1, double theY1, double theX2, double theY2)
{
  return sqrt((theX2 - theX1) * (theX2 - theX1) + (theY2 - theY1) * (theY2 - theY1));
}

// ----------------------------------------------------------------------
/*!
 * \brief Cartesian 3D distance
 *
 * \param theX1 The x-coordinate of point 1
 * \param theY1 The y-coordinate of point 1
 * \param theZ1 The z-coordinate of point 1
 * \param theX2 The x-coordinate of point 2
 * \param theY2 The y-coordinate of point 2
 * \param theZ2 The z-coordinate of point 2
 * \return The distance between the points
 */
// ----------------------------------------------------------------------

double Distance(double theX1, double theY1, double theZ1, double theX2, double theY2, double theZ2)
{
  return sqrt((theX2 - theX1) * (theX2 - theX1) + (theY2 - theY1) * (theY2 - theY1) +
              (theZ2 - theZ1) * (theZ2 - theZ1));
}

// ----------------------------------------------------------------------
/*!
 * \brief Distance between two points along earth surface
 *
 * \param theLon1 Longitude of point 1
 * \param theLat1 Latitude of point 1
 * \param theLon2 Longitude of point 2
 * \param theLat2 Latitude of point 2
 * \return The distance in kilometers
 *
 *  Haversine Formula (from R.W. Sinnott, "Virtues of the Haversine",
 *  Sky and Telescope, vol. 68, no. 2, 1984, p. 159)
 *  will give mathematically and computationally exact results. The
 *  intermediate result c is the great circle distance in radians. The
 *  great circle distance d will be in the same units as R.
 *
 *  When the two points are antipodal (on opposite sides of the Earth),
 *  the Haversine Formula is ill-conditioned, but the error, perhaps
 *  as large as 2 km (1 mi), is in the context of a distance near
 *  20,000 km (12,000 mi). Further, there is a possibility that roundoff
 *  errors might cause the value of sqrt(a) to exceed 1.0, which would
 *  cause the inverse sine to crash without the bulletproofing provided by
 *  the min() function.
 *
 * The code was taken from NFmiLocation::Distance
 */
// ----------------------------------------------------------------------

double GeoDistance(double theLon1, double theLat1, double theLon2, double theLat2)
{
  try
  {
    using namespace std;

    double lo1 = FmiRad(theLon1);
    double la1 = FmiRad(theLat1);

    double lo2 = FmiRad(theLon2);
    double la2 = FmiRad(theLat2);

    double dlon = lo2 - lo1;
    double dlat = la2 - la1;
    double sindlat = sin(dlat / 2);
    double sindlon = sin(dlon / 2);

    double a = sindlat * sindlat + cos(la1) * cos(la2) * sindlon * sindlon;
    double help1 = sqrt(a);
    double c = 2. * asin(std::min(1., help1));

    return kRearth * c;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Cartesian distance of 2D point from line segment
 *
 * The formulas were taken from
 * <a
 * href="http://astronomy.swin.edu.au/~pbourke/geometry/pointline/">http://astronomy.swin.edu.au/~pbourke/geometry/pointline/</a>
 *
 * \param theX The x-coordinate of the point
 * \param theY The y-coordinate of the point
 * \param theX1 The first x-coordinate of the line
 * \param theY1 The first y-coordinate of the line
 * \param theX2 The second x-coordinate of the line
 * \param theY2 The second y-coordinate of the line
 * \return The distance
 */
// ----------------------------------------------------------------------

double DistanceFromLineSegment(
    double theX, double theY, double theX1, double theY1, double theX2, double theY2)
{
  try
  {
    // Length of the line itself

    const double length = Distance(theX1, theY1, theX2, theY2);

    // The special case when the line is a dot is handled quickly
    if (length <= 0)
      return Distance(theX, theY, theX1, theY1);

    // Intersection point in units of 0-1

    const double u =
        ((theX - theX1) * (theX2 - theX1) + (theY - theY1) * (theY2 - theY1)) / (length * length);

    // The nearest point is on the line if u=0..1
    if (u >= 0 && u <= 1)
      return Distance(theX, theY, (1 - u) * theX1 + u * theX2, (1 - u) * theY1 + u * theY2);

    // Otherwise the nearest point is the nearer end point

    if (u < 0)
      return Distance(theX, theY, theX1, theY1);
    else
      return Distance(theX, theY, theX2, theY2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace NFmiGeoTools

// ======================================================================
