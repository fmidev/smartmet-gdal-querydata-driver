// ======================================================================
/*!
 * \file NFmiDataModifierSum.h
 * \brief Interface of class NFmiDataModifierSum
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierSum : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierSum();
  NFmiDataModifierSum(FmiJoinOperator theJoinOperator = kFmiAdd, bool missingValuesAlloved = true);
  NFmiDataModifierSum(const NFmiDataModifierSum& theOther);
  NFmiDataModifier* Clone() const;

  float Sum();
  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

  void Clear();
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult();

 protected:
  float itsSum;

};  // class NFmiDataModifierSum

// ======================================================================
