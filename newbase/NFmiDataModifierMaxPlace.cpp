// ======================================================================
/*!
 * \file NFmiDataModifierMax.cpp
 * \brief Implementation of class NFmiDataModifierMax
 */
// ======================================================================

#include "NFmiDataModifierMaxPlace.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierMaxPlace::~NFmiDataModifierMaxPlace() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierMaxPlace::NFmiDataModifierMaxPlace()
{
  itsExtremeValue = -3.4E+38f;
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------
/*
void NFmiDataModifierMaxPlace::Calculate(float theValue)
{
  if(theValue != kFloatMissing)
        if(itsExtremeValue < theValue)
          itsExtremeValue = theValue;
}
*/
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMaxPlace::Clear()
{
  itsExtremeValue = -3.4E+38f;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierMaxPlace::CalculationResult()
{
  return itsExtremeValue != -3.4E+38f && !(!fMissingValuesAllowed && itsNumberOfMissingValues > 0)
             ? itsExtremeValue
             : kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierMaxPlace::IsNewExtreme(float value)
{
  return value > itsExtremeValue;
}
// ======================================================================
