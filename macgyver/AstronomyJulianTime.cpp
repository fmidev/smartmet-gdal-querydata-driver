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

#include "AstronomyJulianTime.h"
#include "Exception.h"
#include "LocalDateTime.h"

using Fmi::Date;
using Fmi::LocalDateTime;
using Fmi::TimeZonePtr;

namespace Fmi
{
namespace Astronomy
{
/*
 * Construction from UTC date and time
 *
 * Note: -0.5 is required since Gregorian days start at midnight, Julian
 *       at noon. The beginning of a Gregorian day is -12h on the earlier
 *       Julian day.
 */
JulianTime::JulianTime(const Fmi::DateTime& utc)
{
  try
  {
    jd = (utc.date().julian_day() - 0.5 + utc.time_of_day().total_seconds() / (24.0 * 3600));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
 * Convert to Gregorian UTC
 */
Fmi::DateTime JulianTime::ptime_utc() const
{
  try
  {
    if (jd == 0.0)
      return Fmi::DateTime();

    int z = lround(jd);
    double f = (jd + 0.5) - z;

    int A;
    if (z < 2299161)
    {
      A = z;
    }
    else
    {
      int alpha = (int)((z - 1867216.25) / 36524.25);
      A = z + 1 + alpha - INTDIV(alpha / 4);
    }

    int B = A + 1524;
    int C = (int)((B - 122.1) / 365.25);
    int D = (int)(365.25 * C);
    int E = (int)((B - D) / 30.6001);

    auto day = static_cast<std::uint16_t>(B - D - (int)floor(30.6001 * E));
    auto month = static_cast<std::uint16_t>((E < 14) ? E - 1 : E - 13);
    auto year = static_cast<std::uint16_t>((month > 2) ? C - 4716 : C - 4715);

    Fmi::Date date(year, month, day);

    auto ss = (unsigned)floor(f * (24 * 60 * 60));  // f: [0,1)

    return Fmi::DateTime(date, Fmi::Seconds(ss));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
 * Convert to local Gregorian time
 */
Fmi::LocalDateTime JulianTime::ldt(const Fmi::TimeZonePtr& tz) const
{
  try
  {
    /*
     * When constructed this way, Boost takes 'ptime' as UTC.
     */
    return Fmi::LocalDateTime(ptime_utc(), tz);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
 * Calculation functions
 *
 * These are made static, 't' passed through since most calculations need it.
 */

/*
 * Julian century since J2000.0
 */
double JulianTime::T() const
{
  try
  {
    return (jd - 2451545.0) / 36525.0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double JulianTime::GeomMeanAnomalySun(double t)
{
  try
  {
    return 357.52911 + t * (35999.05029 - 0.0001537 * t);  // degrees
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double JulianTime::SunEqOfCenter(double t)
{
  try
  {
    double mrad = deg2rad(GeomMeanAnomalySun(t));
    double sinm = sin(mrad);
    double sin2m = sin(mrad * 2.0);
    double sin3m = sin(mrad * 3.0);
    return (sinm * (1.914602 - t * (0.004817 + 0.000014 * t)) + sin2m * (0.019993 - 0.000101 * t) +
            sin3m * 0.000289);  // degrees
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double JulianTime::SunTrueAnomaly(double t)
{
  try
  {
    return GeomMeanAnomalySun(t) + SunEqOfCenter(t);  // degrees
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double JulianTime::EccentricityEarthOrbit(double t)
{
  try
  {
    return 0.016708634 - t * (0.000042037 + 0.0000001267 * t);  // unitless
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double JulianTime::MeanObliquityOfEcliptic(double t)
{
  try
  {
    double secs = 21.448 - t * (46.8150 + t * (0.00059 - t * (0.001813)));
    return 23.0 + (26.0 + (secs / 60.0)) / 60.0;  // degrees
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double JulianTime::ObliquityCorrection(double t)
{
  try
  {
    double e0 = MeanObliquityOfEcliptic(t);
    double omega = 125.04 - 1934.136 * t;
    return e0 + 0.00256 * cos_deg(omega);  // degrees
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
 * Distance to the Sun in AU
 */
double JulianTime::SunRadVector(double t)
{
  try
  {
    double v = SunTrueAnomaly(t);
    double e = EccentricityEarthOrbit(t);
    return (1.000001018 * (1 - e * e)) / (1 + e * cos_deg(v));  // AU
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double JulianTime::GeomMeanLongSun(double t)
{
  try
  {
    double L0 = 280.46646 + t * (36000.76983 + 0.0003032 * t);

    // NOTE: JavaScript has both ends inclusive: [0,360] range
    //
    while (L0 > 360.0)
      L0 -= 360.0;
    while (L0 < 0.0)
      L0 += 360.0;
    return L0;  // degrees
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double JulianTime::SunTrueLong(double t)
{
  try
  {
    return GeomMeanLongSun(t) + SunEqOfCenter(t);  // degrees
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double JulianTime::SunApparentLong(double t)
{
  try
  {
    double o = SunTrueLong(t);
    double omega = 125.04 - 1934.136 * t;
    return o - 0.00569 - 0.00478 * sin_deg(omega);  // degrees
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
 * Right ascension of the Sun
 */
double JulianTime::SunRtAscension(double t)
{
  try
  {
    double e = ObliquityCorrection(t);
    double lambda = SunApparentLong(t);
    double tananum = cos_deg(e) * sin_deg(lambda);
    double tanadenom = cos_deg(lambda);
    return rad2deg(atan2(tananum, tanadenom));  // degrees
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
 * Declination of the Sun
 */
double JulianTime::SunDeclination() const
{
  try
  {
    double t = T();

    double e = ObliquityCorrection(t);
    double lambda = SunApparentLong(t);
    double sint = sin_deg(e) * sin_deg(lambda);
    return rad2deg(asin(sint));  // degrees
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
 * Difference between true solar time and mean solar time (in minutes of time)
 */
double JulianTime::EquationOfTime() const
{
  try
  {
    double t = T();

    double epsilon = ObliquityCorrection(t);
    double l0 = GeomMeanLongSun(t);
    double e = EccentricityEarthOrbit(t);
    double m = GeomMeanAnomalySun(t);

    double y = tan(deg2rad(epsilon) / 2.0);
    y *= y;

    double sin2l0 = sin_deg(2.0 * l0);
    double cos2l0 = cos_deg(2.0 * l0);
    double sin4l0 = sin_deg(4.0 * l0);
    double sinm = sin_deg(m);
    double sin2m = sin_deg(2.0 * m);

    double et = (y * sin2l0 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos2l0 - 0.5 * y * y * sin4l0 -
                 1.25 * e * e * sin2m);

    return rad2deg(et) * 4.0;  // minutes of time
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
 * Solar noon for the given day at the given location on Earth
 */
JulianTime SolNoon(const Fmi::LocalDateTime& ldt, double lon_e)
{
  try
  {
    // Convert date to UTC date (normally they are the same, at local time
    // noon) but +-11..13hr time zones (with summer time) can be on the other
    // day's side.
    //
    // Note: We never hit the uncertain (duplicate/non-existing) cases, since
    //       we're at local noon.
    //

    Fmi::LocalDateTime lnoon(ldt.local_time().date(),
                          TimeDuration(12, 0, 0),
                          ldt.zone(),
                          Fmi::LocalDateTime::NOT_DATE_TIME_ON_ERROR);

    Fmi::Date date_utc = lnoon.utc_time().date();

    double JD = date_utc.julian_day() - 0.5;

    // First pass uses approximate solar noon to calculate eqtime
    //
    // '.julian_day()' gives us the noon (start of a Julian day),
    // 'lon_e/360.0' adjusts it -12..+12 hours.
    //
    auto jt = JulianTime(JD - lon_e / 360.0);

    // Second round
    //
    double diff = ((-lon_e) * 4.0) - jt.EquationOfTime();  // minutes
    jt = JulianTime(JD + diff / 1440.0);

    // Third round

    diff = ((-lon_e) * 4.0) - jt.EquationOfTime();
    jt = JulianTime(JD + diff / 1440.0);
    jt += 0.5;  // TBR: why is this needed?

    return jt;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

#ifdef _MSC_VER
#define isnan _isnan
#else
using std::isnan;
#endif

/*
 * UTC of sunrise/sunset for the given day at the given location on Earth
 *
 * Returns:  time in minutes from zero Z, "empty" JulianTime if no sunrise/set
 */
JulianTime Sunrise_or_set_UTC(const JulianTime& noon, double lon_e, double lat, bool rise)
{
  try
  {
    // *** First pass to approximate sunrise (using solar noon)
    //
    double eqtime = noon.EquationOfTime();
    double solarDec = noon.SunDeclination();
    double hourAngle = HourAngleSunrise_or_set(lat, solarDec, rise);
    if (isnan(hourAngle))
      return JulianTime();

    double delta = -lon_e - rad2deg(hourAngle);
    double timeDiff = 4 * delta;                   // minutes of time
    double timeUTC = 12 * 60 + timeDiff - eqtime;  // minutes

    // *** Second pass includes fractional day in gamma calc
    //

    /* 'date' has its time part (fractions) taken out; and adjusted to
     * midnight (N - 0.5).
     */

    double tmp = floor(noon.JulianDay() + 0.5) - 0.5;
    if (noon.JulianDay() - (tmp + timeUTC / 1440) > 0.5)
      ++tmp;
    JulianTime date(tmp);

    JulianTime jt(date.JulianDay() + timeUTC / 1440);
    eqtime = jt.EquationOfTime();
    solarDec = jt.SunDeclination();

#if 0
      std::cerr << "NOON: " << noon.ptime_utc() << '\t' << setprecision(16) << noon.JulianDay() << std::endl
          << "DATE: " << date.ptime_utc() << '\t' << setprecision(16) << date.JulianDay() << std::endl
          << "EQ: " << eqtime << std::endl
          << "DELTA: " << delta << std::endl
          << "DIFF: " << timeDiff << std::endl
          << "TIME: " << timeUTC << std::endl
          << "JT  : " << jt.ptime_utc() << std::endl
          << "-------" << std::endl;
#endif

    /* Use the same calculation for both; + for rise, - for set
     */
    hourAngle = HourAngleSunrise_or_set(lat, solarDec, rise);
    if (isnan(hourAngle))
      return JulianTime();

    assert(!isnan(hourAngle));

    delta = -lon_e - rad2deg(hourAngle);
    timeDiff = 4 * delta;
    timeUTC = 720 + timeDiff - eqtime;  // in minutes

    jt = JulianTime(date.JulianDay() + timeUTC / 1440.0);

#if 0
      std::cerr << "DELTA: " << delta << std::endl
          << "DIFF: " << timeDiff << std::endl
          << "TIME: " << timeUTC << std::endl
          << "JT  : " << jt.ptime_utc() << std::endl
          << std::endl;
#endif

    return jt;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Astronomy
}  // namespace Fmi

// ======================================================================
