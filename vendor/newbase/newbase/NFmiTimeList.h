// ======================================================================
/*!
 * \file NFmiTimeList.h
 * \brief Interface of class NFmiTimeList
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"

class NFmiMetTime;
class NFmiTimeBag;

//! Undocumented
class NFmiTimeList
{
 public:
  virtual ~NFmiTimeList();
  NFmiTimeList();
  NFmiTimeList(const NFmiTimeList &theList);
  NFmiTimeList(const NFmiTimeBag &theTimes);

  NFmiTimeList &operator=(const NFmiTimeList &theList);
  bool operator==(const NFmiTimeList &theList) const;

  virtual void Add(NFmiMetTime *theItem, bool fAllowDuplicates = false, bool fAddEnd = true);
  virtual void AddOver(NFmiMetTime *theItem);
  virtual void Add(NFmiTimeList *theList);
  void Clear(bool fDeleteData = false);

  bool Reset() const;
  bool First() const;
  bool Next(NFmiMetTime **theItem) const;
  bool Next() const;
  bool Previous() const;
  bool Find(const NFmiMetTime &theTime);
  bool FindNearestTime(const NFmiMetTime &theTime,
                       FmiDirection theDirection = kCenter,
                       unsigned long theTimeRangeInMinutes = kUnsignedLongMissing);
  NFmiMetTime *Current() const;
  const NFmiMetTime &FirstTime() const;
  const NFmiMetTime &LastTime() const;
  int CurrentResolution() const;

  const NFmiTimeList Combine(NFmiTimeList &theList,
                             int theStartTimeFunction = 0,
                             int theEndTimeFunction = 0);
  const NFmiTimeList GetIntersection(const NFmiMetTime &theStartLimit,
                                     const NFmiMetTime &theEndLimit);
  void PruneTimes(int theMaxTimeCount, bool fFromEnd = true);

  int NumberOfItems() const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  bool Index(int theIndex) const;
  const NFmiMetTime *Time(int theIndex) const;
  int Index() const;
  int FindNearestTimes(const NFmiMetTime &theTime,
                       int theMaxMinuteRange,
                       NFmiMetTime &theTime1,
                       NFmiMetTime &theTime2);
  bool IsInside(const NFmiMetTime &theTime) const;
  void SetNewStartTime(const NFmiMetTime &theTime);

 private:
  bool TimeInSearchRange(const NFmiMetTime &theTime,
                         unsigned long theTimeRangeInMinutes);  // apu funktio nearestTime:en
  bool IndexOk(int theIndex) const;
  bool FindNearestBackwardTime(std::vector<NFmiMetTime *>::iterator &firstNotLess,
                               const NFmiMetTime &theTime,
                               unsigned long theTimeRangeInMinutes);
  bool FindNearestForwardTime(std::vector<NFmiMetTime *>::iterator &firstNotLess,
                              const NFmiMetTime &theTime,
                              unsigned long theTimeRangeInMinutes);
  bool FindNearestTime(std::vector<NFmiMetTime *>::iterator &firstNotLess,
                       const NFmiMetTime &theTime,
                       unsigned long theTimeRangeInMinutes);
  int CalcTimeListIndex(const std::vector<NFmiMetTime *>::iterator &theIter);
  bool IsSearchedTimeInRange(std::vector<NFmiMetTime *>::iterator &foundTimeIter,
                             const NFmiMetTime &theTime,
                             unsigned long theTimeRangeInMinutes);
  bool CheckFoundTimeIter(std::vector<NFmiMetTime *>::iterator &foundTimeIter,
                          const NFmiMetTime &theTime,
                          unsigned long theTimeRangeInMinutes);

  std::vector<NFmiMetTime *> itsVectorList;  // muutin nimen, että se erottuu vanhasta
                                             //  NFmiVoidPtrList *	itsList;
  mutable int itsIndex;

  //  mutable NFmiVoidPtrIterator* itsIter;
  mutable bool itsIsReset;

};  // class NFmiTimeList

//! Undocumented, should be removed

typedef NFmiTimeList *PNFmiTimeList;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiTimeList::NFmiTimeList()
    : itsVectorList(),
      itsIndex(-1)
      //  : itsList(new NFmiVoidPtrList)
      //  , itsIter(0)
      ,
      itsIsReset(false)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiTimeList::NumberOfItems() const
{
  //  return itsList ? itsList->NumberOfItems() : 0;
  return static_cast<int>(itsVectorList.size());
}

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiTimeList
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiTimeList &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiTimeList
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */

// ----------------------------------------------------------------------
inline std::istream &operator>>(std::istream &file, NFmiTimeList &ob)
{
  return ob.Read(file);
}

// ======================================================================
