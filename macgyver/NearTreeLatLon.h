#pragma once

#include <boost/math/constants/constants.hpp>

#include <cmath>
#include <cstddef>
#include <limits>

/*
 * \class Fmi::NearTreeLatLon
 * \brief A proxy object to enable fast NearTree searches for latlon coordinates
 *
 * The basic idea is to convert the latlon coordinate to XYZ coordinates and
 * use the regular Euclidian distance formula instead of the Haversine formula
 * for distance along the surface. The actual distance along the surface is
 * easily converted to the equivalent chord length in 3D, and hence trigonometry
 * is only needed in constructing the search tree and in converting the desired
 * search distance to a an equivalent 3D search distance.
 *
 * We assume the user uses an ID to identify the points converted to NearTreeLatLon objects.
 */

namespace Fmi
{
template <typename T>
class NearTreeLatLon
{
  static constexpr double earth_radius = 6371.220;

 public:
  NearTreeLatLon(double theLon, double theLat) : itsID() { init(theLon, theLat); }
  NearTreeLatLon(double theLon, double theLat, const T& theID) : itsID(theID)
  {
    init(theLon, theLat);
  }

  double X() const { return itsX; }
  double Y() const { return itsY; }
  double Z() const { return itsZ; }
  const T& ID() const { return itsID; }
  static NearTreeLatLon createMaxID(double theLon, double theLat)
  {
    return NearTreeLatLon(theLon, theLat, std::numeric_limits<T>::max());
  }

  static NearTreeLatLon createMinID(double theLon, double theLat)
  {
    return NearTreeLatLon(theLon, theLat, std::numeric_limits<T>::min());
  }

  static double ChordLength(double theLength, double theRadius = earth_radius)
  {
    // First we make sure the search distance does not exceed half the circumference of Earth
    double limit = std::min(theLength, theRadius * boost::math::double_constants::pi);

    // Ref: https://en.wikipedia.org/wiki/Great-circle_distance
    return 2 * sin(limit / (2 * theRadius));
  }

  static double SurfaceLength(double theChord, double theRadius = earth_radius)
  {
    return 2 * theRadius * asin(theChord / 2);
  }

 private:
  NearTreeLatLon() = delete;

  void init(double theLon, double theLat)
  {
    // convert degrees to radians
    double lon = theLon * boost::math::double_constants::degree;
    double lat = theLat * boost::math::double_constants::degree;

    // spherical coordinates to xyz
    itsX = cos(lat) * cos(lon);
    itsY = cos(lat) * sin(lon);
    itsZ = sin(lat);
  }

  double itsX;
  double itsY;
  double itsZ;
  T itsID = T();
};

/*
 * \class Fmi::NearTreeLatLonDistance
 * \brief A distance calculator for NearTreeLatLon objects
 *
 * Note: The square root cannot be optimized away from distance
 * comparisons since the triangle inequality must hold while searching
 * the NearTree.
 */

template <typename T>
class NearTreeLatLonDistance
{
 public:
  double operator()(const T& theLhs, const T& theRhs) const
  {
    double dx = theLhs.X() - theRhs.X();
    double dy = theLhs.Y() - theRhs.Y();
    double dz = theLhs.Z() - theRhs.Z();
    return std::sqrt(dx * dx + dy * dy + dz * dz);
  }
};

}  // namespace Fmi
