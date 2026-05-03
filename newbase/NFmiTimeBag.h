// ======================================================================
/*!
 * \file NFmiTimeBag.h
 * \brief Interface of class NFmiTimeBag
 */
// ======================================================================

#pragma once

#include "NFmiMetTime.h"
#include "NFmiSize.h"

//! Undocumented
class NFmiTimeBag : public NFmiSize
{
 public:
  NFmiTimeBag();
  NFmiTimeBag(const NFmiTimeBag &theBag);
  NFmiTimeBag(const NFmiMetTime &theFirstTime,
              const NFmiMetTime &theLastTime,
              NFmiTimePerioid theTimeResolution);

  const NFmiMetTime &FirstTime() const;
  const NFmiMetTime &LastTime() const;
  const NFmiMetTime &CurrentTime() const;
  unsigned long TimeFromStart() const;
  const NFmiTimePerioid &Resolution() const;
  virtual const NFmiTimeBag Combine(const NFmiTimeBag &theBag,
                                    int theStartTimeFunction = 0,
                                    int theEndTimeFunction = 0);  // should be const. Look at .cpp
  bool CalcIntersection(const NFmiTimeBag &theBag2, NFmiTimeBag &refBag, bool resolOfBag2 = false);
  const NFmiTimeBag GetIntersection(const NFmiMetTime &theStartLimit,
                                    const NFmiMetTime &theEndLimit);
  void PruneTimes(int theMaxTimeCount, bool fFromEnd = true);

  virtual unsigned long GetSize(
      void) const;       //??? Tarvitaanko täällä? //should be const. Look at .cpp
  bool IsEmpty() const;  // Jouduin tekemään IsEmpty -metodin, NFmiTimeBag palauttaa tyhjänä 1:n
                         // kun resolution on 0 jostain historiallisista v. 1998 syistä
  virtual bool Next();
  virtual bool Previous();
  virtual void Reset(FmiDirection directionToIter = kForward);
  bool SetCurrent(const NFmiMetTime &theTime);
  void SetNewStartTime(const NFmiMetTime &theTime);
  bool SetTime(unsigned long theIndex);
  bool IsInside(const NFmiMetTime &theTime) const;
  void MoveByMinutes(long minutes);
  bool FindNearestTime(const NFmiMetTime &theTime,
                       FmiDirection theDirection = kCenter,
                       unsigned long theTimeRangeInMinutes = kUnsignedLongMissing);

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual const char *ClassName() const;

  NFmiTimeBag &operator=(const NFmiTimeBag &theTimeBag);
  bool operator==(const NFmiTimeBag &theTimeBag) const;

 private:
  NFmiMetTime itsFirstTime;
  NFmiMetTime itsCurrentTime;
  NFmiMetTime itsLastTime;
  NFmiTimePerioid itsResolution;

};  // class NFmiTimeBag

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiTimeBag::NFmiTimeBag()
    : NFmiSize(),
      itsFirstTime(NFmiMetTime()),
      itsCurrentTime(itsFirstTime),
      itsLastTime(itsFirstTime),
      itsResolution(0)
{
  NFmiTimeBag::Reset();
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 */
// ----------------------------------------------------------------------

inline NFmiTimeBag::NFmiTimeBag(const NFmiMetTime &theFirstTime,
                                const NFmiMetTime &theLastTime,
                                NFmiTimePerioid theTimeResolution)
    : NFmiSize(),
      itsFirstTime(theFirstTime),
      itsCurrentTime(itsFirstTime),
      itsLastTime(theLastTime),
      itsResolution(theTimeResolution)
{
  NFmiTimeBag::Reset();
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theBag The object to copy
 */
// ----------------------------------------------------------------------

inline NFmiTimeBag::NFmiTimeBag(const NFmiTimeBag &theBag)
    : NFmiSize(theBag.itsIndex, theBag.itsSize),
      itsFirstTime(theBag.itsFirstTime),
      itsCurrentTime(theBag.itsCurrentTime),
      itsLastTime(theBag.itsLastTime),
      itsResolution(theBag.itsResolution)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiMetTime &NFmiTimeBag::FirstTime() const
{
  return itsFirstTime;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiMetTime &NFmiTimeBag::LastTime() const
{
  return itsLastTime;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiMetTime &NFmiTimeBag::CurrentTime() const
{
  return itsCurrentTime;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTimeBag::TimeFromStart() const
{
  return CurrentIndex() * itsResolution;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiTimePerioid &NFmiTimeBag::Resolution() const
{
  return itsResolution;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiTimeBag::ClassName() const
{
  return "NFmiTimeBag";
}
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiTimeBag
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiTimeBag &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiTimeBag
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiTimeBag &ob)
{
  return ob.Read(file);
}

// ======================================================================
