// ======================================================================
/*!
 * \file NFmiDataModifierChange.cpp
 * \brief Implementation of class NFmiDataModifierChange
 */
// ======================================================================

#include "NFmiDataModifierChange.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierChange::~NFmiDataModifierChange() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierChange::NFmiDataModifierChange()
    : itsFirstValueOK(false), itsFirstValue(kFloatMissing), itsLastValue(kFloatMissing)
{
}

NFmiDataModifier* NFmiDataModifierChange::Clone() const
{
  return new NFmiDataModifierChange(*this);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierChange::Calculate(float theValue)
{
  if (!itsFirstValueOK)
  {
    itsFirstValueOK = true;
    itsFirstValue = theValue;
  }
  itsLastValue = theValue;
}

// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierChange::Calculate(NFmiQueryInfo* theQI)
{
  if (theQI)
    Calculate(theQI->FloatValue());
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierChange::Clear()
{
  itsFirstValueOK = false;
  itsFirstValue = kFloatMissing;
  itsLastValue = kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierChange::CalculationResult()
{
  if (!itsFirstValueOK)
    return kFloatMissing;

  if (itsFirstValue == kFloatMissing)
    return itsLastValue;

  if (itsLastValue == kFloatMissing)
    return kFloatMissing;

  return itsLastValue - itsFirstValue;
}

// ======================================================================
