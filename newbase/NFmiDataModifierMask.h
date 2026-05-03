// ======================================================================
/*!
 * \file NFmiDataModifierMask.h
 * \brief Interface of class NFmiDataModiferMask
 */
// ======================================================================
/*!
 * \class NFmiDataModiferMask
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiGlobals.h"  // Added by ClassView

//! Undocumented
class NFmiDataModifierMask : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierMask();
  NFmiDataModifierMask(FmiMaskOperation theCondition = kFmiNoMaskOperation,
                       double theFirstLimit = kFloatMissing,
                       double theSecondLimit = kFloatMissing);
  virtual bool BoolOperation(float theValue);

 protected:
  FmiMaskOperation itsMaskOperation;
  double its1Limit;
  double its2Limit;

};  // class NFmiDataModifierMask

// ======================================================================
