#include "HelmertTransformation.h"
#include "Exception.h"
#include <boost/math/constants/constants.hpp>
#include <cstdio>
#include <sstream>
#include <stdexcept>

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ BOOST_CURRENT_FUNCTION
#endif

Fmi::HelmertTransformation::HelmertTransformation() : m(1), ex(0), ey(0), ez(0), tx(0), ty(0), tz(0)
{
}

std::array<double, 3> Fmi::HelmertTransformation::operator()(
    const std::array<double, 3>& x) const
{
  try
  {
    std::array<double, 3> y = {m * (x[0] + (-ez) * x[1] + (ey)*x[2]) + tx,
                                 m * ((ez)*x[0] + x[1] + (-ex) * x[2]) + ty,
                                 m * ((-ey) * x[0] + (ex)*x[1] + x[2]) + tz};
    return y;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Fmi::HelmertTransformation::set_fmi_sphere_to_reference_ellipsoid_conv(
    double r,
    double lat,
    double lon,
    const ReferenceEllipsoid& ref,
    enum FmiSphereConvScalingType scaling_type)
{
  try
  {
    ex = 0;
    ey = 0;
    ez = 0;
    if (scaling_type == FMI_SPHERE_NO_SCALING)
    {
      m = 1;
      const std::array<double, 3> x0 = ref.to_geocentric(lat, lon, -r);
      tx = x0[0];
      ty = x0[1];
      tz = x0[2];
    }
    else if (scaling_type == FMI_SPHERE_PRESERVE_EAST_WEST_SCALE)
    {
      const double a = ref.get_semimajor_axis();
      const double e = ref.get_eccentricity();
      const double v = sqrt(1 - e * e * sin(lat) * sin(lat));
      m = a / (r * v);
      tx = 0;
      ty = 0;
      tz = -a * e * e * sin(lat) / v;
    }
    else if (scaling_type == FMI_SPHERE_PRESERVE_SOUTH_NORTH_SCALE)
    {
      const double a = ref.get_semimajor_axis();
      const double e = ref.get_eccentricity();
      const double v = sqrt(1 - e * e * sin(lat) * sin(lat));
      const double sf = sin(lat);
      const double cf = cos(lat);
      m = (a * (1 - e * e)) / (r * v * v * v);
      tx = a * e * e * cos(lon) * cf * cf * cf / (v * v * v);
      ty = a * e * e * sin(lon) * cf * cf * cf / (v * v * v);
      tz = -a * e * e * (1 - e * e) * sf * sf * sf / (v * v * v);
    }
    else
    {
      std::ostringstream msg;
      msg << __PRETTY_FUNCTION__ << ": wrong scaling type " << scaling_type;
      throw Fmi::Exception(BCP, msg.str());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Fmi::HelmertTransformation::set_reference_ellipsoid_to_fmi_sphere_conv(
    double r,
    double lat,
    double lon,
    const ReferenceEllipsoid& ref,
    enum FmiSphereConvScalingType scaling_type)
{
  try
  {
    ex = 0;
    ey = 0;
    ez = 0;
    if (scaling_type == FMI_SPHERE_NO_SCALING)
    {
      m = 1;
      const std::array<double, 3> x0 = ref.to_geocentric(lat, lon, -r);
      tx = -x0[0];
      ty = -x0[1];
      tz = -x0[2];
    }
    else if (scaling_type == FMI_SPHERE_PRESERVE_EAST_WEST_SCALE)
    {
      const double a = ref.get_semimajor_axis();
      const double e = ref.get_eccentricity();
      const double v = sqrt(1 - e * e * sin(lat) * sin(lat));
      m = r * v / a;
      tx = 0;
      ty = 0;
      tz = r * e * e * sin(lat);
    }
    else if (scaling_type == FMI_SPHERE_PRESERVE_SOUTH_NORTH_SCALE)
    {
      const double a = ref.get_semimajor_axis();
      const double e = ref.get_eccentricity();
      const double v = sqrt(1 - e * e * sin(lat) * sin(lat));
      const double sf = sin(lat);
      const double cf = cos(lat);
      m = r * v * v * v / (a * (1 - e * e));
      tx = -r * e * e * cos(lon) * cf * cf * cf / (1 - e * e);
      ty = -r * e * e * sin(lon) * cf * cf * cf / (1 - e * e);
      tz = r * e * e * sf * sf * sf;
    }
    else
    {
      std::ostringstream msg;
      msg << __PRETTY_FUNCTION__ << ": wrong scaling type " << scaling_type;
      throw Fmi::Exception(BCP, msg.str());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string Fmi::get_fmi_sphere_towgs84_proj4_string(
    double r,
    double lat,
    double lon,
    enum Fmi::HelmertTransformation::FmiSphereConvScalingType scaling_type)
{
  try
  {
    const double AS = 180.0 * 3600.0 / boost::math::constants::pi<double>();
    Fmi::HelmertTransformation conv;
    conv.set_fmi_sphere_to_reference_ellipsoid_conv(
        r, lat, lon, Fmi::ReferenceEllipsoid::wgs84, scaling_type);
    char buffer[512];
#ifndef _MSC_VER
    snprintf(buffer,
             sizeof(buffer),
#else
    _snprintf(buffer,
              sizeof(buffer),
#endif
             "+towgs84=%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.0f ",
             conv.tx,
             conv.ty,
             conv.tz,
             AS * conv.ex,
             AS * conv.ey,
             AS * conv.ez,
             1e6 * (conv.m - 1));
    buffer[sizeof(buffer) - 1] = 0;
    return buffer;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
