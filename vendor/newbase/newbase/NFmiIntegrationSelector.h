// ======================================================================
/*!
 * \file NFmiIntegrationSelector.h
 * \brief Interface of class NFmiIntegrationSelector
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierBase.h"

//! Undocumented
enum FmiIntegrationSelector
{
  kFmiNoIntegration,
  kFmiMin,
  kFmiMax,
  kFmiMean,
  kFmiProb
};

//! Undocumented
class NFmiIntegrationSelector
{
 public:
  virtual ~NFmiIntegrationSelector();
  NFmiIntegrationSelector(FmiIntegrationSelector theSelectorType = kFmiNoIntegration,
                          FmiProbabilityCondition theProbabilityCondition = kNoCondition,
                          float theProbabilityLowerLimit = kFloatMissing,
                          float theProbabilityUpperLimit = kFloatMissing,
                          float theProbabilityScale = 100);

  NFmiIntegrationSelector(const NFmiIntegrationSelector& theSelector);

  FmiIntegrationSelector Type() const;
  FmiProbabilityCondition ProbabilityCondition() const;
  float ProbabilityLowerLimit() const;
  float ProbabilityUpperLimit() const;
  float ProbabilityScale() const;

  void Type(FmiIntegrationSelector theType);
  void ProbabilityCondition(FmiProbabilityCondition theCondition);
  void ProbabilityLowerLimit(float theLimit);
  void ProbabilityUpperLimit(float theLimit);
  void ProbabilityScale(float theScale);
  void ScaleLowerLimit(float theFactor);
  void ScaleUpperLimit(float theFactor);

 private:
  FmiIntegrationSelector itsType;
  FmiProbabilityCondition itsProbabilityCondition;
  float itsProbabilityLowerLimit;
  float itsProbabilityUpperLimit;
  float itsProbabilityScale;

};  // class NFmiIntegrationSelector

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiIntegrationSelector NFmiIntegrationSelector::Type() const
{
  return itsType;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiProbabilityCondition NFmiIntegrationSelector::ProbabilityCondition() const
{
  return itsProbabilityCondition;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiIntegrationSelector::ProbabilityLowerLimit() const
{
  return itsProbabilityLowerLimit;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiIntegrationSelector::ProbabilityUpperLimit() const
{
  return itsProbabilityUpperLimit;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiIntegrationSelector::ProbabilityScale() const
{
  return itsProbabilityScale;
}
// ----------------------------------------------------------------------
/*!
 * \param theType Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::Type(FmiIntegrationSelector theType)
{
  itsType = theType;
}
// ----------------------------------------------------------------------
/*!
 * \param theCondition Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ProbabilityCondition(FmiProbabilityCondition theCondition)
{
  itsProbabilityCondition = theCondition;
}

// ----------------------------------------------------------------------
/*!
 * \param theLimit Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ProbabilityLowerLimit(float theLimit)
{
  itsProbabilityLowerLimit = theLimit;
}

// ----------------------------------------------------------------------
/*!
 * \param theLimit Undocumented
 *
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ProbabilityUpperLimit(float theLimit)
{
  itsProbabilityUpperLimit = theLimit;
}

// ----------------------------------------------------------------------
/*!
 * \param theScale Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ProbabilityScale(float theScale)
{
  itsProbabilityScale = theScale;
}

// ----------------------------------------------------------------------
/*!
 * \param theFactor Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ScaleLowerLimit(float theFactor)
{
  if (itsProbabilityLowerLimit != kFloatMissing)
    itsProbabilityLowerLimit *= theFactor;
}

// ----------------------------------------------------------------------
/*!
 * \param theFactor Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiIntegrationSelector::ScaleUpperLimit(float theFactor)
{
  if (itsProbabilityLowerLimit != kFloatMissing)
    itsProbabilityUpperLimit *= theFactor;
}

// ======================================================================
