// ======================================================================
/*!
 * \file NFmiDataModifierAllValidSum.h
 * \brief Interface of class NFmiDataModifierAllValidSum
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierSum.h"

class NFmiDataModifierAllValidSum : public NFmiDataModifierSum
{
 public:
  virtual ~NFmiDataModifierAllValidSum();
  NFmiDataModifierAllValidSum();

  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

  void Clear();

 protected:
  long itsCounter;

 private:
  NFmiDataModifierAllValidSum& operator=(const NFmiDataModifierAllValidSum& theOther);

};  // class NFmiDataModifierAllValidSum

// ======================================================================
