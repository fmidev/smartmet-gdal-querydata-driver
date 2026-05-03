// ======================================================================
/*!
 * \file NFmiDataModifierDummy.cpp
 * \brief Implementation of class NFmiDataModifierDummy
 */
// ======================================================================
/*!
 * \class NFmiDataModifierDummy
 *
 * Undocumented
 */
// ======================================================================

#include "NFmiDataModifierDummy.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierDummy::~NFmiDataModifierDummy() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierDummy::NFmiDataModifierDummy() : NFmiDataModifier(), itsValue(kFloatMissing) {}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierDummy::Calculate(float theValue)
{
  itsValue = theValue;
}
// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierDummy::Calculate(NFmiQueryInfo* theQI)
{
  itsValue = theQI->FloatValue();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierDummy::CalculationResult()
{
  return itsValue;
}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierDummy::Clear()
{
  itsValue = kFloatMissing;
}
// ======================================================================
