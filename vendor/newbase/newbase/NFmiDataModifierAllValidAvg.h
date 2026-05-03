// ======================================================================
/*!
 * \file NFmiDataModifierAllValidAvg.h
 * \brief Interface of class NFmiDataModifierAllValidAvg
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierAvg.h"

class NFmiDataModifierAllValidAvg : public NFmiDataModifierAvg
{
 public:
  virtual ~NFmiDataModifierAllValidAvg();
  NFmiDataModifierAllValidAvg();
  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

 private:
  NFmiDataModifierAllValidAvg& operator=(const NFmiDataModifierAllValidAvg& theOther);

};  // class NFmiDataModifierAllValidAvg

// ======================================================================
