// ======================================================================
/*!
 * \file NFmiDataModifierStandardDeviation.cpp
 * \brief Implementation of class NFmiDataModifierStandardDeviation
 */
// ======================================================================
/*!
 * \class NFmiDataModifierStandardDeviation
 *
 * \brief Calculates sample standard deviation
 *
 */
// ======================================================================

#include "NFmiDataModifierStandardDeviation.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierStandardDeviation::~NFmiDataModifierStandardDeviation() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierStandardDeviation::NFmiDataModifierStandardDeviation()
    : NFmiDataModifier(), itsCounter(0), itsSum(0), itsSquaredSum(0)
{
  fCalculationResultOk = false;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierStandardDeviation::Calculate(float theValue)
{
  if (CheckMissingValues(theValue))
  {
    itsCounter++;
    itsSum += theValue;
    itsSquaredSum += theValue * theValue;
    fCalculationResultOk = (itsCounter > 1);
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierStandardDeviation::Clear()
{
  itsCounter = 0;
  itsSum = 0;
  itsSquaredSum = 0;
  fCalculationResultOk = false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierStandardDeviation::CalculationResult()
{
  if (fCalculationResultOk)
    return (itsSquaredSum - itsSum * itsSum / itsCounter) / (itsCounter - 1);
  else
    return kFloatMissing;
}

// ======================================================================
