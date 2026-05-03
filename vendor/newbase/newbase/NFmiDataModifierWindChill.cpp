// ======================================================================
/*!
 * \file NFmiDataModifierWindChill.cpp
 * \brief Implementation of class NFmiDataModifierWindChill
 */
// ======================================================================
/*!
 * \class NFmiDataModifierWindChill
 *
 * Luokka laskee wind chill arvon siten että pyytää info:lta läpötilan
 * ja tuulen nopeuden arvon ja laskee wind chilin niiden avulla.
 * Käytetään esim. queryInfo->CalcTimeData:n yhteydessä.
 *
 * Modifier ottaa parametrina vielä toisen modifierin jonka avulla voidaan
 * laskea esim. max tai min arvo jonkin päivän ajalta.
 *
 */
// ======================================================================

#include "NFmiDataModifierWindChill.h"
#include "NFmiMetMath.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierWindChill::~NFmiDataModifierWindChill() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theQueryInfo Undocumented
 * \param theDataModifier Undocumented
 * \param theLatLonPoint Undocumented
 * \param theTeperatureParamId Undocumented
 * \param theWindSpeedParamId Undocumented
 *
 * \todo Spell the theTeperatureParamId argument correctly.
 */
// ----------------------------------------------------------------------

NFmiDataModifierWindChill::NFmiDataModifierWindChill(NFmiQueryInfo* theQueryInfo,
                                                     NFmiDataModifier* theDataModifier,
                                                     const NFmiPoint& theLatLonPoint,
                                                     int theTeperatureParamId,
                                                     int theWindSpeedParamId)
    : NFmiDataModifier(),
      itsTemperatureParamId(theTeperatureParamId),
      itsWindSpeedParamId(theWindSpeedParamId),
      itsLatLonPoint(theLatLonPoint),
      itsQueryInfo(theQueryInfo),
      itsDataModifier(theDataModifier)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theInfo Undocumented, unused
 *
 * \todo Take new newbase into use
 */
// ----------------------------------------------------------------------

void NFmiDataModifierWindChill::Calculate(float /* theValue */)
{
  if (itsQueryInfo)
  {
    auto oldParam = FmiParameterName(itsQueryInfo->Param().GetParamIdent());

    // unsigned long oldPlaceIndex = itsQueryInfo->LocationIndex(); // tämä toimii vain uudessa
    // newbase:ssa (parempi!)
    NFmiPoint oldLatlon(
        itsQueryInfo->LatLon());  // kommentoi tämä kun uusi versio käytössä!!!!!!!!!

    float T = kFloatMissing;
    if (itsQueryInfo->Param(FmiParameterName(itsTemperatureParamId)))
      T = itsQueryInfo->InterpolatedValue(itsLatLonPoint);
    float WS = kFloatMissing;
    if (itsQueryInfo->Param(FmiParameterName(itsWindSpeedParamId)))
      WS = itsQueryInfo->InterpolatedValue(itsLatLonPoint);  // HUOM 2 on Njurmen tulkinta

    itsDataModifier->Calculate(FmiWindChill(WS, T));
    itsQueryInfo->Param(oldParam);  // palauta parametri alkutilaan!!

    // itsQueryInfo->LocationIndex(oldPlaceIndex); // tämä toimii vain uudessa newbase:ssa
    // (parempi!) // palauta vanha paikka myös (pitäisiköhän
    // itsQueryInfo->InterpolatedValue(itsLatLonPoint); -metodi muuttaa niin että paikka ei
    // muutu???)
    itsQueryInfo->Location(oldLatlon);  // kommentoi tämä kun uusi versio käytössä!!!!!!!!!
  }
  return;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierWindChill::CalculationResult()
{
  return itsDataModifier->CalculationResult();
}

// ----------------------------------------------------------------------
/*!
 * Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierWindChill::Clear()
{
  itsDataModifier->Clear();
}
// ======================================================================
