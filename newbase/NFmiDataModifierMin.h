// ======================================================================
/*!
 * \file NFmiDataModifierMin.h
 * \brief Interface of class NFmiDataModifierMin
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierExtreme.h"

class NFmiDataModifierMin : public NFmiDataModifierExtreme
{
 public:
  virtual ~NFmiDataModifierMin();
  NFmiDataModifierMin();
  NFmiDataModifierMin(const NFmiDataModifierMin& theOther);
  NFmiDataModifier* Clone() const;

  //  virtual void Calculate(float theValue);

  void Clear();
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult();

 protected:
  virtual bool IsNewExtreme(float value);

 private:
  NFmiDataModifierMin& operator=(const NFmiDataModifierMin& theOther);

};  // class NFmiDataModifierMin

// ======================================================================
