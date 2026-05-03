// ======================================================================
/*!
 * \file NFmiDataModifierMaxMean.h
 * \brief Interface of class NFmiDataModifierMaxMean
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierMaxMean : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierMaxMean();
  NFmiDataModifierMaxMean();
  NFmiDataModifierMaxMean(float theWeightFactor);

  void Clear();

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult();

 private:
  long itsCounter;
  float itsAverage;
  float itsMaxValue;
  float itsWeightFactor;

};  // class NFmiDataModifierMaxMean

// ======================================================================
