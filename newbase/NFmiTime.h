// ======================================================================
/*!
 * \file NFmiTime.h
 * \brief Interface of class NFmiTime
 */
// ======================================================================

#pragma once

#include "NFmiStaticTime.h"

#include <macgyver/LocalDateTime.h>
#include <macgyver/DateTime.h>

#define DICT_WEEKDAY_MONDAY "WeekdayMonday"
#define DICT_WEEKDAY_TUESDAY "WeekdayTuesday"
#define DICT_WEEKDAY_WEDNESDAY "WeekdayWednesday"
#define DICT_WEEKDAY_THURSDAY "WeekdayThursday"
#define DICT_WEEKDAY_FRIDAY "WeekdayFriday"
#define DICT_WEEKDAY_SATURDAY "WeekdaySaturday"
#define DICT_WEEKDAY_SUNDAY "WeekdaySunday"

#define DICT_MONTH_JANUARY "MonthJanuary"
#define DICT_MONTH_FEBRUARY "MonthFebruary"
#define DICT_MONTH_MARCH "MonthMarch"
#define DICT_MONTH_APRIL "MonthApril"
#define DICT_MONTH_MAY "MonthMay"
#define DICT_MONTH_JUNE "MonthJune"
#define DICT_MONTH_JULY "MonthJuly"
#define DICT_MONTH_AUGUST "MonthAugust"
#define DICT_MONTH_SEPTEMPER "MonthSeptember"
#define DICT_MONTH_OCTOBER "MonthOctober"
#define DICT_MONTH_NOVEMBER "MonthNovember"
#define DICT_MONTH_DECEMBER "MonthDecember"

class NFmiLocation;

//! Undocumented
class NFmiTime : public NFmiStaticTime
{
 public:
  NFmiTime();
  NFmiTime(const NFmiTime &aTime);
  NFmiTime(long datePart, long timePart);

  NFmiTime(short year, short month, short day);
  NFmiTime(short year, short month, short day, short hour, short minute = 0, short sec = 0);

  NFmiTime(const Fmi::DateTime &thePosixTime);
  NFmiTime(const Fmi::LocalDateTime &theLocalTime);

  NFmiTime &operator=(const NFmiTime &theTime);

  static void Init(const FmiLanguage theLanguage);

  // HUOM! tämä voi olla hämäävä, koska time_t on integer,
  // (pitäisikö thehdä SetTime-metodi konstruktorin sijaan?)

  explicit NFmiTime(std::time_t theTime);

  // Use always even numbers between 0->12
  // 0 = Real length of the night (not implemented yet)

  bool IsNight(const NFmiLocation &theLocation, unsigned short theResolution = 12);

  // Use always even numbers between 0->12
  // 0 = Real length of the day (not implemented yet)
  bool IsDay(const NFmiLocation &theLocation, unsigned short theResolution = 12);

  void ChangeBySeconds(long seconds);
  void ChangeByMinutes(long minutes);
  void ChangeByHours(long hours);
  void ChangeByDays(long days);

  long DifferenceInMinutes(const NFmiTime &anotherTime) const;
  long DifferenceInHours(const NFmiTime &anotherTime) const;
  long DifferenceInDays(const NFmiTime &anotherTime) const;

  short GetZoneDifferenceHour() const;
  short GetWeekday() const;  // mon=1, tue=2,..., sat=6,  sun=7

  const NFmiString Weekday(const FmiLanguage theLanguage = kFinnish) const;
  const NFmiString MonthName(const FmiLanguage theLanguage = kFinnish) const;

  using NFmiStaticTime::ToStr;

  virtual const NFmiString ToStr(const NFmiString theTimeFormat,
                                 const FmiLanguage theLanguage = kFinnish) const;

  void PrintWeekday() const;
  short GetJulianDay() const;

  virtual const NFmiTime UTCTime(float theLongitude = kFloatMissing) const;
  virtual const NFmiTime UTCTime(const NFmiLocation &theLocation) const;
  virtual const NFmiTime LocalTime(float theLongitude = kFloatMissing) const;
  virtual const NFmiTime LocalTime(const NFmiLocation &theLocation) const;

  virtual void SetLocalPlace(float theLongitude);

  static short DaysInYear(short aYear);
  static short DaysInMonth(short aMonth, short aYear);

  long GetCompareValue() const;

 protected:
  void DecodeCompareValue(long aCompareValue);
  void SetZoneDifferenceHour();
  short CalcZoneDifferenceHour(float theLongitude) const;
  const NFmiString RelativeDay(FmiLanguage theLanguage,
                               NFmiString theFormat,
                               int &thePlusInd) const;

  static std::string GetDictionaryString(const char *theMagicWord);

 private:
  short itsZoneDifferenceHour;

};  // class NFmiTime

//! Undocumented, should be removed
typedef NFmiTime *PTFmiTime;

//! Undocumented, should be removed
typedef NFmiTime TFmiTime;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiTime::NFmiTime() : NFmiStaticTime(), itsZoneDifferenceHour()
{
  SetZoneDifferenceHour();
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param aTime The object being copied
 */
// ----------------------------------------------------------------------

inline NFmiTime::NFmiTime(const NFmiTime &aTime)
    : NFmiStaticTime(aTime), itsZoneDifferenceHour(aTime.itsZoneDifferenceHour)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param year Undocumented
 * \param month Undocumented
 * \param day Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiTime::NFmiTime(short year, short month, short day)
    : NFmiStaticTime(year, month, day), itsZoneDifferenceHour()
{
  SetZoneDifferenceHour();
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param year Undocumented
 * \param month Undocumented
 * \param day Undocumented
 * \param hour Undocumented
 * \param minute Undocumented
 * \param sec Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiTime::NFmiTime(short year, short month, short day, short hour, short minute, short sec)
    : NFmiStaticTime(year, month, day, hour, minute, sec), itsZoneDifferenceHour()
{
  SetZoneDifferenceHour();
}

// ----------------------------------------------------------------------
/*!
 * \param minutes Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTime::ChangeByMinutes(long minutes)
{
  if (minutes)
    DecodeCompareValue(GetCompareValue() + minutes);
}

// ----------------------------------------------------------------------
/*!
 * \param hours Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTime::ChangeByHours(long hours)
{
  if (hours)
    DecodeCompareValue(GetCompareValue() + 60L * hours);
}

// ----------------------------------------------------------------------
/*!
 * \param days Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTime::ChangeByDays(long days)
{
  if (days)
    DecodeCompareValue(GetCompareValue() + 60L * 24L * days);
}

// ----------------------------------------------------------------------
/*!
 * \param anotherTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiTime::DifferenceInMinutes(const NFmiTime &anotherTime) const
{
  return (GetCompareValue() - anotherTime.GetCompareValue());
}

// ----------------------------------------------------------------------
/*!
 * \param anotherTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiTime::DifferenceInHours(const NFmiTime &anotherTime) const
{
  return (DifferenceInMinutes(anotherTime) / 60L);
}

// ----------------------------------------------------------------------
/*!
 * \param anotherTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiTime::DifferenceInDays(const NFmiTime &anotherTime) const
{
  return (DifferenceInMinutes(anotherTime) / 60L / 24L);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline short NFmiTime::GetZoneDifferenceHour() const
{
  return itsZoneDifferenceHour;
}

// ======================================================================
