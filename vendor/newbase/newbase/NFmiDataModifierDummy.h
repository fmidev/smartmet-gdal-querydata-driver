// ======================================================================
/*!
 * \file NFmiDataModifierDummy.h
 * \brief Interface of class NFmiDataModifierDummy
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

// tallettaa vain arvon. Voidaan käyttää windChill-modifierista jos ei
// mitään varsinaista modifieria/Lasse

//! Undocumented
class NFmiDataModifierDummy : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierDummy();
  NFmiDataModifierDummy();
  using NFmiDataModifier::Calculate;
  virtual void Calculate(float theValue);

  // Lasse ei kutsuta mistään, aikaisemmat kutsut QI::CalcInterpolatedTimeData ja
  // QI::CalcTimeData:sta jouduttu poistamaan

  virtual void Calculate(NFmiQueryInfo* theQI);
  using NFmiDataModifier::CalculationResult;
  float CalculationResult();
  virtual void Clear();

 protected:
  float itsValue;

};  // class NFmiDataModifierDummy

// ======================================================================
