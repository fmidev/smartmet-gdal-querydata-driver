// ======================================================================
/*!
 * \file NFmiDataModifierMax.h
 * \brief Interface of class NFmiDataModifierMax
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMax
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierExtreme.h"

//! Undocumented
class NFmiDataModifierMax : public NFmiDataModifierExtreme
{
 public:
  virtual ~NFmiDataModifierMax();
  NFmiDataModifierMax();
  NFmiDataModifierMax(const NFmiDataModifierMax& theOther);
  NFmiDataModifier* Clone() const;

  void Clear();
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult();

 protected:
  virtual bool IsNewExtreme(float value);

 private:
  NFmiDataModifierMax& operator=(const NFmiDataModifierMax& theOther);

};  // class NFmiDataModifierMax

// ======================================================================
