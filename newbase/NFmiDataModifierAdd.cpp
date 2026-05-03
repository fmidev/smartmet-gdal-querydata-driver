// ======================================================================
/*!
 * \file NFmiDataModifierAdd.cpp
 * \brief Implementation of class NFmiDataModifierAdd
 */
// ======================================================================
/*!
 * \class NFmiDataModifierAdd
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierAdd.h"

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theAddValue Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierAdd::NFmiDataModifierAdd(float theAddValue) : itsAddValue(theAddValue) {}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierAdd::FloatOperation(float theValue)
{
  if (CheckMissingValues(theValue))
    return (theValue + itsAddValue);
  else
    return theValue;
}

// ======================================================================
