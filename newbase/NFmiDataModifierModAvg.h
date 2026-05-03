// ======================================================================
/*!
 * \file NFmiDataModifierModAvg.h
 * \brief Interface of class NFmiDataModifierModAvg
 */
// ======================================================================
/*!
 * \class NFmiDataModifierModAvg
 *
 * Calculates avg value from given set of modular values (directions 0-360).
 * The difference to normal avg is that if given values are 10 and 340, the
 * avg is not (10 + 340) / 2 = 175, but it takes account of 0/360 continuity
 * and the result is (10 + 360) + 340 / 2 = 355.
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiModMeanCalculator.h"

//! Undocumented
class NFmiDataModifierModAvg : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierModAvg();
  NFmiDataModifierModAvg();
  NFmiDataModifierModAvg(const NFmiDataModifierModAvg& theOther);
  NFmiDataModifierModAvg* Clone() const;
  NFmiDataModifierModAvg& operator=(const NFmiDataModifierModAvg& theOther) = delete;

  void Clear();
  void Calculate(float theValue) override;
  float CalculationResult() override;

  static void DoSomeTestRoutines();

 protected:
  NFmiModMeanCalculator itsModMeanCalculator;
};  // class NFmiDataModifierModAvg

// ======================================================================
