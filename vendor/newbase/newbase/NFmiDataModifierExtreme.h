// ======================================================================
/*!
 * \file NFmiDataModifierExtreme.h
 * \brief abstrakti luokka min/max:ia varten joka myös antaa äärirvon
 *  sattumisajan/Lasse
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiMetTime.h"

//! Undocumented
class NFmiDataModifierExtreme : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierExtreme();
  NFmiDataModifierExtreme();
  NFmiDataModifierExtreme(const NFmiDataModifierExtreme &theOther);

  void SetTime(NFmiQueryInfo *theQI);
  const NFmiTime GetTime();
  virtual void Calculate(NFmiQueryInfo *theQI);
  virtual void Calculate(float theValue);
  virtual void Calculate(float theValue, NFmiQueryInfo *theQI);

 protected:
  virtual bool IsNewExtreme(float) = 0;

 public:
  float itsExtremeValue;
  NFmiTime itsExtremeTime;

};  // class NFmiDataModifierExtreme

// ======================================================================
