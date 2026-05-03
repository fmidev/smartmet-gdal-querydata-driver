// ======================================================================
/*!
 * \file
 * \brief Namespace Fmi::Geometry
 */
// ======================================================================

#pragma once

#include <utility>

namespace Fmi
{
namespace Geometry
{
// Radian conversions

double Radians(double theDegrees);
double Degrees(double theRadians);

// Cartesian distance

double Distance(double theX1, double theY1, double theX2, double theY2);

double Distance(double theX1, double theY1, double theZ1, double theX2, double theY2, double theZ2);

// Distance along earth surface

double GeoDistance(double theLon1, double theLat1, double theLon2, double theLat2);

// Distance from line segment

double DistanceFromLineSegment(
    double theX, double theY, double theX1, double theY1, double theX2, double theY2);

// Bearing

double Bearing(double theLon1, double theLat1, double theLon2, double theLat2);
}  // namespace Geometry
}  // namespace Fmi

// ======================================================================
