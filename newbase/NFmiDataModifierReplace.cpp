// ======================================================================
/*!
 * \file NFmiDataModifierReplace.cpp
 * \brief Implementation of class NFmiDataModifierReplace
 */
// ======================================================================
/*!
 * \class NFmiDataModifierReplace
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierReplace.h"

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theOrigValue Undocumented
 * \param theNewValue Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierReplace::NFmiDataModifierReplace(float theOrigValue, float theNewValue)
    : itsOriginalValue(theOrigValue), itsNewValue(theNewValue)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierReplace::FloatOperation(float theValue)
{
  if (theValue == itsOriginalValue)
    return itsNewValue;
  else
    return theValue;
}

// ======================================================================
