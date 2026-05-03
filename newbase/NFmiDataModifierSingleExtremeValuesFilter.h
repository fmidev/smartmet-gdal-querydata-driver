// ======================================================================
/*!
 * \file NFmiDataModifierSingleExtremeValuesFilter.h
 * \brief Interface of class NFmiDataModifierSingleExtremeValuesFilter
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierAvg.h"

class NFmiDataModifierSingleExtremeValuesFilter : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierSingleExtremeValuesFilter();
  NFmiDataModifierSingleExtremeValuesFilter();
  NFmiDataModifierSingleExtremeValuesFilter(float thePointValue, float theLimit);
  void Clear();
  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult();

 private:
  NFmiDataModifierAvg itsAvgModifier;
  float itsValue;
  float itsLimit;

};  // class NFmiDataModifierSingleExtremeValuesFilter

// ======================================================================
