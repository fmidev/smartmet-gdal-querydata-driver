// ======================================================================
/*!
 * \file NFmiDataModifier.cpp
 * \brief Implementation of class NFmiDataModifier
 */
// ======================================================================
/*!
 * \class NFmiDataModifier
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifier.h"
#include "NFmiCombinedParam.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifier::~NFmiDataModifier()
{
  delete itsCombinedParam;
}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theJoinOperator Undocumented
 * \param missingValuesAllowed Undocumented
 * \param thePotentialCombinedParam Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifier::NFmiDataModifier(FmiJoinOperator theJoinOperator,
                                   bool missingValuesAllowed,
                                   NFmiCombinedParam* thePotentialCombinedParam)
    : NFmiDataModifierBase(theJoinOperator),
      itsCombinedParam(thePotentialCombinedParam ? thePotentialCombinedParam->Clone() : nullptr),
      fCalculationResultOk(true),
      fMissingValuesAllowed(missingValuesAllowed),
      itsNumberOfMissingValues(0),
      fIsCombinedParam(thePotentialCombinedParam ? true : false),
      fIsLimitCheck(false),
      itsLowerLimit(),
      itsUpperLimit()
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theModier The other object being copied
 */
// ----------------------------------------------------------------------

NFmiDataModifier::NFmiDataModifier(const NFmiDataModifier& theModier)
    : NFmiDataModifierBase(theModier),
      itsCombinedParam(theModier.itsCombinedParam ? theModier.itsCombinedParam->Clone() : nullptr),
      fCalculationResultOk(theModier.fCalculationResultOk),
      fMissingValuesAllowed(theModier.fMissingValuesAllowed),
      itsNumberOfMissingValues(theModier.itsNumberOfMissingValues),
      fIsCombinedParam(theModier.fIsCombinedParam),
      fIsLimitCheck(theModier.fIsLimitCheck),
      itsLowerLimit(theModier.itsLowerLimit),
      itsUpperLimit(theModier.itsUpperLimit)
{
}

NFmiDataModifier* NFmiDataModifier::Clone() const
{
  return new NFmiDataModifier(*this);
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifier::IsInside(float theValue)
{
  if (!fIsLimitCheck)
    return true;
  else
    return (((itsLowerLimit != kFloatMissing) && (theValue < itsLowerLimit)) ||
            ((itsUpperLimit != kFloatMissing) && (theValue > itsUpperLimit)))
               ? false
               : true;
}

// ----------------------------------------------------------------------
/*!
 * \param theLowerLimit Undocumented
 * \param theUpperLimit Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifier::SetLimits(float theLowerLimit, float theUpperLimit)
{
  if (theLowerLimit >= theUpperLimit)
    return false;
  else
  {
    itsLowerLimit = theLowerLimit;
    itsUpperLimit = theUpperLimit;
    fIsLimitCheck = true;
    return true;
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifier::IsCombinedParam()
{
  return fIsCombinedParam;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiDataModifier::FloatValue()
{
  return CalculationResult();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam* NFmiDataModifier::CombinedCalculationResult()
{
  return nullptr;
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumenetd
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifier::CheckMissingValues(float theValue)
{
  if (theValue != kFloatMissing && theValue != kRadarPrecipitationMissing)
  {
    // 25.9.01Lasse  esim summasta voisi poistaa vastaavan
    if (fMissingValuesAllowed || itsNumberOfMissingValues < 1)
      fCalculationResultOk = true;
    return true;
  }
  itsNumberOfMissingValues++;
  if (!fMissingValuesAllowed)
    fCalculationResultOk = false;
  return false;
}

void NFmiDataModifier::SetLocationIndex(unsigned long /* theIndex */) {}
void NFmiDataModifier::SetTimeIndex(unsigned long /* theIndex */) {}
void NFmiDataModifier::InitLatlonCache() {}
// ======================================================================
