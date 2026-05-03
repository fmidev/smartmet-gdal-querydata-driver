// ======================================================================
/*!
 * \file NFmiDataModifierMedian.cpp
 * \brief Implementation of class NFmiDataModifierMedian (8.10.03/EL)
 */
// ======================================================================

#include "NFmiDataModifierMedian.h"
#include "NFmiQueryInfo.h"
#include <stdexcept>

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierMedian::~NFmiDataModifierMedian() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierMedian::NFmiDataModifierMedian(float theLimitProsent)
    : NFmiDataModifier(), itsLimitProsent(theLimitProsent)
{
  fCalculationResultOk = false;
}

NFmiDataModifierMedian::NFmiDataModifierMedian(const NFmiDataModifierMedian& theOther)
    : NFmiDataModifier(theOther), itsLimitProsent(theOther.itsLimitProsent)
{
}

NFmiDataModifier* NFmiDataModifierMedian::Clone() const
{
  return new NFmiDataModifierMedian(*this);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------

float NFmiDataModifierMedian::Median()
{
  if (itsMedianArray.empty())
    return kFloatMissing;  // mielestäni tämän pitää palauttaa missing value eikä heittää poikkeusta
                           // (Marko)
  std::sort(itsMedianArray.begin(), itsMedianArray.end());
  //  return itsMedianArray[itsMedianArray.size()/2];
  return itsMedianArray[static_cast<int>(itsMedianArray.size() * itsLimitProsent / 100.f)];
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMedian::Calculate(float theValue)
{
  if (CheckMissingValues(theValue))
  {
    // Just add the input value into median kernel window array.
    // Do not (re)compute median until in method 'Median()'.

    itsMedianArray.push_back(theValue);
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMedian::Calculate(NFmiQueryInfo* theQI)
{
  Calculate(theQI->FloatValue());
}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMedian::Clear()
{
  itsMedianArray.resize(0);
  fCalculationResultOk = false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierMedian::CalculationResult()
{
  if (fCalculationResultOk)
    return Median();

  return kFloatMissing;
}

void NFmiDataModifierMedian::LimitProsent(float newValue)
{
  if (newValue < 0.f)
    itsLimitProsent = 0.f;
  else if (newValue > 100.f)
    itsLimitProsent = 100.f;
  else
    itsLimitProsent = newValue;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// ======================================================================
