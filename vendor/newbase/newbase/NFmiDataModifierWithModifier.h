// ======================================================================
/*!
 * \file NFmiDataModifierWithModifier.h
 * \brief Interface of class NFmiDataModifierWithModifier
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierWithModifier : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierWithModifier();
  NFmiDataModifierWithModifier(NFmiQueryInfo* theQueryInfoCopy,
                               NFmiDataModifier* theDataModifier,
                               int theXRange,
                               int theYRange);

  virtual float FloatOperation(float theValue);
  virtual void Clear();
  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult();

 protected:
  NFmiQueryInfo* itsQueryInfoCopy;
  NFmiDataModifier* itsModifier;  // omistaa!!! tuhotaan destruktorissa
  int itsXRange;
  int itsYRange;

 private:
  NFmiDataModifierWithModifier(const NFmiDataModifierWithModifier& theMod);
  NFmiDataModifierWithModifier& operator=(const NFmiDataModifierWithModifier& theMod);

};  // class NFmiDataModifierWithModifier

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiDataModifierWithModifier::~NFmiDataModifierWithModifier()
{
  delete itsModifier;
}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theQueryInfoCopy Undocumented
 * \param theDataModifier Undocumented
 * \param theXRange Undocumented
 * \param theYRange Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiDataModifierWithModifier::NFmiDataModifierWithModifier(NFmiQueryInfo* theQueryInfoCopy,
                                                                  NFmiDataModifier* theDataModifier,
                                                                  int theXRange,
                                                                  int theYRange)
    : itsQueryInfoCopy(theQueryInfoCopy),
      itsModifier(theDataModifier),
      itsXRange(theXRange),
      itsYRange(theYRange)
{
}

// ----------------------------------------------------------------------
/*
 *
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifierWithModifier::Clear()
{
  itsModifier->Clear();
}
// ----------------------------------------------------------------------
/*
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifierWithModifier::CalculationResult()
{
  return itsModifier->CalculationResult();
}

// ======================================================================
