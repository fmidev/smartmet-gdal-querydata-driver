// ======================================================================
/*!
 * \file NFmiDataModifierAllValidSum.cpp
 * \brief Implementation of class NFmiDataModifierAllValidSum
 */
// ======================================================================
/*!
 * \class NFmiDataModifierAllValidSum
 *
 * Undocumented
 *
 * 2.5.2000/Lasse; käytän sadesummiin
 * AllValid= vaatii että kaikki arvot ovat käypiä
 *
 */
// ======================================================================

#include "NFmiDataModifierAllValidSum.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierAllValidSum::~NFmiDataModifierAllValidSum() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierAllValidSum::NFmiDataModifierAllValidSum() : NFmiDataModifierSum(), itsCounter(0)
{
  fMissingValuesAllowed = false;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAllValidSum::Calculate(float theValue)
{
  if (CheckMissingValues(theValue))
  {
    if (itsCounter == 0)
      fCalculationResultOk = true;

    if (IsInside(theValue))
      itsSum += theValue;
  }
  itsCounter++;
}

// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAllValidSum::Calculate(NFmiQueryInfo* theQI)
{
  float value = theQI->FloatValue();
  // kaikki pitää olla jotta ok
  if (CheckMissingValues(value))
  {
    if (itsCounter == 0)
      fCalculationResultOk = true;

    // sadetta -1 ei saa ottaa
    if (!(theQI && ((theQI->Param().GetParam()->MinValue() != kFloatMissing &&
                     value <= theQI->Param().GetParam()->MinValue()) ||
                    (theQI->Param().GetParam()->MaxValue() != kFloatMissing &&
                     value >= theQI->Param().GetParam()->MaxValue()))))
      itsSum += value;
  }
  else
  {
    fCalculationResultOk = false;
  }
  itsCounter++;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAllValidSum::Clear()
{
  NFmiDataModifier::Clear();
  itsSum = 0;
  fCalculationResultOk = false;
  itsCounter = 0;
}

// ======================================================================
