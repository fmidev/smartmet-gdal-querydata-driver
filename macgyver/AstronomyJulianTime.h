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

#include "AstronomyHelperFunctions.h"

namespace Fmi
{
namespace Astronomy
{
#ifdef _MSC_VER
#define isnan _isnan
#endif

/*=== Julian time =====================*/

/*
 * Julian Day presentation (days since 4713 BC), with fractions leading
 * into sub-day timings.
 *
 * NOTE: Julian day changes at UTC noon (in other words, our dates are "JD(UT)").
 */
class JulianTime
{
 private:
  double jd;  // Julian day, with fractions within the day

 public:
  /*
   * Construction from a Julian day number
   */
  JulianTime(double jd_ = 0.0) : jd(jd_) {}
  JulianTime(const JulianTime& other) = default;
  JulianTime& operator=(const JulianTime& other)
  {
    if (this != &other)
      jd = other.jd;
    return *this;
  }
  bool operator==(const JulianTime& other) { return jd == other.jd; }
  bool operator!=(const JulianTime& other) { return jd != other.jd; }
  /*
   * Construction from UTC date and time
   *
   * Note: -0.5 is required since Gregorian days start at midnight, Julian
   *       at noon. The beginning of a Gregorian day is -12h on the earlier
   *       Julian day.
   */
  JulianTime(const DateTime& utc);

  double JulianDay() const { return jd; }
  bool valid() const { return jd != 0.0; }
  /*
   * Convert to Gregorian UTC
   */
  DateTime ptime_utc() const;
  /*
   * Convert to local Gregorian time
   */
  Fmi::LocalDateTime ldt(const Fmi::TimeZonePtr& tz) const;

  /* Back/forward 24hrs
   */
  JulianTime& operator--()
  {
    jd--;
    return *this;
  }
  JulianTime& operator++()
  {
    jd++;
    return *this;
  }

  JulianTime& operator+=(double diff)
  {
    if (jd != 0)
      jd += diff;
    return *this;
  }
  JulianTime& operator-=(double diff)
  {
    if (jd != 0)
      jd -= diff;
    return *this;
  }

  /*
   * Calculation functions
   *
   * These are made static, 't' passed through since most calculations need it.
   */

  /*
   * Julian century since J2000.0
   */
  double T() const;
  static double GeomMeanAnomalySun(double t);
  static double SunEqOfCenter(double t);
  static double SunTrueAnomaly(double t);
  static double EccentricityEarthOrbit(double t);
  static double MeanObliquityOfEcliptic(double t);
  static double ObliquityCorrection(double t);
  /*
   * Distance to the Sun in AU
   */
  static double SunRadVector(double t);
  static double GeomMeanLongSun(double t);
  static double SunTrueLong(double t);
  static double SunApparentLong(double t);
  /*
   * Right ascension of the Sun
   */
  static double SunRtAscension(double t);
  /*
   * Declination of the Sun
   */
  double SunDeclination() const;
  /*
   * Difference between true solar time and mean solar time (in minutes of time)
   */
  double EquationOfTime() const;
};  // JulianTime

/*
 * Solar noon for the given day at the given location on Earth
 */
JulianTime SolNoon(const Fmi::LocalDateTime& ldt, double lon_e);

/*
 * UTC of sunrise/sunset for the given day at the given location on Earth
 *
 * Returns:  time in minutes from zero Z, "empty" JulianTime if no sunrise/set
 */
JulianTime Sunrise_or_set_UTC(const JulianTime& noon, double lon_e, double lat, bool rise);

}  // namespace Astronomy
}  // namespace Fmi

// ======================================================================
