// ======================================================================
/*!
 * \file NFmiMetTime.cpp
 * \brief Implementation of class NFmiMetTime
 */
// ======================================================================
/*!
 * \class NFmiMetTime
 *
 * Discrete time object for meteorological observations and
 * forecasts.  Watch out for the "="-operator and constructors
 * with 'out of phase' times as the NFmiMetTime object will always
 * try to fit itself into 'the correct phase'!
 *
 * Note that NFmiMetTime is UTC time, while NFmiTime is local time.
 *
 */
// ======================================================================

#include "NFmiMetTime.h"
#include "NFmiLocation.h"
#include <macgyver/Exception.h>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

const NFmiMetTime NFmiMetTime::gMissingTime = NFmiMetTime(1900, 0, 0, 0, 0, 0);

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param timeStepInMinutes Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime::NFmiMetTime(const long timeStepInMinutes)
    : fTimeStepInMinutes(timeStepInMinutes), itsNegativeRange(0), itsPositiveRange(0)
{
  try
  {
    _setCurrent();
    SetSec(0);

    NFmiMetTime aTimeNow = *this;

    ConstructMetTime(static_cast<short>(fTimeStepInMinutes));
    if (*this > aTimeNow)
    {
      PreviousMetTime();
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from a FMI datetime
 */
// ----------------------------------------------------------------------

NFmiMetTime::NFmiMetTime(const Fmi::DateTime &theTime)
    : NFmiTime(theTime), fTimeStepInMinutes(), itsNegativeRange(0), itsPositiveRange(0)
{
  try
  {
    ConstructMetTime(1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert to a FMI datetime
 */
// ----------------------------------------------------------------------

Fmi::DateTime NFmiMetTime::PosixTime() const
{
  try
  {
    Fmi::Date date(GetYear(), GetMonth(), GetDay());

    Fmi::DateTime utc(date,
                      Fmi::Hours(GetHour()) + Fmi::Minutes(GetMin()) + Fmi::Seconds(GetSec()));
    return utc;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from a boost Fmi::LocalDateTime
 */
// ----------------------------------------------------------------------

NFmiMetTime::NFmiMetTime(const Fmi::LocalDateTime &theLocalTime)
    : NFmiTime(theLocalTime), fTimeStepInMinutes(), itsNegativeRange(0), itsPositiveRange(0)
{
  try
  {
    ConstructMetTime(1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiMetTime NFmiMetTime::now()
{
  try
  {
    NFmiTime wallClock;
    NFmiMetTime timeStamp(wallClock.UTCTime());
    timeStamp.SetMin(wallClock.GetMin());
    timeStamp.SetSec(wallClock.GetSec());
    return timeStamp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param aFmiTest Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMetTime::IsEqual(const NFmiSortable &aFmiTest) const
{
  try
  {
    if (itsNegativeRange || itsPositiveRange)
    {
      long diff = DifferenceInMinutes(*static_cast<const NFmiTime *>(&aFmiTest));
      return diff <= itsNegativeRange && (-diff) <= itsPositiveRange;
    }
    return NFmiTime::IsEqual(aFmiTest);
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
 * \param aTime The other time being copied
 * \return The object assigned to
 * \bug NFmiStaticTime::operator= is unknown, hence self assignment
 *      should be protected.
 */
// ----------------------------------------------------------------------

NFmiMetTime &NFmiMetTime::operator=(const NFmiMetTime &aTime)
{
  try
  {
    if (&aTime != this)
    {
      NFmiTime::operator=(aTime);                     // retain present time step
      fTimeStepInMinutes = aTime.fTimeStepInMinutes;  // Persa korjasi Bug'in
      itsNegativeRange = aTime.itsNegativeRange;
      itsPositiveRange = aTime.itsPositiveRange;
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
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------
std::ostream &NFmiMetTime::Write(std::ostream &file) const
{
  try
  {
    file << GetYear() << " " << GetMonth() << " " << GetDay() << " " << GetHour() << " " << GetMin()
         << " " << GetSec();

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
std::ostream & NFmiMetTime::Write(std::ostream & oStream) const
{
  oStream.width( 2 );
  oStream.fill( '0' );
  oStream << GetDay() << ".";
  oStream.width( 2 );
  oStream.fill( '0' );
  oStream << GetMonth() << "." << GetYear() << " ";
  oStream.width( 2 );
  oStream.fill( '0' );
  oStream << GetHour() << ":";
  oStream.width( 2 );
  oStream.fill( '0' );
  oStream << GetMin();
  // no seconds for MetTime!

  return oStream;
}
*/

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::istream &NFmiMetTime::Read(std::istream &file)
{
  try
  {
    short year, month, day, hour, minutes, seconds;
    file >> year >> month >> day >> hour >> minutes >> seconds;
    SetDate(year, month, day);
    SetTime(hour, minutes, seconds);
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime &NFmiMetTime::operator++()
{
  try
  {
    NextMetTime(GetTimeStep());
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiMetTime NFmiMetTime::operator++(int)
{
  try
  {
    NextMetTime(GetTimeStep());
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime &NFmiMetTime::operator--()  // prefix++
{
  try
  {
    PreviousMetTime(GetTimeStep());
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiMetTime NFmiMetTime::operator--(int)  // postfix++
{
  try
  {
    PreviousMetTime(GetTimeStep());
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::NextMetTime()
{
  try
  {
    if (fTimeStepInMinutes.IsDate())
    {
      NextMetTime(fTimeStepInMinutes);
    }
    else
    {
      NextMetTime(GetTimeStep());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param deltaInMinutes Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::NextMetTime(const long deltaInMinutes)
{
  try
  {
    if (deltaInMinutes == 0)
      return;

    long extraMinutes = (60L * GetHour() + GetMin()) % deltaInMinutes;

    // add observation interval and delete extra minutes (if any)
    DecodeCompareValue(GetCompareValue() + deltaInMinutes - extraMinutes);
    SetSec(static_cast<short>(0));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param thePerioid Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::NextMetTime(const NFmiTimePerioid &thePerioid)
{
  try
  {
    if (thePerioid.IsDate())
    {
      if ((GetMonth() + thePerioid.Month()) > 12)
      {
        SetYear(short(GetYear() + thePerioid.Year() + 1));
        SetMonth(short(GetMonth() + thePerioid.Month() - 12));
        return;
      }
      else
      {
        SetYear(short(GetYear() + thePerioid.Year()));
        SetMonth(short(GetMonth() + thePerioid.Month()));
        if (thePerioid.Day())
          DecodeCompareValue(GetCompareValue() + (thePerioid.Day() * 24L * 60L));

        return;
      }
    }

    DecodeCompareValue(GetCompareValue() + (thePerioid.Hour() * 60L + thePerioid.Minute()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param deltaInMinutes Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::PreviousMetTime(const long deltaInMinutes)
{
  try
  {
    if (deltaInMinutes == 0)
      return;

    long extraMinutes = (60L * GetHour() + GetMin()) % deltaInMinutes;

    if (extraMinutes > 0)
      // delete only extra minutes
      DecodeCompareValue(GetCompareValue() - extraMinutes);
    else
      // subtract given interval
      DecodeCompareValue(GetCompareValue() - deltaInMinutes);

    SetSec(static_cast<short>(0));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param thePerioid Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::PreviousMetTime(const NFmiTimePerioid &thePerioid)
{
  try
  {
    if (thePerioid.IsDate())
    {
      if ((GetMonth() - thePerioid.Month()) <= 0)
      {
        SetYear(short(GetYear() - thePerioid.Year() - 1));
        SetMonth(short(12 - GetMonth() + thePerioid.Month()));
        return;
      }
      else
      {
        SetYear(short(GetYear() - thePerioid.Year()));
        SetMonth(short(GetMonth() - thePerioid.Month()));
        if (thePerioid.Day())
          DecodeCompareValue(GetCompareValue() - (thePerioid.Day() * 24L * 60L));
        return;
      }
    }

    DecodeCompareValue(GetCompareValue() - thePerioid);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param deltaInMinutes Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::NearestMetTime(const long deltaInMinutes, FmiDirection theDirect)
{
  try
  {
    if (deltaInMinutes == 0)
      return;
    long extraMinutes = (60L * GetHour() + GetMin()) % deltaInMinutes;

    if (extraMinutes == 0)  // already a meteorological time!
      return;               // add vk 940824

    if (theDirect == kForward)
      NextMetTime(deltaInMinutes);
    else if (theDirect == kBackward)
      PreviousMetTime(deltaInMinutes);
    else
    {
      if (extraMinutes < (deltaInMinutes - extraMinutes))
        PreviousMetTime(deltaInMinutes);
      else
        NextMetTime(deltaInMinutes);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param timeStepInMinutes Undocumented
 * \param fSetTime Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMetTime::SetTimeStep(const long timeStepInMinutes, bool fSetTime, FmiDirection theDirect)
{
  try
  {
    fTimeStepInMinutes = timeStepInMinutes;
    // timestepin voi muuttaa ilman, että aikaa ruvetaan säätämään,
    // oletusarvoisesti aika säädetään kuten ennenkin
    if (fSetTime)
      ConstructMetTime(fTimeStepInMinutes, theDirect);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

struct tm NFmiMetTime::GetSystemTime()
{
  try
  {
    time_t t;
    static_cast<void>(time(&t));
    tm ret;

#ifdef _MSC_VER
    // OBS! There are no thread safe localtime(_r) or gmtime(_r) functions in MSVC++ 2008 (or
    // before). Closest things available are some what safer (but not thread safe) and with almost
    // same function definitions are the localtime_s and gmtime_s -functions. Parameters are ordered
    // otherway round and their return value is success status, not struct tm pointer.

    ::gmtime_s(&ret, &t);

#else
    gmtime_r(&t, &ret);
#endif

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLongitude Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiMetTime::UTCTime(float theLongitude) const
{
  try
  {
    NFmiTime theTime(*this);
    theTime.ChangeByHours(static_cast<short>(CalcZoneDifferenceHour(theLongitude)));

    return theTime;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLongitude Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiMetTime::LocalTime(float theLongitude) const
{
  try
  {
    NFmiTime theTime(*this);
    theTime.ChangeByHours(static_cast<short>(-CalcZoneDifferenceHour(theLongitude)));

    return theTime;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiMetTime::LocalTime(const NFmiLocation &theLocation) const
{
  try
  {
    NFmiTime theTime(*this);
    theTime.SetLocalPlace(static_cast<float>(theLocation.GetLongitude()));
    theTime.ChangeByHours(static_cast<short>(-theTime.GetZoneDifferenceHour()));

    return theTime;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiMetTime::UTCTime(const NFmiLocation &theLocation) const
{
  try
  {
    return UTCTime(static_cast<float>(theLocation.GetLongitude()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * This method converts the UTC time represented by this into
 * a local time NFmiTime object correctly. Note that the other LocalTime
 * methods use various approximations which make them inexact.
 *
 * \return The local time
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiMetTime::CorrectLocalTime() const
{
  try
  {
    // The UTC time
    struct ::tm utc;
    utc.tm_sec = GetSec();
    utc.tm_min = GetMin();
    utc.tm_hour = GetHour();
    utc.tm_mday = GetDay();
    utc.tm_mon = GetMonth() - 1;     // tm months start from 0
    utc.tm_year = GetYear() - 1900;  // tm years start from 1900
    utc.tm_wday = -1;
    utc.tm_yday = -1;
    utc.tm_isdst = -1;

    ::time_t epochtime = NFmiStaticTime::my_timegm(&utc);

    // As local time

    struct ::tm local;

#ifdef _MSC_VER
    // OBS! There are no thread safe localtime(_r) or gmtime(_r) functions in MSVC++ 2008 (or
    // before). Closest things available are some what safer (but not thread safe) and with almost
    // same function definitions are the localtime_s and gmtime_s -functions. Parameters are ordered
    // otherway round and their return value is success status, not struct tm pointer.

    ::localtime_s(&local, &epochtime);
#else
    ::localtime_r(&epochtime, &local);
#endif

    // And build a NFmiTime from the result

    NFmiTime out(static_cast<short>(local.tm_year + 1900),
                 static_cast<short>(local.tm_mon + 1),
                 static_cast<short>(local.tm_mday),
                 static_cast<short>(local.tm_hour),
                 static_cast<short>(local.tm_min),
                 static_cast<short>(local.tm_sec));

    return out;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
