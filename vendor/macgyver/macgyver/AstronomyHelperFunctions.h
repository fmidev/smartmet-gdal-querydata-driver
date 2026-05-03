// ======================================================================
/*!
 * \brief Astronomical calculations
 *
 * Solar calculations
 *
 * Based on NOAA JavaScript at
 *       <http://www.srrb.noaa.gov/highlights/sunrise/azel.html>
 *       <http://www.srrb.noaa.gov/highlights/sunrise/sunrise.html>
 *
 * Reference:
 *       Solar Calculation Details
 *       <http://www.srrb.noaa.gov/highlights/sunrise/calcdetails.html>
 *
 * Transformation to C++ by Ilmatieteen Laitos, 2008.
 *
 * License:
 *       UNKNOWN (not stated in JavaScript)
 */
// ======================================================================

#pragma once

#include "Exception.h"
#include "DateTime.h"
#include "LocalDateTime.h"
#include <cmath>

#include <boost/math/constants/constants.hpp>

namespace Fmi
{
namespace Astronomy
{
#define INTDIV(x) (x)

inline double rad2deg(double rad)
{
  return rad * boost::math::constants::radian<double>();
}
inline double deg2rad(double deg)
{
  return deg * boost::math::constants::degree<double>();
}
inline double sin_deg(double deg)
{
  return sin(deg2rad(deg));
}
inline double cos_deg(double deg)
{
  return cos(deg2rad(deg));
}
inline double tan_deg(double deg)
{
  return tan(deg2rad(deg));
}
/* Clamp to range [a,b] */
inline void clamp_to(double& v, double a, double b)
{
  v = (v < a) ? a : (v > b) ? b : v;
}
/*
 * Check 'lon' and 'lat' parameters for validity; clamp to (-180,180] and
 * [-89.8,89.9] range before calculations.
 */
inline void check_lonlat(double& lon, double& lat)
{
  if (fabs(lon) > 180.0)
    throw Fmi::Exception(BCP, "Longitude must be in range [-180,180]");

  if (fabs(lat) > 90.0)
    throw Fmi::Exception(BCP, "Latitude must be in range [-90,90]");

  clamp_to(lat, -89.8, 89.8);  // exclude poles
}

/*
 * hour angle of the Sun at sunrise for the latitude
 *
 * Returns: hour angle of sunrise/set in radians ('nan' if no sunrise/set)
 */

inline double HourAngleSunrise_or_set(double lat, double solarDec, bool rise)
{
  double ha =
      acos(cos_deg(90.833) / (cos_deg(lat) * cos_deg(solarDec)) - tan_deg(lat) * tan_deg(solarDec));

  return rise ? ha : -ha;  // rad
}

inline double rad(double d)
{
  return d * 0.017453292519943295;
}
inline double Deg(double d1)
{
  return (d1 * 180) / 3.1415926535897931;
}
inline double julianDay(const DateTime& utc)
{
  double d3 = utc.time_of_day().total_seconds();
  double d1 = utc.date().day() + d3 / 86400;
  int month = utc.date().month();
  int year = utc.date().year();
  if (month <= 2)
  {
    month += 12;
    year--;
  }
  int k1 = year / 100;
  int l1 = (2 - k1) + k1 / 4;
  double d2 = static_cast<long>(365.25 * (year + 4716)) +
              static_cast<long>(30.600100000000001 * (month + 1)) + d1 + l1 + -1524.5;
  return d2;
}

inline double reduce(double d1)
{
  d1 -= 6.2831853071795862 * static_cast<int>(d1 / 6.2831853071795862);
  if (d1 < 0.0)
    d1 += 6.2831853071795862;
  return d1;
}

/**
 * Takes the day, month, year and hours in the day and returns the
 * modified julian day number defined as mjd = jd - 2400000.5
 * checked OK for Greg era dates - 26th Dec 02
 */
inline double modifiedJulianDate(short month, short day, short year)
{
  if (month <= 2)
  {
    month += 12;
    year--;
  }

  double a = 10000.0 * year + 100.0 * month + day;
  double b = 0.0;
  if (a <= 15821004.1)
  {
    b = -2 * (int)((year + 4716) / 4) - 1179;
  }
  else
  {
    b = (int)(year / 400) - (int)(year / 100) + (int)(year / 4);
  }

  a = 365 * year - 679004;

  return a + b + (int)(30.6001 * (month + 1)) + day;
}

}  // namespace Astronomy
}  // namespace Fmi

// ======================================================================
