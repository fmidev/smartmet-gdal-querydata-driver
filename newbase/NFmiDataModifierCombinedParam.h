// ======================================================================
/*!
 * \file NFmiDataModifierCombinedParam.h
 * \brief Interface of class NFmiDataModifierCombinedParam
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiCombinedParam;

//! Undocumented

class NFmiDataModifierCombinedParam : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierCombinedParam();
  NFmiDataModifierCombinedParam(const NFmiCombinedParam &theParam);

  virtual void Clear();

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult();

  virtual NFmiCombinedParam *CombinedCalculationResult();

 protected:
  NFmiCombinedParam *itsCombinedParam;

 private:
  NFmiDataModifierCombinedParam(const NFmiDataModifierCombinedParam &theParam);
  NFmiDataModifierCombinedParam &operator=(const NFmiDataModifierCombinedParam &theParam);

};  // class NFmiDataModifierCombinedParam

// ======================================================================
