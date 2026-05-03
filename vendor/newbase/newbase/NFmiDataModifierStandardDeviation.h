// ======================================================================
/*!
 * \file NFmiDataModifierStandardDeviation.h
 * \brief Interface of class NFmiDataModifierStandardDeviation
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierStandardDeviation : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierStandardDeviation();
  NFmiDataModifierStandardDeviation();

  void Clear();

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult();

 protected:
  long itsCounter;
  float itsSum;
  float itsSquaredSum;

};  // class NFmiDataModifierStandardDeviation

// ======================================================================
