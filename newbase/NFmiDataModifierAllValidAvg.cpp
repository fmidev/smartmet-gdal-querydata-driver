// ======================================================================
/*!
 * \file NFmiDataModifierAllValidAvg.cpp
 * \brief Implementation of class NFmiDataModifierAllValidAvg
 */
// ======================================================================
/*!
 * \class NFmiDataModifierAllValidAvg
 *
 * Undocumented
 *
 * AllValid= vaatii että kaikki arvot ovat käypiä
 *
 */
// ======================================================================

#include "NFmiDataModifierAllValidAvg.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierAllValidAvg::~NFmiDataModifierAllValidAvg() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierAllValidAvg::NFmiDataModifierAllValidAvg() : NFmiDataModifierAvg()
{
  fMissingValuesAllowed = false;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAllValidAvg::Calculate(float theValue)
{
  if (CheckMissingValues(theValue))
  {
    itsCounter++;

    // "conversion to 'float' from 'long int' may alter its value"
    //    --AKa 14-Sep-2009 (Ubuntu 9.04 -Wall)
    //
    //  long itsCounter;
    //  float itsAverage;

    // Float can carry integers with 23 bits of resolution.
    // 'itsCounter' is expected to be within this (is it?)
    //
    assert(itsCounter < (1 << 24));
    itsAverage -= (itsAverage - theValue) / ((float)itsCounter);

    // Was:
    // itsAverage-=((itsAverage-theValue)/itsCounter);
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAllValidAvg::Calculate(NFmiQueryInfo* theQI)
{
  if (theQI)
  {
    float value = theQI->FloatValue();
    if (CheckMissingValues(value))
    {
      if (itsCounter == 0)
        fCalculationResultOk = true;
      itsCounter++;
      if ((theQI->Param().GetParam()->MinValue() != kFloatMissing &&
           value <= theQI->Param().GetParam()->MinValue()) ||
          (theQI->Param().GetParam()->MaxValue() != kFloatMissing &&
           value >= theQI->Param().GetParam()->MaxValue()))
      {
        value = 0;  // toiminee ainakin hav. sateella 12.9.01/Lasse
      }

      assert(itsCounter < (1 << 24));
      itsAverage -= (itsAverage - value) / ((float)itsCounter);

      // Was:
      // itsAverage-=((itsAverage-value)/itsCounter);
    }
    else
    {
      fCalculationResultOk = false;
    }
  }
}

// ======================================================================
