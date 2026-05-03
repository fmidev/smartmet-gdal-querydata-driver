// ======================================================================
/*!
 * \file NFmiAreaMaskImpl.cpp
 * \brief Implementation of class NFmiAreaMaskImpl
 */
// ======================================================================
/*!
 * \class NFmiAreaMaskImpl
 *
 * A basic NFmiAreaMask implementation, including the possibility for
 * indexable submasks. Supermask is with index 1, submasks have increasing
 * indices.
 *
 * \bug Should define a copy constructor since there are pointer
 *      members.
 */
// ======================================================================

#include "NFmiAreaMaskImpl.h"
#include "NFmiSimpleCondition.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiAreaMaskImpl::NFmiAreaMaskImpl()
    : itsMaskCondition(),
      itsMaskType(kNoType),
      itsDataType(NFmiInfoData::kNoDataType),
      itsCalculationOperationType(NoType),
      itsCalculationOperator(NotOperation),
      itsPostBinaryOperator(kNoValue),
      itsMathFunctionType(NotMathFunction),
      itsFunctionType(NotFunction),
      itsSecondaryFunctionType(NotFunction),
      itsMetFunctionDirection(NoDirection),
      itsIntegrationFunctionType(0),
      itsFunctionArgumentCount(0),
      fHasSubMasks(false),
      fEnabled(true),
      itsSimpleCondition()
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theOperation Undocumented
 * \param theMaskType Undocumented
 * \param theDataType Undocumented
 * \param thePostBinaryOperator Undocumented
 */
// ----------------------------------------------------------------------

NFmiAreaMaskImpl::NFmiAreaMaskImpl(const NFmiCalculationCondition &theOperation,
                                   Type theMaskType,
                                   NFmiInfoData::Type theDataType,
                                   BinaryOperator thePostBinaryOperator)
    : itsMaskCondition(theOperation),
      itsMaskType(theMaskType),
      itsDataType(theDataType),
      itsCalculationOperationType(NoType)  // tämä pitää asettaa erikseen funktiosta!
      ,
      itsCalculationOperator(NotOperation),
      itsPostBinaryOperator(thePostBinaryOperator),
      itsMathFunctionType(NotMathFunction),
      itsFunctionType(NotFunction),
      itsSecondaryFunctionType(NotFunction),
      itsMetFunctionDirection(NoDirection),
      itsIntegrationFunctionType(0),
      itsFunctionArgumentCount(0),
      fHasSubMasks(false),
      fEnabled(true),
      itsSimpleCondition()
{
}

NFmiAreaMaskImpl::NFmiAreaMaskImpl(const NFmiAreaMaskImpl &theOther)
    : itsMaskCondition(theOther.itsMaskCondition),
      itsMaskType(theOther.itsMaskType),
      itsDataType(theOther.itsDataType),
      itsCalculationOperationType(theOther.itsCalculationOperationType),
      itsCalculationOperator(theOther.itsCalculationOperator),
      itsPostBinaryOperator(theOther.itsPostBinaryOperator),
      itsMathFunctionType(theOther.itsMathFunctionType),
      itsFunctionType(theOther.itsFunctionType),
      itsSecondaryFunctionType(theOther.itsSecondaryFunctionType),
      itsMetFunctionDirection(theOther.itsMetFunctionDirection),
      itsIntegrationFunctionType(theOther.itsIntegrationFunctionType),
      itsFunctionArgumentCount(theOther.itsFunctionArgumentCount),
      fHasSubMasks(theOther.fHasSubMasks),
      fEnabled(theOther.fEnabled),
      itsSimpleCondition(theOther.itsSimpleCondition ? theOther.itsSimpleCondition->Clone()
                                                     : nullptr),
      itsFunctionDataTimeOffsetInHours(theOther.itsFunctionDataTimeOffsetInHours)
{
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiAreaMaskImpl::~NFmiAreaMaskImpl() = default;

void NFmiAreaMaskImpl::Initialize()
{
  try
  {
    if (itsSimpleCondition)
      itsSimpleCondition->Initialize();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::IsMasked(const NFmiPoint &theLatLon) const
{
  try
  {
    if (!fEnabled)
      return true;  // jos maski ei ole käytössä, on maski aina 'päällä'

    double testValue = CalcValueFromLocation(theLatLon);  // CalcValueFromLocation on virtuaalinen
    return itsMaskCondition.IsMasked(testValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Laskee arvon 0:n ja 1:n välille riippuen maskista (tarkoitettu
 * laskemaan liukuvia maskeja ja niistä johtuvia kertoimia)
 *
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiAreaMaskImpl::MaskValue(const NFmiPoint &theLatLon) const
{
  try
  {
    if (!fEnabled)
      return 1.;

    double testValue = CalcValueFromLocation(theLatLon);  // CalcValueFromLocation on virtuaalinen
    return itsMaskCondition.MaskValue(testValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatlon Undocumented
 * \param theTime Undocumented, unused
 * \param theTimeIndex Undocumented, unused
 */
// ----------------------------------------------------------------------

double NFmiAreaMaskImpl::Value(const NFmiCalculationParams &theCalculationParams,
                               bool /* fUseTimeInterpolationAlways */)
{
  try
  {
    // useimmille maskiluokille tämä riittää, koska ne eivät
    // ole riippuvaisia ajasta.
    return CalcValueFromLocation(theCalculationParams.UsedLatlon());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::Time(const NFmiMetTime & /* theTime */)
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented, unused
 * \param theLevel Undocumented, unused
 * \return Undocumented, always false
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::IsWantedParam(const NFmiDataIdent & /* theParam */,
                                     const NFmiLevel * /* theLevel */) const
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiAreaMaskImpl::MaskString() const
{
  try
  {
    NFmiString subStr(MakeSubMaskString());
    NFmiString returnValue(itsMaskCondition.MaskString(subStr));
    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiDataIdent *NFmiAreaMaskImpl::DataIdent() const
{
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiParam *NFmiAreaMaskImpl::Param() const
{
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiLevel *NFmiAreaMaskImpl::Level() const
{
  return nullptr;
}

void NFmiAreaMaskImpl::Level(const NFmiLevel & /* theLevel */) {}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::UseLevelInfo() const
{
  return false;
}

bool NFmiAreaMaskImpl::UsePressureLevelInterpolation() const
{
  return false;
}

void NFmiAreaMaskImpl::UsePressureLevelInterpolation(bool /* newValue */) {}

double NFmiAreaMaskImpl::UsedPressureLevelValue() const
{
  return kFloatMissing;
}

void NFmiAreaMaskImpl::UsedPressureLevelValue(double /* newValue */) {}

// ----------------------------------------------------------------------
/*!
 * \param theMask Undocumented, unused
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::AddMask(NFmiAreaMask * /* theMask */)
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa joko this:in jos index = 1 ja muuten indeksillä
 * osoitetun 'ali'-maskin, tai 0:n.
 *
 * \param theIndex Unused, undocumented
 * \return Undocumented, always zero
 *
 */
// ----------------------------------------------------------------------

NFmiAreaMask *NFmiAreaMaskImpl::AreaMask(int /* theIndex */) const
{
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented, unused
 * \return Undocumented, always false
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::RemoveSubMask(int /* theIndex*/)
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented, unused
 * \return Undocumented, always kFloatMissing
 */
// ----------------------------------------------------------------------

double NFmiAreaMaskImpl::CalcValueFromLocation(const NFmiPoint & /* theLatLon */) const
{
  return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always empty string
 */
// ----------------------------------------------------------------------

const NFmiString NFmiAreaMaskImpl::MakeSubMaskString() const
{
  try
  {
    NFmiString returnVal;
    return returnVal;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theCondition
 */
// ----------------------------------------------------------------------

void NFmiAreaMaskImpl::Condition(const NFmiCalculationCondition &theCondition)
{
  try
  {
    itsMaskCondition = theCondition;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::IsRampMask() const
{
  try
  {
    return itsMaskCondition.IsRampMask();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================

void NFmiAreaMaskImpl::DoIntegrationCalculations(float value) {}

void NFmiAreaMaskImpl::InitializeIntegrationValues() {}

bool NFmiAreaMaskImpl::SimpleConditionCheck(const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    if (itsSimpleCondition)
      return itsSimpleCondition->CheckCondition(theCalculationParams, true);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiAreaMaskImpl::CalculationPointValue(int theOffsetX,
                                              int theOffsetY,
                                              const NFmiMetTime &theInterpolationTime,
                                              bool useInterpolatedTime)
{
  return kFloatMissing;
}
