// ======================================================================
/*!
 * \file NFmiDataModifierChange.h
 * \brief Interface of class NFmiDataModifierChange
 *
 * Returns the difference between the last and the first value.
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierChange : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierChange();
  NFmiDataModifierChange();
  NFmiDataModifier* Clone() const;

  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

  void Clear();
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult();

 protected:
  bool itsFirstValueOK;
  float itsFirstValue;
  float itsLastValue;

};  // class NFmiDataModifierChange

// ======================================================================
