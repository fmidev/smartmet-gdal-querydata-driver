// ======================================================================
/*!
 * \file NFmiMap.h
 * \brief Interface of class NFmiMap
 */
// ======================================================================

#pragma once

#include "NFmiRect.h"

//! Undocumented
class NFmiMap
{
 public:
  virtual ~NFmiMap();
  NFmiMap();
  NFmiMap(const NFmiMap &theMap);
  NFmiMap(const NFmiPoint &theOffset, const NFmiPoint &theScale);

  bool operator==(const NFmiMap &theMap) const;
  bool operator!=(const NFmiMap &theMap) const;
  const NFmiPoint Transform(const NFmiPoint &thePoint) const;
  const NFmiPoint Reverse(const NFmiPoint &thePoint) const;
  const NFmiRect Transform(const NFmiRect &theRect) const;

 private:
  NFmiPoint itsOffset;
  NFmiPoint itsScale;

};  // class NFmiMap

//! Undocumented, should be removed

typedef NFmiMap *PNFmiMap;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiMap::~NFmiMap() {}
// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theMap The object to compare with
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

inline bool NFmiMap::operator==(const NFmiMap &theMap) const
{
  return (itsOffset == theMap.itsOffset && itsScale == theMap.itsScale);
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theMap The object to compare with
 * \return True if the objects are not equal
 */
// ----------------------------------------------------------------------

inline bool NFmiMap::operator!=(const NFmiMap &theMap) const
{
  return !operator==(theMap);
}
// ----------------------------------------------------------------------
/*!
 * \param thePoint Undocumented
 * \return Undocumented
 */
//___________________________________________________________ Scaled

inline const NFmiPoint NFmiMap::Transform(const NFmiPoint &thePoint) const
{
  NFmiPoint newPoint(thePoint);
  newPoint *= itsScale;
  newPoint += itsOffset;
  return newPoint;
}

// ----------------------------------------------------------------------
/*!
 * \param theRect Undocumented
 * \return theRect Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiMap::Transform(const NFmiRect &theRect) const
{
  return NFmiRect(Transform(theRect.TopLeft()), Transform(theRect.BottomRight()));
}

// ----------------------------------------------------------------------
/*!
 * \param thePoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiMap::Reverse(const NFmiPoint &thePoint) const
{
  NFmiPoint newPoint(thePoint);
  newPoint -= itsScale;
  newPoint /= itsOffset;
  return newPoint;
}

// ======================================================================
