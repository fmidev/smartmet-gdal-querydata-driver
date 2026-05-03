// ======================================================================
/*!
 * \file NFmiDataModifierBoolean.h
 * \brief Interface of class NFmiDataModifierBoolean
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

//! Undocumented
enum FmiModifierBoolOperations
{
  kFmiModifierValueLessThan,
  kFmiModifierValueGreaterThan,
  kFmiModifierValueLessOrEqualThan,
  kFmiModifierValueGreaterOrEqualThan,
  kFmiModifierValueIsEqual
};

//! Undocumented
class NFmiDataModifierBoolean : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierBoolean();
  NFmiDataModifierBoolean(FmiModifierBoolOperations theCondition,
                          NFmiDataModifier* theFirstValue,
                          NFmiDataModifier* theSecondValue);

  virtual bool BooleanValue();

  virtual std::ostream& WriteOperator(std::ostream& file) const;
  virtual std::ostream& WriteExpressionBody(std::ostream& file);

  operator bool();

 protected:
  NFmiDataModifier* itsSecondValue;
  NFmiDataModifier* itsFirstValue;
  FmiModifierBoolOperations itsCondition;
  bool itsBooleanValue;

 private:
  NFmiDataModifierBoolean(const NFmiDataModifierBoolean& theMod);
  NFmiDataModifierBoolean& operator=(const NFmiDataModifierBoolean& theMod);

};  // class NFmiDataModifierBoolean

// ======================================================================
