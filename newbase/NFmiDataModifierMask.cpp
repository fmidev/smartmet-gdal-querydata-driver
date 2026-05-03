// ======================================================================
/*!
 * \file NFmiDataModifierMask.cpp
 * \brief Implementation of class NFmiDataModifierMask
 */
// ======================================================================

#include "NFmiDataModifierMask.h"
#include <cmath>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierMask::~NFmiDataModifierMask() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theCondition Undocumented
 * \param theFirstLimit Undocumented
 * \param theSecondLimit Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierMask::NFmiDataModifierMask(FmiMaskOperation theCondition,
                                           double theFirstLimit,
                                           double theSecondLimit)
    : itsMaskOperation(theCondition), its1Limit(theFirstLimit), its2Limit(theSecondLimit)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierMask::BoolOperation(float theValue)
{
  if (theValue == kFloatMissing)
    return false;

  switch (itsMaskOperation)
  {
    case kFmiNoMaskOperation:
      return true;
    case kFmiMaskEqual:
      return (theValue == its1Limit);  // kuinka suurella tarkkuudella???
    case kFmiMaskNotEqual:
      return (theValue != its1Limit);  // kuinka suurella tarkkuudella???
    case kFmiMaskGreaterThan:
      return (theValue > its1Limit);
    case kFmiMaskLessThan:
      return (theValue < its1Limit);
    case kFmiMaskGreaterOrEqualThan:
      return (theValue >= its1Limit);
    case kFmiMaskLessOrEqualThan:
      return (theValue <= its1Limit);
    case kFmiMaskGreaterThanAndLessThan:
      return (theValue > its1Limit && theValue < its2Limit);
    case kFmiMaskGreaterOrEqualAndLessOrEqual:
      return (theValue >= its1Limit && theValue <= its2Limit);
    case kFmiMaskGreaterOrEqualAndLessThan:
      return (theValue >= its1Limit && theValue < its2Limit);
    case kFmiMaskGreaterThanAndLessOrEqual:
      return (theValue > its1Limit && theValue <= its2Limit);
    case kFmiMaskLessThanOrGreaterThan:
      return (theValue < its1Limit || theValue > its2Limit);
    case kFmiMaskLessOrEqualOrGreaterOrEqual:
      return (theValue <= its1Limit || theValue >= its2Limit);
    case kFmiMaskLessOrEqualOrGreaterThan:
      return (theValue <= its1Limit || theValue > its2Limit);
    case kFmiMaskLessThanOrGreaterOrEqual:
      return (theValue < its1Limit || theValue >= its2Limit);
    case kFmiMaskEqualOrEqual:
      return (round(theValue) == its1Limit || round(theValue) == its2Limit);
    case kFmiMaskAll:
      return true;
    case kFmiMaskRisingRamp:
      return (theValue > its1Limit);  // tähän ei ole oikeastaan kunnon tarkastelua, todellinen
                                      // käyttö kun lasketaan maskikertoimia!
    case kFmiMaskLoweringRamp:
      return (theValue < its1Limit);  // tähän ei ole oikeastaan kunnon tarkastelua, todellinen
                                      // käyttö kun lasketaan maskikertoimia!
    case kFmiMaskDoubleRamp:
      return (theValue < its1Limit);  // tähän ei ole oikeastaan kunnon tarkastelua, todellinen
                                      // käyttö kun lasketaan maskikertoimia!
  }
  return false;
}

// ======================================================================
