// ======================================================================
/*!
 * \file NFmiDataModifierAvgAbs.cpp
 * \brief Implementation of class NFmiDataModifierAvgAbs
 */
// ======================================================================
/*!
 * \class NFmiDataModifierAvgAbs
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierAvgAbs.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierAvgAbs::~NFmiDataModifierAvgAbs() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierAvgAbs::NFmiDataModifierAvgAbs() : NFmiDataModifier(), itsCounter(0), itsSum(0)
{
  fCalculationResultOk = false;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAvgAbs::Calculate(float theValue)
{
  if (CheckMissingValues(theValue))
  {
    itsCounter++;
    itsSum += std::abs(theValue);
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAvgAbs::Calculate(NFmiQueryInfo* theQI)
{
  Calculate(theQI->FloatValue());
}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAvgAbs::Clear()
{
  itsCounter = 0;
  itsSum = 0;
  fCalculationResultOk = false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierAvgAbs::CalculationResult()
{
  if (fCalculationResultOk && itsCounter > 0)
    return itsSum / itsCounter;
  else
    return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiDataModifierAvgAbs::Counter()
{
  return itsCounter;
}
// ======================================================================
