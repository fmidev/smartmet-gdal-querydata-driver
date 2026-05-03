// ======================================================================
/*!
 * \file NFmiLocation.cpp
 * \brief Implementation of class NFmiLocation
 */
// ======================================================================
/*!
 * \class NFmiLocation
 *
 * An NFmiLocation instance is an object with a name and geographic
 * coordinates. The name service is provided by the base class
 * NFmiIndividual, NFmiLocation adds coordinate properties.
 *
 * The class is old fashioned and should not be used. The class also
 * contains many methods which would be better off outside the class
 * as namespace members, including all the astronomical methods.
 * Inheritance from NFmiIndividual should be removed and a name
 * variable provided instead.
 *
 */
// ======================================================================

#include "NFmiLocation.h"
#include "NFmiAngle.h"
#include "NFmiGeoTools.h"
#include "NFmiMetTime.h"
#include <macgyver/Hash.h>
#include <fmt/format.h>
#include <macgyver/Exception.h>
#include <algorithm>
#include <fstream>

using namespace std;

const double kRefractCorr = -0.0145386;  // See comments in the CalcSunriseOrSunsetTime()

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theLocation The location to compare with
 * \return True if the locations are equal
 */
// ----------------------------------------------------------------------

bool NFmiLocation::operator==(const NFmiLocation &theLocation) const
{
  try
  {
    return (GetName() == theLocation.GetName() && itsLatlon == theLocation.itsLatlon);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Less than comparison
 *
 * \param theLocation The location to compare with
 * \return True if this is smaller than theLocation
 */
// ----------------------------------------------------------------------

bool NFmiLocation::operator<(const NFmiLocation &theLocation) const
{
  try
  {
    if (GetName() != theLocation.GetName())
      return GetName() < theLocation.GetName();

    if (itsLatlon.Y() != theLocation.itsLatlon.Y())
      return itsLatlon.Y() < theLocation.itsLatlon.Y();

    return itsLatlon.X() < theLocation.itsLatlon.X();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSortable Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLocation::IsEqual(const NFmiSortable &theSortable) const
{
  try
  {
    return (itsLatlon.Y() == (static_cast<const NFmiLocation *>(&theSortable))->itsLatlon.Y() &&
            itsLatlon.X() == (static_cast<const NFmiLocation *>(&theSortable))->itsLatlon.X());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSortable Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLocation::IsLessThan(const NFmiSortable &theSortable) const
{
  try
  {
    return (itsLatlon.Y() > (static_cast<const NFmiLocation *>(&theSortable))->itsLatlon.Y() &&
            itsLatlon.X() > (static_cast<const NFmiLocation *>(&theSortable))->itsLatlon.X());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theLocation The object being copied
 * \return The object assigned to
 * \todo Should protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiLocation &NFmiLocation::operator=(const NFmiLocation &theLocation)
{
  try
  {
    if (this != &theLocation)
    {
      NFmiIndividual::operator=(theLocation);
      itsLatlon = theLocation.itsLatlon;
    }
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLocation::SetLocation(const NFmiLocation &theLocation)
{
  try
  {
    itsLatlon = theLocation.itsLatlon;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * [Venäläinen: lisenssiaattitutkimus ANNEX (A4)]
 * The time given as a parameter must be a local solar time.
 * These calculations of a declination angle of the Sun does not give very accurate
 * estimations. May be the inaccuracy is mostly caused because of inaccurate
 * calculation of the yearAngle (a measure to Earths circulation around the Sun).
 * This inaccuracy causes an inaccurate elevation angle. But the calculation
 * of the elevation angle uses also the method NFmiLocation::AtzimuthAngle(),
 * where calculates a correction term, which quite successfully compensates
 * errors of the elevation angle.
 *
 * \param theSolarTime a local solar time
 * \return a declination angle in radians
 */
// ----------------------------------------------------------------------

double NFmiLocation::CalcDeclinationAngle(const NFmiTime &theSolarTime)
{
  try
  {
    double yearAngle =
        2.0 * kPii *
        (theSolarTime.GetJulianDay() +
         (theSolarTime.GetHour() + theSolarTime.GetMin() / 60. + theSolarTime.GetSec() / 3600.) /
             24.  // 17.09.2002/Viljo adds the seconds
         - 1.0) /
        365.;
    return (0.006918 - 0.399912 * cos(yearAngle) + 0.070257 * sin(yearAngle) -
            0.006758 * cos(2 * yearAngle) + 0.000907 * sin(2 * yearAngle) -
            0.002697 * cos(3 * yearAngle) + 0.00148 * sin(3 * yearAngle));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * The time given as a parameter must be a local solar time. Here the
 * atzimuth angle = 0 when the Sun is in the North.
 * There's also an odd correction term anEqualizerTerm (Laura Thölix:
 * pro gradu, page 16), which perhaps somehow compensates the inaccurate
 * calculation of the yearAngle (a measure to the Earths circulaton around the Sun).
 * See also comments of the method NFmiLocation::CalcDeclinationAngle().
 *
 * \param theSolarTime a local solar time for this NFmiLocation
 * \return the atzimuth angle in radians
 */
// ----------------------------------------------------------------------

double NFmiLocation::CalcAtzimuthAngle(const NFmiTime &theSolarTime)
{
  try
  {
    double hour =
        theSolarTime.GetHour() + theSolarTime.GetMin() / 60. + theSolarTime.GetSec() / 3600.;
    double yearAngle = 2.0 * kPii * (theSolarTime.GetJulianDay() + hour / 24. - 1.0) / 365.;
    double anEqualizerTerm = (0.0172 + 0.4281 * cos(yearAngle) - 7.3515 * sin(yearAngle) -
                              3.3495 * cos(2. * yearAngle) - 9.3619 * sin(2. * yearAngle)) /
                             60.;
    return (hour + anEqualizerTerm) * kPii / 12.;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * [Venäläinen: lisenssiaattitutkimus ANNEX (A3)]
 * These calculations of a elevation angle of the Sun does not give very
 * accurate estimations. This inaccuracy is partly caused by
 * the inaccurate calculations of declination angle.
 * The time given as a parameter must be a local solar time.
 *
 * \param theSolarTime a local solar time
 * \return a elevation angle in radians
 */
// ----------------------------------------------------------------------

double NFmiLocation::ElevationAngleFromSolarTime(const NFmiTime &theSolarTime)
{
  try
  {
    return asin(sin(CalcDeclinationAngle(theSolarTime)) * sin(GetLatitude() * kPii / 180.) -
                cos(CalcDeclinationAngle(theSolarTime)) * cos(GetLatitude() * kPii / 180.) *
                    cos(CalcAtzimuthAngle(theSolarTime)));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime, UTC-time
 * \return a declination angle in degrees
 */
// ----------------------------------------------------------------------

double NFmiLocation::DeclinationAngle(const NFmiMetTime &theTime)
{
  try
  {
    NFmiTime solarTime(LocalSolarTime(theTime));
    return 180. / kPii * CalcDeclinationAngle(solarTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return an elevation angle in degrees
 */
// ----------------------------------------------------------------------

double NFmiLocation::ElevationAngle(const NFmiMetTime &theTime)
{
  try
  {
    NFmiTime solarTime(LocalSolarTime(theTime));
    return 180. / kPii * ElevationAngleFromSolarTime(solarTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime, UTC-time
 * \return an atzimuth angle in degrees
 */
// ----------------------------------------------------------------------

double NFmiLocation::AtzimuthAngle(const NFmiMetTime &theTime)
{
  try
  {
    NFmiTime solarTime(LocalSolarTime(theTime));
    return 180. / kPii * CalcAtzimuthAngle(solarTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * theTime given as a parameter must be an UTC-time (= a solar time
 * in Greenwich, where itsLongitude = 0). The transformation is
 *
 *		+4 min/1 deg towards east from Greenwich
 *
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiTime NFmiLocation::LocalSolarTime(const NFmiMetTime &theTime)
{
  try
  {
    NFmiTime t(static_cast<NFmiTime>(theTime));
    t.ChangeBySeconds(long(240 * itsLatlon.X()));
    return t;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * This conversion is reverse to the LocalSolarTime().
 *
 * \param theSolarTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime NFmiLocation::UtcTimeFromSolarTime(const NFmiTime &theSolarTime)
{
  try
  {
    NFmiMetTime t(theSolarTime, 1);
    t.ChangeBySeconds(long(-240 * itsLatlon.X()));
    if (t.GetSec() > 29)
      t.ChangeByMinutes(1);
    t.SetSec(0);  // NFmiMetTime does not have seconds
    return t;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * In ordinary case returns is the time of sunrise to the current day
 * (the day gets from the local solar time, see comments e.g.in the
 *  method NextTimeOfSunset).
 *
 * If case of the
 *
 *   - polar night:   returns the NEXT sunrise time
 *   - midnight sun:  returns the LAST sunrise time
 *
 * and the bool& isCurrentDay sets to false.
 *
 * \param theTime, UTC-time
 * \param isCurrentDay, true if the returnig time fits into the current day, others false
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime NFmiLocation::TimeOfSunrise(const NFmiMetTime &theTime, bool &isCurrentDay)
{
  try
  {
    bool isMidNightSun, isPolarNight;
    NFmiMetTime sunrise = LastTimeOfSunrise(theTime, isMidNightSun, isPolarNight);
    if (isPolarNight)
      sunrise = NextTimeOfSunrise(theTime, isMidNightSun, isPolarNight);
    isCurrentDay = !isMidNightSun && !isPolarNight;
    return sunrise;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * In ordinary case returns the time of sunset to the current day
 * (the day gets from the local solar time, see comments e.g.in the
 *  method NextTimeOfSunset).
 *
 * If case of the
 *
 *		- polar night:   returns the LAST sunset time.
 *		- midnight sun:  returns the NEXT sunset time.
 *
 * and the bool& isCurrentDay sets to false.
 *
 * \param theTime Undocumented
 * \param isCurrentDay, true if the returnig time fits into the current day, others false
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime NFmiLocation::TimeOfSunset(const NFmiMetTime &theTime, bool &isCurrentDay)
{
  try
  {
    bool isMidNightSun, isPolarNight;
    NFmiMetTime sunset = NextTimeOfSunset(theTime, isMidNightSun, isPolarNight);
    if (isPolarNight)
      sunset = LastTimeOfSunset(theTime, isMidNightSun, isPolarNight);
    isCurrentDay = !isMidNightSun && !isPolarNight;
    return sunset;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Here calculates the time of the next sunset. First calculates the local solar
 * time from the UTC-time theTime. The use of the solarTime makes sure that
 * calculations begins from the correct day. In case of the polar night or the
 * midnight sun finds first the next day, when the sunset will happen.
 * Before the calculation of the time of the sunset the solarTime must be set
 * to the time 00:00.
 *
 * \param theTime Undocumented
 * \param isMidNightSun Undocumented
 * \param isPolarNight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime NFmiLocation::NextTimeOfSunset(const NFmiMetTime &theTime,
                                           bool &isMidNightSun,
                                           bool &isPolarNight)
{
  try
  {
    NFmiTime solarTime = LocalSolarTime(theTime);
    solarTime.SetHour(12);
    solarTime.SetMin(0);
    solarTime.SetSec(0);
    isMidNightSun = false;
    isPolarNight = false;
    if (ElevationAngleFromSolarTime(solarTime) < kRefractCorr)
    {
      // polar night
      isPolarNight = true;
      do
      {
        solarTime.ChangeByDays(1);  // finds the next day of the sunset
      } while (ElevationAngleFromSolarTime(solarTime) < kRefractCorr);
      solarTime.ChangeByHours(12);  // next midnight
    }
    else
    {
      solarTime.ChangeByHours(12);  // next midnight
      if (ElevationAngleFromSolarTime(solarTime) > kRefractCorr)
      {
        // midnight sun
        isMidNightSun = true;
        do
        {
          solarTime.ChangeByDays(1);  // finds the next day of the sunset
        } while (ElevationAngleFromSolarTime(solarTime) > kRefractCorr);
      }
    }

    CalcSunriseOrSunsetTime(solarTime);  // solarTime == 24:00
    return UtcTimeFromSolarTime(solarTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Here calculates the time of the next sunrise. First calculates the local solar
 * time from the UTC-time theTime. The use of the solarTime makes sure that
 * calculations begins from the correct day. In case of the polar night or the
 * midnight sun finds first the next day, when the sunrise will happen.
 * Before the calculation of the time of the sunrise the solarTime must be set
 * to the time 12:00.
 *
 * \param theTime Undocumented
 * \param isMidNightSun Undocumented
 * \param isPolarNight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime NFmiLocation::NextTimeOfSunrise(const NFmiMetTime &theTime,
                                            bool &isMidNightSun,
                                            bool &isPolarNight)
{
  try
  {
    NFmiTime solarTime = LocalSolarTime(theTime);
    solarTime.SetHour(12);
    solarTime.SetMin(0);
    solarTime.SetSec(0);
    isMidNightSun = false;
    isPolarNight = false;
    if (ElevationAngleFromSolarTime(solarTime) < kRefractCorr)
    {
      // polar night
      isPolarNight = true;
      do
      {
        solarTime.ChangeByDays(1);  // finds the next day of the sunrise
      } while (ElevationAngleFromSolarTime(solarTime) < kRefractCorr);
    }
    else
    {
      solarTime.ChangeByHours(12);  // next midnight
      if (ElevationAngleFromSolarTime(solarTime) > kRefractCorr)
      {
        // midnight sun
        isMidNightSun = true;
        do
        {
          solarTime.ChangeByDays(1);  // finds the next day of the sunrise
        } while (ElevationAngleFromSolarTime(solarTime) > kRefractCorr);
      }
      solarTime.SetHour(12);
    }
    CalcSunriseOrSunsetTime(solarTime);  // solarTime == 12:00
    return UtcTimeFromSolarTime(solarTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Here calculates the time of the last sunset. First calculates the local solar
 * time from the UTC-time theTime. The use of the solarTime makes sure that
 * calculations begins from the correct day. In case of the polar night or the
 * midnight sun finds first the last day, when the sunset has happened.
 * Before the calculation of the time of the sunset the solarTime must be set
 * to the time 00:00.
 *
 * \param theTime Undocumented
 * \param isMidNightSun Undocumented
 * \param isPolarNight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime NFmiLocation::LastTimeOfSunset(const NFmiMetTime &theTime,
                                           bool &isMidNightSun,
                                           bool &isPolarNight)
{
  try
  {
    NFmiTime solarTime = LocalSolarTime(theTime);
    solarTime.SetHour(12);
    solarTime.SetMin(0);
    solarTime.SetSec(0);
    isMidNightSun = false;
    isPolarNight = false;
    if (ElevationAngleFromSolarTime(solarTime) < kRefractCorr)
    {
      // polar night
      isPolarNight = true;
      do
      {
        solarTime.ChangeByDays(-1);  // finds the last day of the sunset
      } while (ElevationAngleFromSolarTime(solarTime) < kRefractCorr);
      solarTime.ChangeByHours(12);  // midnight
    }
    else
    {
      solarTime.SetHour(0);  // previous midnight
      if (ElevationAngleFromSolarTime(solarTime) > kRefractCorr)
      {
        // midnight sun
        isMidNightSun = true;
        do
        {
          solarTime.ChangeByDays(-1);  // finds the last day of the sunset
        } while (ElevationAngleFromSolarTime(solarTime) > kRefractCorr);
      }
    }
    CalcSunriseOrSunsetTime(solarTime);  // solarTime == 24:00
    return UtcTimeFromSolarTime(solarTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Here calculates the time of the last sunrise. First calculates the local solar
 * time from the UTC-time theTime. The use of the solarTime makes sure that
 * calculations begins from the correct day. In case of the polar night or the
 * midnight sun finds first the last day, when the sunrise has happened.
 * Before the calculation of the time of the sunrise the solarTime must be set
 * to the time 12:00.
 *
 * \param theTime Undocumented
 * \param isMidNightSun Undocumented
 * \param isPolarNight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime NFmiLocation::LastTimeOfSunrise(const NFmiMetTime &theTime,
                                            bool &isMidNightSun,
                                            bool &isPolarNight)
{
  try
  {
    NFmiTime solarTime = LocalSolarTime(theTime);
    solarTime.SetHour(12);
    solarTime.SetMin(0);
    solarTime.SetSec(0);
    isMidNightSun = false;
    isPolarNight = false;

    if (ElevationAngleFromSolarTime(solarTime) < kRefractCorr)
    {
      // polar night
      isPolarNight = true;
      do
      {
        solarTime.ChangeByDays(-1);  // finds the last day of the sunrise
      } while (ElevationAngleFromSolarTime(solarTime) < kRefractCorr);
    }
    else
    {
      solarTime.SetHour(0);  // previous midnight
      if (ElevationAngleFromSolarTime(solarTime) > kRefractCorr)
      {
        // midnight sun
        isMidNightSun = true;
        do
        {
          solarTime.ChangeByDays(-1);  // finds the last day of the sunrise
        } while (ElevationAngleFromSolarTime(solarTime) > kRefractCorr);
      }
      solarTime.SetHour(12);
    }
    CalcSunriseOrSunsetTime(solarTime);  // solarTime == 12:00
    return UtcTimeFromSolarTime(solarTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *	This protected method calculates time of sunrise or sunset.
 *	Before calling this method, the hour of the parameter time theSolarTime
 *  must be set to 12:00 or 24:00 - the first setting leads to the
 *  calculation	of sunrise, the latter to sunset..
 *  Sunrise/sunset is considered to occur when the Sun's upper limb is
 *  35 arc minutes (0.583 degrees) below the horizon (refraction of the
 *  Earth's atmosphere). When the correction from the center to the upper
 *  limb of the Sun is taken into consideration, the total correction
 *  is around
 *			kRefractCorr = -0.833 degrees
 *  Calculations of sunrise and sunset uses kRefractCorr instead of the
 *  null elevation angle due to the center of the Sun.
 *
 *  Next iterative searching of the sunrise or the sunset isn't sophisticated but
 *  simple. If the sunrise have to been found, backwards iteration begins from
 *  tempSolarTime = 12 and in case of sunset from  tempSolarTime = 24 (next midnight).
 *  Iterations time step is 1 hour and lenght 12 hours. When the hour of sunrise or
 *  sunset has been found, iterates minutes (forwards).
 *
 *	Sunrise: beginHour =12:00, stopHour = 23:00 (previous night)
 *	Sunset:  beginHour =00:00, stopHour = 11:00 (previous day)
 *
 * \param theSolarTime Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLocation::CalcSunriseOrSunsetTime(NFmiTime &theSolarTime)
{
  try
  {
    short stopHour = 23;
    double angle1 = kFloatMissing, angle2 = kFloatMissing;
    if (theSolarTime.GetHour() == 0)  // Sunset
    {
      stopHour = 11;
      angle2 = -1 * kFloatMissing;
      ;  // angle2 must have a negative initial value in case of sunset (XOR-checking)
    }

    while (theSolarTime.GetHour() != stopHour)
    {
      theSolarTime.ChangeByHours(-1);
      angle1 = ElevationAngleFromSolarTime(theSolarTime);
      if ((angle1 < kRefractCorr) ^ (angle2 < kRefractCorr))  // XOR
      {
        do
        {
          theSolarTime.ChangeByMinutes(1);
          angle2 = ElevationAngleFromSolarTime(theSolarTime);
          if ((angle1 < kRefractCorr) ^ (angle2 < kRefractCorr))  // XOR
          {
            if (fabs(angle1 + kRefractCorr) < fabs(angle2 + kRefractCorr))
              theSolarTime.ChangeByMinutes(-1);  // rounding to a nearest minute
            return;
          }
          angle1 = angle2;
        } while (theSolarTime.GetMin() != 0);  // beginnig of the next hour is already handled
      }
      angle2 = angle1;
    }
    return;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return std::shared_ptr
 */
// ----------------------------------------------------------------------

NFmiLocation *NFmiLocation::Clone() const
{
  try
  {
    return new NFmiLocation(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiLocation::Write(std::ostream &file) const
{
  try
  {
    file << fmt::format("{} {}\n", itsLatlon.X(), itsLatlon.Y());

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiLocation::Read(std::istream &file)
{
  try
  {
    double lon = kFloatMissing;
    double lat = kFloatMissing;
    file >> lon >> lat;
    itsLatlon = NFmiPoint(lon, lat);
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the azimuth bearing (in degrees [0..360] clockwise, 0 = north)
 * between points p1 = (current point) and p2 = (lo2, la2), both in degrees:
 *
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLocation::Direction(const NFmiPoint &theLatLon) const
{
  try
  {
    // Returns the azimuth bearing (in degrees [0..360] CLOCKWISE, 0 = north, 180 = south)
    // between points (lon1, lat1) (current point) and (lon2, lat2):

    double lon1 = FmiRad(GetLongitude());
    double lat1 = FmiRad(GetLatitude());

    double lon2 = FmiRad(theLatLon.X());
    double lat2 = FmiRad(theLatLon.Y());
    double dlon = lon2 - lon1;

    if (dlon == 0.)
    {
      // Points along meridian
      if (lat2 - lat1 > 0.)
        return 0.;  // Exact north

      return 180.;  // Exact south
    }

    ////////////////!!!!!!!!!!!!!!!
    // From:
    //  Movable Type Ltd:
    // http://www.movable-type.co.uk/
    //
    // -AND-
    //
    // calculate (initial) bearing (in radians clockwise) between two points
    //
    // from: Ed Williams' Aviation Formulary, http://williams.best.vwh.net/avform.htm#Crs
    //

    return FmiDeg(atan2(sin(dlon) * cos(lat2),
                        cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dlon)));  // [deg]
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
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
 * \param theLocation
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLocation::Distance(const NFmiLocation &theLocation) const
{
  try
  {
    return NFmiGeoTools::GeoDistance(
        GetLongitude(), GetLatitude(), theLocation.GetLongitude(), theLocation.GetLatitude());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================

// 9.8.04/EL -->
NFmiLocation NFmiLocation::ComputeLocation(double theAzimuthInDegrees,
                                           double theDistanceInMeters,
                                           bool usePacificView) const
{
  // Palauttaa maantieteellisen paikan nykysijainnin suhteen azimuutin (asteissa) ja etäisyyden
  // (metreissä) avulla.
  // Azimuutti kasvaa myötäpäivään pohjoisesta, pohjoinen = 0 astetta
  //
  // Lähde:
  //
  // "Ask Dr. Math - Questions and Answers from our Archive"
  // Date: 02/13/97 at 12:59:12
  // From: Doctor Mitteldorf
  // Subject: Re: Calculating new position and heading after a walk around the world

  // Alkuperäinen pseudokoodi:

  /*

  First, definitions:

  Start at a point with given longitude, we'll call this long1, and
  given latitude, we'll call this lat1.

  Travel in a compass heading, we'll call this hdng1, and go a certain
  distance, we'll call this dist.  The compass heading is measured from
  due north, which is equal to 0.

  Define alpha = dist/[earth radius] to be the angular distance covered
  on the earth's surface.

  You're looking for coordinates of the point that you reach, which
  we'll call lat2 and long2.

  You also want to know your new compass heading, which we'll call
  hdng2.

  First calculate the new latitude:

  1) sin(lat2) = cos(hdng1)*cos(lat1)*sin(alpha) + sin(lat1)*cos(alpha)

  Then calculate the new longitude, using your result for lat2:

                                                          cos(alpha) - sin(lat1)*sin(lat2)
  2)    cos(long2-long1) =   ----------------------------------
                                                                  cos(lat1)*cos(lat2)



  Again, using lat2 you can calculate the new compass heading:

                                          sin(lat1) - sin(lat2)*cos(alpha)
  3)    cos(hdng2) =   ----------------------------------
                                                  cos(lat2)*sin(alpha)

  hdng2 is the compass setting from point 2 to point 1; it's just 180
  degrees from the compass heading that one has when traveling from
  point 1 to point 2 and continuing on from there.
  */

  // Tämä metodi laskee uuden maantiet. sijainnin (long2, lat2),
  // muttei kuitenkaan uutta "kompassisuuntaa" 'hdng2'.
  // min() ja max() -funktioilla estetään mahd. pyöristysvirheiden vaikutus:

  try
  {
    double azimuthInDegrees =
        theAzimuthInDegrees - int(theAzimuthInDegrees / 360) * 360;  // Kulma välille [-360..360]
    double alpha = theDistanceInMeters / kRearth;
    double lat1 = FmiRad(itsLatlon.Y());
    double value = cos(FmiRad(azimuthInDegrees)) * cos(lat1) * sin(alpha) + sin(lat1) * cos(alpha);
    double lat2 = asin(std::min(std::max(value, -1.0), 1.0));
    double newLatitude = FmiDeg(lat2);

    // Lasketaan uusi longitudi:

    value = (cos(alpha) - sin(lat1) * sin(lat2)) / (cos(lat1) * cos(lat2));
    double longDiff = FmiDeg(acos(std::min(std::max(value, -1.0), 1.0)));

    int sign = 1;

    if (((azimuthInDegrees < 0.) && (azimuthInDegrees >= -180.)) ||
        ((azimuthInDegrees >= 180.) && (azimuthInDegrees < 360.)))
      sign = -1;

    double newLongitude = itsLatlon.X() + sign * longDiff;
    NFmiLongitude longitude(newLongitude, usePacificView);
    newLongitude = longitude.Value();

    return NFmiLocation(newLongitude, newLatitude);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiLocation::SetLocation(double theAzimuthInDegrees,
                               double theDistanceInMeters,
                               bool usePacificView)
{
  try
  {
    // Laskee maantieteellisen paikan nykysijainnin suhteen azimuutin (asteissa) ja etäisyyden
    // (metreissä) avulla.
    // Azimuutti kasvaa myötäpäivään pohjoisesta, pohjoinen = 0 astetta
    // HUOM! Tämä metodi muuttaa olion datajäseniä (*this) eli
    // käytännössä muuttaa olion maant. sijainnin argumentteja vastaavaksi sijainniksi

    *this = ComputeLocation(theAzimuthInDegrees, theDistanceInMeters, usePacificView);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiLocation NFmiLocation::GetLocation(double theAzimuthInDegrees,
                                             double theDistanceInMeters,
                                             bool usePacificView) const
{
  try
  {
    // Laskee maantieteellisen paikan nykysijainnin suhteen azimuutin (asteissa) ja etäisyyden
    // (metreissä) avulla.
    // Azimuutti kasvaa myötäpäivään pohjoisesta, pohjoinen = 0 astetta
    // Metodi EI muuta datajäseniään; ainoastaan palauttaa argumentteja vastaavan maant. sijainnin

    return ComputeLocation(theAzimuthInDegrees, theDistanceInMeters, usePacificView);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for the location
 */
// ----------------------------------------------------------------------

std::size_t NFmiLocation::HashValue() const
{
  try
  {
    std::size_t hash = NFmiIndividual::HashValue();
    Fmi::hash_combine(hash, itsLatlon.HashValue());
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
