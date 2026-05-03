// ======================================================================
/*!
 * \file NFmiInfoModifier.h
 * \brief Interface of class NFmiInfoModifier
 */
// ======================================================================

#pragma once

#include "NFmiParamDataModifier.h"

class NFmiQueryInfo;

//! Undocumented
class NFmiInfoModifier : public NFmiParamDataModifier
{
 public:
  virtual ~NFmiInfoModifier();
  NFmiInfoModifier(NFmiDataIdent* theParam,
                   NFmiLevel* theLevel,
                   NFmiQueryInfo* theData = 0,
                   FmiJoinOperator theJoinOperator = kFmiAdd);
  NFmiQueryInfo* Info();
  virtual double FloatValue();
  virtual double PeekValue(long dx, long dy, long dt);

 protected:
  NFmiQueryInfo* itsData;

 private:
  NFmiInfoModifier(const NFmiInfoModifier& theMod);
  NFmiInfoModifier& operator=(const NFmiInfoModifier& theMod);

};  // class NFmiInfoModifier

// ======================================================================
