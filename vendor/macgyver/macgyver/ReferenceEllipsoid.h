/**
 *
 * @brief Conversion between geodetic coordinates on reference ellipsoid
 *        and geocentric coordinates
 *
 */

#pragma once

#include <array>
#include <cmath>

namespace Fmi
{
/**
 * @brief Conversion between geodetic coordinates on reference ellipsoid
 *        and geocentric coordinates
 */
class ReferenceEllipsoid
{
 public:
  /**
   *   @brief Constructor for reference ellipsoid object
   *
   *   @param theA Semimajor axis or reference ellipsoid
   *   @param theF Flattening of ellipsoid (note - @b not inverse flattening)
   */
  ReferenceEllipsoid(double theA, double theF);

  /**
   *   @brief Converts provided geodetic coordinates to geocentric ones
   *
   *   @param lat the latitude (radians)
   *   @param lon the latitude (radians)
   *   @param height the height above reference ellipsoid
   *   @return Geocentric coordinates as std::array of size 3
   */
  std::array<double, 3> to_geocentric(double lat, double lon, double height = 0) const;

  /**
   *   @brief Converts provided geocentric coordinates to geodetic ones
   */
  void to_geodetic(const std::array<double, 3> &x,
                   double *lat,
                   double *lon,
                   double *height = nullptr) const;

  double transverse_curvature_radius(double lat) const;

  double meridional_curvature_radius(double lat) const;

  double conformal_tangent_sphere_radius(double lat) const;

  double get_semimajor_axis() const { return a; }
  double get_eccentricity() const { return sqrt(e2); }

 private:
  double a;
  double e2;

 public:
  static const ReferenceEllipsoid wgs84;
};
}  // namespace Fmi
