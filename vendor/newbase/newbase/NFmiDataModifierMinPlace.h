// ======================================================================
/*!
 * \file NFmiDataModifierMinPlace.h
 * \brief Interface of class NFmiDataModifierMinPlace
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMinPlace
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierExtremePlace.h"

//! Undocumented
class NFmiDataModifierMinPlace : public NFmiDataModifierExtremePlace
{
 public:
  virtual ~NFmiDataModifierMinPlace();
  NFmiDataModifierMinPlace();

  void Clear();

  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult();

 protected:
  virtual bool IsNewExtreme(float value);

 private:
  NFmiDataModifierMinPlace& operator=(const NFmiDataModifierMinPlace& theOther);

};  // class NFmiDataModifierMinPlace

// ======================================================================
