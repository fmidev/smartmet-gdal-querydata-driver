// ======================================================================
/*!
 * \file NFmiDataModifierSingleExtremeValuesFilter.cpp
 * \brief Interface of class NFmiDataModifierSingleExtremeValuesFilter
 */
// ======================================================================
/*!
 * \class NFmiDataModifierSingleExtremeValuesFilter
 *
 * Poistaa yksittäiset äärikaiut, jotka ylittävät annetun raja-arvon (itsLimit),
 * mm. Tallinnan laivat
 *
 */
// ======================================================================

#include "NFmiDataModifierSingleExtremeValuesFilter.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierSingleExtremeValuesFilter::~NFmiDataModifierSingleExtremeValuesFilter() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierSingleExtremeValuesFilter::NFmiDataModifierSingleExtremeValuesFilter()
    : NFmiDataModifier(), itsAvgModifier(), itsValue(), itsLimit()
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param thePointValue Undocumented
 * \param theLimit Undocumented
 * \todo Use an initialization list
 */
// ----------------------------------------------------------------------

NFmiDataModifierSingleExtremeValuesFilter::NFmiDataModifierSingleExtremeValuesFilter(
    float thePointValue, float theLimit)
    : itsAvgModifier(), itsValue(thePointValue), itsLimit(theLimit)
{
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierSingleExtremeValuesFilter::Clear() {}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierSingleExtremeValuesFilter::Calculate(float theValue)
{
  if (itsValue >= itsLimit && itsValue != kFloatMissing && itsLimit != kFloatMissing)
    itsAvgModifier.Calculate(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierSingleExtremeValuesFilter::CalculationResult()
{
  float avg = itsAvgModifier.CalculationResult();
  if ((avg / itsValue < 0.12 && itsValue != kFloatMissing))
    return 0;
  else
    return itsValue;
}

// ======================================================================
