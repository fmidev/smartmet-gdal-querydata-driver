// ======================================================================
/*!
 * \file NFmiTimeDescriptor.h
 * \brief Interface of class NFmiTimeDescriptor
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"
#include "NFmiGlobals.h"
#include "NFmiMetTime.h"
#include "NFmiSaveBaseFactory.h"
#include "NFmiTimeBag.h"

class NFmiTimeList;

//! Undocumented
class NFmiTimeDescriptor : public NFmiDataDescriptor
{
 public:
  virtual ~NFmiTimeDescriptor();

  NFmiTimeDescriptor();

  NFmiTimeDescriptor(const NFmiTimeDescriptor &theTimeDescriptor);

  NFmiTimeDescriptor(const NFmiMetTime &theOriginTime,
                     const NFmiTimeBag &theValidTimeBag,
                     FmiTimeLocalzation theIsLocalTime = kUTC,
                     bool theIsInterpolation = false);

  NFmiTimeDescriptor(const NFmiMetTime &theOriginTime,
                     const NFmiTimeList &theTimeList,
                     FmiTimeLocalzation theIsLocalTime = kUTC,
                     bool theIsInterpolation = false);

  NFmiTimeDescriptor(const NFmiTimeBag &theOriginTimeBag, const NFmiMetTime &theValidTime);

  NFmiTimeDescriptor(const NFmiTimeBag &theOriginTimeBag, unsigned long theForecastPeriod);

  NFmiTimeDescriptor(const NFmiMetTime &theOriginTimeBag,
                     unsigned long theForecastPeriodMin,
                     unsigned long theForecastPeriodMax);

  // -----------------------------------------------------------------
  // Following methods operate in the list part of the TimeDescriptor;
  // it can be either OriginTimeBag, or ValidTimeBag, depending on how
  // the TimeDescriptor was constructed
  // -----------------------------------------------------------------

  virtual bool Next();
  virtual bool Previous();
  virtual void Reset();
  void First()
  {
    Reset();
    Next();
  };

  const NFmiMetTime &Time() const;
  bool Time(const NFmiMetTime &theTime);
  bool TimeToNearestStep(const NFmiMetTime &theTime,
                         FmiDirection theDirection = kCenter,
                         unsigned long theTimeRangeInMinutes = kUnsignedLongMissing);
  bool Time(const unsigned long &theIndex);
  const NFmiMetTime &FirstTime() const;
  const NFmiMetTime &LastTime() const;

  // -------------------------------------------------
  // ForecastPeriod(): returns current forecast length
  // -------------------------------------------------

  unsigned long ForecastPeriod() const;

  // ------------------------------------------------------------------------------------
  // If OriginTime (or ValidTime) is a list, method OriginTime() (or ValidTime() ) is
  // equivalent with Time(); it returns the current list item.
  // If OriginTime (or ValidTime) is'nt a list, it always returns the constant time
  // ------------------------------------------------------------------------------------

  const NFmiMetTime &OriginTime() const;
  void OriginTime(const NFmiMetTime &newTime);
  const NFmiMetTime &ValidTime() const;

  bool IsOriginTime() const;
  bool IsValidTime() const;

  // ---------------------------------------------------------------------
  // Following methods operate just in TimeDescriptor's list part
  // ---------------------------------------------------------------------

  virtual unsigned long Index() const;
  virtual unsigned long Size() const;
  bool IsEmpty() const;  // Jouduin tekemään IsEmpty -metodin, koska Size -metodi palauttaa
  // joskus tyhjänä 1:n (NFmiTimeBag palauttaa tyhjänä 1:n kun resolution
  // on 0 jostain historiallisista v. 1998 syistä)
  virtual unsigned long SizeActive() const;

  virtual bool NextActive();
  virtual bool PreviousActive();
  virtual bool FirstActive();
  virtual bool LastActive();
  virtual bool IsActive() const;
  virtual bool SetActivity(bool theActivityState);
  virtual bool SetActivePeriod(bool theActivityState, const NFmiTimeBag &thePeriod);
  virtual NFmiTimeBag GetActivePeriod();

  void SetLocalTimes(const float theLongitude);  // Muuttaa ajan paikalliseksi
  FmiTimeLocalzation TimeLocalzation() const;
  bool IsInterpolation() const;
  bool IsOriginLastest() const;
  void OriginLastest(bool theLastest);
  const NFmiTimePerioid Resolution() const;
  const NFmiTimePerioid ActiveResolution();

  NFmiTimeBag *ValidTimeBag() const;
  NFmiTimeBag *OriginTimeBag() const;
  NFmiTimeList *ValidTimeList() const;
  bool UseTimeList() const;

  NFmiTimeDescriptor &operator=(const NFmiTimeDescriptor &theTimeDescriptor);
  bool operator==(const NFmiTimeDescriptor &theTimeDescriptor) const;

  virtual NFmiTimeDescriptor Combine(const NFmiTimeDescriptor &theCombine,
                                     int theStartTimeFunction = 0,
                                     int theEndTimeFunction = 0) const;
  NFmiTimeDescriptor GetIntersection(const NFmiMetTime &theStartLimit,
                                     const NFmiMetTime &theEndLimit) const;
  void PruneTimes(int theMaxTimeCount, bool fFromEnd = true);

  virtual unsigned long ClassId() const;
  virtual const char *ClassName() const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  unsigned long LocalTimeStep();
  void ReduseTimeBag();
  void ExtendTimeBag();
  //  unsigned long TimeResolution(); // poistettu, käytä Resolution-metodia
  bool IsInside(const NFmiMetTime &theTime) const;
  bool FindNearestTime(const NFmiMetTime &theTime,
                       FmiDirection theDirection = kCenter,
                       unsigned long theTimeRangeInMinutes = kUnsignedLongMissing);
  void SetNewStartTime(const NFmiMetTime &theTime);

 protected:
  void Destroy();

 private:
  // ----
  // data
  // ----

  NFmiTimeBag *itsOriginTimeBag;
  NFmiTimeBag *itsValidTimeBag;
  NFmiTimeList *itsTimeList;

  // -------------------------------------------------------
  // ItsTimeBagIdent: True if validTime, False if originTime
  // -------------------------------------------------------

  bool itsTimeBagIdent;
  FmiTimeLocalzation itsIsLocalTime;
  bool itsIsInterpolation;
  bool itsIsOriginLastest;
  unsigned long itsLocalTimeStep;  // Kesken
  bool *itsActivity;

  // Hmm, should be removed??
  friend class NFmiDB;

};  // class NFmiTimeDescriptor

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiTimeLocalzation NFmiTimeDescriptor::TimeLocalzation() const
{
  return itsIsLocalTime;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::IsInterpolation() const
{
  return itsIsInterpolation;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::IsOriginLastest() const
{
  return itsIsOriginLastest;
}
// ----------------------------------------------------------------------
/*!
 * \param theLastest Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTimeDescriptor::OriginLastest(bool theLastest)
{
  itsIsOriginLastest = theLastest;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiTimeBag *NFmiTimeDescriptor::ValidTimeBag() const
{
  return itsValidTimeBag;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiTimeBag *NFmiTimeDescriptor::OriginTimeBag() const
{
  return itsOriginTimeBag;
}
inline NFmiTimeList *NFmiTimeDescriptor::ValidTimeList() const
{
  return itsTimeList;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTimeDescriptor::ForecastPeriod() const
{
  return itsTimeList ? 0 : (ValidTime().DifferenceInMinutes(OriginTime()));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::IsOriginTime() const
{
  return !itsTimeBagIdent;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::IsValidTime() const
{
  return itsTimeBagIdent;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::IsActive() const
{
  return (itsActivity[Index()]);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTimeDescriptor::UseTimeList() const
{
  return itsTimeList != 0;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTimeDescriptor::ClassId() const
{
  return kNFmiTimeDescriptor;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiTimeDescriptor::ClassName() const
{
  return "NFmiTimeDescriptor";
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTimeDescriptor::LocalTimeStep()
{
  return itsLocalTimeStep;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
/*
inline
unsigned long NFmiTimeDescriptor::TimeResolution()
{
  return itsValidTimeBag->Resolution();
}
*/
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiTimeDescriptor
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiTimeDescriptor &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiTimeDescriptor
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiTimeDescriptor &ob)
{
  return ob.Read(file);
}

// ======================================================================
