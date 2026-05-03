// ======================================================================
/*!
 * \file NFmiDataModifierMaxMean.cpp
 * \brief Implementation of class NFmiDataModifierMaxMean
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMaxMean
 *
 * Undocumented
 *
 * painokerroin (itsWeightFactor) 1 palauttaa keskiarvon, 0 palauttaa maksimin,
 * 0.5 palauttaa (max+avg):n keskiarvon
 * jos painokerrointa ei anneta erikseen, ohjelma palauttaa pistekohtaisesti
 * 'omasta mielestään' kuvaavimman arvon
 *
 */
// ======================================================================

#include "NFmiDataModifierMaxMean.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierMaxMean::~NFmiDataModifierMaxMean() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierMaxMean::NFmiDataModifierMaxMean()
    : NFmiDataModifier(),
      itsCounter(0),
      itsAverage(0.),
      itsMaxValue(-3.4E+38f),
      itsWeightFactor(kFloatMissing)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theWeightFactor Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierMaxMean::NFmiDataModifierMaxMean(float theWeightFactor)
    : itsCounter(0), itsAverage(0.), itsMaxValue(-3.4E+38f), itsWeightFactor(theWeightFactor)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMaxMean::Calculate(float theValue)
{
  if (CheckMissingValues(theValue))
  {
    fCalculationResultOk = true;
    itsCounter++;
    itsAverage -= ((itsAverage - theValue) / itsCounter);

    if (itsMaxValue < theValue)
      itsMaxValue = theValue;
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMaxMean::Clear()
{
  NFmiDataModifier::Clear();
  itsMaxValue = -3.4E+38f;
  itsCounter = 0;
  itsAverage = 0;
  fCalculationResultOk = false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierMaxMean::CalculationResult()
{
  float avg = kFloatMissing, max = kFloatMissing;
  if (fCalculationResultOk == true)
    avg = itsAverage;
  else
    avg = kFloatMissing;
  if (itsMaxValue != -3.4E+38f)
    max = itsMaxValue;
  else
    max = kFloatMissing;

  if (itsWeightFactor == kFloatMissing && avg != kFloatMissing && max != kFloatMissing)
  {
    float weightFactor = avg / max;
    if (weightFactor < 0.4)
      return avg;
    else if (weightFactor >= 0.4 && weightFactor <= 0.6)
      return (avg + max) / 2;
    else if (weightFactor > 0.6 && weightFactor != kFloatMissing)
      return max;
    else
      return kFloatMissing;
  }
  else if (itsWeightFactor != kFloatMissing && avg != kFloatMissing && max != kFloatMissing)
  {
    float result = (avg * itsWeightFactor) + (max * (1 - itsWeightFactor));
    return result;
  }
  else
    return kFloatMissing;
}

// ======================================================================
