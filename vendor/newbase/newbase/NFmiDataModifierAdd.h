// ======================================================================
/*!
 * \file NFmiDataModifierAdd.h
 * \brief Interface of class NFmiDataModifierAdd
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierAdd : public NFmiDataModifier
{
 public:
  NFmiDataModifierAdd(float theAddValue = 0);
  virtual float FloatOperation(float theValue);

 private:
  float itsAddValue;

};  // class NFmiDataModifierAdd

// ======================================================================
