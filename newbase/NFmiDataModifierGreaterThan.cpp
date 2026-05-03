// ======================================================================
/*!
 * \file NFmiDataModifierGreaterThan.cpp
 * \brief Implementation of class NFmiDataModifierGreaterThan
 */
// ======================================================================
/*!
 * \class NFmiDataModifierGreaterThan
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierGreaterThan.h"

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierGreaterThan::NFmiDataModifierGreaterThan(float theValue) : itsLimit(theValue) {}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierGreaterThan::BoolOperation(float theValue)
{
  return theValue > itsLimit;
}
// ======================================================================
