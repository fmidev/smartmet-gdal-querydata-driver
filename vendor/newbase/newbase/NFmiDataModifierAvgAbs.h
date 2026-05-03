// ======================================================================
/*!
 * \file NFmiDataModifierAvgAbs.h
 * \brief Interface of class NFmiDataModifierAvgAbs
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierAvgAbs : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierAvgAbs();
  NFmiDataModifierAvgAbs();

  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

  void Clear();
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult();
  long Counter();

 protected:
  long itsCounter;
  float itsSum;
  float itsAverage;

};  // class NFmiDataModifierAvgAbs

// ======================================================================
