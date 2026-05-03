// ======================================================================
/*!
 * \file NFmiDataModifierMedian.h
 * \brief Interface of class NFmiDataModifierMedian (8.10.03/EL)
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMedian
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiDataModifier.h"

class NFmiDataModifierMedian : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierMedian();
  NFmiDataModifierMedian(float theLimitProsent = 50.f);
  NFmiDataModifierMedian(const NFmiDataModifierMedian& theOther);
  NFmiDataModifier* Clone() const;

  float Median();
  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

  void Clear();
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult();
  float LimitProsent() const { return itsLimitProsent; }
  void LimitProsent(float newValue);

 protected:
  std::vector<float> itsMedianArray;
  float itsLimitProsent;  // ok median filter nimensä puolesta palauttaa puolivälistä, mutta lisäsin
                          // kuitenkin
  // säädettävän rajan, jonka mukaan arvo palautetaan. Defaulttina raja on 50% eli juuri puoliväli

};  // class NFmiDataModifierMedian

// ======================================================================
