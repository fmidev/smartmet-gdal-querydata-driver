// ======================================================================
/*!
 * \file NFmiDataModifierAvg.cpp
 * \brief Implementation of class NFmiDataModifierAvg
 */
// ======================================================================
/*!
 * \class NFmiDataModifierAvg
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierAvg.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierAvg::~NFmiDataModifierAvg() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierAvg::NFmiDataModifierAvg() : NFmiDataModifier(), itsCounter(0), itsAverage(0.)
{
  fCalculationResultOk = false;
}

NFmiDataModifierAvg::NFmiDataModifierAvg(const NFmiDataModifierAvg& theOther)

    = default;

NFmiDataModifier* NFmiDataModifierAvg::Clone() const
{
  return new NFmiDataModifierAvg(*this);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierAvg::Avg()
{
  return itsAverage;
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAvg::Calculate(float theValue)
{
  if (CheckMissingValues(theValue))
  {
    itsCounter++;
    itsAverage -= ((itsAverage - theValue) / itsCounter);
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAvg::Calculate(NFmiQueryInfo* theQI)
{
  Calculate(theQI->FloatValue());
}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAvg::Clear()
{
  itsCounter = 0;
  itsAverage = 0;
  fCalculationResultOk = false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierAvg::CalculationResult()
{
  if (fCalculationResultOk)
    return itsAverage;
  else
    return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiDataModifierAvg::Counter()
{
  return itsCounter;
}
// ======================================================================
