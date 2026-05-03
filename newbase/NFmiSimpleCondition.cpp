#include "NFmiFastQueryInfo.h"
#include "NFmiSimpleCondition.h"
#include <macgyver/Exception.h>

namespace
{
bool CheckForStationaryData(const std::shared_ptr<NFmiAreaMask> &mask)
{
  try
  {
    if (mask)
    {
      if (mask->GetDataType() == NFmiInfoData::kStationary)
        return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool UseTimeInterpolation(bool maskIsStationaryData, bool normalInterpolationCondition)
{
  try
  {
    if (maskIsStationaryData)
      return false;

    return normalInterpolationCondition;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double CalculateValue(double value1,
                      double value2,
                      NFmiAreaMask::CalculationOperator calculationOperator)
{
  try
  {
    if (value1 == kFloatMissing || value2 == kFloatMissing)
      return kFloatMissing;

    switch (calculationOperator)
    {
      case NFmiAreaMask::Add:
        return value1 + value2;
      case NFmiAreaMask::Sub:
        return value1 - value2;
      case NFmiAreaMask::Mul:
        return value1 * value2;
      case NFmiAreaMask::Div:
      {
        if (value2 == 0)
          return kFloatMissing;
        else
          return value1 / value2;
      }
      case NFmiAreaMask::Mod:
      {
        if (value2 == 0)
          return kFloatMissing;
        else
          return std::fmod(value1, value2);
      }
      case NFmiAreaMask::Pow:
        return std::pow(value1, value2);
      default:
        throw Fmi::Exception(
            BCP,
            "Internal program error with SimpleCondition's calculation operator - unknown "
            "operator");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double GetPressureValue(std::shared_ptr<NFmiAreaMask> &mask,
                        double pressure,
                        const NFmiCalculationParams &calculationParams,
                        bool useTimeInterpolation)
{
  if (mask && mask->Info() && mask->Info()->SizeLevels() <= 1)
  {
    // jos käytössä on 1 levelinen pintadata, pyydetään vain pinta-arvoa
    return mask->Value(calculationParams, useTimeInterpolation);
  }
  else
    return mask->PressureValue(pressure, calculationParams);
}

double GetHeightValue(std::shared_ptr<NFmiAreaMask> &mask,
                      double height,
                      const NFmiCalculationParams &calculationParams,
                      bool useTimeInterpolation)
{
  if (mask && mask->Info() && mask->Info()->SizeLevels() <= 1)
  {
    // jos käytössä on 1 levelinen pintadata, pyydetään vain pinta-arvoa
    return mask->Value(calculationParams, useTimeInterpolation);
  }
  else
    return mask->HeightValue(height, calculationParams);
}

}  // namespace
   // namespace
   // *****************************************************************
   // **************   NFmiSimpleConditionPart   **********************
   // *****************************************************************

NFmiSimpleConditionPart::~NFmiSimpleConditionPart() = default;

NFmiSimpleConditionPart::NFmiSimpleConditionPart(
    std::shared_ptr<NFmiAreaMask> &mask1,
    NFmiAreaMask::CalculationOperator calculationOperator,
    std::shared_ptr<NFmiAreaMask> &mask2)
    : itsMask1(mask1), itsCalculationOperator(calculationOperator), itsMask2(mask2)
{
}

NFmiSimpleConditionPart::NFmiSimpleConditionPart(const NFmiSimpleConditionPart &theOther)
    : itsMask1(theOther.itsMask1 ? theOther.itsMask1->Clone() : nullptr),
      isMask1StationaryData(theOther.isMask1StationaryData),
      itsCalculationOperator(theOther.itsCalculationOperator),
      itsMask2(theOther.itsMask2 ? theOther.itsMask2->Clone() : nullptr),
      isMask2StationaryData(theOther.isMask2StationaryData),
      itsPreviousValue(theOther.itsPreviousValue)
{
}

void NFmiSimpleConditionPart::Initialize()
{
  try
  {
    isMask1StationaryData = ::CheckForStationaryData(itsMask1);
    isMask2StationaryData = ::CheckForStationaryData(itsMask2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiSimpleConditionPart *NFmiSimpleConditionPart::Clone() const
{
  try
  {
    return new NFmiSimpleConditionPart(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiSimpleConditionPart::Value(const NFmiCalculationParams &theCalculationParams,
                                      bool fUseTimeInterpolationAlways)
{
  try
  {
    double value1 =
        itsMask1->Value(theCalculationParams,
                        ::UseTimeInterpolation(isMask1StationaryData, fUseTimeInterpolationAlways));
    if (!itsMask2)
      return value1;

    double value2 =
        itsMask2->Value(theCalculationParams,
                        ::UseTimeInterpolation(isMask2StationaryData, fUseTimeInterpolationAlways));
    return ::CalculateValue(value1, value2, itsCalculationOperator);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiSimpleConditionPart::PressureValue(double thePressure,
                                              const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    auto doTimeInterp = ::UseTimeInterpolation(isMask1StationaryData, true);
    double value1 = ::GetPressureValue(itsMask1, thePressure, theCalculationParams, doTimeInterp);
    if (!itsMask2)
      return value1;

    double value2 = ::GetPressureValue(itsMask2, thePressure, theCalculationParams, doTimeInterp);
    return ::CalculateValue(value1, value2, itsCalculationOperator);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiSimpleConditionPart::HeightValue(double theHeight,
                                            const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    auto doTimeInterp = ::UseTimeInterpolation(isMask1StationaryData, true);
    double value1 = ::GetHeightValue(itsMask1, theHeight, theCalculationParams, doTimeInterp);
    if (!itsMask2)
      return value1;

    double value2 = ::GetHeightValue(itsMask2, theHeight, theCalculationParams, doTimeInterp);
    return ::CalculateValue(value1, value2, itsCalculationOperator);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiSimpleConditionPart::PreviousValue(double newPreviousValue)
{
  auto returnValue = itsPreviousValue;
  itsPreviousValue = newPreviousValue;
  return returnValue;
}

void NFmiSimpleConditionPart::ResetPreviousValue() { itsPreviousValue = kFloatMissing; }

// *****************************************************************
// ****************   NFmiSingleCondition   ************************
// *****************************************************************

NFmiSingleCondition::~NFmiSingleCondition() = default;

NFmiSingleCondition::NFmiSingleCondition(const std::shared_ptr<NFmiSimpleConditionPart> &thePart1,
                                         FmiMaskOperation theConditionOperand1,
                                         const std::shared_ptr<NFmiSimpleConditionPart> &thePart2,
                                         FmiMaskOperation theConditionOperand2,
                                         const std::shared_ptr<NFmiSimpleConditionPart> &thePart3)
    : part1(thePart1),
      conditionOperand1(theConditionOperand1),
      part2(thePart2),
      conditionOperand2(theConditionOperand2),
      part3(thePart3)
{
}

NFmiSingleCondition::NFmiSingleCondition(const NFmiSingleCondition &theOther)
    : part1(theOther.part1 ? theOther.part1->Clone() : nullptr),
      conditionOperand1(theOther.conditionOperand1),
      part2(theOther.part2 ? theOther.part2->Clone() : nullptr),
      conditionOperand2(theOther.conditionOperand2),
      part3(theOther.part3 ? theOther.part3->Clone() : nullptr)
{
}

static void InitializePart(std::shared_ptr<NFmiSimpleConditionPart> &part)
{
  try
  {
    if (part)
      part->Initialize();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiSingleCondition::Initialize()
{
  try
  {
    InitializePart(part1);
    InitializePart(part2);
    InitializePart(part3);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiSingleCondition *NFmiSingleCondition::Clone() const
{
  try
  {
    return new NFmiSingleCondition(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool EvaluateCondition(double value1, FmiMaskOperation operand, double value2)
{
  try
  {
    if (value1 == kFloatMissing || value2 == kFloatMissing)
      return false;

    if (operand == kFmiNoMaskOperation)
      return false;

    switch (operand)
    {
      case kFmiMaskEqual:
        return value1 == value2;
      case kFmiMaskGreaterThan:
        return value1 > value2;
      case kFmiMaskLessThan:
        return value1 < value2;
      case kFmiMaskGreaterOrEqualThan:
        return value1 >= value2;
      case kFmiMaskLessOrEqualThan:
        return value1 <= value2;
      case kFmiMaskNotEqual:
        return value1 != value2;
      default:
        throw Fmi::Exception(
            BCP,
            "Error in SimpleCondition's EvaluateCondition function: unsupported condition operand");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsGreaterOperand(FmiMaskOperation operand)
{
  try
  {
    return (operand == kFmiMaskGreaterThan || operand == kFmiMaskGreaterOrEqualThan);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Note: only range option accepted are:
// 1. In between case e.g. "limit1 < x < limit2", where operands can be < or <=
// 2. Outside case e.g. "limit1 > x > limit2", where operands can be > or >=
static bool EvaluateRangeCondition(double value1,
                                   FmiMaskOperation operand1,
                                   double value2,
                                   FmiMaskOperation operand2,
                                   double value3)
{
  try
  {
    bool condition1 = ::EvaluateCondition(value1, operand1, value2);
    bool condition2 = ::EvaluateCondition(value2, operand2, value3);
    if (::IsGreaterOperand(operand1))
      // Outside limits case e.g. "-5 > T > 0", only other condition is true
      return condition1 ^ condition2;

    // In between case e.g. "-5 < T < 0", both conditions must be true
    return condition1 && condition2;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// value2:n pitää olla jotenkin value1:n ja previousValue1:n välissä
static bool EvaluateContinuousEqualCase(double value1, double value2, double previousValue1)
{
  if (value1 != kFloatMissing && value2 != kFloatMissing && previousValue1 != kFloatMissing)
  {
    if (value1 >= value2 && previousValue1 <= value2) return true;
    if (previousValue1 >= value2 && value1 <= value2) return true;
  }
  return false;
}

bool NFmiSingleCondition::CheckCondition(const NFmiCalculationParams &theCalculationParams,
                                         bool fUseTimeInterpolationAlways)
{
  try
  {
    double value1 = part1->Value(theCalculationParams, fUseTimeInterpolationAlways);
    double value2 = part2->Value(theCalculationParams, fUseTimeInterpolationAlways);
    if (conditionOperand1 == kFmiMaskContinuousEqual)
    {
      return ::EvaluateContinuousEqualCase(value1, value2, part1->PreviousValue(value1));
    }
    else if (!part3)
    {
      return ::EvaluateCondition(value1, conditionOperand1, value2);
    }

    double value3 = part3->Value(theCalculationParams, fUseTimeInterpolationAlways);
    return ::EvaluateRangeCondition(value1, conditionOperand1, value2, conditionOperand2, value3);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiSingleCondition::CheckPressureCondition(double thePressure,
                                                 const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    double value1 = part1->PressureValue(thePressure, theCalculationParams);
    double value2 = part2->PressureValue(thePressure, theCalculationParams);
    if (conditionOperand1 == kFmiMaskContinuousEqual)
    {
      return ::EvaluateContinuousEqualCase(value1, value2, part1->PreviousValue(value1));
    }
    else if (!part3)
    {
      return ::EvaluateCondition(value1, conditionOperand1, value2);
    }

    double value3 = part3->PressureValue(thePressure, theCalculationParams);
    return ::EvaluateRangeCondition(value1, conditionOperand1, value2, conditionOperand2, value3);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiSingleCondition::CheckHeightCondition(double theHeight,
                                               const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    double value1 = part1->HeightValue(theHeight, theCalculationParams);
    double value2 = part2->HeightValue(theHeight, theCalculationParams);
    if (conditionOperand1 == kFmiMaskContinuousEqual)
    {
      return ::EvaluateContinuousEqualCase(value1, value2, part1->PreviousValue(value1));
    }
    else if (!part3)
    {
      return ::EvaluateCondition(value1, conditionOperand1, value2);
    }

    double value3 = part3->HeightValue(theHeight, theCalculationParams);
    return ::EvaluateRangeCondition(value1, conditionOperand1, value2, conditionOperand2, value3);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiSingleCondition::ResetPreviousValue()
{
  if (part1) part1->ResetPreviousValue();
  if (part2) part2->ResetPreviousValue();
  if (part3) part3->ResetPreviousValue();
}

// *****************************************************************
// ****************   NFmiSimpleCondition   ************************
// *****************************************************************

NFmiSimpleCondition::~NFmiSimpleCondition() = default;

NFmiSimpleCondition::NFmiSimpleCondition(
    const std::shared_ptr<NFmiSingleCondition> &theCondition1,
    NFmiAreaMask::BinaryOperator theConditionOperator,
    const std::shared_ptr<NFmiSingleCondition> &theCondition2)
    : condition1(theCondition1), conditionOperator(theConditionOperator), condition2(theCondition2)
{
}

NFmiSimpleCondition::NFmiSimpleCondition(const NFmiSimpleCondition &theOther)
    : condition1(theOther.condition1 ? theOther.condition1->Clone() : nullptr),
      conditionOperator(theOther.conditionOperator),
      condition2(theOther.condition2 ? theOther.condition2->Clone() : nullptr)
{
}

static void InitializePart(std::shared_ptr<NFmiSingleCondition> &condition)
{
  try
  {
    if (condition)
      condition->Initialize();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// T�t� kutsutaan konstruktorin j�lkeen, t�ss� alustetaan ainakin tieto siit� onko maski ns.
// stationaarist� dataa
void NFmiSimpleCondition::Initialize()
{
  try
  {
    ::InitializePart(condition1);
    ::InitializePart(condition2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiSimpleCondition *NFmiSimpleCondition::Clone() const
{
  try
  {
    return new NFmiSimpleCondition(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool EvaluateBinaryCondition(bool condition1,
                                    NFmiAreaMask::BinaryOperator conditionOperator,
                                    bool condition2)
{
  try
  {
    switch (conditionOperator)
    {
      case NFmiAreaMask::kAnd:
        return condition1 && condition2;
      case NFmiAreaMask::kOr:
        return condition1 || condition2;
      case NFmiAreaMask::kXor:
        return condition1 ^ condition2;
      default:
        throw Fmi::Exception(
            BCP,
            "Error in SimpleCondition's EvaluateCondition function: unsupported binary operator");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Eri tapauksia varten omat tarkastelu funktiot
bool NFmiSimpleCondition::CheckCondition(const NFmiCalculationParams &theCalculationParams,
                                         bool fUseTimeInterpolationAlways)
{
  try
  {
    bool conditionValue1 =
        condition1->CheckCondition(theCalculationParams, fUseTimeInterpolationAlways);
    if (!condition2)
      return conditionValue1;

    bool conditionValue2 =
        condition2->CheckCondition(theCalculationParams, fUseTimeInterpolationAlways);
    return ::EvaluateBinaryCondition(conditionValue1, conditionOperator, conditionValue2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiSimpleCondition::CheckPressureCondition(double thePressure,
                                                 const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    bool conditionValue1 = condition1->CheckPressureCondition(thePressure, theCalculationParams);
    if (!condition2)
      return conditionValue1;

    bool conditionValue2 = condition2->CheckPressureCondition(thePressure, theCalculationParams);
    return ::EvaluateBinaryCondition(conditionValue1, conditionOperator, conditionValue2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiSimpleCondition::CheckHeightCondition(double theHeight,
                                               const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    bool conditionValue1 = condition1->CheckHeightCondition(theHeight, theCalculationParams);
    if (!condition2)
      return conditionValue1;

    bool conditionValue2 = condition2->CheckHeightCondition(theHeight, theCalculationParams);
    return ::EvaluateBinaryCondition(conditionValue1, conditionOperator, conditionValue2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiSimpleCondition::ResetPreviousValue()
{
  if (condition1) condition1->ResetPreviousValue();
  if (condition2) condition2->ResetPreviousValue();
}
