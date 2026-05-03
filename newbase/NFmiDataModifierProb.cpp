// ======================================================================
/*!
 * \file NFmiDataModifierProb.cpp
 * \brief Implementation of class NFmiDataModifierProb
 */
// ======================================================================
/*!
 * \class NFmiDataModifierProb
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierProb.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierProb::~NFmiDataModifierProb() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theCondition Undocumented
 * \param theFirstLimit Undocumented
 * \param theJoinOperator Undocumented
 * \param theCombinedParam Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierProb::NFmiDataModifierProb(FmiProbabilityCondition theCondition,
                                           double theFirstLimit,
                                           FmiJoinOperator theJoinOperator,
                                           NFmiCombinedParam* theCombinedParam)
    : NFmiDataModifier(theJoinOperator, false, theCombinedParam),
      itsTotalCounter(),
      itsCounter(),
      its1Limit(theFirstLimit),
      its2Limit(theFirstLimit),
      itsCondition(theCondition)
{
  Clear();
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theCondition Undocumented
 * \param theFirstLimit Undocumented
 * \param theSecondLimit Undocumented
 * \param theJoinOperator Undocumented
 * \param theCombinedParam Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierProb::NFmiDataModifierProb(FmiProbabilityCondition theCondition,
                                           double theFirstLimit,
                                           double theSecondLimit,
                                           FmiJoinOperator theJoinOperator,
                                           NFmiCombinedParam* theCombinedParam)
    : NFmiDataModifier(theJoinOperator, false, theCombinedParam),
      itsTotalCounter(),
      itsCounter(),
      its1Limit(theFirstLimit),
      its2Limit(theSecondLimit == kFloatMissing ? theFirstLimit : theSecondLimit),
      itsCondition(theCondition)
{
  Clear();
}

// ----------------------------------------------------------------------
/*!
 * Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierProb::Clear()
{
  itsCounter = 0;
  itsTotalCounter = 0;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierProb::CheckParams(double theValue)
{
  if (theValue == kFloatMissing || its1Limit == kFloatMissing)
    return false;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param theCondition Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierProb::AddCounter(bool theCondition)
{
  if (theCondition)
    itsCounter++;

  itsTotalCounter++;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Remove the C-style casts
 */
// ----------------------------------------------------------------------

float NFmiDataModifierProb::CalculationResult()
{
  return itsTotalCounter
             ? 100.0f * (static_cast<float>(itsCounter) / static_cast<float>(itsTotalCounter))
             : kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierProb::Calculate(float theValue)
{
  if (!CheckParams(theValue))
    return;
  switch (itsCondition)
  {
    case kValueLessThanLimit:
      AddCounter(theValue < its1Limit);
      break;
    case kValueLessOrEqualThanLimit:
      AddCounter(theValue <= its1Limit);
      break;
    case kValueGreaterThanLimit:
      AddCounter(theValue > its1Limit);
      break;
    case kValueGreaterOrEqualThanLimit:
      AddCounter(theValue >= its1Limit);
      break;
    case kValueBetweenLimits:
      AddCounter(theValue >= its1Limit && theValue < its2Limit);
      break;
    case kValueOutOfLimits:
      AddCounter(theValue < its1Limit || theValue > its2Limit);
      break;
    case kValueIsEqual:
      AddCounter(theValue == its1Limit);
      break;
    case kNoCondition:
      break;
  }
}

// ======================================================================
