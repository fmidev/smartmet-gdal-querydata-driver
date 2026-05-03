// ======================================================================
/*!
 * \file NFmiTime.cpp
 * \brief Implementation of class NFmiTime
 */
// ======================================================================
/*!
 * \class NFmiTime
 *
 * ....
 * The class also holds the month and weekday names in different languages.
 * The time can be converted into a string according to a given format:
 * e.g. "%Weather on% Wwww D.M %at% H:mm"->Weather on Friday 6.11 at 13:21
 *
 *
 */
// ======================================================================

#include "NFmiTime.h"
#include "NFmiLocation.h"
#include "NFmiSettings.h"
#include "NFmiValueString.h"
#include <macgyver/Exception.h>
#include <sys/timeb.h>
#include <ctime>
#include <iostream>

using namespace std;

namespace
{
int monthLength[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

//! languages in order: Finnish, Swedish, English, Spanish, Germany, French, Saame, Latvian

const char *weekdays[] = {
    "Maanantai",
    "Tiistai",
    "Keskiviikko",
    "Torstai",
    "Perjantai",
    "Lauantai",
    "Sunnuntai",
    "M\345ndag",
    "Tisdag",
    "Onsdag",
    "Torsdag",
    "Fredag",
    "L\366rdag",
    "S\366ndag",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday",
    "Lunes",
    "Martes",
    "Mi\351rcoles",
    "Jueves",
    "Viernes",
    "S\341bado",
    "Domingo",
    "Montag",
    "Dienstag",
    "Mittwoch",
    "Donnerstag",
    "Freitag",
    "Samstag",
    "Sonntag",
    "Lundi",
    "Mardi",
    "Mercredi",
    "Jeudi",
    "Vendredi",
    "Samedi",
    "Dimanche",

    //! The special characters of Saame language will work only with the Saame font

    "Vuoss\341rga",
    "Ma\271\271eb\341rga",
    "Gaskavahkku",
    "Duorastat",
    "Bearjadat",
    "L\341vvardat",
    "Sotnabeaivi",
    "Pirmdiena",
    "Otrdiena",
    "Tre\145\360diena",
    "Ceturtdiena",
    "Piektdiena",
    "Sestdiena",
    "Svetdiena"};

std::string weekdayStrings[7];

const char *months[] = {
    "Tammikuu",
    "Helmikuu",
    "Maaliskuu",
    "Huhtikuu",
    "Toukokuu",
    "Kes\344kuu",
    "Hein\344kuu",
    "Elokuu",
    "Syyskuu",
    "Lokakuu",
    "Marraskuu",
    "Joulukuu",

    "Januari",
    "Februari",
    "Mars",
    "April",
    "Maj",
    "Juni",
    "Juli",
    "Augusti",
    "September",
    "Oktober",
    "November",
    "December",

    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",

    "Enero",
    "Febrero",
    "Marzo",
    "Abril",
    "Mayo",
    "Junio",
    "Julio",
    "Agosto",
    "Semtiembre",
    "Octubre",
    "Noviembre",
    "Diciembre",

    "Januar",
    "Februar",
    "M\344rz",
    "April",
    "Mai",
    "Juni",
    "Juli",
    "August",
    "September",
    "Oktober",
    "November",
    "Dezember",

    "Janvier",
    "Fevrier",
    "Mars",
    "Avril",
    "Mai",
    "Juin",
    "Juillet",
    "Aout",
    "Septembre",
    "Oktobre",
    "Novembre",
    "Decembre",

    //! The special characters of Saame language will work only with the Saame font

    "Oddajagem\341nnu",
    "Guovvam\341nnu",
    "Njukcam\341nnu",
    "Cuo\271om\341nnu",
    "Miessem\341nnu",
    "Geassem\341nnu",
    "Suoidnem\341nnu",
    "Borgem\341nnu",
    "Cakcam\341nnu",
    "Golggotm\341nnu",
    "Sk\341bmam\341nnu",
    "Juovlam\341nnu",

    "Janvaris",
    "Februaris",
    "Marts",
    "Aprilis",  // päivitä latvia
    "Maijs",
    "Birzelis",
    "Julijs",
    "Augusts",
    "Septembris",
    "Oktobris",
    "Novembris",
    "Decembris"};

std::string monthStrings[12];

const char *atNotation[] =  // latvia?
    {"klo", "kl.", "at", "a las", "??", "um?", "dmu", "zvans"};

const char *reldays[] =  // OBS also in NFmiPressTime ??
    {"alivuoto",
     "Toissap\344iv\344n\344",
     "Eilen",
     "T\344n\344\344n",
     "Huomenna",
     "Ylihuomenna",
     "ylivuoto",

     "alivuoto",
     "I f\366rrg\345r",
     "I g\345r",
     "I dag",
     "I morgon",
     "I \366vermorgon",
     "ylivuoto",

     "alivuoto",
     "The day before yesterday",
     "Yesterday",
     "Today",
     "Tomorrow",
     "The day after tomorrow",
     "ylivuoto",

     "alivuoto",
     "The day before yesterday",
     "Yesterday",
     "Today",
     "Tomorrow",
     "The day after tomorrow",
     "ylivuoto",
     "alivuoto",
     "Vorgestern",
     "Gestern",
     "Heute",
     "Morgen",
     "\334bermorgen",
     "ylivuoto",
     "alivuoto",
     "The day before yesterday",
     "Yesterday",
     "Today",
     "Tomorrow",
     "The day after tomorrow",
     "ylivuoto",
     "alivuoto",
     "The day before yesterday",
     "ikte",
     "\345dne",
     "Tomorrow",
     "The day after tomorrow",
     "ylivuoto",
     // latvia, korjaa Vorgestern
     "alivuoto",
     "korjaa",
     "Vakar",
     "Sodien",
     "Rit",
     "Parit",
     "ylivuoto"};

}  // namespace

// ----------------------------------------------------------------------
/*!
 * Constructor using Metview Date and Time fields
 *
 * \param datePart should be <= 10 (relative) or in format YYMMDD (absolute)
 * \param timePart interpreted as hours HH when < 24, otherwise in format HHMM
 *
 */
// ----------------------------------------------------------------------

NFmiTime::NFmiTime(const long datePart, const long timePart)
    : NFmiStaticTime(), itsZoneDifferenceHour()
{
  try
  {
    SetZoneDifferenceHour();
    _setCurrent();

    if (datePart <= 10)
      ChangeByDays(static_cast<short>(
          datePart));  // Persa lisÃ¤si shortin jottei tulisi varoituksia kÃ¤Ã¤ntÃ¤essÃ¤
    else
    {
      if (datePart > 101)
        SetDate(static_cast<short>(datePart / 10000),
                static_cast<short>((datePart / 100) % 100),
                static_cast<short>(datePart % 100));
    }

    if (timePart < 24)
      SetTime(static_cast<short>(timePart), 0, 0);
    else
      SetTime(static_cast<short>(timePart / 100),
              static_cast<short>(timePart % 100),
              static_cast<short>(0));
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
 * \param theTime Undocumented
 */
// ----------------------------------------------------------------------

NFmiTime::NFmiTime(time_t theTime) : NFmiStaticTime(theTime, true), itsZoneDifferenceHour()
{
  try
  {
    SetZoneDifferenceHour();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from boost Fmi::DateTime
 */
// ----------------------------------------------------------------------

NFmiTime::NFmiTime(const Fmi::DateTime &thePosixTime)
    : NFmiStaticTime(2000, 1, 1, 0, 0, 0), itsZoneDifferenceHour()
{
  try
  {
    // Use these accessors only once for speed. Hence the fixed dummy time above.
    auto d = thePosixTime.date();
    auto t = thePosixTime.time_of_day();

    SetDate(d.year(), d.month(), d.day());
    SetTime(t.hours(), t.minutes(), t.seconds());

    SetZoneDifferenceHour();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiTime &NFmiTime::operator=(const NFmiTime &theTime)
{
  try
  {
    if (this != &theTime)
    {
      NFmiStaticTime::operator=(theTime);
      itsZoneDifferenceHour = theTime.itsZoneDifferenceHour;
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
 * \brief Construct from local date time
 */
// ----------------------------------------------------------------------

NFmiTime::NFmiTime(const Fmi::LocalDateTime &theLocalTime)
    : NFmiStaticTime(2000, 1, 1, 0, 0, 0), itsZoneDifferenceHour()
{
  try
  {
    // Use these accessors only once for speed. Hence the fixed dummy time above.
    auto d = theLocalTime.date();
    auto t = theLocalTime.time_of_day();

    SetDate(d.year(), d.month(), d.day());
    SetTime(t.hours(), t.minutes(), t.seconds());

    Fmi::TimeDuration offset = theLocalTime.offset();
    itsZoneDifferenceHour = offset.hours();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiTime::Init(const FmiLanguage theLanguage)
{
  try
  {
    // Check if localized strings are defined
    std::string baseWords = "MetEditor::Dictionary::";

    // Read localized weekday strings
    if (NFmiSettings::IsSet(baseWords + DICT_WEEKDAY_MONDAY))
    {
      weekdayStrings[0] = GetDictionaryString(DICT_WEEKDAY_MONDAY);
      weekdays[(theLanguage - 1) * 7 + 0] = weekdayStrings[0].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_WEEKDAY_TUESDAY))
    {
      weekdayStrings[1] = GetDictionaryString(DICT_WEEKDAY_TUESDAY);
      weekdays[(theLanguage - 1) * 7 + 1] = weekdayStrings[1].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_WEEKDAY_WEDNESDAY))
    {
      weekdayStrings[2] = GetDictionaryString(DICT_WEEKDAY_WEDNESDAY);
      weekdays[(theLanguage - 1) * 7 + 2] = weekdayStrings[2].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_WEEKDAY_THURSDAY))
    {
      weekdayStrings[3] = GetDictionaryString(DICT_WEEKDAY_THURSDAY);
      weekdays[(theLanguage - 1) * 7 + 3] = weekdayStrings[3].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_WEEKDAY_FRIDAY))
    {
      weekdayStrings[4] = GetDictionaryString(DICT_WEEKDAY_FRIDAY);
      weekdays[(theLanguage - 1) * 7 + 4] = weekdayStrings[4].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_WEEKDAY_SATURDAY))
    {
      weekdayStrings[5] = GetDictionaryString(DICT_WEEKDAY_SATURDAY);
      weekdays[(theLanguage - 1) * 7 + 5] = weekdayStrings[5].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_WEEKDAY_SUNDAY))
    {
      weekdayStrings[6] = GetDictionaryString(DICT_WEEKDAY_SUNDAY);
      weekdays[(theLanguage - 1) * 7 + 6] = weekdayStrings[6].c_str();
    }

    // Read localized month strings
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_JANUARY))
    {
      monthStrings[0] = GetDictionaryString(DICT_MONTH_JANUARY);
      months[(theLanguage - 1) * 12 + 0] = monthStrings[0].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_FEBRUARY))
    {
      monthStrings[1] = GetDictionaryString(DICT_MONTH_FEBRUARY);
      months[(theLanguage - 1) * 12 + 1] = monthStrings[1].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_MARCH))
    {
      monthStrings[2] = GetDictionaryString(DICT_MONTH_MARCH);
      months[(theLanguage - 1) * 12 + 2] = monthStrings[2].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_APRIL))
    {
      monthStrings[3] = GetDictionaryString(DICT_MONTH_APRIL);
      months[(theLanguage - 1) * 12 + 3] = monthStrings[3].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_MAY))
    {
      monthStrings[4] = GetDictionaryString(DICT_MONTH_MAY);
      months[(theLanguage - 1) * 12 + 4] = monthStrings[4].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_JUNE))
    {
      monthStrings[5] = GetDictionaryString(DICT_MONTH_JUNE);
      months[(theLanguage - 1) * 12 + 5] = monthStrings[5].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_JULY))
    {
      monthStrings[6] = GetDictionaryString(DICT_MONTH_JULY);
      months[(theLanguage - 1) * 12 + 6] = monthStrings[6].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_AUGUST))
    {
      monthStrings[7] = GetDictionaryString(DICT_MONTH_AUGUST);
      months[(theLanguage - 1) * 12 + 7] = monthStrings[7].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_SEPTEMPER))
    {
      monthStrings[8] = GetDictionaryString(DICT_MONTH_SEPTEMPER);
      months[(theLanguage - 1) * 12 + 8] = monthStrings[8].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_OCTOBER))
    {
      monthStrings[9] = GetDictionaryString(DICT_MONTH_OCTOBER);
      months[(theLanguage - 1) * 12 + 9] = monthStrings[9].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_NOVEMBER))
    {
      monthStrings[10] = GetDictionaryString(DICT_MONTH_NOVEMBER);
      months[(theLanguage - 1) * 12 + 10] = monthStrings[10].c_str();
    }
    if (NFmiSettings::IsSet(baseWords + DICT_MONTH_DECEMBER))
    {
      monthStrings[11] = GetDictionaryString(DICT_MONTH_DECEMBER);
      months[(theLanguage - 1) * 12 + 11] = monthStrings[11].c_str();
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param seconds Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTime::ChangeBySeconds(long seconds)
{
  try
  {
    long theSeconds = seconds + GetSec();
    int secLeft = theSeconds % 60;

    if (secLeft >= 0)
    {
      ChangeByMinutes(theSeconds / 60);
      SetSec(static_cast<short>(secLeft));
    }
    else  // if(secLeft<0)
    {
      ChangeByMinutes(theSeconds / 60 - 1);
      SetSec(static_cast<short>(secLeft + 60));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return The weekday, with Monday=1 ... Sunday=7
 */
// ----------------------------------------------------------------------

short NFmiTime::GetWeekday() const
{
  try
  {
    NFmiTime Sunday(static_cast<short>(1830),
                    static_cast<short>(1),
                    static_cast<short>(4));  // toimiikohan?????

    return static_cast<short>(1 + DifferenceInDays(Sunday) % 7);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLanguage Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiTime::MonthName(const FmiLanguage theLanguage) const
{
  try
  {
    return NFmiString(months[(theLanguage - 1) * 12 + GetMonth() - 1]);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLanguage Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiTime::Weekday(const FmiLanguage theLanguage) const
{
  try
  {
    // TODO JL: tÃ¤tÃ¤ kÃ¤ytetÃ¤Ã¤n datan latauksessa
    return NFmiString(weekdays[(theLanguage - 1) * 7 + GetWeekday() - 1]);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiTime::PrintWeekday() const
{
  try
  {
    std::cout << weekdays[GetWeekday() - 1];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns time in minutes since 01.01.2001 00:00
 *
 * The formula used here calculates the gregorian day count, and then
 * adds the effect of hours and minutes. The formula can be found
 * easily with google, for example from
 * <a href="http://en.wikipedia.org/wiki/Julian_day">here</a>
 *
 * It is important that all the divisions are integer divisions.
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiTime::GetCompareValue() const
{
  try
  {
    long a = (14 - GetMonth()) / 12;
    long y = GetYear() + 4800 - a;
    long m = GetMonth() + 12 * a - 3;
    long day = GetDay() + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;

    // 1.1.2002 00:00 = Gregorian day 2452276

    long result = (day - 2452276) * 24 * 60 + 60 * GetHour() + GetMin();
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param aCompareValue Time in minutes since 01.01.2002 00:00
 */
// ----------------------------------------------------------------------

void NFmiTime::DecodeCompareValue(long aCompareValue)
{
  try
  {
    long d = aCompareValue;

    int min = d % 60;
    d /= 60;
    int hour = d % 24;
    d /= 24;

    if (min < 0)
    {
      min += 60;
      --hour;
    }
    if (hour < 0)
    {
      hour += 24;
      --d;
    }

    d += 731156;
    long t = (((4 * (d + 36525)) / 146097) | 0) - 1;
    int y = 100 * t;
    d -= 36524 * t + (t >> 2);
    t = (((4 * (d + 366)) / 1461) | 0) - 1;
    y += t;
    d -= 365 * t + (t >> 2);
    int m = ((5 * d + 2) / 153) | 0;
    d -= (((2 + m * 153) / 5) | 0);
    if (m > 9)
    {
      m -= 12;
      y++;
    }

    SetYear(static_cast<short>(y));
    SetMonth(static_cast<short>(m + 3));
    SetDay(static_cast<short>(d + 1));
    SetMin(static_cast<short>(min));
    SetHour(static_cast<short>(hour));
    SetSec(0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param aYear Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

short NFmiTime::DaysInYear(const short aYear)
{
  try
  {
    if (aYear % 4 != 0)
      return 365;

    if (aYear % 400 == 0)
      return 366;

    if (aYear % 100 == 0)
      return 365;

    return 366;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param aMonth Undocumented
 * \param aYear Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

short NFmiTime::DaysInMonth(const short aMonth, const short aYear)
{
  try
  {
    if (aMonth == 2)
      return static_cast<short>(monthLength[1] + DaysInYear(aYear) - 365);

    return static_cast<short>(monthLength[aMonth - 1]);
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

short NFmiTime::GetJulianDay() const
{
  try
  {
    // Juliaaninen pÃ¤ivÃ¤ on pÃ¤ivÃ¤n jÃ¤rjestysnumero kuluvana vuonna,
    // esim. tammikuun 1:nÃ¤ palautetaan 1, helmikuun 1:nÃ¤ 32

    long days = 0;
    for (short month = 1; month < GetMonth(); month++)
      days += DaysInMonth(month, GetYear());

    days += GetDay();
    return static_cast<short>(days);
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
// 6.9.2001/Marko Muutin metodin const:iksi ja poistin this-otuksen muokkauksen,
// kun kÃ¤ytettiin SetLocalPlace:a, ettÃ¤ saatiin laskettua ZoneDifferenceHour. Nyt
// metodi kÃ¤yttÃ¤Ã¤ uutta CalcZoneDifferenceHour-metodia, joka laskee ZoneDifferenceHour:in

const NFmiTime NFmiTime::UTCTime(const float theLongitude) const
{
  try
  {
    NFmiTime theTime(*this);
    theTime.ChangeByHours(CalcZoneDifferenceHour(theLongitude));
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

const NFmiTime NFmiTime::UTCTime(const NFmiLocation &theLocation) const
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
 * \param theLongitude Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiTime::LocalTime(const float theLongitude) const
{
  try
  {
    // koskelam: What's the idea of this function? It always returns
    // the same result.

    if (theLongitude != kFloatMissing)
      return *this;

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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTime NFmiTime::LocalTime(const NFmiLocation &theLocation) const
{
  try
  {
    // koskelam: What's the idea of this function? It always returns
    // the same result.

    if (theLocation.GetLongitude() != kFloatMissing)
      return *this;

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

// Suomessa talvella -2, kesÃ¤llÃ¤ -3

void NFmiTime::SetZoneDifferenceHour()
{
  try
  {
    // time  returns the time since the Epoch (00:00:00 UTC, JanÂ­
    // uary 1, 1970), measured in seconds.

    const time_t now = time(nullptr);

    // The localtime() function converts the calendar time  timep
    // to  broken-time  representation, expressed relative to the
    // user's specified  time  zone.     The  function  sets  the
    // external  variables tzname with information about the curÂ­
    // rent time zone, timezone with the difference between CoorÂ­
    // dinated  Universal  Time  (UTC) and local standard time in
    // seconds, and daylight to a non-zero value if  standard  US
    // daylight savings time rules apply.

    struct tm nowparts;

#ifdef _MSC_VER
    // OBS! There are no thread safe localtime(_r) or gmtime(_r) functions in MSVC++ 2008 (or
    // before). Closest things available are some what safer (but not thread safe) and with almost
    // same function definitions are the localtime_s and gmtime_s -functions. Parameters are ordered
    // otherway round and their return value is success status, not struct tm pointer.

    ::localtime_s(&nowparts, &now);

#else
    ::localtime_r(&now, &nowparts);
#endif

#ifdef _MSC_VER
    itsZoneDifferenceHour = static_cast<short>(
        _timezone / 3600 -
        nowparts.tm_isdst);  // Visual C++ 2015 ei enää tye epäturvallista globaali
                             // muuttujaa timezone:a, siksi pitää käyttää _timezone
                             // makroa (löytyy myös vanhemmista VC++ versioista)
#else
    itsZoneDifferenceHour = static_cast<short>(timezone / 3600 - nowparts.tm_isdst);
#endif
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLongitude Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTime::SetLocalPlace(float theLongitude)
{
  try
  {
    // virhetuilanteessÃ¤ palauttaa nollan  // 6.9.2001/Marko Muutin funktion kÃ¤yttÃ¤mÃ¤Ã¤n
    // uutta CalcZoneDifferenceHour-metodia, joka on const-metodi ja sitÃ¤ voi kÃ¤yttÃ¤Ã¤
    // mm. constiksi muutetussa UTCTime-metodissa
    itsZoneDifferenceHour = CalcZoneDifferenceHour(theLongitude);
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

short NFmiTime::CalcZoneDifferenceHour(float theLongitude) const
{
  try
  {
    if (theLongitude == kFloatMissing)
      return itsZoneDifferenceHour;  // tÃ¤mÃ¤ on virhetilanne, mutta ei tehdÃ¤ muuta kuin
                                     // palautetaan vanha arvo

    struct tm newtime;
    time_t aclock;

    time(&aclock);

#ifdef _MSC_VER
    // OBS! There are no thread safe localtime(_r) or gmtime(_r) functions in MSVC++ 2008 (or
    // before). Closest things available are some what safer (but not thread safe) and with almost
    // same function definitions are the localtime_s and gmtime_s -functions. Parameters are ordered
    // otherway round and their return value is success status, not struct tm pointer.

    ::localtime_s(&newtime, &aclock);

#else
    ::localtime_r(&aclock, &newtime);
#endif

    auto theWinterTimeDiff = short(newtime.tm_isdst);

    if (theLongitude < 0)
      return static_cast<short>((((-1 * theLongitude) + 7.5) / 15) + theWinterTimeDiff);
    else
      return static_cast<short>((-(theLongitude + 7.5) / 15) - theWinterTimeDiff);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \param theResolution Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTime::IsDay(const NFmiLocation &theLocation, unsigned short theResolution)
{
  try
  {
    int halfResolution = theResolution / 2;
    short hour = LocalTime(theLocation).GetHour();
    return hour > halfResolution && hour <= 24 - halfResolution;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \param theResolution Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTime::IsNight(const NFmiLocation &theLocation, unsigned short theResolution)
{
  try
  {
    int halfResolution = theResolution / 2;

    short hour = LocalTime(theLocation).GetHour();
    return hour > 24 - halfResolution || hour <= halfResolution;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
// ----------------------------------------------------------------------
/*!
 * Generates a time string also containing other arbitrary text fragments
 * according to the given format. The known languages are handled properly
 * when months and weekdays are coded as strings (also the "at" notation).
 *
 *
 * \code
 * sallittuja formaattiosia vapaassa jÃ¤rjestyksessÃ¤ (aika olkoon pe 5.8.99
 *               kello 7:31:23):
 *		YYYY ->	        1999
 *		YY		        99
 *		MM		        08
 *		M		        8
 *		DD		        05
 *		D		        5
 *		HH		        07
 *		H		        7
 *		mm		        31  (huom pienet koska isot =kuukausi)
 *		SS		        23
 *		kk		      * klo  (jos kieliriippuvuutta halutaan)
 *		Wwww wwww WWWW*	Perjantai perjantai PERJANTAI
 *		Ww ww WW	  *	Pe pe PE
 *		Nnnn nnnn NNNN*	Maaliskuu maaliskuu MAALISKUU
 *		Nnn nnn NNN	  *	Maa maa MAA
 *		Tttt tttt TTTT* aika nykyhetkestÃ¤: Eilen tÃ¤nÃ¤Ã¤n HUOMENNA jne
 *		TtttÂ±n jne    * edellinen kun ensin nykyhetkeÃ¤ muutetaan n pÃ¤ivÃ¤Ã¤
 *		I		        5 (perjantai)
 *		%abc%           abc (jos lopussa pÃ¤Ã¤ttÃ¤vÃ¤Ã¤ %:a ei tarvita)
 *		%%				%
 * *= kieliriippuva(suomi,ruotsi,engl.,saksa,esp.,ranska,saame)
 * osat voidaan antaa mielivaltaisessa jÃ¤rjestyksessÃ¤
 *
 * Normaalille seassa olevalle tekstille suositellaan siis sijoittamista
 * %-merkkien vÃ¤liin vaikka tuleekin toistaiseksi tekstinÃ¤ jos ei kuulu
 * yllÃ¤ lueteltuihin formaatteihin. Tulevat laajennukset kun saattavat koodata
 * ne joksikin aikamerkinnÃ¤ksi. Kuitenkin taataan ettÃ¤ vÃ¤limerkit: . , : vÃ¤lilyÃ¶nti
 * jne aina toimivat sellaisenaan.
 *		esim:
 *		"H:mm:SS" ->	7:31:23
 *		"%SÃ¤Ã¤% D.M.YY kk HH:mm" -> SÃ¤Ã¤ 5.8.99 klo 07:31
 *                                 (kÃ¤y myÃ¶s "%SÃ¤Ã¤% D.M.YY %klo% HH:mm")
 *		"YYMMDD" -> 990805
 *      "Tttt wwww%na%"  ->  Huomenna perjantaina  (jos nyt on torstai)
 *      "tttt+1 Ww"  ->  i morgon Fr  (kieli=ruotsi, nyt keskiviikko (tuote
 *                          asiakkaille huomenna(lehti)))
 * \endcode
 *
 * \param theTimeCode the format for the result string
 * \param theLanguage the language to be used (default Finnish)
 * \return the generated time string
 * \todo Muuta Doxygen kommentit kunnollisiksi
 */
// ----------------------------------------------------------------------

const NFmiString NFmiTime::ToStr(const NFmiString theTimeCode, const FmiLanguage theLanguage) const
{
  try
  {
    bool noConversion = false;
    int ind = 1;
    int len = theTimeCode.GetLen();
    NFmiString str6, str4, str3, str2, str1, strh;
    NFmiValueString theString;
    int plusInd, subPlusInd;

    while (ind <= len)
    {
      str1 = theTimeCode.GetChars(ind, 1);
      // %:ien vÃ¤liset merkit sellaisenaan
      if (str1 == NFmiString("%"))
      {
        str1 = theTimeCode.GetChars(ind + 1, 1);
        // kahdella perÃ¤kkÃ¤isellÃ¤ prosentilla saadaan prosentti
        if (str1 == NFmiString("%"))
        {
          theString += NFmiString("%");
          ind++;
          continue;
        }
        else
        {
          noConversion = !noConversion;
          ind++;
          continue;
        }
      }
      else if (noConversion)
      {
        theString += str1;
        ind++;
        continue;
      }

      str4 = theTimeCode.GetChars(ind, 4);
      plusInd = 4;
      if (str4 == NFmiString("YYYY") || str4 == NFmiString("yyyy"))
        theString += NFmiValueString(GetYear(), "%04d");
      else if (str4 == NFmiString("Wwww"))
        theString += Weekday(theLanguage);
      else if (str4 == NFmiString("wwww"))
      {
        strh = Weekday(theLanguage);
        strh.LowerCase();
        theString += strh;
      }
      else if (str4 == NFmiString("WWWW"))
      {
        strh = Weekday(theLanguage);
        strh.UpperCase();
        theString += strh;
      }
      else if (str4 == NFmiString("Nnnn"))
        theString += MonthName(theLanguage);
      else if (str4 == NFmiString("nnnn"))
      {
        strh = MonthName(theLanguage);
        strh.LowerCase();
        theString += strh;
      }
      else if (str4 == NFmiString("NNNN"))
      {
        strh = MonthName(theLanguage);
        strh.UpperCase();
        theString += strh;
      }
      else if (str4 == NFmiString("Tttt") || str4 == NFmiString("tttt") ||
               str4 == NFmiString("TTTT"))
      {
        str6 = theTimeCode.GetChars(ind, 6);
        theString += RelativeDay(theLanguage, str6, subPlusInd);
        plusInd += subPlusInd;
      }
      else
      {
        str3 = theTimeCode.GetChars(ind, 3);
        plusInd += -1;
        if (str3 == NFmiString("Nnn"))
          theString += MonthName(theLanguage).GetChars(1, 3);
        else if (str3 == NFmiString("nnn"))
        {
          strh = MonthName(theLanguage).GetChars(1, 3);
          strh.LowerCase();
          theString += strh;
        }
        else if (str3 == NFmiString("NNN"))
        {
          strh = MonthName(theLanguage).GetChars(1, 3);
          strh.UpperCase();
          theString += strh;
        }
        else if (str3 == NFmiString("Www"))
          theString += Weekday(theLanguage).GetChars(1, 3);
        else if (str3 == NFmiString("www"))
        {
          strh = Weekday(theLanguage).GetChars(1, 3);
          strh.LowerCase();
          theString += strh;
        }
        else if (str3 == NFmiString("WWW"))
        {
          strh = Weekday(theLanguage).GetChars(1, 3);
          strh.UpperCase();
          theString += strh;
        }
        else
        {
          str2 = theTimeCode.GetChars(ind, 2);
          plusInd += -1;
          if (str2 == NFmiString("YY") || str2 == NFmiString("yy"))
            theString += NFmiValueString(GetYear(), "%02d").GetChars(3, 2);
          else if (str2 == NFmiString("MM"))
            theString += NFmiValueString(GetMonth(), "%02d");
          else if (str2 == NFmiString("DD") || str2 == NFmiString("dd"))
            theString += NFmiValueString(GetDay(), "%02d");
          else if (str2 == NFmiString("HH") || str2 == NFmiString("hh"))
            theString += NFmiValueString(GetHour(), "%02d");
          else if (str2 == NFmiString("mm"))
            theString += NFmiValueString(GetMin(), "%02d");
          else if (str2 == NFmiString("SS"))
            theString += NFmiValueString(GetSec(), "%02d");
          else if (str2 == NFmiString("Ww"))
            theString += Weekday(theLanguage).GetChars(1, 2);
          else if (str2 == NFmiString("ww"))
          {
            strh = Weekday(theLanguage).GetChars(1, 2);
            strh.LowerCase();
            theString += strh;
          }
          else if (str2 == NFmiString("WW"))
          {
            strh = Weekday(theLanguage).GetChars(1, 2);
            strh.UpperCase();
            theString += strh;
          }
          else if (str2 == NFmiString("kk"))
            theString += atNotation[theLanguage - 1];
          else
          {
            plusInd += -1;
            if (str1 == NFmiString("M"))
              theString += NFmiValueString(GetMonth(), "%d");
            else if (str1 == NFmiString("D"))
              theString += NFmiValueString(GetDay(), "%d");
            else if (str1 == NFmiString("H"))
              theString += NFmiValueString(GetHour(), "%d");
            else if (str1 == NFmiString("I"))
              theString += NFmiValueString(GetWeekday(), "%01d");
            else
              theString += str1;
            // pitÃ¤isi ottaa ruotsista pÃ¤ivÃ¤n jÃ¤lkeinen piste pois
          }
        }
      }
      ind += plusInd;
    }
    return theString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * see ToStr(const NFmiString theTimeCode,.....
 *
 * \param theLanguage Undocumented
 * \param theFormat Undocumented
 * \param thePlusInd Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiTime::RelativeDay(FmiLanguage theLanguage,
                                       NFmiString theFormat,
                                       int &thePlusInd) const
{
  try
  {
    NFmiTime currentTime;
    NFmiString retString;
    thePlusInd = 0;

    int diff = GetJulianDay() - currentTime.GetJulianDay();
    if (diff <
        -100)  // vuodenvaihde ja karkausvuosi hoidettu, voisi viedÃ¤ NFmiTime:een; ei tastattu
      diff += DaysInYear(currentTime.GetYear());
    if (diff > 100)
      diff -= DaysInYear(GetYear());

    if (theFormat.GetChars(5, 1) == NFmiString("+") || theFormat.GetChars(5, 1) == NFmiString("-"))
    {
      int addDiff;
      NFmiValueString vString = NFmiValueString(theFormat);
      vString.ConvertToInt(addDiff, 5, 2);
      diff += -addDiff;
      thePlusInd = 2;
    }
    diff = std::min(std::max(diff, -3), 3);  // pysÃ¤ytetÃ¤Ã¤n ali/ylivuotoon
    retString = NFmiString(reldays[(theLanguage - 1) * 7 + diff + 3]);
    if (theFormat.GetChars(1, 4) == NFmiString("tttt"))
      retString.LowerCase();
    if (theFormat.GetChars(1, 4) == NFmiString("TTTT"))
      retString.UpperCase();

    return retString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// HUOM! TÃ¤mÃ¤ on kopio NFmiEditMapGeneralDataDoc-luokan metodista, kun en voinut antaa tÃ¤nne
// dokumenttia
std::string NFmiTime::GetDictionaryString(const char *theMagicWord)
{
  try
  {
    const std::string baseWords = "MetEditor::Dictionary::";

    std::string finalMagicWord(baseWords);
    finalMagicWord += theMagicWord;
    return NFmiSettings::Optional<std::string>(finalMagicWord.c_str(), std::string("XXXXX"));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
