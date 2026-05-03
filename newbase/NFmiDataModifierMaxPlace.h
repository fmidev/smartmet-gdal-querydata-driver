// ======================================================================
/*!
 * \file NFmiDataModifierMaxPlace.h
 * \brief Interface of class NFmiDataModifierMaxPlace
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMaxPlace
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierExtremePlace.h"

//! Undocumented
class NFmiDataModifierMaxPlace : public NFmiDataModifierExtremePlace
{
 public:
  virtual ~NFmiDataModifierMaxPlace();
  NFmiDataModifierMaxPlace();

  void Clear();

  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult();

 protected:
  virtual bool IsNewExtreme(float value);

 private:
  NFmiDataModifierMaxPlace& operator=(const NFmiDataModifierMaxPlace& theOther);

};  // class NFmiDataModifierMaxPlace

// ======================================================================
