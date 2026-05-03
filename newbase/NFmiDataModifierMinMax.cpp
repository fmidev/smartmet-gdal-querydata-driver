// ======================================================================
/*!
 * \file NFmiDataModifierMinMax.cpp
 * \brief Implementation of class NFmiDataModifierMinMax
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMinMax
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierMinMax.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierMinMax::~NFmiDataModifierMinMax() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierMinMax::NFmiDataModifierMinMax() : itsMinValue(FLT_MAX), itsMaxValue(-FLT_MAX) {}

NFmiDataModifierMinMax::NFmiDataModifierMinMax(const NFmiDataModifierMinMax& theOther)

    = default;

NFmiDataModifier* NFmiDataModifierMinMax::Clone() const
{
  return new NFmiDataModifierMinMax(*this);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierMinMax::MinValue()
{
  if (itsMinValue != FLT_MAX)
    return itsMinValue;
  else
    return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierMinMax::MaxValue()
{
  if (itsMaxValue != -FLT_MAX)
    return itsMaxValue;
  else
    return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMinMax::Calculate(float theValue)
{
  if (CheckMissingValues(theValue))
  {
    if (itsMaxValue < theValue)
      itsMaxValue = theValue;
    if (itsMinValue > theValue)
      itsMinValue = theValue;
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMinMax::Clear()
{
  NFmiDataModifier::Clear();
  itsMaxValue = -FLT_MAX;
  itsMinValue = FLT_MAX;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierMinMax::CalculationResult()
{
  return kFloatMissing;
}
// ======================================================================
