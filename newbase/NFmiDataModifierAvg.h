// ======================================================================
/*!
 * \file NFmiDataModifierAvg.h
 * \brief Interface of class NFmiDataModifierAvg
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierAvg : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierAvg();
  NFmiDataModifierAvg();
  NFmiDataModifierAvg(const NFmiDataModifierAvg& theOther);
  NFmiDataModifier* Clone() const;

  float Avg();
  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

  void Clear();
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult();
  long Counter();

 protected:
  long itsCounter;
  float itsAverage;

};  // class NFmiDataModifierAvg

// ======================================================================
