// ======================================================================
/*!
 * \file NFmiDataModifierMinMax.h
 * \brief Interface of class NFmiDataModifierMinMax
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

// Mika: For some reason Red-Hat Linux does not have FLT_MAX...?

#ifdef UNIX
#include <limits.h>
#ifndef FLT_MAX
#define FLT_MAX 1e+37
#endif
#else
#include <limits>
#endif

class NFmiDataModifierMinMax : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierMinMax();
  NFmiDataModifierMinMax();
  NFmiDataModifierMinMax(const NFmiDataModifierMinMax& theOther);
  NFmiDataModifier* Clone() const;

  float MinValue();
  float MaxValue();

  void Clear();

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult();  // kysy erikseen min ja max!

 private:
  float itsMinValue;
  float itsMaxValue;

};  // class NFmiDataModifierMinMax

// ======================================================================
