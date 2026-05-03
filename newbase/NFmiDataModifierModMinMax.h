// ======================================================================
/*!
 * \file NFmiDataModifierModMinMax.h
 * \brief Interface of class NFmiDataModifierModMinMax
 */
// ======================================================================
/*!
 * \class NFmiDataModifierModMinMax
 *
 * Calculates min/max values from given set of modular values (directions 0-360).
 * The difference is that if set of values are close to each others around 0/360
 * pivot-point, then the min is start-of-cluster and max is end-of-cluster.
 * Normal case values: 30, 80, 100 and 120 ==> min = 30 and max is 120.
 * Modular case values: 20, 50, 90 and 320 ==> min = 320 and max is 90,
 * because 320 is closest to 20 but it's on the 'left' side on circle of values.
 * This modular case applies only if there are values on each side of 0/360 point
 * and all the values are in close enough bunch which is inside 180 degrees spread.
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

#include <set>

//! Undocumented
class NFmiDataModifierModMinMax : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierModMinMax();
  NFmiDataModifierModMinMax(bool returnMinValue = true);
  NFmiDataModifierModMinMax(const NFmiDataModifierModMinMax& theOther);
  NFmiDataModifierModMinMax* Clone() const;
  NFmiDataModifierModMinMax& operator=(const NFmiDataModifierModMinMax& theOther) = delete;

  void Clear();
  void Calculate(float theValue) override;
  float CalculationResult() override;
  float Min();
  float Max();

  static void DoSomeTestRoutines();

 protected:
  void DoCalculations();
  void DoCalculations2();

  std::multiset<float> itsRawValues;
  float itsMinValue = kFloatMissing;
  float itsMaxValue = kFloatMissing;
  bool fReturnMinValue = true;
  bool fCalculationsAreMade = false;
};  // class NFmiDataModifierModMinMax

// ======================================================================
