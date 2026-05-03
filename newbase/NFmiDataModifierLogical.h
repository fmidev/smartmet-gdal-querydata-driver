// ======================================================================
/*!
 * \file NFmiDataModifierLogical.h
 * \brief Interface of classNFmiDataModifierLogical
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierBoolean.h"

//! Undocumented

class NFmiDataModifierLogical : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierLogical();
  NFmiDataModifierLogical(NFmiDataModifierBoolean* theCondition = 0,
                          NFmiDataModifier* thePrimaryModifier = 0,
                          NFmiDataModifier* theSecondaryModifier = 0);

  virtual std::ostream& WriteOperand(std::ostream& file) const;
  virtual double FloatValue();

 protected:
  NFmiDataModifierBoolean* itsCondition;
  NFmiDataModifier* itsSecondaryModifier;
  NFmiDataModifier* itsPrimaryModifier;

 private:
  NFmiDataModifierLogical(const NFmiDataModifierLogical& theMod);
  NFmiDataModifierLogical& operator=(const NFmiDataModifierLogical& theMod);

};  // class NFmiDataModifierLogical

// ======================================================================
