// Ported to C++ from https://gml.noaa.gov/grad/solcalc/main.js
#include "Astronomy.h"
#include "AstronomyHelperFunctions.h"
#include "Exception.h"
#include "LocalDateTime.h"
#include "StringConversion.h"
#include <array>
#include <cmath>
#include <tuple>

using JulianTime = double;

namespace
{
// Function to calculate date components from Julian day
std::tuple<int, int, int> calcDateFromJD(JulianTime jd)
{
  int z = std::lround(jd);
  double f = (jd + 0.5) - z;

  int A = 0;
  if (z < 2299161)
    A = z;
  else
  {
    int alpha = static_cast<int>((z - 1867216.25) / 36524.25);
    A = z + 1 + alpha - alpha / 4;
  }

  int B = A + 1524;
  int C = static_cast<int>((B - 122.1) / 365.25);
  int D = static_cast<int>(365.25 * C);
  int E = static_cast<int>((B - D) / 30.6001);
  int day = B - D - static_cast<int>(30.6001 * E) + f;
  int month = (E < 14 ? E - 1 : E - 13);
  int year = (month > 2 ? C - 4716 : C - 4715);

  return std::make_tuple(year, month, day);
}

// Tools for Boost.Date_time

Fmi::DateTime calcPtimeFromJD(JulianTime jd)
{
  try
  {
    auto ymd = calcDateFromJD(jd);
    auto y = std::get<0>(ymd);
    auto m = std::get<1>(ymd);
    auto d = std::get<2>(ymd);

    try
    {
      Fmi::Date date(y, m, d);

      auto f = jd + 0.5 - lround(jd);
      auto ss = static_cast<int>(std::floor(f * 24 * 60 * 60));
      return {date, Fmi::Seconds(ss)};
    }
    catch (...)
    {
      throw Fmi::Exception(BCP, "Operation failed!")
          .addParameter("juliandate", Fmi::to_string(jd))
          .addParameter("year", Fmi::to_string(y))
          .addParameter("month", Fmi::to_string(m))
          .addParameter("day", Fmi::to_string(d));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Fmi::LocalDateTime calcLocalTimeFromJD(JulianTime jd, const Fmi::TimeZonePtr& tz)
{
  try
  {
    return {calcPtimeFromJD(jd), tz};
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*************************************************************/
/* NOAA Solar position calculation functions */
/*************************************************************/

// Function to calculate the Julian centuries since epoch from Julian day
double calcTimeJulianCent(double jd)
{
  double T = (jd - 2451545.0) / 36525.0;
  return T;
}

#if 0
// Function to calculate Julian day from Julian centuries since epoch
JulianTime calcJDFromJulianCent(double t)
{
  double JD = t * 36525.0 + 2451545.0;
  return JD;
}
#endif

// Function to check if a given year is a leap year
bool isLeapYear(int yr)
{
  return ((yr % 4 == 0 && yr % 100 != 0) || yr % 400 == 0);
}

// Function to calculate day of year from Julian day
int calcDoyFromJD(JulianTime jd)
{
  auto date = calcDateFromJD(jd);
  int y = std::get<0>(date);
  int m = std::get<1>(date);
  int d = std::get<2>(date);
  int k = isLeapYear(y ? 1 : 2);
  int doy = static_cast<int>((275 * m) / 9) - k * static_cast<int>((m + 9) / 12) + d - 30;

  return doy;
}

// Function to convert radians to degrees
double radToDeg(double angleRad)
{
  return 180.0 * angleRad / M_PI;
}

// Function to convert degrees to radians
double degToRad(double angleDeg)
{
  return M_PI * angleDeg / 180.0;
}

// Function to calculate the geometric mean longitude of the sun
double calcGeomMeanLongSun(JulianTime t)
{
  double L0 = 280.46646 + t * (36000.76983 + t * 0.0003032);
  while (L0 > 360.0)
    L0 -= 360.0;

  while (L0 < 0.0)
    L0 += 360.0;

  return L0;  // in degrees
}

// Function to calculate the geometric mean anomaly of the sun
double calcGeomMeanAnomalySun(JulianTime t)
{
  double M = 357.52911 + t * (35999.05029 - 0.0001537 * t);
  return M;  // in degrees
}

// Function to calculate the eccentricity of Earth's orbit
double calcEccentricityEarthOrbit(JulianTime t)
{
  double e = 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
  return e;  // unitless
}

// Function to calculate the equation of center for the sun
double calcSunEqOfCenter(JulianTime t)
{
  double m = calcGeomMeanAnomalySun(t);
  double mrad = degToRad(m);
  double sinm = std::sin(mrad);
  double sin2m = std::sin(mrad + mrad);
  double sin3m = std::sin(mrad + mrad + mrad);
  double C = sinm * (1.914602 - t * (0.004817 + 0.000014 * t)) + sin2m * (0.019993 - 0.000101 * t) +
             sin3m * 0.000289;
  return C;  // in degrees
}

// Function to calculate the true longitude of the sun
double calcSunTrueLong(JulianTime t)
{
  double l0 = calcGeomMeanLongSun(t);
  double c = calcSunEqOfCenter(t);
  double O = l0 + c;
  return O;  // in degrees
}

#if 0
// Function to calculate the true anomaly of the sun
double calcSunTrueAnomaly(JulianTime t)
{
  double m = calcGeomMeanAnomalySun(t);
  double c = calcSunEqOfCenter(t);
  double v = m + c;
  return v;  // in degrees
}

// Function to calculate the radius vector of the sun
double calcSunRadVector(JulianTime t)
{
  double v = calcSunTrueAnomaly(t);
  double e = calcEccentricityEarthOrbit(t);
  double R = (1.000001018 * (1 - e * e)) / (1 + e * std::cos(degToRad(v)));
  return R;  // in astronomical units (AUs)
}
#endif

// Function to calculate the apparent longitude of the sun
double calcSunApparentLong(JulianTime t)
{
  double o = calcSunTrueLong(t);
  double omega = 125.04 - 1934.136 * t;
  double lambda = o - 0.00569 - 0.00478 * std::sin(degToRad(omega));
  return lambda;  // in degrees
}

// Function to calculate the mean obliquity of the ecliptic
double calcMeanObliquityOfEcliptic(JulianTime t)
{
  double seconds = 21.448 - t * (46.8150 + t * (0.00059 - t * 0.001813));
  double e0 = 23.0 + (26.0 + (seconds / 60.0)) / 60.0;
  return e0;  // in degrees
}

// Function to calculate the obliquity correction
double calcObliquityCorrection(JulianTime t)
{
  double e0 = calcMeanObliquityOfEcliptic(t);
  double omega = 125.04 - 1934.136 * t;
  double e = e0 + 0.00256 * std::cos(degToRad(omega));
  return e;  // in degrees
}

#if 0
// Function to calculate the right ascension of the
double calcSunRtAscension(JulianTime t)
{
  double e = calcObliquityCorrection(t);
  double lambda = calcSunApparentLong(t);
  double tananum = (std::cos(degToRad(e)) * std::sin(degToRad(lambda)));
  double tanadenom = (std::cos(degToRad(lambda)));
  double alpha = radToDeg(std::atan2(tananum, tanadenom));
  return alpha;  // in degrees
}
#endif

// Function to calculate the declination of the sun
double calcSunDeclination(JulianTime t)
{
  double e = calcObliquityCorrection(t);
  double lambda = calcSunApparentLong(t);
  double sint = std::sin(degToRad(e)) * std::sin(degToRad(lambda));
  double theta = radToDeg(std::asin(sint));
  return theta;  // in degrees
}

// Function to calculate the equation of time
double calcEquationOfTime(JulianTime t)
{
  double epsilon = calcObliquityCorrection(t);
  double l0 = calcGeomMeanLongSun(t);
  double e = calcEccentricityEarthOrbit(t);
  double m = calcGeomMeanAnomalySun(t);

  double y = std::tan(degToRad(epsilon) / 2.0);
  y *= y;

  double sin2l0 = std::sin(2.0 * degToRad(l0));
  double sinm = std::sin(degToRad(m));
  double cos2l0 = std::cos(2.0 * degToRad(l0));
  double sin4l0 = std::sin(4.0 * degToRad(l0));
  double sin2m = std::sin(2.0 * degToRad(m));

  double Etime = y * sin2l0 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos2l0 - 0.5 * y * y * sin4l0 -
                 1.25 * e * e * sin2m;
  return radToDeg(Etime) * 4.0;  // in minutes of time
}

// Function to calculate the hour angle of sunrise
double calcHourAngleSunrise(double lat, double solarDec)
{
  double latRad = degToRad(lat);
  double sdRad = degToRad(solarDec);
  double HAarg = (std::cos(degToRad(90.833)) / (std::cos(latRad) * std::cos(sdRad)) -
                  std::tan(latRad) * std::tan(sdRad));
  double HA = std::acos(HAarg);
  return HA;  // in radians (for sunset, use -HA)
}

// Function to calculate Julian day from date components
JulianTime getJD(int year, int month, int day)
{
  if (month <= 2)
  {
    year -= 1;
    month += 12;
  }
  int A = static_cast<int>(year / 100);
  int B = 2 - A + A / 4;
  double JD = static_cast<int>(365.25 * (year + 4716)) + static_cast<int>(30.6001 * (month + 1)) +
              day + B - 1524.5;
  return JD;
}

// Function to calculate atmospheric refraction correction
double calcRefraction(double elev)
{
  double correction = 0.0;

  if (elev <= 85.0)
  {
    double te = std::tan(degToRad(elev));
    if (elev > 5.0)
      correction = 58.1 / te - 0.07 / (te * te * te) + 0.000086 / (te * te * te * te * te);
    else if (elev > -0.575)
      correction = 1735.0 + elev * (-518.2 + elev * (103.4 + elev * (-12.79 + elev * 0.711)));
    else
      correction = -20.774 / te;
    correction = correction / 3600.0;
  }

  return correction;
}

// Function to calculate azimuth and elevation of the sun
std::tuple<double, double> calcAzEl(
    double T, double localtime, double latitude, double longitude, double zone)
{
  double eqTime = calcEquationOfTime(T);
  double theta = calcSunDeclination(T);

  double solarTimeFix = eqTime + 4.0 * longitude - 60.0 * zone;

  // Unused variable, but calculated in NOAA www-page
  // double earthRadVec = calcSunRadVector(T);

  double trueSolarTime = localtime + solarTimeFix;
  while (trueSolarTime > 1440)
    trueSolarTime -= 1440;

  double hourAngle = trueSolarTime / 4.0 - 180.0;
  if (hourAngle < -180)
    hourAngle += 360.0;

  double haRad = degToRad(hourAngle);
  double csz = sin(degToRad(latitude)) * sin(degToRad(theta)) +
               cos(degToRad(latitude)) * cos(degToRad(theta)) * cos(haRad);
  if (csz > 1.0)
    csz = 1.0;
  else if (csz < -1.0)
    csz = -1.0;

  double zenith = radToDeg(acos(csz));
  double azDenom = (cos(degToRad(latitude)) * sin(degToRad(zenith)));
  double azimuth = 0.0;
  if (std::abs(azDenom) > 0.001)
  {
    double azRad =
        ((sin(degToRad(latitude)) * cos(degToRad(zenith))) - sin(degToRad(theta))) / azDenom;
    if (std::abs(azRad) > 1.0)
    {
      if (azRad < 0)
        azRad = -1.0;
      else
        azRad = 1.0;
    }
    azimuth = 180.0 - radToDeg(acos(azRad));
    if (hourAngle > 0.0)
      azimuth = -azimuth;
  }
  else
  {
    if (latitude > 0.0)
      azimuth = 180.0;
    else
      azimuth = 0.0;
  }
  if (azimuth < 0.0)
    azimuth += 360.0;
  double exoatmElevation = 90.0 - zenith;

  // Atmospheric Refraction correction
  double refractionCorrection = calcRefraction(exoatmElevation);

  double solarZen = zenith - refractionCorrection;
  double elevation = 90.0 - solarZen;

  return std::make_tuple(azimuth, elevation);
}

// Function to calculate solar noon time in minutes
double calcSolNoon(JulianTime jd, double longitude, double timezone)
{
  double tnoon = calcTimeJulianCent(jd - longitude / 360.0);
  double eqTime = calcEquationOfTime(tnoon);
  double solNoonOffset = 720.0 - longitude * 4 - eqTime;  // in minutes
  double newt = calcTimeJulianCent(jd - 0.5 + solNoonOffset / 1440.0);
  eqTime = calcEquationOfTime(newt);
  double solNoonLocal = 720 - longitude * 4 - eqTime + timezone * 60.0;  // in minutes
  while (solNoonLocal < 0.0)
    solNoonLocal += 1440.0;

  while (solNoonLocal >= 1440.0)
    solNoonLocal -= 1440.0;

  return solNoonLocal;
}

// Function to calculate UTC time of sunrise or sunset
JulianTime calcSunriseSetUTC(bool rise, JulianTime JD, double latitude, double longitude)
{
  double t = calcTimeJulianCent(JD);
  double eqTime = calcEquationOfTime(t);
  double solarDec = calcSunDeclination(t);
  double hourAngle = calcHourAngleSunrise(latitude, solarDec);
  if (!rise)
    hourAngle = -hourAngle;

  double delta = longitude + radToDeg(hourAngle);
  double timeUTC = 720 - (4.0 * delta) - eqTime;  // in minutes

  return timeUTC;
}

// Function to calculate Julian day of next or previous sunrise or sunset
JulianTime calcJDofNextPrevRiseSet(
    bool next, bool rise, JulianTime JD, double latitude, double longitude, double tz)
{
  JulianTime julianday = JD;
  double increment = (next ? 1.0 : -1.0);
  JulianTime time = calcSunriseSetUTC(rise, julianday, latitude, longitude);

  while (!std::isfinite(time))
  {
    julianday += increment;
    time = calcSunriseSetUTC(rise, julianday, latitude, longitude);
  }

  JulianTime timeLocal = time + tz * 60.0;

  while ((timeLocal < 0.0) || (timeLocal >= 1440.0))
  {
    double incr = (timeLocal < 0 ? 1 : -1);
    timeLocal += incr * 1440.0;
    julianday -= incr;
  }
  return julianday;
}

// Function to calculate local time of sunrise or sunset
// rise = 1 for sunrise, 0 for sunset
std::tuple<double, double, double> calcSunriseSet(
    bool rise, double JD, double latitude, double longitude, double timezone)
{
  while (true)
  {
    double timeUTC = calcSunriseSetUTC(rise, JD, latitude, longitude);
    double newTimeUTC = calcSunriseSetUTC(rise, JD + timeUTC / 1440.0, latitude, longitude);

    if (std::isfinite(newTimeUTC))
    {
      double timeLocal = newTimeUTC + (timezone * 60.0);
      double riseT = calcTimeJulianCent(JD + newTimeUTC / 1440.0);
      auto azel = calcAzEl(riseT, timeLocal, latitude, longitude, timezone);
      auto azimuth = std::get<0>(azel);

      double jday = JD;

      if ((timeLocal < 0.0) || (timeLocal >= 1440.0))
      {
        double increment = ((timeLocal < 0) ? 1 : -1);
        while ((timeLocal < 0.0) || (timeLocal >= 1440.0))
        {
          timeLocal += increment * 1440.0;
          jday -= increment;
        }
      }

      return std::make_tuple(jday, timeLocal, azimuth);
    }

    // no sunrise/set found
    int doy = calcDoyFromJD(JD);

    // Original NOAA code used lat=66.4 as a reference value here. However, since the latest
    // code has added refraction terms, the comparison incorrectly deduced that Kuusamo Finland
    // at latitude 65.96 should be treated as if it were in the southern hemisphere. It is
    // sufficient to compare the latitude against zero and get Kuusamo to work again.
    //
    // Note that we had to modify the day of year limits by one or there would be an infinite
    // loop since consecutive iterations would alter between the two if-branches for poles.

    double jday = 0;

    if (((latitude > 0) && (doy > 80) && (doy < 266)) ||
        ((latitude < 0) && ((doy < 82) || (doy > 262))))
    {
      // previous sunrise/next sunset
      jday = calcJDofNextPrevRiseSet(!rise, rise, JD, latitude, longitude, timezone);

      if (!rise)
        JD = (jday > JD ? jday : JD + 1);
      else
        JD = (jday < JD ? jday : JD - 1);
    }
    else
    {
      // previous sunset/next sunrise
      jday = calcJDofNextPrevRiseSet(rise, rise, JD, latitude, longitude, timezone);

      if (rise)
        JD = (jday > JD ? jday : JD + 1);
      else
        JD = (jday < JD ? jday : JD - 1);
    }
  }
}
}  // namespace

/*=== Public interface =====================*/

namespace Fmi
{
namespace Astronomy
{
/*
 * Calculate daylength. The code assumes that DST changes do not occur
 * at the given timezone during polar nights or midnight sun events
 * so that a day is either 0 or 24 hours long in local time.
 *
 * Possible cases with sunrise (R) and sunset (S) with respect to
 * solar noon when divided into separate days from midnight are listed
 * below. Solar noon is always in the current day, since by definition
 * the sun may not be visible at solar noon. Adjacent sunrises and
 * sunsets do not necessarily happen during the same day.
 *
 *  -N  -1  0  +1  +N days
 *
 *  R |   | NS|   |       ==> S - prev midnight        (2)
 *  R |   | N |S  |       ==> 24
 *  R |   | N |   |S      ==> 24
 *    | R | NS|   |       ==> S - R                    (1)
 *    | R | N |S  |       ==> 24
 *    | R | N |   |S      ==> 24
 *    |   |RNS|   |       ==> S - R                    (1)  <--- normal case
 *    |   |RN |S  |       ==> S - R                    (1)
 *    |   |RN |   |S      ==> next midnight - R        (3)
 *
 *  S |   | NR|   |       ==> next midnight - R        (b)
 *  S |   | N |R  |       ==> 0                        (a)
 *  S |   | N |   |R      ==> 0                        (a)
 *    | S | NR|   |       ==> next midnight - R        (b)
 *    | S | N |R  |       ==> 0                        (a)
 *    | S | N |   |R      ==> 0                        (a)
 *    |   |SNR|   |       ==> next midnight - R        (b)
 *    |   |SN |R  |       ==> 0                        (a)
 *    |   |SN |   |R      ==> 0                        (a)
 *
 */

Fmi::TimeDuration solar_time_t::daylength() const
{
  try
  {
    Fmi::TimeDuration td = sunset - sunrise;
    auto diff = td.total_seconds();

    if (diff < 0)
      return Fmi::Seconds(0);

    if (diff > 24 * 3600)
      return Fmi::Hours(24);

    return Fmi::Seconds(diff);

#if 0
    if (diff < 0)  // just two possibilities in the table above
    {
      diff = (noon - sunrise + Fmi::Hours(12)).total_seconds();  // next midnight - sunrise
      if (diff < 0)
        return Fmi::Seconds(0);   // case (a)
      return Fmi::Seconds(diff);  // case (b)
    }

    // 30 hours accounts for possible slightly longer day lengths than 24 hours
    if (diff < 36 * 3600)
      return Fmi::Seconds(diff);  // case (1)

    // This logic does not really work since midnight and solar noon may differ more than one hour from respective local times due to DST changes.
    
    diff = (sunset - noon - Fmi::Hours(12)).total_seconds();  // sunset - prev midnight
    if (diff < 24 * 3600)
      return Fmi::Seconds(diff);  // case (2)

    diff = (noon - sunrise).total_seconds();
    if (diff < 12 * 3600)
      return noon + Fmi::Hours(12) - sunrise;  // case (3) next midnight - sunrise

    return Fmi::Hours(24);
#endif
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
 * Calculate sunrise, sunset and noon times on a certain day.
 * The algorithm was copied from NOAA calculate function and then several guesses were made
 * to get the correct input and output parameters.
 */
solar_time_t solar_time(const Fmi::LocalDateTime& ldt, double lon_e, double lat)
{
  try
  {
    check_lonlat(lon_e, lat);

    // TZ offset in hours
    auto offset_duration = ldt.local_time() - ldt.utc_time();
    auto offset_hours = offset_duration.total_seconds() / 3600.0;

    // This fixes Kiribati issues (GMT+14 but lon=-157)
    if (offset_hours >= 12 && lon_e < 0)
      lon_e += 360;

    // Julian day
    auto local_time = ldt.local_time();
    auto date = local_time.date();
    // auto date = ldt.utc_time().date();
    auto jday = getJD(date.year(), date.month(), date.day());

    // Solar noon
    auto solnoon = calcSolNoon(jday, lon_e, offset_hours);

    // Sunrise and sunset surrounding the noon

    auto sunrise_jday_time_az = calcSunriseSet(true, jday, lat, lon_e, offset_hours);
    auto sunset_jday_time_az = calcSunriseSet(false, jday, lat, lon_e, offset_hours);

    // Convert to output format
    auto jd_rise = std::get<0>(sunrise_jday_time_az);
    auto t_rise = std::get<1>(sunrise_jday_time_az);
    auto jd_set = std::get<0>(sunset_jday_time_az);
    auto t_set = std::get<1>(sunset_jday_time_az);

    // Local time stamps
    jd_rise += t_rise / 1440.0 - offset_hours / 24.0;
    jd_set += t_set / 1440.0 - offset_hours / 24.0;
    auto jd_noon = jday + solnoon / 1440.0 - offset_hours / 24.0;

    auto sunrise = calcLocalTimeFromJD(jd_rise, ldt.zone());
    auto sunset = calcLocalTimeFromJD(jd_set, ldt.zone());
    auto noon = calcLocalTimeFromJD(jd_noon, ldt.zone());

    return {sunrise, sunset, noon};
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Astronomy
}  // namespace Fmi
