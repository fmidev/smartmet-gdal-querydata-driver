// ======================================================================
/*!
 * \file NFmiDataModifierPrecFormUnion.cpp
 * \brief Implementation of class NFmiDataModifierPrecFormUnion
 */
// ======================================================================
/*!
 * \class NFmiDataModifierPrecFormUnion
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierPrecFormUnion.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierPrecFormUnion::~NFmiDataModifierPrecFormUnion() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theJoinOperator Undocumented
 * \param missingValuesAlloved Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierPrecFormUnion::NFmiDataModifierPrecFormUnion(FmiJoinOperator theJoinOperator,
                                                             bool missingValuesAlloved)
    : NFmiDataModifier(theJoinOperator, missingValuesAlloved),
      fIsRain(false),
      fIsSleet(false),
      fIsSnow(false),
      fIsFreezing(false)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierPrecFormUnion::Result()
{
  return static_cast<float>(fIsRain * 1 + fIsSleet * 2 + fIsSnow * 4 + fIsFreezing * 8);
}

// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierPrecFormUnion::Calculate(NFmiQueryInfo* theQI)
{
  Calculate(theQI->FloatValue());
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierPrecFormUnion::Clear()
{
  NFmiDataModifier::Clear();
  fIsRain = false;
  fIsSleet = false;
  fIsSnow = false;
  fIsFreezing = false;
  fCalculationResultOk = false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierPrecFormUnion::CalculationResult()
{
  if (fCalculationResultOk)
    return Result();
  else
    return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue
 */
// ----------------------------------------------------------------------

void NFmiDataModifierPrecFormUnion::Calculate(float theValue)
{
  if (CheckMissingValues(theValue))
  {
    if (IsInside(theValue))
    {
      if (NFmiWeatherAndCloudiness::IsFair(theValue))
        ;
      else if (NFmiWeatherAndCloudiness::IsFreezing(theValue))
        fIsFreezing = true;
      else if (NFmiWeatherAndCloudiness::IsLiquid(theValue))
        fIsRain = true;
      else if (NFmiWeatherAndCloudiness::IsHalfLiquid(theValue))
        fIsSleet = true;
      else if (NFmiWeatherAndCloudiness::IsSolid(theValue))
        fIsSnow = true;
    }
  }
}
