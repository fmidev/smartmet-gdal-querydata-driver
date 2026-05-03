// ======================================================================
/*!
 * \file NFmiDataModifierMode.cpp
 * \brief Implementation of class NFmiDataModifierMode
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMode
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierMode.h"
#include <algorithm>

NFmiDataModifierMode::NFmiDataModifierMode() : NFmiDataModifier(), itsDataVector() {}
NFmiDataModifierMode::~NFmiDataModifierMode() = default;
NFmiDataModifierMode::NFmiDataModifierMode(const NFmiDataModifierMode& theOther)

    = default;

NFmiDataModifier* NFmiDataModifierMode::Clone() const
{
  return new NFmiDataModifierMode(*this);
}

void NFmiDataModifierMode::Calculate(float theValue)
{
  if (theValue != kFloatMissing)
  {
    auto pos = std::find(itsDataVector.begin(), itsDataVector.end(), Data(theValue));
    if (pos != itsDataVector.end())
      (*pos).Increase();
    else
      itsDataVector.push_back(Data(theValue));
  }
}

void NFmiDataModifierMode::Clear()
{
  itsDataVector.clear();
}
float NFmiDataModifierMode::CalculationResult()
{
  auto pos = std::max_element(itsDataVector.begin(), itsDataVector.end());
  if (pos != itsDataVector.end())
    return (*pos).Value();
  return kFloatMissing;
}
