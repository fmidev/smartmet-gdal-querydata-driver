// ======================================================================
/*!
 * \file NFmiTransformList.h
 * \brief Interface of class NFmiTransformList
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiPoint.h"

//! Undocumented
class NFmiTransformList
{
 public:
  virtual ~NFmiTransformList();
  NFmiTransformList(int theMaxPairNumber = 50);

  bool AddDataMapping(const NFmiPoint& firstPair, const NFmiPoint& lastPair);
  double Transform(const double value, int theRangeIndex = -1);

  const NFmiPoint FirstPair();
  const NFmiPoint LastPair();

  double FirstInputValue();
  double LastInputValue();

  double FirstInputValue(int theRangeIndex);
  double LastInputValue(int theRangeIndex);

  double FirstOutputValue(int theRangeIndex);
  double LastOutputValue(int theRangeIndex);

  int RangeIndex(double theValue);
  int RangeIndex();
  int MaxRangeIndex();

  bool IsConstantRange(int theRangeIndex);
  int PreviousInputRangeIndex();

  NFmiTransformList& operator=(const NFmiTransformList& theList);
  bool operator==(const NFmiTransformList& theList);
  bool operator!=(const NFmiTransformList& theList);
  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 protected:
  void First();
  virtual void Init(int theMaxPairNumber);
  virtual bool Allocate(int theMaxPairNumber);
  virtual bool Interpolate(const NFmiPoint& firstPair, const NFmiPoint& lastPair);
  virtual bool Interpolate(const int theRangeIndex, double theValue);
  bool Equal(NFmiPoint& p1, NFmiPoint& p2);

 private:
  NFmiTransformList(const NFmiTransformList& theList);

  int itsMaxPairNumber;

  int itsIncrementSize;
  int itsCurrentIndex;
  int itsCurrentMaxIndex;
  int itsPreviousInputRangeIndex;
  double itsPreviousInputValue;
  double itsPreviousOutputValue;
  double itsMinInputValue;
  double itsMaxInputValue;

  double itsInputValue;
  double itsOutputValue;

  NFmiPoint* itsFirstPair;
  NFmiPoint* itsLastPair;
  NFmiPoint itsVeryFirstPair;
  NFmiPoint itsVeryLastPair;

  NFmiPoint itsEps;

  double* itsOutputInputRatio;

};  // class NFmiTransformList

// ----------------------------------------------------------------------
/*!
 * \return The first of all of the pairs in the list
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiTransformList::FirstPair()
{
  return itsVeryFirstPair;
}
// ----------------------------------------------------------------------
/*!
 * \return The last of all of the pairs in the list
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiTransformList::LastPair()
{
  return itsVeryLastPair;
}
// ----------------------------------------------------------------------
/*!
 * \return The first input value of all of the pairs in the list
 */
// ----------------------------------------------------------------------

inline double NFmiTransformList::FirstInputValue()
{
  return itsVeryFirstPair.X();
}
// ----------------------------------------------------------------------
/*!
 * \return The last input value of all of the pairs in the list
 */
// ----------------------------------------------------------------------

inline double NFmiTransformList::LastInputValue()
{
  return itsVeryLastPair.X();
}
// ----------------------------------------------------------------------
/*!
 * \param theRangeIndex Undocumented
 * \return The first input value in the CURRENT range
 */
// ----------------------------------------------------------------------

inline double NFmiTransformList::FirstInputValue(int theRangeIndex)
{
  return itsFirstPair[theRangeIndex].X();
}

// ----------------------------------------------------------------------
/*!
 * \param theRangeIndex Undocumented
 * \return The last input value in the CURRENT range
 */
// ----------------------------------------------------------------------

inline double NFmiTransformList::LastInputValue(int theRangeIndex)
{
  return itsLastPair[theRangeIndex].X();
}

// ----------------------------------------------------------------------
/*!
 * \param theRangeIndex Undocumented
 * \return The first output value in the CURRENT range
 */
// ----------------------------------------------------------------------

inline double NFmiTransformList::FirstOutputValue(int theRangeIndex)
{
  return itsFirstPair[theRangeIndex].Y();
}

// ----------------------------------------------------------------------
/*!
 * \param theRangeIndex Undocumented
 * \return The last output value in the CURRENT range
 */
// ----------------------------------------------------------------------

inline double NFmiTransformList::LastOutputValue(int theRangeIndex)
{
  return itsLastPair[theRangeIndex].Y();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiTransformList::RangeIndex()
{
  return itsPreviousInputRangeIndex;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiTransformList::MaxRangeIndex()
{
  return itsCurrentMaxIndex;
}
// ----------------------------------------------------------------------
/*!
 * \param theRangeIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiTransformList::IsConstantRange(int theRangeIndex)
{
  return itsOutputInputRatio[theRangeIndex] == 0.;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiTransformList::PreviousInputRangeIndex()
{
  return itsPreviousInputRangeIndex;
}
// ----------------------------------------------------------------------
/*!
 * Global output operator from NFmiTransformList class
 *
 * \param os The output stream to write to
 * \param item The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& os, const NFmiTransformList& item)
{
  return item.Write(os);
}

// ----------------------------------------------------------------------
/*!
 * Global input operator from NFmiTransformList class
 *
 * \param is The input stream to read from
 * \param item The object to hold the read data
 * \return The input stream read from
 */
// ----------------------------------------------------------------------
inline std::istream& operator>>(std::istream& is, NFmiTransformList& item)
{
  return item.Read(is);
}

// ======================================================================
