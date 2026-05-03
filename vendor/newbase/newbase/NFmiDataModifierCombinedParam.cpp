// ======================================================================
/*!
 * \file NFmiDataModifierCombinedParam.cpp
 * \brief Implementation of class NFmiDataModifierCombinedParam
 */
// ======================================================================
/*!
 * \class NFmiDataModifierCombinedParam
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierCombinedParam.h"
#include "NFmiCombinedParam.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombinedParam::~NFmiDataModifierCombinedParam()
{
  delete itsCombinedParam;
}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theParam The other object being copied
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombinedParam::NFmiDataModifierCombinedParam(const NFmiCombinedParam& theParam)
    : itsCombinedParam(theParam.Clone())
{
  itsCombinedParam->InitIntegration();
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierCombinedParam::Calculate(float theValue)
{
  if (theValue != kFloatMissing)
  {
    itsCombinedParam->Integrate(theValue);
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierCombinedParam::CalculationResult()
{
  itsCombinedParam->EndIntegration();
  return itsCombinedParam->TransformedFloatValue();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam* NFmiDataModifierCombinedParam::CombinedCalculationResult()
{
  itsCombinedParam->EndIntegration();
  return itsCombinedParam;
}

// ----------------------------------------------------------------------
/*!
 * Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierCombinedParam::Clear()
{
  itsCombinedParam->ClearIntegration();
}
// ======================================================================
