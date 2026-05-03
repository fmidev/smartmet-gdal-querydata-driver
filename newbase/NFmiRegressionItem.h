// ======================================================================
/*!
 * \file NFmiRegressionItem.h
 * \brief Interface of class NFmiRegressionItem
 */
// ======================================================================

#pragma once

#include "NFmiInfoModifier.h"

class NFmiSuperSmartInfo;
class NFmiDataIdent;

//! Undocumented
class NFmiRegressionItem : public NFmiInfoModifier
{
 public:
  virtual ~NFmiRegressionItem();
  NFmiRegressionItem(double theCoefficient = 1.0,
                     NFmiDataIdent* theDataIdent = 0,
                     NFmiLevel* theLevel = 0,
                     NFmiQueryInfo* theData = 0);

  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(NFmiQueryInfo* theData);
  virtual void Calculate(float theValue) { NFmiInfoModifier::Calculate(theValue); }
  virtual float CalculationResult();
  virtual double FloatValue();

 protected:
  double itsReturnValue;

 private:
  double itsCoefficient;
  double itsConstant;

};  // class NFmiRegressionItem

// ======================================================================
