// ======================================================================
/*!
 * \file NFmiSortable.h
 * \brief Interface of class NFmiSortable
 */
// ======================================================================
/*!
 * \class NFmiSortable
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"

//! Undocumented
class NFmiSortable
{
 public:
  virtual ~NFmiSortable();
  NFmiSortable();
  virtual bool IsEqual(const NFmiSortable& aFmiTest) const = 0;
  virtual bool IsLessThan(const NFmiSortable& aFmiTest) const = 0;

  bool operator<(const NFmiSortable& aObjTest) const;
  bool operator>(const NFmiSortable& aObjTest) const;
  bool operator>=(const NFmiSortable& aObjTest) const;
  bool operator<=(const NFmiSortable& aObjTest) const;

  bool operator==(const NFmiSortable& aObjTest) const;
  bool operator!=(const NFmiSortable& aObjTest) const;

};  // class NFmiSortable

// ----------------------------------------------------------------------
/*!
 * \param aObjTest The object to compare with
 * \return True if the comparison result is true
 */
// ----------------------------------------------------------------------

inline bool NFmiSortable::operator==(const NFmiSortable& aObjTest) const
{
  return (IsEqual(aObjTest));
}

// ----------------------------------------------------------------------
/*!
 * \param aObjTest The object to compare with
 * \return True if the comparison result is true
 */
// ----------------------------------------------------------------------

inline bool NFmiSortable::operator!=(const NFmiSortable& aObjTest) const
{
  return (!(IsEqual(aObjTest)));
}

// ----------------------------------------------------------------------
/*!
 * \param aObjTest The object to compare with
 * \return True if the comparison result is true
 */
// ----------------------------------------------------------------------

inline bool NFmiSortable::operator<(const NFmiSortable& aObjTest) const
{
  return (IsLessThan(aObjTest));
}

// ----------------------------------------------------------------------
/*!
 * \param aObjTest The object to compare with
 * \return True if the comparison result is true
 */
// ----------------------------------------------------------------------

inline bool NFmiSortable::operator>(const NFmiSortable& aObjTest) const
{
  return (!(*this < aObjTest) && (*this != aObjTest));
}

// ----------------------------------------------------------------------
/*!
 * \param aObjTest The object to compare with
 * \return True if the comparison result is true
 */
// ----------------------------------------------------------------------

inline bool NFmiSortable::operator>=(const NFmiSortable& aObjTest) const
{
  return (!(*this < aObjTest));
}

// ----------------------------------------------------------------------
/*!
 * \param aObjTest The object to compare with
 * \return True if the comparison result is true
 */
// ----------------------------------------------------------------------

inline bool NFmiSortable::operator<=(const NFmiSortable& aObjTest) const
{
  return (*this < aObjTest || *this == aObjTest);
}

// Marko trying to get rid of fmibase library
//! Undocumented, should be removed.
typedef NFmiSortable TSortable;

// ======================================================================
