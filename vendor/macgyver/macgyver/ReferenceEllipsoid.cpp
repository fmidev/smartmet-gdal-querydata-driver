#include "ReferenceEllipsoid.h"
#include <algorithm>
#include <cmath>

const Fmi::ReferenceEllipsoid Fmi::ReferenceEllipsoid::wgs84(6378137.0, 1.0 / 298.257223563);

Fmi::ReferenceEllipsoid::ReferenceEllipsoid(double theA, double theF)
    : a(theA), e2(theF * (2 - theF))
{
}

std::array<double, 3> Fmi::ReferenceEllipsoid::to_geocentric(double lat,
                                                               double lon,
                                                               double h) const
{
  std::array<double, 3> x;
  double N = transverse_curvature_radius(lat);
  x[0] = (N + h) * cos(lat) * cos(lon);
  x[1] = (N + h) * cos(lat) * sin(lon);
  x[2] = ((1 - e2) * N + h) * sin(lat);
  return x;
}

void Fmi::ReferenceEllipsoid::to_geodetic(const std::array<double, 3> &x,
                                          double *lat,
                                          double *lon,
                                          double *height) const
{
  *lon = atan2(x[1], x[0]);

  const double xy = hypot(x[0], x[1]);
  double v;
  double prev_lat;
  double h = 0;
  *lat = atan2(x[2], xy);

  do
  {
    prev_lat = *lat;
    v = transverse_curvature_radius(*lat);
    *lat = atan2(x[2] + v * e2 * sin(*lat), xy);
    h = std::abs(*lat) < 1 ? xy / cos(*lat) - v : x[2] / sin(*lat) - v * (1 - e2);
  } while (std::abs(*lat - prev_lat) > 1e-10);

  if (height)
  {
    *height = h;
  }
}

double Fmi::ReferenceEllipsoid::transverse_curvature_radius(double lat) const
{
  return a / sqrt(1 - e2 * sin(lat) * sin(lat));
}

double Fmi::ReferenceEllipsoid::meridional_curvature_radius(double lat) const
{
  double w = 1 - e2 * sin(lat) * sin(lat);
  return a * (1 - e2) / (w * sqrt(w));
}

double Fmi::ReferenceEllipsoid::conformal_tangent_sphere_radius(double lat) const
{
  return a * sqrt(1 - e2) / (1 - e2 * sin(lat) * sin(lat));
}
