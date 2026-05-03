// ======================================================================
/*!
 * \file NFmiVoid.h
 * \brief Interface of class NFmiVoid
 */
// ======================================================================
/*!
 * \class NFmiVoid
 *
 * A helper class for NFmiDataPool. This really should be documented better,
 * especially since the member variables are not encapsulated.
 *
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"

//! Undocumented
class NFmiVoid
{
 public:
  ~NFmiVoid();
  NFmiVoid();
  NFmiVoid& operator=(const NFmiVoid& theVoid);

  void* itsVoid;
  unsigned long itsNumber;
  unsigned short itsUnitSize;
  unsigned long itsSize;

 private:
  NFmiVoid(const NFmiVoid& theVoid);

};  // class NFmiVoid

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special.
 */
// ----------------------------------------------------------------------

inline NFmiVoid::~NFmiVoid() {}
// ----------------------------------------------------------------------
/*!
 * Void constructor does nothing special, meaning all member variables
 * will be zeroed.
 */
// ----------------------------------------------------------------------

inline NFmiVoid::NFmiVoid() : itsVoid(), itsNumber(), itsUnitSize(), itsSize() {}
// ----------------------------------------------------------------------
/*!
 * Assignment operator performs a bitwise copy.
 *
 * \param theVoid The object to copy
 * \return This
 */
// ----------------------------------------------------------------------

inline NFmiVoid& NFmiVoid::operator=(const NFmiVoid& theVoid)
{
  itsVoid = theVoid.itsVoid;
  itsNumber = theVoid.itsNumber;
  itsUnitSize = theVoid.itsUnitSize;
  itsSize = theVoid.itsSize;
  return *this;
}

// ======================================================================
