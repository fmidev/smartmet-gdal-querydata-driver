// ======================================================================
/*!
 * \file NFmiStaticTime.h
 * \brief Interface of class NFmiStaticTime
 */
// ======================================================================

#pragma once

#include "NFmiSortable.h"
#include "NFmiString.h"
#include <ctime>
#include <iostream>

const unsigned long kShortYear = 1;  // YY
const unsigned long kLongYear = 2;   // YYYY
const unsigned long kMonth = 4;      // MM
const unsigned long kDay = 8;        // DD

const unsigned long kHour = 16;    // HH
const unsigned long kMinute = 32;  // MM
const unsigned long kSecond = 64;  // SS

const unsigned long kYYMMDD = kShortYear + kMonth + kDay;
const unsigned long kYYYYMMDD = kLongYear + kMonth + kDay;
const unsigned long kYYMMDDHH = kShortYear + kMonth + kDay + kHour;
const unsigned long kYYYYMMDDHH = kLongYear + kMonth + kDay + kHour;
const unsigned long kYYMMDDHHMMSS = kShortYear + kMonth + kDay + kHour + kMinute + kSecond;
const unsigned long kYYYYMMDDHHMM = kLongYear + kMonth + kDay + kHour + kMinute;
const unsigned long kYYMMDDHHMM = kShortYear + kMonth + kDay + kHour + kMinute;
const unsigned long kYYYYMMDDHHMMSS = kLongYear + kMonth + kDay + kHour + kMinute + kSecond;
const unsigned long kMMDDHH = kMonth + kDay + kHour;
const unsigned long kDDHHMM = kDay + kHour + kMinute;

//! Undocumented
class NFmiStaticTime : public NFmiSortable
{
 public:
  virtual ~NFmiStaticTime() {}
  NFmiStaticTime();
  NFmiStaticTime(const NFmiStaticTime& theTime);
  NFmiStaticTime(short year, short month, short day);
  NFmiStaticTime(short year, short month, short day, short hour, short minute = 0, short sec = 0);
  NFmiStaticTime(std::time_t theTime, bool fMakeLocal);

  virtual bool IsEqual(const NFmiSortable& aFmiTest) const;
  virtual bool IsLessThan(const NFmiSortable& aFmiTest) const;

  NFmiStaticTime& operator=(const NFmiStaticTime& theTime);

  void SetDate(short year, short month, short day);
  void GetDate(short& year, short& month, short& day) const;
  void SetTime(short hour, short minute = 0, short sec = 0);
  void GetTime(short& hour, short& minute, short& sec) const;

  short GetYear() const;
  short GetMonth() const;
  short GetDay() const;
  short GetHour() const;
  short GetMin() const;
  short GetSec() const;

  void ReadDateTime();
  void XPrint() const;
  void XPrint(const char* str) const;
  virtual void Print() const;

  const NFmiString ToStr(const unsigned long theTimeMask) const;
  void FromStr(const NFmiString theTimeString, const unsigned long theTimeMask = kYYYYMMDDHHMMSS);
  double CalcHourOfDay() const;

  void SetYear(short year);
  void SetMonth(short month);
  void SetDay(short day);
  void SetHour(short hour);
  void SetMin(short minute);
  void SetSec(short sec);
  void SetMissing();
  bool IsMissing();

  virtual const char* ClassName() const;

  virtual std::ostream& Write(std::ostream& file) const;

  std::time_t EpochTime() const;
  static std::time_t my_timegm(struct ::tm* t);

 protected:
  void _setCurrent();
  void _set2CurrentLocalTime(std::time_t theTime);
  virtual struct std::tm GetSystemTime();

 private:
  short fYear;
  short fMonth;
  short fDay;
  short fHour;
  short fMin;
  short fSec;

};  // class NFmiStaticTime

//! Undocumented, should be removed
typedef NFmiStaticTime* PTFmiStaticTime;

//! Undocumented, should be removed
typedef NFmiStaticTime TFmiStaticTime;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline short NFmiStaticTime::GetYear() const
{
  return fYear;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline short NFmiStaticTime::GetMonth() const
{
  return fMonth;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline short NFmiStaticTime::GetDay() const
{
  return fDay;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline short NFmiStaticTime::GetHour() const
{
  return fHour;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline short NFmiStaticTime::GetMin() const
{
  return fMin;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline short NFmiStaticTime::GetSec() const
{
  return fSec;
}
// ----------------------------------------------------------------------
/*!
 * \return Meneillään olevan vuorokauden tunti desimaalilukuna
 */
// ----------------------------------------------------------------------

inline double NFmiStaticTime::CalcHourOfDay() const
{
  return GetHour() + GetMin() / 60. + GetSec() / 3600.;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiStaticTime::ClassName() const
{
  return "NFmiStaticTime";
}
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiStaticTime
 *
 * \param os The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const NFmiStaticTime& ob)
{
  return ob.Write(os);
}

// ======================================================================
