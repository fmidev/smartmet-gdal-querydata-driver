// ======================================================================
/*!
 * \file NFmiDataModifierMin.cpp
 * \brief Implementation of class NFmiDataModifierMin
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMin
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierMin.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierMin::~NFmiDataModifierMin() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierMin::NFmiDataModifierMin()
{
  itsExtremeValue = 3.4E+38f;
}
NFmiDataModifierMin::NFmiDataModifierMin(const NFmiDataModifierMin& theOther)

    = default;

NFmiDataModifier* NFmiDataModifierMin::Clone() const
{
  return new NFmiDataModifierMin(*this);
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------
/*
void NFmiDataModifierMin::Calculate(float theValue)
{
  if(theValue != kFloatMissing)
        if(itsExtremeValue > theValue)
          itsExtremeValue = theValue;
}
*/
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMin::Clear()
{
  itsExtremeValue = 3.4E+38f;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierMin::CalculationResult()
{
  return itsExtremeValue != 3.4E+38f && !(!fMissingValuesAllowed && itsNumberOfMissingValues > 0)
             ? itsExtremeValue
             : kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierMin::IsNewExtreme(float value)
{
  return value < itsExtremeValue;
}
// ======================================================================
