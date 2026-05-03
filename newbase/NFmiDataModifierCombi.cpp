// ======================================================================
/*!
 * \file NFmiDataModifierCombi.cpp
 * \brief Implementation of class NFmiDataModifierCombi
 */
// ======================================================================
/*!
 * \class NFmiDataModifierCombi
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierCombi.h"
#include "NFmiIntegrationSelector.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombi::~NFmiDataModifierCombi()
{
  if (itsFrequencies)
    delete[] itsFrequencies;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theNumberOfValues Undocumented
 * \param theProbabilityScale Undocumented
 * \param theJoinOperator Undocumented
 * \param missingValuesAllowed Undocumented
 * \param theCombinedParam Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombi::NFmiDataModifierCombi(unsigned int theNumberOfValues,
                                             float theProbabilityScale,
                                             FmiJoinOperator theJoinOperator,
                                             bool missingValuesAllowed,
                                             NFmiCombinedParam* theCombinedParam)
    : NFmiDataModifier(theJoinOperator, missingValuesAllowed, theCombinedParam),
      itsProbabilityScale(theProbabilityScale),
      itsNumberOfValues(theNumberOfValues),
      itsTotalCounter(),
      itsMin(),
      itsMax(),
      itsSum(),
      itsFrequencies(nullptr)
{
  if (itsNumberOfValues > 0)
    itsFrequencies = new int[itsNumberOfValues];
  Clear();
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param other The other object being copied
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombi::NFmiDataModifierCombi(const NFmiDataModifierCombi& other)
    : NFmiDataModifier(other),
      itsProbabilityScale(other.itsProbabilityScale),
      itsNumberOfValues(other.itsNumberOfValues),
      itsTotalCounter(other.itsTotalCounter),
      itsMin(other.itsMin),
      itsMax(other.itsMax),
      itsSum(other.itsSum),
      itsFrequencies(nullptr)
{
  if (itsNumberOfValues > 0)
  {
    itsFrequencies = new int[itsNumberOfValues];
    for (int i = 0; i < itsNumberOfValues; i++)
      itsFrequencies[i] = other.itsFrequencies[i];
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierCombi::Clear()
{
  itsSum = 0;
  itsMax = -kFloatMissing;
  itsMin = kFloatMissing;
  itsTotalCounter = 0;
  if (!itsNumberOfValues)
    return;
  for (int i = 0; i < itsNumberOfValues; i++)
    itsFrequencies[i] = 0;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierCombi::CheckParams(double theValue)
{
  if (theValue == kFloatMissing || theValue >= itsNumberOfValues || theValue < 0)
    return false;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierCombi::Calculate(float theValue)
{
  if (!CheckParams(theValue))
    return;

  itsTotalCounter++;

  itsSum += theValue;

  if (theValue > itsMax)
    itsMax = theValue;

  if (theValue < itsMin)
    itsMin = theValue;

  if (itsNumberOfValues)
  {
    // Ensimmäisellä kierroksella muutetaan puuttuva nollaksi
    if (itsFrequencies[static_cast<int>(theValue)] == kFloatMissing)
    {
      itsFrequencies[static_cast<int>(theValue)] = 0;
    }
    itsFrequencies[static_cast<int>(theValue)]++;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSelector Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierCombi::CalcResult(const NFmiIntegrationSelector& theSelector)
{
  switch (theSelector.Type())
  {
    case kFmiMin:
      return itsMin;
    case kFmiMax:
      return itsMax == -kFloatMissing ? kFloatMissing : itsMax;
    case kFmiMean:
      if (itsTotalCounter)
        return itsSum / static_cast<float>(itsTotalCounter);
      else
        return kFloatMissing;
    case kFmiProb:
      return ProbValue(theSelector);
    default:
      return kFloatMissing;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSelector Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierCombi::ProbValue(const NFmiIntegrationSelector& theSelector)
{
  float returnValue = 0;
  for (int i = 0; i < itsNumberOfValues; i++)
  {
    if (CheckFrequency(static_cast<float>(i), theSelector))
    {
      returnValue += itsFrequencies[i];
    }
  }
  return itsProbabilityScale * returnValue / static_cast<float>(itsTotalCounter);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theSelector Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierCombi::CheckFrequency(float theValue,
                                           const NFmiIntegrationSelector& theSelector)
{
  if (!CheckParams(theValue))
    return false;

  switch (theSelector.ProbabilityCondition())
  {
    case kValueLessThanLimit:
      return theValue < theSelector.ProbabilityLowerLimit();
    case kValueLessOrEqualThanLimit:
      return theValue <= theSelector.ProbabilityLowerLimit();
    case kValueGreaterThanLimit:
      return theValue > theSelector.ProbabilityLowerLimit();
    case kValueGreaterOrEqualThanLimit:
      return theValue >= theSelector.ProbabilityLowerLimit();
    case kValueBetweenLimits:
      return theValue >= theSelector.ProbabilityLowerLimit() &&
             theValue < theSelector.ProbabilityUpperLimit();
    case kValueOutOfLimits:
      return theValue < theSelector.ProbabilityLowerLimit() ||
             theValue > theSelector.ProbabilityUpperLimit();
    case kValueIsEqual:
      return theValue == theSelector.ProbabilityLowerLimit();
    default:
      return false;
  }
}

// ======================================================================
