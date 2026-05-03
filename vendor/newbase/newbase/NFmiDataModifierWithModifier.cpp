// ======================================================================
/*!
 * \file NFmiDataModifierWithModifier.cpp
 * \brief Implementation of class NFmiDataModifierWithModifier
 */
// ======================================================================
/*!
 * \class NFmiDataModifierWithModifier
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierWithModifier.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierWithModifier::FloatOperation(float /* theValue */)
{
  itsModifier->Clear();
  for (int i = -itsXRange; i <= itsXRange; i++)
    for (int j = -itsYRange; j <= itsYRange; j++)
    {
      itsModifier->Calculate(itsQueryInfoCopy->PeekLocationValue(i, j));
    }
  return itsModifier->CalculationResult();
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierWithModifier::Calculate(float theValue)
{
  FloatOperation(theValue);
}
// ======================================================================
