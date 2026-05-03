// ======================================================================
/*!
 * \file NFmiDataModifierPrecFormUnion.h
 * \brief Interface of class NFmiDataModifierPrecFormUnion
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiWeatherAndCloudiness.h"

class NFmiDataModifierPrecFormUnion : public NFmiDataModifier
{
  // HUOM missingValuesAlloved = true koska pouta tuo puuttuvan !!!

 public:
  virtual ~NFmiDataModifierPrecFormUnion();
  NFmiDataModifierPrecFormUnion(FmiJoinOperator theJoinOperator = kFmiAdd,
                                bool missingValuesAlloved = true);

  void Clear();
  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult();
  float Result();

 private:
  bool fIsRain;
  bool fIsSleet;
  bool fIsSnow;
  bool fIsFreezing;

};  // class NFmiDataModifierPrecFormUnion

// ======================================================================
