// ======================================================================
/*!
 * \file NFmiMaskedDataIterator.h
 * \brief Interface of class NFmiMaskedDataIterator
 */
// ======================================================================

#pragma once

#include "NFmiDataIterator.h"

class NFmiSuperSmartInfo;
class NFmiIntegrationSelector;

//! Undocumented
class NFmiMaskedDataIterator : public NFmiDataIterator
{
 public:
  virtual ~NFmiMaskedDataIterator();
  NFmiMaskedDataIterator(NFmiSuperSmartInfo* theData,
                         bool isCombinedParam = false,
                         NFmiIntegrationSelector* theIntegrationSelector = 0);

  virtual void DoForEach(NFmiDataModifier* theModifier);

  bool fIsCombinedParam;
  NFmiIntegrationSelector* itsIntegrationSelector;

 private:
  NFmiMaskedDataIterator(const NFmiMaskedDataIterator& theIter);
  NFmiMaskedDataIterator& operator=(const NFmiMaskedDataIterator& theIter);

};  // class NFmiMaskedDataIterator

// ======================================================================
