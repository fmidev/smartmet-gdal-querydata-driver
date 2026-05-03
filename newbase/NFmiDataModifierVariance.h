// ======================================================================
/*!
 * \file NFmiDataModifierVariance.h
 * \brief Interface of class NFmiDataModifierVariance
 */
// ======================================================================

#pragma once

#include "NFmiRegressionModifier.h"

#include <boost/thread.hpp>
typedef boost::shared_mutex MutexType;

//! Undocumented
class NFmiDataModifierVariance : public NFmiRegressionModifier
{
 public:
  virtual ~NFmiDataModifierVariance();
  NFmiDataModifierVariance(NFmiDataIdent* theDataIdent,
                           NFmiLevel* theLevel,
                           NFmiQueryInfo* theData = 0,
                           float theVarianceLevel = 1.0,
                           bool onlyVariance = false);

  virtual double FloatValue();

 protected:
  double GaussianRandom();
  bool fVarianceOnly;
  double itsVarianceLevel;
  mutable MutexType itsMutex;

};  // class NFmiDataModifierVariance

// ======================================================================
