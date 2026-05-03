#include "NFmiAreaMaskHelperStructures.h"

NFmiMacroParamValue::NFmiMacroParamValue() = default;

NFmiCalculationParams::NFmiCalculationParams() = default;

NFmiCalculationParams::NFmiCalculationParams(const NFmiPoint &theLatlon,
                                             unsigned long theLocationIndex,
                                             const NFmiMetTime &theTime,
                                             unsigned long theTimeIndex,
                                             bool specialCalculation,
                                             float thePressureHeight)
    : itsLatlon(theLatlon),
      itsLocationIndex(theLocationIndex),
      itsTime(theTime),
      itsTimeIndex(theTimeIndex),
      fSpecialCalculationCase(specialCalculation),
      itsPressureHeight(thePressureHeight)
{
}

const NFmiPoint &NFmiCalculationParams::UsedLatlon(bool forceCalculationGridPoint) const
{
  if (fUseModifiedLatlon || forceCalculationGridPoint)
  {
    return itsLatlon;
  }

  return itsActualCalculationPoint ? *itsActualCalculationPoint : itsLatlon;
}

void NFmiCalculationParams::SetModifiedLatlon(const NFmiPoint &modifiedLatlon,
                                              bool setUseModifiedFlag)
{
  itsLatlon = modifiedLatlon;
  if (setUseModifiedFlag)
  {
    fUseModifiedLatlon = true;
  }
}
