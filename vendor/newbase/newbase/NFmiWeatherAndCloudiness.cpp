// ======================================================================
/*!
 * \file NFmiWeatherAndCloudiness.cpp
 * \brief Implementation of class NFmiWeatherAndCloudiness
 * \todo Refactor class independent code to a separate file
 */
// ======================================================================
/*!
 * \class NFmiWeatherAndCloudiness
 *
 * Undocumented
 *
 * \todo This class really needs more magic numbers littered in the code.
 *       The more the merrier.
 */
// ======================================================================

#include "NFmiWeatherAndCloudiness.h"
#include "NFmiDataIdent.h"
#include "NFmiDataModifierClasses.h"
#include "NFmiDataModifierCombi.h"
#include "NFmiIntegrationSelector.h"
#include "NFmiParamBag.h"
#include "NFmiPoint.h"
#include "NFmiQueryInfo.h"
#include <macgyver/Exception.h>
#include <algorithm>
#include <cmath>

using namespace std;

namespace
{
// Marko: Jouduin siirtämään parametrin tänne DLL ongelmien takia
// (gsm-lib + WeatherAndCloudiness ei linkkaantunut jostain syystä
// DLL versiona)

bool fDefaultPrecipitationTypeIsLargeScale = false;

// näitä lukuja käytetään eri versioissa vertailussa, onko sadetta vai ei

const float gMinimumPrecipitationForV6 = 0.1f;
const float gMinimumPrecipitationForV7 = 0.015f;
}  // namespace

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::DefaultPrecipitationTypeIsLargeScale() const
{
  return fDefaultPrecipitationTypeIsLargeScale;
}

// ----------------------------------------------------------------------
/*!
 * \param newState Undocumented
 */
// ----------------------------------------------------------------------

void NFmiWeatherAndCloudiness::DefaultPrecipitationTypeIsLargeScale(bool newState)
{
  fDefaultPrecipitationTypeIsLargeScale = newState;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiWeatherAndCloudiness::NFmiWeatherAndCloudiness(double theInfoVersion)
    : NFmiCombinedParam(theInfoVersion),
      itsData(),
      itsTemperature(kFloatMissing),
      itsCrossCheckIndex(),
      itsPrecipitationProbabilityLimit1(),
      itsPrecipitationProbabilityLimit2()
{
  try
  {
    itsData.longType = kTCombinedWeatherMissing;
    fDataOk = true;
    SetPrecipitationProbabilityLimits();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 *
 * \todo The current info version is hard coded in here, it should
 *       be an external constant
 */
// ----------------------------------------------------------------------

NFmiWeatherAndCloudiness::NFmiWeatherAndCloudiness()
    : NFmiCombinedParam(7.),
      itsData(),
      itsTemperature(kFloatMissing),
      itsCrossCheckIndex(),
      itsPrecipitationProbabilityLimit1(),
      itsPrecipitationProbabilityLimit2()
{
  try
  {
    itsData.longType = kTCombinedWeatherMissing;
    fDataOk = true;
    SetPrecipitationProbabilityLimits();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theValue Undocumented
 * \param theParamType Undocumented
 * \param theTemperature Undocumented
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiWeatherAndCloudiness::NFmiWeatherAndCloudiness(unsigned long theValue,
                                                   FmiWeatherParamType theParamType,
                                                   float theTemperature,
                                                   double theInfoVersion)
    : NFmiCombinedParam(theInfoVersion),
      itsData(),
      itsTemperature(theTemperature),
      itsCrossCheckIndex(),
      itsPrecipitationProbabilityLimit1(),
      itsPrecipitationProbabilityLimit2()
{
  try
  {
    if (theParamType == kFmiPackedWeather)  // tekee mistä tahansa luvusta kiltisti säätä
    {                                       // tarkistettava ulkopuolella!!!!
      itsData.longType = theValue;
      fDataOk = true;  // pitäisi tarkistaa?!
    }
    else
    {
      itsData.longType = kTCombinedWeatherMissing;
      fDataOk = FromHessaa(theValue);
    }

    SetPrecipitationProbabilityLimits();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theValue Undocumented
 * \param theParamType Undocumented
 * \param theTemperature Undocumented
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiWeatherAndCloudiness::NFmiWeatherAndCloudiness(float theValue,
                                                   FmiWeatherParamType theParamType,
                                                   float theTemperature,
                                                   double theInfoVersion)
    : NFmiCombinedParam(theInfoVersion),
      itsData(),
      itsTemperature(theTemperature),
      itsCrossCheckIndex(),
      itsPrecipitationProbabilityLimit1(),
      itsPrecipitationProbabilityLimit2()
{
  try
  {
    if (theParamType == kFmiPackedWeather)  // tekee mistä tahansa luvusta kiltisti säätä
    {                                       // tarkistettava ulkopuolella!!!!
      itsData.longType = ConvertFloatToLong(theValue);
      fDataOk = true;  // pitäisi tarkistaa?!
    }
    else
    {
      itsData.longType = kTCombinedWeatherMissing;
      fDataOk = FromHessaa(ConvertFloatToLong(theValue));
    }

    SetPrecipitationProbabilityLimits();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructori
 *
 * \todo Unfinished, for example does not set cloudiness correctly.
 *       Marko, 18.09.2000
 */
// ----------------------------------------------------------------------

NFmiWeatherAndCloudiness::NFmiWeatherAndCloudiness(unsigned long theSynopCloudValue,
                                                   unsigned long theSynopWeatherValue,
                                                   float theTemperature,
                                                   double theInfoVersion)
    : NFmiCombinedParam(theInfoVersion),
      itsData(),
      itsTemperature(theTemperature),
      itsCrossCheckIndex(),
      itsPrecipitationProbabilityLimit1(),
      itsPrecipitationProbabilityLimit2()
{
  try
  {
    // konstruktoriin kaikki pilvet oletusarvoilla
    // PITÄÄKÖ ALUSTAA TYHJÄKSI=k4BitMissing, JOS theSynopWeatherValue=PUUTTUVA
    // (=k4BitMissing VAI 32700?)

    itsData.longType = kTCombinedWeatherMissing;

    SetTotalCloudiness(theSynopCloudValue);

    SetTotalPrecipitation(
        CalcPrecipitationFromHessaa(CalcSynopPrecipitationAmount(theSynopWeatherValue)));
    SetPrecipitationType(CalcSynopPrecipitationType(theSynopWeatherValue, theSynopCloudValue));
    SetPrecipitationForm(CalcSynopPrecipitationForm(theSynopWeatherValue, theSynopCloudValue));
    SetThunderProbability(CalcSynopThunderProb(theSynopWeatherValue));

    SetFogIntensity(CalcSynopFogIntensity(theSynopWeatherValue));

    // tarkistukset puuttuu
    fDataOk = true;

    SetPrecipitationProbabilityLimits();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor for synop codes
 *
 * \param theSynopCloudValue Undocumented
 * \param theSynopWeatherValue Undocumented
 * \param theHumidity Undocumented
 * \param theTemperature Undocumented
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiWeatherAndCloudiness::NFmiWeatherAndCloudiness(unsigned long theSynopCloudValue,
                                                   unsigned long theSynopWeatherValue,
                                                   float theHumidity,
                                                   float theTemperature,
                                                   double theInfoVersion)
    : NFmiCombinedParam(theInfoVersion),
      itsData(),
      itsTemperature(theTemperature),
      itsCrossCheckIndex(),
      itsPrecipitationProbabilityLimit1(),
      itsPrecipitationProbabilityLimit2()
{
  try
  {
    itsData.longType = kTCombinedWeatherMissing;
    SetTotalCloudiness(theSynopCloudValue);

    SetPrecipitationType(CalcSynopPrecipitationType(theSynopWeatherValue, theSynopCloudValue));
    SetPrecipitationForm(CalcSynopPrecipitationForm(
        theSynopWeatherValue, theSynopCloudValue, theHumidity, theTemperature));
    SetThunderProbability(CalcSynopThunderProb(theSynopWeatherValue));

    SetFogIntensity(CalcSynopFogIntensity(theSynopWeatherValue));

    // tarkistukset puuttuu
    fDataOk = true;

    SetPrecipitationProbabilityLimits();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theCloudSymbol Undocumented
 * \param theHSade Undocumented
 * \param theTemperature Undocumented
 * \param theTotalCloudiness Undocumented
 * \param theFogSymbol Undocumented
 * \param thePrecipitation Undocumented
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiWeatherAndCloudiness::NFmiWeatherAndCloudiness(float theCloudSymbol,
                                                   float theHSade,
                                                   float theTemperature,
                                                   float theTotalCloudiness,
                                                   float theFogSymbol,
                                                   float thePrecipitation,
                                                   double theInfoVersion)
    : NFmiCombinedParam(theInfoVersion),
      itsData(),
      itsTemperature(theTemperature),
      itsCrossCheckIndex(),
      itsPrecipitationProbabilityLimit1(),
      itsPrecipitationProbabilityLimit2()
{
  try
  {
    itsData.longType = kTCombinedWeatherMissing;
    if (theHSade != kFloatMissing)
      theHSade = float(int(theHSade) % 1000);
    if (theTotalCloudiness != kFloatMissing && (theHSade != kFloatMissing))
    {
      fDataOk = true;
      SetTotalCloudiness(static_cast<unsigned long>(round(theTotalCloudiness / 10)));
      CalcClouds(static_cast<unsigned long>(theCloudSymbol));  // arvot ala-, keski- ja yläpilville

      if (theFogSymbol != kFloatMissing)
        SetFogIntensity(CalcFogIntensity(static_cast<unsigned long>(theFogSymbol)));
      SetThunderProbability(CalcSynopThunderProb(static_cast<unsigned long>(theHSade)));

      if (thePrecipitation == kFloatMissing && theHSade != kFloatMissing)
        SetTotalPrecipitation(CalcPrecipitationFromHessaa(
            CalcSynopPrecipitationAmount(static_cast<unsigned long>(theHSade))));
      else if (thePrecipitation != kFloatMissing)
      {
        if (theHSade == 0 && thePrecipitation >= ((itsInfoVersion >= 7.)
                                                      ? gMinimumPrecipitationForV7
                                                      : gMinimumPrecipitationForV6))  // Marko testi
        {
          thePrecipitation = 0;
        }
        else if (thePrecipitation < ((itsInfoVersion >= 7.) ? gMinimumPrecipitationForV7
                                                            : gMinimumPrecipitationForV6) ||
                 TotalCloudiness() == 0 || theHSade == 0)  // Marko testi
        {
          thePrecipitation = 0;
          theHSade = 0;
        }
        SetTotalPrecipitation(PrecipitationScale(thePrecipitation));
      }
      if (theHSade != 0 && TotalPrecipitation() != 0)
      {
        SetPrecipitationType(CalcPrecipitationType(static_cast<unsigned long>(theHSade)));
        SetPrecipitationForm(CalcPrecipitationForm(static_cast<unsigned long>(theHSade)));
      }
    }
    else
      fDataOk = false;

    SetPrecipitationProbabilityLimits();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theWeather The other object being copied
 */
// ----------------------------------------------------------------------

NFmiWeatherAndCloudiness::NFmiWeatherAndCloudiness(const NFmiWeatherAndCloudiness &theWeather)

    = default;

NFmiWeatherAndCloudiness& NFmiWeatherAndCloudiness::operator = (const NFmiWeatherAndCloudiness&) = default;


// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiWeatherAndCloudiness::~NFmiWeatherAndCloudiness() = default;
// ----------------------------------------------------------------------
/*!
 * \param theCloudSymbol Undocumented
 */
// ----------------------------------------------------------------------

void NFmiWeatherAndCloudiness::CalcClouds(unsigned long theCloudSymbol)
{
  try
  {
    if (theCloudSymbol != kFloatMissing)
    {
      if (theCloudSymbol == 0)
      {
        SetHighClouds(0);
        SetLowClouds(0);
        SetMiddleClouds(0);
      }
      if (theCloudSymbol / 1000 == 1 && TotalCloudiness() == kT4BitMissing)
        SetTotalCloudiness(1);
      else if (theCloudSymbol / 1000 == 2 && TotalCloudiness() == kT4BitMissing)
        SetTotalCloudiness(5);
      else if (theCloudSymbol / 1000 == 3 && TotalCloudiness() == kT4BitMissing)
        SetTotalCloudiness(9);

      if ((theCloudSymbol / 100) % 10 == 5)
      {
        SetHighClouds(TotalCloudiness());
        SetLowClouds(0);
        SetMiddleClouds(0);
      }
      if ((theCloudSymbol / 100) % 10 == 4)
      {
        SetMiddleClouds(TotalCloudiness());
        SetLowClouds(0);
        SetHighClouds(HighCloudsMissingValue());
      }
      if ((theCloudSymbol / 100) % 10 == 3)
      {
        SetLowClouds(TotalCloudiness());
        SetMiddleClouds(kT4BitMissing);
        SetHighClouds(HighCloudsMissingValue());
      }
      if ((theCloudSymbol / 100) % 10 == 6)
      {
        SetHighClouds(TotalCloudiness());
        SetLowClouds(TotalCloudiness());
        SetMiddleClouds(TotalCloudiness());
      }
      // viimeisenkin luvun voisi käsitellä!
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theProducer Undocumented
 * \param theSecondaryProducerList Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent *NFmiWeatherAndCloudiness::CreateParam(
    const NFmiProducer &theProducer, NFmiVoidPtrList *theSecondaryProducerList) const
{
  try
  {
    NFmiParam param;
    NFmiParamBag subParamBag;

    param = NFmiParam(kFmiTotalCloudCover, "Total Cloud Cover", 0, 100, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param = NFmiParam(kFmiLowCloudCover, "Low Cloud Cover", 0, 100, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param = NFmiParam(kFmiMediumCloudCover, "Medium Cloud Cover", 0, 100, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param = NFmiParam(kFmiHighCloudCover, "High Cloud Cover", 0, 100, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param = NFmiParam(
        kFmiMiddleAndLowCloudCover, "Middle+Low Cloud Cover", 0, 100, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param = NFmiParam(kFmiPrecipitation1h, "Precipitation mm/h", 0, 98, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param =
        NFmiParam(kFmiPrecipitationForm, "Precipitation Form", 0, 4, 1, 0, "%.1f", kNearestPoint);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kNumberParam));
    param =
        NFmiParam(kFmiPrecipitationType, "Precipitation Type", 0, 1, 1, 0, "%.1f", kNearestPoint);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kNumberParam));
    param = NFmiParam(
        kFmiProbabilityThunderstorm, "Probability of Thunder", 0, 100, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param = NFmiParam(kFmiFogIntensity, "Density of Fog", 0, 2, 1, 0, "%.1f", kNearestPoint);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kNumberParam));
    param = NFmiParam(kFmiWeatherSymbol3, "Weather Symbol", 1, 63, 1, 0, "%.1f", kNearestPoint);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kNumberParam));
    param =
        NFmiParam(kFmiWeatherSymbol1, "Precipitation Symbol", 0, 100, 1, 0, "%.1f", kNearestPoint);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kNumberParam));

    param = NFmiParam(kFmiWeatherAndCloudiness,
                      "Weather",
                      kFloatMissing,
                      kFloatMissing,
                      1,
                      0,
                      "%.1f",
                      kByCombinedParam);
    auto *theDataIdent = new NFmiDataIdent(param,
                                           theProducer,
                                           kContinuousParam,
                                           true,
                                           true,
                                           true,
                                           true,
                                           true,
                                           &subParamBag,
                                           theSecondaryProducerList);
    return theDataIdent;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param missingValue Undocumented
 * \param value1 Undocumented
 * \param factor1 Undocumented
 * \param value2 Undocumented
 * \param factor2 Undocumented
 * \param value3 Undocumented
 * \param factor3 Undocumented
 * \param value4 Undocumented
 * \param factor4 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

int NFmiWeatherAndCloudiness::WeightedMean(int missingValue,
                                           int value1,
                                           float factor1,
                                           int value2,
                                           float factor2,
                                           int value3,
                                           float factor3,
                                           int value4,
                                           float factor4) const
{
  try
  {
    // HUOM faktoreiden summan ei tartte olla 1

    double sum = 0.;
    double facSum = 0.;
    bool any = false;

    if (value1 != missingValue)
    {
      any = true;
      sum += value1 * factor1;
      facSum += factor1;
    }
    if (value2 != missingValue)
    {
      any = true;
      sum += value2 * factor2;
      facSum += factor2;
    }
    if (value3 != missingValue && factor3 != kFloatMissing)
    {
      any = true;
      sum += value3 * factor3;
      facSum += factor3;
    }
    if (value4 != missingValue && factor4 != kFloatMissing)
    {
      any = true;
      sum += value4 * factor4;
      facSum += factor4;
    }

    if (!any)
      return missingValue;

    if (facSum > 0.)
      return static_cast<int>(sum / facSum + .5);

    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param missingValue Undocumented
 * \param considerMax Undocumented
 * \param value1 Undocumented
 * \param factor1 Undocumented
 * \param value2 Undocumented
 * \param factor2 Undocumented
 * \param value3 Undocumented
 * \param factor3 Undocumented
 * \param value4 Undocumented
 * \param factor4 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiWeatherAndCloudiness::PrecipitationWeightedMean(unsigned long missingValue,
                                                          bool considerMax,
                                                          unsigned long value1,
                                                          float factor1,
                                                          unsigned long value2,
                                                          float factor2,
                                                          unsigned long value3,
                                                          float factor3,
                                                          unsigned long value4,
                                                          float factor4) const
{
  try
  {
    // suora keskiarvo milleissä
    // tai kohti maksimia hilattu (considerMax asetettu) joka soveltuu aikaintegrointiin

    bool any = false;
    double sum = 0.;
    double facSum = 0.;
    double prec;
    double maxvalue = 0.;
    if (value1 != missingValue)
    {
      any = true;
      prec = PrecipitationScale(value1);
      maxvalue = FmiMax(maxvalue, prec);
      prec = prec * factor1;
      sum += prec;
      facSum += factor1;
    }
    if (value2 != missingValue)
    {
      any = true;
      prec = PrecipitationScale(value2);
      maxvalue = FmiMax(maxvalue, prec);
      prec = prec * factor2;
      sum += prec;
      facSum += factor2;
    }
    if (value3 != missingValue && factor3 != kFloatMissing)
    {
      any = true;
      prec = PrecipitationScale(value3);
      maxvalue = FmiMax(maxvalue, prec);
      prec = prec * factor3;
      sum += prec;
      facSum += factor3;
    }
    if (value4 != missingValue && factor4 != kFloatMissing)
    {
      any = true;
      prec = PrecipitationScale(value4);
      maxvalue = FmiMax(maxvalue, prec);
      prec = prec * factor4;
      sum += prec;
      facSum += factor4;
    }
    if (!any)
      return kFloatMissing;

    if (facSum > 0.)
    {
      if (considerMax)
        return static_cast<float>((maxvalue + (sum / facSum)) / 2.);

      return static_cast<float>(sum / facSum);
    }

    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param type Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsMemberParam(FmiParameterName type) const
{
  try
  {
    switch (type)
    {
      case kFmiTotalCloudCover:
      case kFmiLowCloudCover:
      case kFmiMediumCloudCover:
      case kFmiHighCloudCover:
      case kFmiPrecipitation1h:
      case kFmiPrecipitationForm:
      case kFmiPrecipitationType:
      case kFmiProbabilityThunderstorm:
      case kFmiFogIntensity:
      case kFmiMiddleAndLowCloudCover:
        return true;
      default:
        return false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param param Undocumented
 * \param isTimePeriod Undocumented
 * \param theWeatherAndCloudiness1 Undocumented
 * \param fac1 Undocumented
 * \param theWeatherAndCloudiness2 Undocumented
 * \param fac2 Undocumented
 * \param theWeatherAndCloudiness3 Undocumented
 * \param fac3 Undocumented
 * \param theWeatherAndCloudiness4 Undocumented
 * \param fac4 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiWeatherAndCloudiness::MeanFloatValue(
    FmiParameterName param,
    bool isTimePeriod,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness1,
    float fac1,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness2,
    float fac2,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness3,
    float fac3,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness4,
    float fac4) const
{
  try
  {
    NFmiWeatherAndCloudiness w(MeanWeather(isTimePeriod,
                                           theWeatherAndCloudiness1,
                                           fac1,
                                           theWeatherAndCloudiness2,
                                           fac2,
                                           theWeatherAndCloudiness3,
                                           fac3,
                                           theWeatherAndCloudiness4,
                                           fac4));

    return static_cast<float>(w.SubValue(param));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param isTimePeriod Undocumented
 * \param theWeatherAndCloudiness1 Undocumented
 * \param fac1 Undocumented
 * \param theWeatherAndCloudiness2 Undocumented
 * \param fac2 Undocumented
 * \param theWeatherAndCloudiness3 Undocumented
 * \param fac3 Undocumented
 * \param theWeatherAndCloudiness4 Undocumented
 * \param fac4 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiWeatherAndCloudiness NFmiWeatherAndCloudiness::MeanWeather(
    bool isTimePeriod,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness1,
    float fac1,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness2,
    float fac2,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness3,
    float fac3,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness4,
    float fac4) const
{
  try
  {
    NFmiWeatherAndCloudiness newWeatherAndCloudiness(itsInfoVersion);

    float factorSum = fac1 + fac2;  //+fac3+fac4;
    if (fac3 != kFloatMissing)
      factorSum += fac3;
    if (fac4 != kFloatMissing)
      factorSum += fac4;

    float factor1 = fac1 / factorSum;
    float factor2 = fac2 / factorSum;
    float factor3, factor4;
    if (fac3 == kFloatMissing)
      factor3 = kFloatMissing;
    else
      factor3 = fac3 / factorSum;
    if (fac4 == kFloatMissing)
      factor4 = kFloatMissing;
    else
      factor4 = fac4 / factorSum;

    newWeatherAndCloudiness.SetLowClouds(WeightedMean(kT4BitMissing,
                                                      theWeatherAndCloudiness1.LowClouds(),
                                                      factor1,
                                                      theWeatherAndCloudiness2.LowClouds(),
                                                      factor2,
                                                      theWeatherAndCloudiness3.LowClouds(),
                                                      factor3,
                                                      theWeatherAndCloudiness4.LowClouds(),
                                                      factor4));

    newWeatherAndCloudiness.SetMiddleClouds(WeightedMean(kT4BitMissing,
                                                         theWeatherAndCloudiness1.MiddleClouds(),
                                                         factor1,
                                                         theWeatherAndCloudiness2.MiddleClouds(),
                                                         factor2,
                                                         theWeatherAndCloudiness3.MiddleClouds(),
                                                         factor3,
                                                         theWeatherAndCloudiness4.MiddleClouds(),
                                                         factor4));

    newWeatherAndCloudiness.SetHighClouds(WeightedMean(HighCloudsMissingValue(),
                                                       theWeatherAndCloudiness1.HighClouds(),
                                                       factor1,
                                                       theWeatherAndCloudiness2.HighClouds(),
                                                       factor2,
                                                       theWeatherAndCloudiness3.HighClouds(),
                                                       factor3,
                                                       theWeatherAndCloudiness4.HighClouds(),
                                                       factor4));

    newWeatherAndCloudiness.SetTotalCloudiness(
        WeightedMean(kT4BitMissing,
                     theWeatherAndCloudiness1.TotalCloudiness(),
                     factor1,
                     theWeatherAndCloudiness2.TotalCloudiness(),
                     factor2,
                     theWeatherAndCloudiness3.TotalCloudiness(),
                     factor3,
                     theWeatherAndCloudiness4.TotalCloudiness(),
                     factor4));

    newWeatherAndCloudiness.SubValue(
        PrecipitationWeightedMean(TotalPrecipitationMissingValue(),
                                  isTimePeriod,
                                  theWeatherAndCloudiness1.TotalPrecipitation(),
                                  factor1,
                                  theWeatherAndCloudiness2.TotalPrecipitation(),
                                  factor2,
                                  theWeatherAndCloudiness3.TotalPrecipitation(),
                                  factor3,
                                  theWeatherAndCloudiness4.TotalPrecipitation(),
                                  factor4),
        kFmiPrecipitation1h);

    newWeatherAndCloudiness.SetPrecipitationType(
        WeightedMean(kT2BitMissing,
                     theWeatherAndCloudiness1.PrecipitationType(),
                     factor1,
                     theWeatherAndCloudiness2.PrecipitationType(),
                     factor2,
                     theWeatherAndCloudiness3.PrecipitationType(),
                     factor3,
                     theWeatherAndCloudiness4.PrecipitationType(),
                     factor4));

    newWeatherAndCloudiness.SetPrecipitationForm(
        PrecipitationFormWeightedMean(theWeatherAndCloudiness1.PrecipitationForm(),
                                      theWeatherAndCloudiness1.TotalPrecipitation(),
                                      factor1,
                                      theWeatherAndCloudiness2.PrecipitationForm(),
                                      theWeatherAndCloudiness2.TotalPrecipitation(),
                                      factor2,
                                      theWeatherAndCloudiness3.PrecipitationForm(),
                                      theWeatherAndCloudiness3.TotalPrecipitation(),
                                      factor3,
                                      theWeatherAndCloudiness4.PrecipitationForm(),
                                      theWeatherAndCloudiness4.TotalPrecipitation(),
                                      factor4));

    newWeatherAndCloudiness.SetThunderProbability(
        WeightedMean(kTThunderProbabilityMissing,
                     theWeatherAndCloudiness1.ThunderProbability(),
                     factor1,
                     theWeatherAndCloudiness2.ThunderProbability(),
                     factor2,
                     theWeatherAndCloudiness3.ThunderProbability(),
                     factor3,
                     theWeatherAndCloudiness4.ThunderProbability(),
                     factor4));

    newWeatherAndCloudiness.SetFogIntensity(WeightedMean(kT2BitMissing,
                                                         theWeatherAndCloudiness1.FogIntensity(),
                                                         factor1,
                                                         theWeatherAndCloudiness2.FogIntensity(),
                                                         factor2,
                                                         theWeatherAndCloudiness3.FogIntensity(),
                                                         factor3,
                                                         theWeatherAndCloudiness4.FogIntensity(),
                                                         factor4));

    return newWeatherAndCloudiness;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theCombinedParam1 Undocumented
 * \param fac1 Undocumented
 * \param theCombinedParam2 Undocumented
 * \param fac2 Undocumented
 * \param theCombinedParam3 Undocumented
 * \param fac3 Undocumented
 * \param theCombinedParam4 Undocumented
 * \param fac4 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::SetToWeightedMean(NFmiCombinedParam *theCombinedParam1,
                                                 float fac1,
                                                 NFmiCombinedParam *theCombinedParam2,
                                                 float fac2,
                                                 NFmiCombinedParam *theCombinedParam3,
                                                 float fac3,
                                                 NFmiCombinedParam *theCombinedParam4,
                                                 float fac4)
{
  try
  {
    // suora keskiarvo paitsi sateen olomuoto pitäisi saada erikoiskäsittelyn
    // käytetään qinfosta painotettujen keskiarvojen laskemiseen
    // (latlon:n ympäröivistä 4:stä pisteestä)

    auto *theWeatherAndCloudiness1 = static_cast<NFmiWeatherAndCloudiness *>(theCombinedParam1);
    auto *theWeatherAndCloudiness2 = static_cast<NFmiWeatherAndCloudiness *>(theCombinedParam2);
    auto *theWeatherAndCloudiness3 = static_cast<NFmiWeatherAndCloudiness *>(theCombinedParam3);
    auto *theWeatherAndCloudiness4 = static_cast<NFmiWeatherAndCloudiness *>(theCombinedParam4);

    float factorSum = fac1 + fac2;
    if (fac3 != kFloatMissing)
      factorSum += fac3;
    if (fac4 != kFloatMissing)
      factorSum += fac4;

    float factor1 = fac1 / factorSum;
    float factor2 = fac2 / factorSum;
    float factor3, factor4;
    if (fac3 == kFloatMissing)
      factor3 = kFloatMissing;
    else
      factor3 = fac3 / factorSum;
    if (fac4 == kFloatMissing)
      factor4 = kFloatMissing;
    else
      factor4 = fac4 / factorSum;

    SetLowClouds(WeightedMean(kT4BitMissing,
                              theWeatherAndCloudiness1->LowClouds(),
                              factor1,
                              theWeatherAndCloudiness2->LowClouds(),
                              factor2,
                              theWeatherAndCloudiness3->LowClouds(),
                              factor3,
                              theWeatherAndCloudiness4->LowClouds(),
                              factor4));
    SetMiddleClouds(WeightedMean(kT4BitMissing,
                                 theWeatherAndCloudiness1->MiddleClouds(),
                                 factor1,
                                 theWeatherAndCloudiness2->MiddleClouds(),
                                 factor2,
                                 theWeatherAndCloudiness3->MiddleClouds(),
                                 factor3,
                                 theWeatherAndCloudiness4->MiddleClouds(),
                                 factor4));

    SetHighClouds(WeightedMean(HighCloudsMissingValue(),
                               theWeatherAndCloudiness1->HighClouds(),
                               factor1,
                               theWeatherAndCloudiness2->HighClouds(),
                               factor2,
                               theWeatherAndCloudiness3->HighClouds(),
                               factor3,
                               theWeatherAndCloudiness4->HighClouds(),
                               factor4));

    // pitääkö tämä ollenkaan laskea
    SetTotalCloudiness(WeightedMean(kT4BitMissing,
                                    theWeatherAndCloudiness1->TotalCloudiness(),
                                    factor1,
                                    theWeatherAndCloudiness2->TotalCloudiness(),
                                    factor2,
                                    theWeatherAndCloudiness3->TotalCloudiness(),
                                    factor3,
                                    theWeatherAndCloudiness4->TotalCloudiness(),
                                    factor4));

    SetPrecipitationType(WeightedMean(kT2BitMissing,
                                      theWeatherAndCloudiness1->PrecipitationType(),
                                      factor1,
                                      theWeatherAndCloudiness2->PrecipitationType(),
                                      factor2,
                                      theWeatherAndCloudiness3->PrecipitationType(),
                                      factor3,
                                      theWeatherAndCloudiness4->PrecipitationType(),
                                      factor4));
    SetPrecipitationForm(
        PrecipitationFormWeightedMean(theWeatherAndCloudiness1->PrecipitationForm(),
                                      theWeatherAndCloudiness1->TotalPrecipitation(),
                                      factor1,
                                      theWeatherAndCloudiness2->PrecipitationForm(),
                                      theWeatherAndCloudiness2->TotalPrecipitation(),
                                      factor2,
                                      theWeatherAndCloudiness3->PrecipitationForm(),
                                      theWeatherAndCloudiness3->TotalPrecipitation(),
                                      factor3,
                                      theWeatherAndCloudiness4->PrecipitationForm(),
                                      theWeatherAndCloudiness4->TotalPrecipitation(),
                                      factor4));

    SubValue(PrecipitationWeightedMean(TotalPrecipitationMissingValue(),
                                       false,  // sade ei kohti maksimia
                                       theWeatherAndCloudiness1->TotalPrecipitation(),
                                       factor1,
                                       theWeatherAndCloudiness2->TotalPrecipitation(),
                                       factor2,
                                       theWeatherAndCloudiness3->TotalPrecipitation(),
                                       factor3,
                                       theWeatherAndCloudiness4->TotalPrecipitation(),
                                       factor4),
             kFmiPrecipitation1h);

    SetThunderProbability(WeightedMean(kTThunderProbabilityMissing,
                                       theWeatherAndCloudiness1->ThunderProbability(),
                                       factor1,
                                       theWeatherAndCloudiness2->ThunderProbability(),
                                       factor2,
                                       theWeatherAndCloudiness3->ThunderProbability(),
                                       factor3,
                                       theWeatherAndCloudiness4->ThunderProbability(),
                                       factor4));

    SetFogIntensity(WeightedMean(kT2BitMissing,
                                 theWeatherAndCloudiness1->FogIntensity(),
                                 factor1,
                                 theWeatherAndCloudiness2->FogIntensity(),
                                 factor2,
                                 theWeatherAndCloudiness3->FogIntensity(),
                                 factor3,
                                 theWeatherAndCloudiness4->FogIntensity(),
                                 factor4));

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param form1 Undocumented
 * \param amount1 Undocumented
 * \param factor1 Undocumented
 * \param form2 Undocumented
 * \param amount2 Undocumented
 * \param factor2 Undocumented
 * \param form3 Undocumented
 * \param amount3 Undocumented
 * \param factor3 Undocumented
 * \param form4 Undocumented
 * \param amount4 Undocumented
 * \param factor4 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::PrecipitationFormWeightedMean(unsigned long form1,
                                                                      unsigned long amount1,
                                                                      float factor1,
                                                                      unsigned long form2,
                                                                      unsigned long amount2,
                                                                      float factor2,
                                                                      unsigned long form3,
                                                                      unsigned long amount3,
                                                                      float factor3,
                                                                      unsigned long form4,
                                                                      unsigned long amount4,
                                                                      float factor4) const
{
  try
  {
    // painotus sademäärän mukaan

    double sums[FmiNumberOfPrecipitationForms];
    for (double &sum : sums)
      sum = 0;

    if (form1 != kT3BitMissing && amount1 != TotalPrecipitationMissingValue())
    {
      sums[form1] += PrecipitationScale(amount1) * factor1;
    }
    if (form2 != kT3BitMissing && amount2 != TotalPrecipitationMissingValue())
    {
      sums[form2] += PrecipitationScale(amount2) * factor2;
    }
    if (factor3 != kFloatMissing && form3 != kT3BitMissing &&
        amount3 != TotalPrecipitationMissingValue())
    {
      sums[form3] += PrecipitationScale(amount3) * factor3;
    }
    if (factor4 != kFloatMissing && form4 != kT3BitMissing &&
        amount4 != TotalPrecipitationMissingValue())
    {
      sums[form4] += PrecipitationScale(amount4) * factor4;
    }
    return PrecipitationFormFromWeightTable(sums);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param formTable Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::PrecipitationFormFromWeightTable(double *formTable) const
{
  try
  {
    double sum = 0;
    for (unsigned int i = 0; i < FmiNumberOfPrecipitationForms; i++)
      sum += formTable[i];

    // toimiiko vaikka näin
    if (sum <= 0.)
      return kT3BitMissing;
    else if ((formTable[5] + formTable[4]) / sum > .3)
    {
      if (formTable[5] >= formTable[4])
        return kTFreezingRain;
      else
        return kTFreezingDrizzle;
    }
    else if (formTable[6] / sum > .3)
      return kTHail;
    else if ((formTable[0] + formTable[4]) / sum > .7)  // 5.11.99 +jäätavä //25.5.00 oli .8
      return kTDrizzle;
    else
    {  // 6.2.03 tihkut pois jotta tihku+lumi ei ->räntää
      double rainSum = formTable[1] + .5 * formTable[2] + formTable[5];
      double snowSum = formTable[3] + .5 * formTable[2];  // ei rakeita tänne
      double rainSnowRelation;
      if (snowSum > 0.)
        rainSnowRelation = rainSum / snowSum;
      else
        rainSnowRelation = 100.;  // oikeastaan ääretön

      if (rainSnowRelation > 9.)  // rajat oli 20/80, nyt 10/90%
        return kTRain;
      else if (rainSnowRelation < 1. / 9.)
        return kTSnow;
      else
        return kTSleet;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theWeatherAndCloudiness1 Undocumented
 * \param fac1 Undocumented
 * \param theWeatherAndCloudiness2 Undocumented
 * \param fac2 Undocumented
 * \param theWeatherAndCloudiness3 Undocumented
 * \param fac3 Undocumented
 * \param theWeatherAndCloudiness4 Undocumented
 * \param fac4 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::TimeIntegratedHessaa(
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness1,
    float fac1,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness2,
    float fac2,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness3,
    float fac3,
    const NFmiWeatherAndCloudiness &theWeatherAndCloudiness4,
    float fac4)
{
  try
  {
    // EI PÄIVITETTY
    NFmiWeatherAndCloudiness newWeatherAndCloudiness(itsInfoVersion);

    float factorSum = fac1 + fac2;  //+fac3;
    if (fac3 != kFloatMissing)
      factorSum += fac3;
    if (fac4 != kFloatMissing)
      factorSum += fac4;
    float factor3, factor4;

    float factor1 = fac1 / factorSum;
    float factor2 = fac2 / factorSum;
    if (fac3 != kFloatMissing)
      factor3 = fac3 / factorSum;
    else
      factor3 = kFloatMissing;
    if (fac4 != kFloatMissing)
      factor4 = fac4 / factorSum;
    else
      factor4 = kFloatMissing;

    newWeatherAndCloudiness.SetLowClouds(WeightedMean(kT4BitMissing,
                                                      theWeatherAndCloudiness1.LowClouds(),
                                                      factor1,
                                                      theWeatherAndCloudiness2.LowClouds(),
                                                      factor2,
                                                      theWeatherAndCloudiness3.LowClouds(),
                                                      factor3,
                                                      theWeatherAndCloudiness4.LowClouds(),
                                                      factor4));

    newWeatherAndCloudiness.SetMiddleClouds(WeightedMean(kT4BitMissing,
                                                         theWeatherAndCloudiness1.MiddleClouds(),
                                                         factor1,
                                                         theWeatherAndCloudiness2.MiddleClouds(),
                                                         factor2,
                                                         theWeatherAndCloudiness3.MiddleClouds(),
                                                         factor3,
                                                         theWeatherAndCloudiness4.MiddleClouds(),
                                                         factor4));

    newWeatherAndCloudiness.SetHighClouds(WeightedMean(HighCloudsMissingValue(),
                                                       theWeatherAndCloudiness1.HighClouds(),
                                                       factor1,
                                                       theWeatherAndCloudiness2.HighClouds(),
                                                       factor2,
                                                       theWeatherAndCloudiness3.HighClouds(),
                                                       factor3,
                                                       theWeatherAndCloudiness4.HighClouds(),
                                                       factor4));

    newWeatherAndCloudiness.SetTotalCloudiness(
        WeightedMean(kT4BitMissing,
                     theWeatherAndCloudiness1.TotalCloudiness(),
                     factor1,
                     theWeatherAndCloudiness2.TotalCloudiness(),
                     factor2,
                     theWeatherAndCloudiness3.TotalCloudiness(),
                     factor3,
                     theWeatherAndCloudiness4.TotalCloudiness(),
                     factor4));

    newWeatherAndCloudiness.SetTotalPrecipitation(
        PrecipitationScale(PrecipitationWeightedMean(TotalPrecipitationMissingValue(),
                                                     true,
                                                     theWeatherAndCloudiness1.TotalPrecipitation(),
                                                     factor1,
                                                     theWeatherAndCloudiness2.TotalPrecipitation(),
                                                     factor2,
                                                     theWeatherAndCloudiness3.TotalPrecipitation(),
                                                     factor3,
                                                     theWeatherAndCloudiness4.TotalPrecipitation(),
                                                     factor4)));

    newWeatherAndCloudiness.SetPrecipitationType(
        WeightedMean(kT2BitMissing,
                     theWeatherAndCloudiness1.PrecipitationType(),
                     factor1,
                     theWeatherAndCloudiness2.PrecipitationType(),
                     factor2,
                     theWeatherAndCloudiness3.PrecipitationType(),
                     factor3,
                     theWeatherAndCloudiness4.PrecipitationType(),
                     factor4));

    bool snow = false;
    bool rain = false;
    if (!theWeatherAndCloudiness1.IsFair())
    {
      if (theWeatherAndCloudiness1.PrecipitationForm() == kTSleet)
      {
        snow = true;
        rain = true;
      }
      else if (theWeatherAndCloudiness1.PrecipitationForm() == kTSnow)
        snow = true;
      else
        rain = true;
    }
    if (!theWeatherAndCloudiness2.IsFair())
    {
      if (theWeatherAndCloudiness2.PrecipitationForm() == kTSleet)
      {
        snow = true;
        rain = true;
      }
      else if (theWeatherAndCloudiness2.PrecipitationForm() == kTSnow)
        snow = true;
      else
        rain = true;
    }
    if (!theWeatherAndCloudiness3.IsFair())
    {
      if (theWeatherAndCloudiness3.PrecipitationForm() == kTSleet)
      {
        snow = true;
        rain = true;
      }
      else if (theWeatherAndCloudiness3.PrecipitationForm() == kTSnow)
        snow = true;
      else
        rain = true;
    }
    if (snow && rain)
      newWeatherAndCloudiness.SetPrecipitationForm(kTSleet);
    else if (snow)
      newWeatherAndCloudiness.SetPrecipitationForm(kTSnow);
    else if (rain)
      newWeatherAndCloudiness.SetPrecipitationForm(kTRain);
    newWeatherAndCloudiness.SetThunderProbability(
        WeightedMean(kTThunderProbabilityMissing,
                     theWeatherAndCloudiness1.ThunderProbability(),
                     factor1,
                     theWeatherAndCloudiness2.ThunderProbability(),
                     factor2,
                     theWeatherAndCloudiness3.ThunderProbability(),
                     factor3,
                     theWeatherAndCloudiness4.ThunderProbability(),
                     factor4));

    newWeatherAndCloudiness.SetFogIntensity(WeightedMean(kT2BitMissing,
                                                         theWeatherAndCloudiness1.FogIntensity(),
                                                         factor1,
                                                         theWeatherAndCloudiness2.FogIntensity(),
                                                         factor2,
                                                         theWeatherAndCloudiness3.FogIntensity(),
                                                         factor3,
                                                         theWeatherAndCloudiness4.FogIntensity(),
                                                         factor4));

    return static_cast<unsigned long>(newWeatherAndCloudiness.ToHessaa());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSynopWeatherCode Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcSynopThunderProb(
    unsigned long theSynopWeatherCode) const
{
  try
  {
    switch (theSynopWeatherCode)
    {
      case 17:
      case 27:
      case 29:
      case 91:
      case 92:
      case 93:
      case 94:
      case 90:
        return 2;
      case 95:
      case 96:
        return 6;
      case 97:
      case 98:
      case 99:
        return 9;
      case 18:
      case 19:
      case 89:
        return 1;
      default:
        return 0;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSynopWeatherCode Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcSynopFogIntensity(
    unsigned long theSynopWeatherCode) const
{
  try
  {
    switch (theSynopWeatherCode)
    {
      case 12:
      case 42:
      case 43:
      case 44:
      case 45:
      case 46:
      case 47:
      case 48:
      case 49:
        return 2;  // dense
      case 11:
      case 28:
      case 40:
      case 41:
        return 1;  // moderate
      default:
        return 0;  // no
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFogSymbol Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcFogIntensity(unsigned long theFogSymbol) const
{
  try
  {
    switch (theFogSymbol)
    {
      case 607:
        return 1;  // moderate
      default:
        return 0;  // no
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSynopWeatherCode Undocumented
 * \param theTotalCloudiness Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcSynopPrecipitationType(
    unsigned long theSynopWeatherCode, unsigned long theTotalCloudiness) const
{
  try
  {
    if (theTotalCloudiness < 3)  // Taivas on SELKIINTYNYT
      return kT2BitMissing;

    switch (theSynopWeatherCode)
    {
      case 20:
      case 21:
      case 22:
      case 23:
      case 24:
      case 50:
      case 51:
      case 52:
      case 53:
      case 54:
      case 55:
      case 56:
      case 57:
      case 58:
      case 59:
      case 60:
      case 61:
      case 62:
      case 63:
      case 64:
      case 65:
      case 66:
      case 67:
      case 68:
      case 69:
      case 70:
      case 71:
      case 72:
      case 73:
      case 74:
      case 75:
      case 76:
      case 77:
      case 78:
      case 79:
        return kTLargeScalePrecipitation;  // LargeScale
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 29:
      case 25:
      case 26:
      case 27:
      case 80:
      case 81:
      case 82:
      case 83:
      case 84:
      case 85:
      case 86:
      case 87:
      case 88:
      case 89:
      case 90:
      case 91:
      case 92:
      case 93:
      case 94:
      case 95:
      case 96:
      case 97:
      case 98:
      case 99:
        return kTConvectivePrecipitation;  // Convective
      default:
        return kT2BitMissing;  // no
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSynopWeatherCode Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcPrecipitationType(
    unsigned long theSynopWeatherCode) const
{
  try
  {
    switch (theSynopWeatherCode)
    {
      case 20:
      case 21:
      case 22:
      case 23:
      case 24:
      case 50:
      case 51:
      case 52:
      case 53:
      case 54:
      case 55:
      case 56:
      case 57:
      case 58:
      case 59:
      case 60:
      case 61:
      case 62:
      case 63:
      case 64:
      case 65:
      case 66:
      case 67:
      case 68:
      case 69:
      case 70:
      case 71:
      case 72:
      case 73:
      case 74:
      case 75:
      case 76:
      case 77:
      case 78:
      case 79:
        return kTLargeScalePrecipitation;  // LargeScale
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 29:
      case 25:
      case 26:
      case 27:
      case 80:
      case 81:
      case 82:
      case 83:
      case 84:
      case 85:
      case 86:
      case 87:
      case 88:
      case 89:
      case 90:
      case 93:
      case 94:
      case 96:
      case 91:
      case 92:
      case 95:
      case 97:
      case 98:
      case 99:
        return kTConvectivePrecipitation;  // Convective
      default:
        return kT2BitMissing;  // no
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSynopWeatherCode Undocumented
 * \param theTotalCloudiness Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcSynopPrecipitationForm(
    unsigned long theSynopWeatherCode, unsigned long theTotalCloudiness) const
{
  try
  {
    if (theTotalCloudiness < 3)  // Taivas on SELKIINTYNYT
      return kT3BitMissing;

    switch (theSynopWeatherCode)
    {
      case 27:
      case 87:
      case 88:
      case 89:
      case 90:
      case 93:  // rakeita???
      case 94:
      case 96:
      case 99:
        return kTHail;
      case 24:
      case 66:
      case 67:
        return kTFreezingRain;  // Tarkista Kaisalta tämän ja seuraavan suhteeet
      case 56:
      case 57:
        return kTFreezingDrizzle;  // Tarkista Kaisalta (vrt. ed.)
      case 22:
      case 26:
      case 70:
      case 71:
      case 72:
      case 73:
      case 74:
      case 75:
      case 76:
      case 77:
      case 78:
      case 79:
      case 85:
      case 86:
        return kTSnow;
      case 23:
      case 68:
      case 69:
      case 83:
      case 84:
        return kTSleet;
      case 25:
      case 21:
      case 60:
      case 61:
      case 62:
      case 63:
      case 64:
      case 65:
      case 80:
      case 81:
      case 82:
      case 91:
      case 92:
      case 95:
      case 97:
        return kTRain;
      // 	case 20:     //Sensuroitu=tihkusadetta (ei jäätävää) tai lumijyväsiä
      case 50:
      case 51:
      case 52:
      case 53:
      case 54:
      case 55:
      case 58:
      case 59:
        return kTDrizzle;
      default:
        return kT3BitMissing;  // no tai 98
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSynopWeatherCode Undocumented
 * \param theTotalCloudiness Undocumented
 * \param theHumidity Undocumented
 * \param theTemperature Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcSynopPrecipitationForm(
    unsigned long theSynopWeatherCode,
    unsigned long theTotalCloudiness,
    float theHumidity,
    float theTemperature) const
{
  try
  {
    if (theTotalCloudiness < 3)  // Taivas on SELKIINTYNYT
      return kT3BitMissing;

    switch (theSynopWeatherCode)
    {
      case 27:
      case 87:
      case 88:
      case 89:
      case 90:
      case 93:  // Näitä koodeja 93 ja 94 ei mielestämme voi määritellä rakeiksi Mikael J. ja Mikael
                // F. 12.10.2000
      case 94:  // Mutta Kukkonen määritteli ne rakeiksi
      case 96:
      case 99:
        return kTHail;
      case 24:
      case 66:
      case 67:
        return kTFreezingRain;
      case 56:
      case 57:
        return kTFreezingDrizzle;
      case 20:
      {
        if ((theTemperature == kFloatMissing) || (theHumidity == kFloatMissing))
          return kT3BitMissing;

        else
        {
          long double y = -154.26 + 9.5583 * theHumidity - 0.19816 * pow(theHumidity, 2) +
                          0.0017163 * pow(theHumidity, 3) - 0.0000053613 * pow(theHumidity, 4);

          if (theTemperature >= y)
            return kTDrizzle;
          else
            return kTSnow;
        }
      }
      case 26:
      {
        if ((theTemperature == kFloatMissing) || (theHumidity == kFloatMissing))
          return kT3BitMissing;

        else
        {
          long double y = +59.020 - 2.4723 * theHumidity + 0.043376 * pow(theHumidity, 2) -
                          0.00035229 * pow(theHumidity, 3) + 0.0000010723 * pow(theHumidity, 4);

          if (theTemperature >= y)
            return kTSleet;
          else
            return kTSnow;
        }
      }
      case 22:
      case 70:
      case 71:
      case 72:
      case 73:
      case 74:
      case 75:
      case 76:
      case 77:
      case 78:
      case 79:
      case 85:
      case 86:
        return kTSnow;
      case 23:
      case 68:
      case 69:
      case 83:
      case 84:
        return kTSleet;
      case 21:
      case 25:
      case 60:
      case 61:
      case 62:
      case 63:
      case 64:
      case 65:
      case 80:
      case 81:
      case 82:
      case 91:
      case 92:
        return kTRain;
      case 95:
      case 97:
      {
        if ((theTemperature == kFloatMissing) || (theHumidity == kFloatMissing))
          return kT3BitMissing;

        else if ((theTemperature >= 3.6 /*&& theHumidity>=39*/ && theHumidity <= 40) ||
                 (theTemperature >= 3.5 && theHumidity >= 41 && theHumidity <= 42) ||
                 (theTemperature >= 3.4 && theHumidity >= 43 && theHumidity <= 44) ||
                 (theTemperature >= 3.3 && theHumidity == 45) ||
                 (theTemperature >= 3.2 && theHumidity == 46) ||
                 (theTemperature >= 3.1 && theHumidity == 47) ||
                 (theTemperature >= 3.0 && theHumidity == 48) ||
                 (theTemperature >= 2.8 && theHumidity == 49) ||
                 (theTemperature >= 2.6 && theHumidity == 50) ||
                 (theTemperature >= 0.6 && theHumidity >= 51 && theHumidity <= 60) ||
                 (theTemperature >= 0.7 && theHumidity >= 61 && theHumidity <= 74) ||
                 (theTemperature >= 0.8 && theHumidity >= 75 && theHumidity <= 78) ||
                 (theTemperature >= 0.9 && theHumidity >= 79 && theHumidity <= 82) ||
                 (theTemperature >= 1 && theHumidity >= 83 && theHumidity <= 90))
          return kTRain;

        else if ((theTemperature <= 3.5 /*&& theHumidity>=39*/ && theHumidity <= 40) ||
                 (theTemperature <= 3.4 && theHumidity >= 41 && theHumidity <= 42) ||
                 (theTemperature <= 3.3 && theHumidity >= 43 && theHumidity <= 44) ||
                 (theTemperature <= 3.2 && theHumidity == 45) ||
                 (theTemperature <= 3.1 && theHumidity == 46) ||
                 (theTemperature <= 3.0 && theHumidity == 47) ||
                 (theTemperature <= 2.9 && theHumidity == 48) ||
                 (theTemperature <= 2.7 && theHumidity == 49) ||
                 (theTemperature <= 2.5 && theHumidity == 50) ||
                 (theTemperature <= 0.5 && theHumidity >= 51 && theHumidity <= 60) ||
                 (theTemperature <= 0.6 && theHumidity >= 61 && theHumidity <= 72) ||
                 (theTemperature <= 0.3 && theHumidity >= 73 && theHumidity <= 74) ||
                 (theTemperature <= 0.2 && theHumidity >= 75 && theHumidity <= 78) ||
                 (theTemperature <= 0.1 && theHumidity == 79) ||
                 (theTemperature <= 0 && theHumidity == 80) ||
                 (theTemperature <= -0.3 && theHumidity == 81) ||
                 (theTemperature <= -0.5 && theHumidity == 82) ||
                 (theTemperature <= -0.6 && theHumidity == 83) ||
                 (theTemperature <= -0.7 && theHumidity >= 84 && theHumidity <= 85) ||
                 (theTemperature <= -0.8 && theHumidity >= 86 && theHumidity <= 87) ||
                 (theTemperature <= -0.9 && theHumidity >= 88 && theHumidity <= 90) ||
                 (theTemperature <= -1.6 && theHumidity == 91) ||
                 (theTemperature <= -1.8 && theHumidity == 92) ||
                 (theTemperature <= -1.9 && theHumidity == 93) ||
                 (theTemperature <= -2 && theHumidity >= 94 && theHumidity <= 95) ||
                 (theTemperature <= -2.1 && theHumidity >= 96 && theHumidity <= 97) ||
                 (theTemperature <= -2.2 && theHumidity >= 98 /*&& theHumidity<=100*/))
          return kTSnow;

        else
          return kTSleet;
      }
      // 	case 20:     //Sensuroitu=tihkusadetta (ei jäätävää) tai lumijyväsiä
      case 50:
      case 51:
      case 52:
      case 53:
      case 54:
      case 55:
      case 58:
      case 59:
        return kTDrizzle;
      default:
        return kT3BitMissing;  // no tai 98
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSynopWeatherCode Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcPrecipitationForm(
    unsigned long theSynopWeatherCode) const
{
  try
  {
    switch (theSynopWeatherCode)
    {
      case 27:
      case 87:
      case 88:
      case 89:
      case 90:
      case 93:  // rakeita??? Laura 18061999
      case 94:
      case 96:
      case 99:
        return kTHail;
      case 24:
      case 66:
      case 67:
        return kTFreezingRain;  // Tarkista Kaisalta tämän ja seuraavan suhteeet
      case 56:
      case 57:
        return kTFreezingDrizzle;  // Tarkista Kaisalta (vrt. ed.)
      case 22:
      case 26:
      case 70:
      case 71:
      case 72:
      case 73:
      case 74:
      case 75:
      case 76:
      case 77:
      case 78:
      case 79:
      case 85:
      case 86:
        return kTSnow;
      case 23:
      case 68:
      case 69:
      case 83:
      case 84:
        return kTSleet;
      case 21:
      case 25:  // Mikael J. ja Mikael F. lisäsivät vesikuuron
      case 60:
      case 61:
      case 62:
      case 63:
      case 64:
      case 65:
      case 80:
      case 81:
      case 82:
      case 91:
      case 92:
      case 95:
      case 97:
        return kTRain;

      // 	case 20:     //Sensuroitu=tihkusadetta (ei jäätävää) tai lumijyväsiä
      case 50:
      case 51:
      case 52:
      case 53:
      case 54:
      case 55:
      case 58:
      case 59:
        return kTDrizzle;
      default:
        return kT3BitMissing;  // no tai 98
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSynopWeatherCode Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcSynopPrecipitationAmount(
    unsigned theSynopWeatherCode) const
{
  try
  {
    switch (theSynopWeatherCode)
    {
      case 27:
      case 64:
      case 65:
      case 69:
      case 74:
      case 75:
      case 82:
      case 84:
      case 86:
      case 88:
      case 90:
      case 94:
      case 97:
      case 99:
        return kTMuchPrecipitation;
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 54:
      case 55:
      case 57:
      case 59:
      case 62:
      case 63:
      case 67:
      case 72:
      case 73:
      case 77:
      case 79:
      case 81:
      case 87:
      case 89:
      case 92:
      case 93:
      case 95:
      case 96:
        return kTModeratePrecipitation;
      case 50:
      case 51:
      case 52:
      case 53:
      case 56:
      case 58:
      case 60:
      case 61:
      case 66:
      case 68:
      case 70:
      case 71:
      case 76:
      case 78:
      case 80:
      case 83:
      case 85:
      case 91:
        return kTSomePrecipitation;
      default:
        return kTNoPrecipitation;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param info Undocumented
 * \param theLonLat Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 * \param considerMax Undocumented
 * \param factor1 Undocumented
 * \param factor2 Undocumented
 * \param factor3 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::SetToWeightedPeriod(NFmiQueryInfo *info,
                                                   const NFmiPoint &theLonLat,
                                                   const NFmiMetTime &theStartTime,
                                                   const NFmiMetTime &theEndTime,
                                                   bool considerMax,
                                                   float factor1,
                                                   float factor2,
                                                   float factor3)
{
  try
  {
    bool locationMissing = theLonLat.X() == 0. && theLonLat.Y() == 0.;

    NFmiQueryInfo *newInfo = info->Clone();
    NFmiMetTime middleTime(theStartTime);
    double halfPeriod = theEndTime.DifferenceInHours(theStartTime) / 2.;
    middleTime.ChangeByHours(static_cast<long>(halfPeriod));

    double totalCloudinessSum = 0.;
    double lowCloudsSum = 0.;
    double middleCloudsSum = 0.;
    double highCloudsSum = 0.;
    double totalPrecipitationSum = 0.;
    double precipitationTypeSum = 0.;
    double thunderProbabilitySum = 0.;
    double fogIntensitySum = 0.;
    double precipitationForms[FmiNumberOfPrecipitationForms];
    for (double &precipitationForm : precipitationForms)
      precipitationForm = 0;

    double totalCloudinessFactorSum = 0;
    double lowCloudsFactorSum = 0;
    double middleCloudsFactorSum = 0;
    double highCloudsFactorSum = 0;
    double totalPrecipitationFactorSum = 0;
    double precipitationTypeFactorSum = 0;
    double thunderProbabilityFactorSum = 0;
    double fogIntensityFactorSum = 0;
    double maxPrecipitation = 0.;
    double maxThunder = 0.;
    double factor;

    if (!newInfo->TimeToNearestStep(theStartTime))
    {
      delete newInfo;
      return false;
    }

    newInfo->Param(kFmiWeatherSymbol3);
    while (newInfo->Time() <= theEndTime)
    {
      {
        // jos lonlat annettu  pitää ottaa paikan mukaan interpoloitu
        if (!locationMissing)
          newInfo->InterpolatedValue(theLonLat);

        const auto *weather =
            static_cast<const NFmiWeatherAndCloudiness *>(newInfo->CombinedParam());
        if (weather)
        {
          if (newInfo->Time() < info->Time())
          {
            factor = (static_cast<float>(newInfo->Time().DifferenceInHours(theStartTime))) /
                         halfPeriod * (factor2 - factor1) +
                     factor1;
          }
          else
          {
            factor = (static_cast<float>(newInfo->Time().DifferenceInHours(middleTime))) /
                         halfPeriod * (factor3 - factor2) +
                     factor2;
          }

          if (weather->TotalCloudiness() != kT4BitMissing)
          {
            totalCloudinessSum += weather->TotalCloudiness() * factor;
            totalCloudinessFactorSum += factor;
          }
          if (weather->LowClouds() != kT4BitMissing)
          {
            lowCloudsSum += weather->LowClouds() * factor;
            lowCloudsFactorSum += factor;
          }
          if (weather->MiddleClouds() != kT4BitMissing)
          {
            middleCloudsSum += weather->MiddleClouds() * factor;
            middleCloudsFactorSum += factor;
          }
          if (weather->HighClouds() != HighCloudsMissingValue())
          {
            highCloudsSum += weather->HighClouds() * factor;
            highCloudsFactorSum += factor;
          }
          if (weather->TotalPrecipitation() != TotalPrecipitationMissingValue())
          {
            double prec = PrecipitationScale(weather->TotalPrecipitation());
            maxPrecipitation = FmiMax(maxPrecipitation, prec);
            totalPrecipitationSum += prec * factor;
            totalPrecipitationFactorSum += factor;
          }
          if (weather->PrecipitationType() != kT2BitMissing)
          {
            precipitationTypeSum += weather->PrecipitationType() * factor;
            precipitationTypeFactorSum += factor;
          }
          if (weather->PrecipitationForm() != kT3BitMissing &&
              weather->TotalPrecipitation() != TotalPrecipitationMissingValue())
          {  // painotus sademäärän mukaan
            precipitationForms[weather->PrecipitationForm()] +=
                PrecipitationScale(weather->TotalPrecipitation()) * factor;
          }
          if (weather->ThunderProbability() != kTThunderProbabilityMissing)
          {
            double thunder = weather->ThunderProbability() * 10.;  // 29.5.00
            thunderProbabilitySum += thunder * factor;
            thunderProbabilityFactorSum += factor;
            maxThunder = FmiMax(maxThunder, thunder);
          }
          if (weather->FogIntensity() != kT2BitMissing)
          {
            fogIntensitySum += weather->FogIntensity() * factor;
            fogIntensityFactorSum += factor;
          }
        }
      }
      if (!newInfo->NextTime())
        break;
    }

    //	pitäisi olla joku resetointi ensin
    itsData.longType = kTCombinedWeatherMissing;  // riittääkö?

    if (lowCloudsFactorSum > 0.)
      SetLowClouds(static_cast<unsigned long>(lowCloudsSum / lowCloudsFactorSum + .5));
    if (middleCloudsFactorSum > 0.)
      SetMiddleClouds(static_cast<unsigned long>(middleCloudsSum / middleCloudsFactorSum + .5));
    if (highCloudsFactorSum > 0.)
      SetHighClouds(static_cast<unsigned long>(highCloudsSum / highCloudsFactorSum + .5));
    if (totalCloudinessFactorSum > 0.)
      SetTotalCloudiness(
          static_cast<unsigned long>(totalCloudinessSum / totalCloudinessFactorSum + .5));
    if (precipitationTypeFactorSum > 0.)
      SetPrecipitationType(
          static_cast<unsigned long>(precipitationTypeSum / precipitationTypeFactorSum + .5));

    if (totalPrecipitationFactorSum > 0.)
    {
      double mmWithMax;
      if (considerMax)
        mmWithMax = (maxPrecipitation + totalPrecipitationSum / totalPrecipitationFactorSum) / 2.;
      else
        mmWithMax = totalPrecipitationSum / totalPrecipitationFactorSum;

      SubValue(mmWithMax + 0.01, kFmiPrecipitation1h);
    }

    SetPrecipitationForm(PrecipitationFormFromWeightTable(precipitationForms));

    if (thunderProbabilityFactorSum > 0.)
    {
      double thunderWithMax;
      if (considerMax)
        thunderWithMax = (maxThunder + thunderProbabilitySum / thunderProbabilityFactorSum) / 2.;
      else
        thunderWithMax = thunderProbabilitySum / thunderProbabilityFactorSum;

      SubValue(thunderWithMax, kFmiProbabilityThunderstorm);
    }
    if (fogIntensityFactorSum > 0.)
      SetFogIntensity(static_cast<unsigned long>(fogIntensitySum / fogIntensityFactorSum + .5));

    delete newInfo;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param info Undocumented
 * \param theLonLat Undocumented
 * \param period Undocumented
 * \param considerMax Undocumented
 * \param factor1 Undocumented
 * \param factor2 Undocumented
 * \param factor3 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::SetToWeightedPeriod(NFmiQueryInfo *info,
                                                   const NFmiPoint &theLonLat,
                                                   unsigned long period,
                                                   bool considerMax,
                                                   float factor1,
                                                   float factor2,
                                                   float factor3)
{
  try
  {
    bool locationMissing = theLonLat.X() == 0. && theLonLat.Y() == 0.;

    NFmiQueryInfo *newInfo = info->Clone();
    NFmiMetTime startTime(info->Time());
    NFmiMetTime middleTime(startTime);
    NFmiMetTime endTime(startTime);
    long halfPeriod = period / 2;
    startTime.ChangeByHours(-halfPeriod);
    endTime.ChangeByHours(halfPeriod);

    double totalCloudinessSum = 0.;
    double lowCloudsSum = 0.;
    double middleCloudsSum = 0.;
    double highCloudsSum = 0.;
    double totalPrecipitationSum = 0.;
    double precipitationTypeSum = 0.;
    double thunderProbabilitySum = 0.;
    double fogIntensitySum = 0.;
    double precipitationForms[FmiNumberOfPrecipitationForms];
    for (double &precipitationForm : precipitationForms)
      precipitationForm = 0;

    double totalCloudinessFactorSum = 0;
    double lowCloudsFactorSum = 0;
    double middleCloudsFactorSum = 0;
    double highCloudsFactorSum = 0;
    double totalPrecipitationFactorSum = 0;
    double precipitationTypeFactorSum = 0;
    double thunderProbabilityFactorSum = 0;
    double fogIntensityFactorSum = 0;
    double maxPrecipitation = 0.;
    double maxThunder = 0.;
    float factor;

    if (!newInfo->TimeToNearestStep(startTime) || !newInfo->TimeToNearestStep(endTime))
    {
      delete newInfo;
      return false;
    }

    newInfo->Param(kFmiWeatherSymbol3);
    while (newInfo->Time() <= endTime)
    {
      {
        // jos lonlat annettu  pitää ottaa paikan mukaan interpoloitu
        if (!locationMissing)
          newInfo->InterpolatedValue(theLonLat);

        const auto *weather =
            static_cast<const NFmiWeatherAndCloudiness *>(newInfo->CombinedParam());
        if (weather)
        {
          if (newInfo->Time() < info->Time())
          {
            factor = (static_cast<float>(newInfo->Time().DifferenceInHours(startTime))) /
                         halfPeriod * (factor2 - factor1) +
                     factor1;
          }
          else
          {
            factor = (static_cast<float>(newInfo->Time().DifferenceInHours(middleTime))) /
                         halfPeriod * (factor3 - factor2) +
                     factor2;
          }

          if (weather->TotalCloudiness() != kT4BitMissing)
          {
            totalCloudinessSum += weather->TotalCloudiness() * factor;
            totalCloudinessFactorSum += factor;
          }
          if (weather->LowClouds() != kT4BitMissing)
          {
            lowCloudsSum += weather->LowClouds() * factor;
            lowCloudsFactorSum += factor;
          }
          if (weather->MiddleClouds() != kT4BitMissing)
          {
            middleCloudsSum += weather->MiddleClouds() * factor;
            middleCloudsFactorSum += factor;
          }
          if (weather->HighClouds() != HighCloudsMissingValue())
          {
            highCloudsSum += weather->HighClouds() * factor;
            highCloudsFactorSum += factor;
          }
          if (weather->TotalPrecipitation() != TotalPrecipitationMissingValue())
          {
            double prec = PrecipitationScale(weather->TotalPrecipitation());
            maxPrecipitation = FmiMax(maxPrecipitation, prec);
            totalPrecipitationSum += prec * factor;
            totalPrecipitationFactorSum += factor;
          }
          if (weather->PrecipitationType() != kT2BitMissing)
          {
            precipitationTypeSum += weather->PrecipitationType() * factor;
            precipitationTypeFactorSum += factor;
          }
          if (weather->PrecipitationForm() != kT3BitMissing &&
              weather->TotalPrecipitation() != TotalPrecipitationMissingValue())
          {  // painotus sademäärän mukaan
            precipitationForms[weather->PrecipitationForm()] +=
                PrecipitationScale(weather->TotalPrecipitation()) * factor;
          }
          if (weather->ThunderProbability() != kTThunderProbabilityMissing)
          {
            double thunder = weather->ThunderProbability() * 10.;  // 29.5.00
            thunderProbabilitySum += thunder * factor;
            thunderProbabilityFactorSum += factor;
            maxThunder = FmiMax(maxThunder, thunder);
          }
          if (weather->FogIntensity() != kT2BitMissing)
          {
            fogIntensitySum += weather->FogIntensity() * factor;
            fogIntensityFactorSum += factor;
          }
        }
      }
      if (!newInfo->NextTime())
        break;
    }

    //	pitäisi olla joku resetointi ensin
    itsData.longType = kTCombinedWeatherMissing;  // riittääkö?

    if (lowCloudsFactorSum > 0.)
      SetLowClouds(static_cast<unsigned long>(lowCloudsSum / lowCloudsFactorSum + .5));
    if (middleCloudsFactorSum > 0.)
      SetMiddleClouds(static_cast<unsigned long>(middleCloudsSum / middleCloudsFactorSum + .5));
    if (highCloudsFactorSum > 0.)
      SetHighClouds(static_cast<unsigned long>(highCloudsSum / highCloudsFactorSum + .5));
    if (totalCloudinessFactorSum > 0.)
      SetTotalCloudiness(
          static_cast<unsigned long>(totalCloudinessSum / totalCloudinessFactorSum + .5));
    if (precipitationTypeFactorSum > 0.)
      SetPrecipitationType(
          static_cast<unsigned long>(precipitationTypeSum / precipitationTypeFactorSum + .5));

    if (totalPrecipitationFactorSum > 0.)
    {
      double mmWithMax;
      if (considerMax)
        mmWithMax = (maxPrecipitation + totalPrecipitationSum / totalPrecipitationFactorSum) / 2.;
      else
        mmWithMax =
            totalPrecipitationSum / totalPrecipitationFactorSum;  // 16.11.99 oli maxPr.. !!!

      SubValue(mmWithMax + 0.01, kFmiPrecipitation1h);  // 25.5.00 pieni pyöristyslisä
    }

    SetPrecipitationForm(PrecipitationFormFromWeightTable(precipitationForms));

    if (thunderProbabilityFactorSum > 0.)
    {
      double thunderWithMax;
      if (considerMax)
        thunderWithMax = (maxThunder + thunderProbabilitySum / thunderProbabilityFactorSum) / 2.;
      else
        thunderWithMax = thunderProbabilitySum / thunderProbabilityFactorSum;

      SubValue(thunderWithMax, kFmiProbabilityThunderstorm);
    }
    if (fogIntensityFactorSum > 0.)
      SetFogIntensity(static_cast<unsigned long>(fogIntensitySum / fogIntensityFactorSum + .5));

    delete newInfo;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::TotalCloudinessToHessaa() const
{
  try
  {
    unsigned long clouds = kUnsignedLongMissing;

    // Lasse otti uudet kynnysarvot

    if (TotalCloudiness() <= 3)
      clouds = 1;
    else if (TotalCloudiness() > 3 && TotalCloudiness() <= 7)
      clouds = 2;
    else if (TotalCloudiness() > 7 && TotalCloudiness() <= 10)
      clouds = 3;
    return clouds;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::TotalPrecipitationToHessaa() const
{
  try
  {
    unsigned long preci = kUnsignedLongMissing;
    if (CalcTotalPrecipitation() == kTNoPrecipitation)
      preci = kUnsignedLongMissing;  // vai sittenkin nolla?!
    else if (CalcTotalPrecipitation() == kTSomePrecipitation)
      preci = 1;
    else if (CalcTotalPrecipitation() == kTModeratePrecipitation)
      preci = 2;
    else if (CalcTotalPrecipitation() == kTMuchPrecipitation)
      preci = 3;
    return preci;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::PartlyCloudyHessaa()
{
  try
  {
    unsigned long hessaa = kUnsignedLongMissing;
    if (IsConvective())  // tyyppi on jo asetettu
    {
      if (IsSnow())
      {
        hessaa = 40;  // sataa, konv, lunta
        fDataOk = true;
      }
      else if (IsSleet())
      {
        hessaa = 70;  // sataa, konv, räntää
        fDataOk = true;
      }
      else if (PrecipitationForm() != kT3BitMissing)
      {
        hessaa = 20;  // sataa, konv, vettä tai tihkua
        fDataOk = true;
      }
      else if (fAutoUpdate)  // olomuoto puuttuva
      {
        if (itsTemperature > -1)  // saa olla puuttuvakin
        {
          SetPrecipitationForm(kTRain);
          hessaa = 20;
        }
        else
        {
          SetPrecipitationForm(kTSnow);
          hessaa = 40;
        }
        fDataOk = true;
      }
      else
        fDataOk = false;  // sateen olomuoto puuttuu
    }
    // seuraavan voisi yhdistää edelliseen
    else if (PrecipitationType() == kTLargeScalePrecipitation)
    {
      if (IsSnow())
      {
        hessaa = 40;  // 50; //lunta
        fDataOk = true;
      }
      else if (IsSleet())
      {
        hessaa = 70;  // räntää
        fDataOk = true;
      }
      else if (PrecipitationForm() != kT3BitMissing)
      {
        hessaa = 20;  // 30;  //sataa, konv, vettä tai tihkua
        fDataOk = true;
      }
      else if (fAutoUpdate)  // olomuoto puuttuva
      {
        if (itsTemperature > -1)  // saa olla puuttuvakin
        {
          SetPrecipitationForm(kTRain);
          hessaa = 20;  // 30;
        }
        else
        {
          SetPrecipitationForm(kTSnow);
          hessaa = 40;  // 50;
        }
        fDataOk = true;
      }
      else
        fDataOk = false;  // sateen olomuoto puuttuu
    }
    else if (fAutoUpdate)
    {
      SetPrecipitationType(kTConvectivePrecipitation);
      if (PrecipitationForm() == kT3BitMissing)
      {
        if (itsTemperature > -1)  // saa olla puuttuvakin
        {
          SetPrecipitationForm(kTRain);
          hessaa = 20;
        }
        else
        {
          SetPrecipitationForm(kTSnow);
          hessaa = 40;
        }
      }
      else if (IsSnow())
        hessaa = 40;
      else if (IsSleet())
        hessaa = 70;
      else
        hessaa = 20;
      fDataOk = true;
    }
    else
      fDataOk = false;  // sateen tyyppi väärin noille pilville
    long temp = TotalPrecipitationToHessaa();
    if (temp != kLongMissing)
      hessaa += temp;
    else
      hessaa = kUnsignedLongMissing;

    return hessaa;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::HalfCloudyHessaa()
{
  try
  {
    unsigned long hessaa = kUnsignedLongMissing;
    unsigned long preci = TotalPrecipitationToHessaa();
    if (preci == kUnsignedLongMissing)  // ei sada sittenkään
      fDataOk = false;
    else if (IsConvective())
    {
      if (IsSnow())
      {
        hessaa = 40 + preci;  // sataa lunta
        fDataOk = true;
      }
      else if (IsSleet())
      {
        hessaa = 70 + preci;  // sataa lunta
        fDataOk = true;
      }
      else if (PrecipitationForm() != kT3BitMissing)
      {
        hessaa = 20 + preci;  // ei sada lunta, vaan jotain muuta
        fDataOk = true;
      }
      else if (fAutoUpdate)
      {
        if (PrecipitationForm() == kT3BitMissing)  // jos ei sada lunta
        {
          if (itsTemperature > -1)  // saa olla puuttuvakin
          {
            SetPrecipitationForm(kTRain);
            hessaa = 20 + preci;
          }
          else
          {
            SetPrecipitationForm(kTSnow);
            hessaa = 40 + preci;
          }
        }
        fDataOk = true;
      }
      else
        fDataOk = false;  // sateen olomuoto puuttuu
    }
    // yhdistä seuraava edelliseen
    else if (PrecipitationType() == kTLargeScalePrecipitation)
    {
      if (IsSnow())  // entäs räntä??
      {
        hessaa = 40 + preci;  // 50+preci; //sataa lunta
        fDataOk = true;
      }
      else if (IsSleet())
      {
        hessaa = 70 + preci;  // 50+preci; //sataa räntää
        fDataOk = true;
      }
      else if (PrecipitationForm() != kT3BitMissing)
      {
        hessaa = 20 + preci;  // 30+preci; //ei sada lunta, vaan jotain muuta
        fDataOk = true;
      }
      else if (fAutoUpdate)
      {
        if (PrecipitationForm() == kT3BitMissing)  // jos ei sada lunta
        {
          if (itsTemperature > -1)  // saa olla puuttuvakin
          {
            SetPrecipitationForm(kTRain);
            hessaa = 20 + preci;  // 30+preci;
          }
          else
          {
            SetPrecipitationForm(kTSnow);
            hessaa = 40 + preci;  // 50+preci;
          }
        }
        fDataOk = true;
      }
      else
        fDataOk = false;  // sateen olomuoto puuttuu
    }
    else if (fAutoUpdate)
    {
      SetPrecipitationType(kTConvectivePrecipitation);
      if (PrecipitationForm() == kT3BitMissing)  // jos ei sada lunta
      {
        if (itsTemperature > -1)  // saa olla puuttuvakin
        {
          SetPrecipitationForm(kTRain);
          hessaa = 20 + preci;
        }
        else
        {
          SetPrecipitationForm(kTSnow);
          hessaa = 40 + preci;
        }
      }
      else if (!IsSnow() && !IsSleet())
      {
        SetPrecipitationForm(kTRain);  // asetataan vesisadetta
        hessaa = 20 + preci;
      }
      else if (IsSleet())
        hessaa = 70 + preci;
      else
        hessaa = 40 + preci;  // olomuoto on jo lumi
      fDataOk = true;
    }
    else  // jatkuvaa sadetta ja puolipilvistä
      fDataOk = false;

    return hessaa;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::OverCastHessaa()
{
  try
  {
    unsigned long hessaa = kUnsignedLongMissing;
    unsigned long preci = TotalPrecipitationToHessaa();
    if (preci == kUnsignedLongMissing)  // ei sada sittenkään
    {
      fDataOk = false;
      return hessaa;
    }
    if (!IsConvective() && PrecipitationType() != kT2BitMissing)
    {
      if (IsSnow())
      {
        hessaa = 50 + preci;  // sataa lunta
        fDataOk = true;
      }
      else if (IsSleet())
      {
        hessaa = 80 + preci;  // sataa lunta
        fDataOk = true;
      }
      else if (PrecipitationForm() != kT3BitMissing)
      {
        hessaa = 30 + preci;  // sataa muuta kuin lunta
        fDataOk = true;
      }
      else if (fAutoUpdate)  // ja olomuotoa ei ole määritetty
      {
        SetPrecipitationType(kTLargeScalePrecipitation);

        if (!IsSnow() && !IsSleet())  // jos ei sada lunta
        {
          SetPrecipitationForm(kTRain);  // asetataan vesisadetta
          hessaa = 30 + preci;
        }
        else if (IsSleet())  // jos ei sada lunta
        {
          SetPrecipitationForm(kTSleet);  // asetataan vesisadetta
          hessaa = 80 + preci;
        }
        else
          hessaa = 50 + preci;

        fDataOk = true;
      }
      else
        fDataOk = false;  // sateen olomuoto puuttuu
    }
    else if (IsConvective())
    {
      if (IsSnow())
      {
        hessaa = 50 + preci;  // 40+preci; //sataa lunta
        fDataOk = true;
      }
      else if (IsSleet())
      {
        hessaa = 80 + preci;
        fDataOk = true;
      }
      else if (PrecipitationForm() != kT3BitMissing)
      {
        hessaa = 30 + preci;  // 20+preci; //sataa muuta kuin lunta
        fDataOk = true;
      }
      else if (fAutoUpdate)  // ja olomuotoa ei ole määritetty
      {
        if (itsTemperature > -1)
        {
          SetPrecipitationForm(kTRain);  // asetataan vesisadetta
          hessaa = 30 + preci;           // 20+preci;
        }
        else
        {
          SetPrecipitationForm(kTSnow);  // asetataan vesisadetta
          hessaa = 50 + preci;           // 40+preci;
        }
      }
      else
        fDataOk = false;  // sateen olomuoto puuttuu
    }
    else if (fAutoUpdate)
    {
      SetPrecipitationType(kTLargeScalePrecipitation);
      if (!IsSnow() && !IsSleet())  // jos ei sada lunta
      {
        SetPrecipitationForm(kTRain);  // asetataan vesisadetta
        hessaa = 30 + preci;
      }
      else if (IsSleet())  // jos ei sada lunta
      {
        SetPrecipitationForm(kTSleet);  // asetataan vesisadetta
        hessaa = 80 + preci;
      }
      else
        hessaa = 50 + preci;  // olomuoto on jo lumi
    }
    else
      fDataOk = false;  // konvektiivista sadetta ja täyspilvistä tai tyyppi puuttuu

    return hessaa;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

/*
        Seuraava ukkos-hessaa määritys on otettu Lassen lehtisää spekseistä:
        Hessaa koodit 61 - 64
        Oletus: Tähän metodiin (ThunderToHessaa) tullaan vain jos ukkosen tn >= 30.
        Koodi rajat määritelty seuraavasti:
        Täys pilvisyyden raja (63 ja 64) on N >= 85, muuten puolipilvistä (vaikka ei olisi pilveä
   ollenkaan)
        Voimakkaan ukkosen rajat (62 ja 64) on että ukkosen tn >= 60 %
        Oletus arvo jos ukkosen tn >= 30 , jos muut arvot (rr ja N) ei puuttuvia on 61
*/

unsigned long NFmiWeatherAndCloudiness::ThunderToHessaa() const
{
  try
  {
    double thunderProb = ThunderProbabilityValue();
    double N = TotalCloudinessValue();
    double rr = Precipitation1hValue();
    if (thunderProb != kFloatMissing && N != kFloatMissing && rr != kFloatMissing)
    {
      if (thunderProb >= 60)
      {  // kovaa ukkosta 62 ja 64
        if (N >= 85)
          return 64;
        else
          return 62;
      }
      else
      {  // heikompaa ukkosta 61 ja 63
        if (N >= 85)
          return 63;
        else
          return 61;
      }
    }
    // oletus oli että tähän metodiin tullaan vain jos ukkosen tn >= 30, joten tarkastetaan tämä
    // vielä erikseen
    if (thunderProb != kFloatMissing && thunderProb >= 30)
      return 61;
    // jos jokin arvoista oli puuttuvaa, palautetaan puuttuva arvo
    return kUnsignedLongMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::ToHessaa()
{
  try
  {
    unsigned long hessaa = kUnsignedLongMissing;
    if (itsData.longType == kTCombinedWeatherMissing)
      return hessaa;
    if (ThunderProbability() >= 3 &&
        ThunderProbability() != kTThunderProbabilityMissing)  // ukkonen otetaan huomioon, jos sen
                                                              // tn on >= 30 (Lassen lehtisää
                                                              // speksien mukaiseksi muutettu)
      return ThunderToHessaa();
    if (IsFair())  // ei sada
      return TotalCloudinessToHessaa();

    else  // sataa, ei ukkosta
    {
      if (TotalCloudiness() > kTSkyClear && TotalCloudiness() < kTPartlyCloudy)
        hessaa = PartlyCloudyHessaa();
      else if (TotalCloudiness() >= kTPartlyCloudy && TotalCloudiness() <= kTNearlyCloudy)
        hessaa = HalfCloudyHessaa();
      else if (TotalCloudiness() > kTNearlyCloudy && TotalCloudiness() <= kTOverCast)
        hessaa = OverCastHessaa();
      else  // pilvet puuttuu
      {
        fDataOk = false;                     // pilvet puuttuu, vaikka sataa
        hessaa = TotalCloudinessToHessaa();  // koemielessä täällä (laura 051099)
      }
    }
    return hessaa;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::CheckHessaa(unsigned long theValue) const
{
  try
  {
    bool retVal = false;
    if (theValue != kFloatMissing && theValue != kUnsignedLongMissing)
    {
      if (theValue >= 1 &&
          (theValue < 4 || (theValue / 10 <= 8 && theValue % 10 <= 3)))  // muutin tarkastelussa
                                                                         // (theValue/10 <= 8) 6:n
                                                                         // 8:ksi, koska rännät
                                                                         // tulivat kuvioihin
        retVal = true;
    }
    return retVal;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::ToHsade() const
{
  try
  {
    unsigned long hsade = kUnsignedLongMissing;
    if (CalcTotalPrecipitation() == kTNoPrecipitation)
      hsade = 0;
    else if (CalcTotalPrecipitation() == kTSomePrecipitation)
      hsade = 60;
    else if (CalcTotalPrecipitation() == kTModeratePrecipitation)
      hsade = 63;
    else if (CalcTotalPrecipitation() == kTMuchPrecipitation)
      hsade = 65;
    if (hsade >= 60 && hsade <= 65)
    {
      if (IsLargeScale())
      {
        if (IsRain())  // vettä
        {
          hsade = hsade;
        }
        else if (IsSleet())  // räntää
        {
          hsade = 68;
        }
        else if (IsSnow())  // lunta
        {
          hsade += 10;
        }
        // ukkoset myös tänne ainakin toistaiseksi (laura 23091999)
        if (IsThunder() && ThunderProbability() >= 8)
          hsade = 97;
        else if (IsThunder() && ThunderProbability() >= 4)
          hsade = 95;
      }
      else  // kuuroja
      {
        if (ThunderProbability() != kTThunderProbabilityMissing &&
            ThunderProbability() >= 8)  // tarkista luku!!
        {
          if (IsSnow())
          {
            if (hsade >= 63)
              hsade = 86;
            else
              hsade = 85;
          }
          else
            hsade = 97;
        }
        else if (ThunderProbability() != kTThunderProbabilityMissing && ThunderProbability() >= 4)
        {
          if (IsSnow())
          {
            if (hsade >= 63)
              hsade = 86;
            else
              hsade = 85;
          }
          else if (PrecipitationForm() != kT3BitMissing)
          {
            hsade = 95;
          }
        }
        else  // ei ukkosta
        {
          if (IsSnow())
          {
            if (hsade >= 63)
              hsade = 86;
            else
              hsade = 85;
          }
          else if (IsSleet())
          {
            // 22.01.2002/Viljo Showers of rain and snow mixed were not handled att all
            // 83     Shower(s) of rain and snow mixed, slight
            // 84     Shower(s) of rain and snow mixed, moderate or heavy
            if (hsade >= 63)
              hsade = 84;
            else
              hsade = 83;
          }
          else if (PrecipitationForm() != kT3BitMissing)
          {
            // 22.01.2002/Viljo adds next comments:
            // 80     Rain shower(s), slight
            // 81     Rain shower(s), moderate or heavy
            // 82     Rain shower(s), violent
            // 02.10.2002/Viljo and Marko made the change
            if (hsade == 63)
              hsade = 81;         // lisäsin uuden 'kohtalaista' kuuroa koodin
            else if (hsade > 63)  // 31.10.2002/Viljo adds missing else
              hsade = 82;
            else
              hsade = 80;
          }
        }
      }
      if (PrecipitationForm() == kTDrizzle)
      {
        // lumijyväset puuttuu!!
        if (hsade <= 61)
          hsade -= 10;
        else if (itsTemperature < -1)  // suuret tihkut kuuroiksi
          hsade = 85;
        else
          hsade = 80;
      }
      if (hsade >= 60 && hsade <= 65)  // 18.1.2000/Marko lisäsi jäätämis jutut
      {
        if (PrecipitationForm() == kTFreezingRain)
        {
          if (hsade <= 61)
            hsade = 66;
          else
            hsade = 67;
        }
        if (PrecipitationForm() == kTFreezingDrizzle)
        {
          if (hsade <= 61)
            hsade = 56;
          else
            hsade = 57;
        }
      }  // 18.1.2000/Marko lisäsi jäätämis jutut
    }
    return hsade;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::FromHessaa(unsigned long theValue)
{
  try
  {
    if (CheckHessaa(theValue))
    {
      SetTotalCloudiness(TotalCloudinessFromHessaa(theValue));
      SetTotalPrecipitation(TotalPrecipitationFromHessaa(theValue));
      SetPrecipitationType(PrecipitationTypeFromHessaa(theValue));
      SetPrecipitationForm(PrecipitationFormFromHessaa(theValue));
      SetThunderProbability(ThunderProbabilityFromHessaa(theValue));
      return true;
    }
    else
    {
      itsData.longType = kTCombinedWeatherMissing;
      return false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::TotalCloudinessFromHessaa(unsigned long theValue)
{
  try
  {
    switch (theValue)
    {
      case 1:
        return kTSkyClear;
      case 2:
      case 21:
      case 41:
      case 71:
        return kTPartlyCloudy;
      case 22:
      case 23:
      case 42:
      case 43:
      case 72:
      case 73:
      case 61:
        return kTHalfCloudy;
      case 3:
      case 31:
      case 32:
      case 33:
      case 51:
      case 52:
      case 53:
      case 81:
      case 82:
      case 83:
      case 62:
      case 63:
        return kTOverCast;
      default:
        return kT4BitMissing;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::TotalPrecipitationFromHessaa(unsigned long theValue)
{
  try
  {
    unsigned long retVal = kUnsignedLongMissing;
    switch (theValue)
    {
      case 1:
      case 2:
      case 3:
        retVal = kTNoPrecipitation;
        break;
      case 21:
      case 31:
      case 41:
      case 51:
      case 71:
      case 81:
        retVal = kTSomePrecipitation;
        break;
      case 22:
      case 32:
      case 42:
      case 52:
      case 72:
      case 82:
      case 61:
        retVal = kTModeratePrecipitation;
        break;
      case 23:
      case 33:
      case 43:
      case 53:
      case 73:
      case 83:
      case 62:
      case 63:
        retVal = kTMuchPrecipitation;
        break;
      default:
        return TotalPrecipitationMissingValue();
    }
    unsigned long mmValue =
        CalcPrecipitationFromHessaa(retVal);  // muunnetaan milleiksi ja biteiksi
    return mmValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcPrecipitationFromHessaa(unsigned long theValue)
{
  try
  {
    // täällä pitäisi varmaan huomioida precipitaionType mm-rajoissa
    double mmValue = kFloatMissing;
    switch (theValue)
    {
      case kTNoPrecipitation:
        mmValue = 0;
        break;
      case kTSomePrecipitation:
        mmValue = 0.1;
        break;
      case kTModeratePrecipitation:
        mmValue = 0.5;
        break;
      case kTMuchPrecipitation:
        mmValue = 4.0;
        break;
      default:
        break;
    }
    return PrecipitationScale(mmValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::PrecipitationTypeFromHessaa(unsigned long theValue)
{
  try
  {
    switch (theValue)
    {
      case 1:
      case 2:
      case 3:
        return kT2BitMissing;
      case 21:
      case 22:
      case 23:
      case 41:
      case 42:
      case 43:
      case 71:
      case 72:
      case 73:
      case 61:
      case 62:
      case 63:
        return kTConvectivePrecipitation;
      case 31:
      case 32:
      case 33:
      case 51:
      case 52:
      case 53:
      case 81:
      case 82:
      case 83:
        return kTLargeScalePrecipitation;
      default:
        return kT2BitMissing;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::PrecipitationFormFromHessaa(unsigned long theValue)
{
  try
  {
    switch (theValue)
    {
      case 1:
      case 2:
      case 3:
        return kT3BitMissing;
      case 21:
      case 22:
      case 23:
      case 31:
      case 32:
      case 33:
      case 61:
      case 62:
      case 63:
        return kTRain;
      case 41:
      case 42:
      case 43:
      case 51:
      case 52:
      case 53:
        return kTSnow;
      case 71:
      case 72:
      case 73:
      case 81:
      case 82:
      case 83:
        return kTSleet;
      default:
        return kT3BitMissing;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::ThunderProbabilityFromHessaa(unsigned long theValue)
{
  try
  {
    switch (theValue)
    {
      case 1:
      case 2:
      case 3:
      case 21:
      case 31:
      case 32:
      case 33:
      case 41:
      case 42:
      case 51:
      case 52:
      case 53:
      case 71:
      case 72:
      case 73:
      case 81:
      case 82:
      case 83:
        return 0;
      case 22:
      case 43:
        return 0;
      case 23:
        return 0;
      case 61:
        return 4;
      case 62:
        return 6;
      case 63:
        return 8;
      default:
        return kT3BitMissing;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
static const double gNoPrecipitationLimitV7 = 0.05;
static const double gNoPrecipitationLimitV6 = 0.1;
unsigned long NFmiWeatherAndCloudiness::CalcTotalPrecipitation() const
{
  try
  {
    // tänne sateen intensiteettirajat!!!!
    if (TotalPrecipitation() != TotalPrecipitationMissingValue())
    {
      if (TotalPrecipitation() < PrecipitationScale(itsInfoVersion >= 7. ? gNoPrecipitationLimitV7
                                                                         : gNoPrecipitationLimitV6))
        return kTNoPrecipitation;

      if (TotalPrecipitation() >= PrecipitationScale(itsInfoVersion >= 7.
                                                         ? gNoPrecipitationLimitV7
                                                         : gNoPrecipitationLimitV6) &&
          TotalPrecipitation() < PrecipitationScale(0.5))
        return kTSomePrecipitation;
      if (TotalPrecipitation() >= PrecipitationScale(0.5) &&
          TotalPrecipitation() < PrecipitationScale(4.0))
        return kTModeratePrecipitation;
      if (TotalPrecipitation() >= PrecipitationScale(4.0))
        return kTMuchPrecipitation;
    }
    return TotalPrecipitationMissingValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::LowAndMiddleClouds() const
{
  try
  {
    unsigned long returnValue;
    if (HighClouds() == HighCloudsMissingValue() || HighClouds() == 0)
    {
      if (TotalCloudiness() == kT4BitMissing)
        returnValue = kT4BitMissing;
      else
        returnValue = TotalCloudiness() * 10;
    }
    else
    {
      if (LowClouds() != kT4BitMissing)
      {
        if (MiddleClouds() != kT4BitMissing)
          returnValue = (FmiMax(LowClouds(), MiddleClouds())) * 10;  // suurempi prosentteina
        else
          returnValue = LowClouds() * 10;
      }
      else
      {
        if (MiddleClouds() == kT4BitMissing)
          returnValue = kT4BitMissing;
        else
          returnValue = MiddleClouds() * 10;
      }
    }
    if (returnValue == kT4BitMissing)
      returnValue = kUnsignedLongMissing;

    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::SubValue(double theValue, FmiParameterName theParam)
{
  try
  {
    bool returnValue = false;
    switch (theParam)
    {
      case kFmiTotalCloudCover:
        if (theValue == kFloatMissing)
          returnValue = TotalCloudiness(kT4BitMissing);
        else
          returnValue =
              TotalCloudiness(static_cast<unsigned long>(FmiMax(0, round(theValue / 10.0))));
        break;
      case kFmiLowCloudCover:
        if (theValue == kFloatMissing)
          returnValue = LowClouds(kT4BitMissing);
        else
          returnValue = LowClouds(static_cast<unsigned long>(FmiMax(0, round(theValue / 10.0))));
        break;
      case kFmiMediumCloudCover:
        if (theValue == kFloatMissing)
          returnValue = MiddleClouds(kT4BitMissing);
        else
          returnValue = MiddleClouds(static_cast<unsigned long>(FmiMax(0, round(theValue / 10.0))));
        break;
      case kFmiHighCloudCover:
      {
        if (theValue == kFloatMissing)
          returnValue = HighClouds(HighCloudsMissingValue());
        else
        {
          unsigned long tempValue = 0;
          if (itsInfoVersion >= 7.)
            tempValue = HighCloudsV7(theValue);
          else
            tempValue = static_cast<unsigned long>(FmiMax(0, round(theValue / 10.0)));
          returnValue = HighClouds(tempValue);
        }
        break;
      }
      case kFmiPrecipitation1h:
        if (theValue == kFloatMissing)
          returnValue = TotalPrecipitation(TotalPrecipitationMissingValue());
        else
          returnValue = TotalPrecipitation(PrecipitationScale(theValue));
        break;
      case kFmiPrecipitationType:
        if (theValue == kFloatMissing)
          returnValue = PrecipitationType(kT2BitMissing);
        else
          returnValue = PrecipitationType(static_cast<unsigned long>(FmiMax(0.0, theValue)));
        break;
      case kFmiPrecipitationForm:
        if (theValue == kFloatMissing)
          returnValue = PrecipitationForm(kT3BitMissing);
        else
          returnValue = PrecipitationForm(static_cast<unsigned long>(FmiMax(0.0, theValue)));
        break;
      case kFmiProbabilityThunderstorm:
        if (theValue == kFloatMissing)
          returnValue = ThunderProbability(kT4BitMissing);
        else
          returnValue =
              ThunderProbability(static_cast<unsigned long>(FmiMax(0, round(theValue / 10.0))));
        break;
      case kFmiFogIntensity:
        if (theValue == kFloatMissing)
          returnValue = FogIntensity(kT2BitMissing);
        else
          returnValue = FogIntensity(static_cast<unsigned long>(FmiMax(0.0, theValue)));
        break;
      case kFmiWeatherSymbol3:
        if (theValue == kFloatMissing)
          returnValue = FromHessaa(kUnsignedLongMissing);
        else
          returnValue = FromHessaa(static_cast<unsigned long>(FmiMax(0.0, theValue)));
        break;
      default:
        break;
    }
    CrossCheck(theParam);
    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::HighCloudsV7(double theValue) const
{
  try
  {
    if (theValue == kFloatMissing)
      return kT3BitMissing;
    if (theValue < 5.)
      return 0;
    if (theValue < 20.)
      return 1;
    if (theValue < 40.)
      return 2;
    if (theValue < 60.)
      return 3;
    if (theValue < 80.)
      return 4;
    if (theValue < 95.)
      return 5;

    return 6;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiWeatherAndCloudiness::HighCloudsV7(unsigned long theValue) const
{
  try
  {
    switch (theValue)
    {
      case 0:
        return 0;
      case 1:
        return 10;
      case 2:
        return 30;
      case 3:
        return 50;
      case 4:
        return 70;
      case 5:
        return 90;
      case 6:
        return 100;
      default:
        return kFloatMissing;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param theSelector Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::IntegratedSubValue(
    FmiParameterName theParam, NFmiIntegrationSelector *theSelector) const
{
  try
  {
    if (fIntegrationMode && fIntegrationReady && theSelector)
    {
      NFmiDataModifierCombi *integrator =
          const_cast<NFmiWeatherAndCloudiness *>(this)->FindSubParamIntegrator(theParam);
      if (integrator)
      {
        NFmiIntegrationSelector tmpSelector(*theSelector);
        switch (theParam)
        {
          case kFmiTotalCloudCover:
          case kFmiLowCloudCover:
          case kFmiMediumCloudCover:
          case kFmiHighCloudCover:
          case kFmiProbabilityThunderstorm:
            tmpSelector.ScaleLowerLimit(0.1f);
            tmpSelector.ScaleUpperLimit(0.1f);
            break;
          case kFmiPrecipitation1h:
          {
            float tmpLimit = theSelector->ProbabilityLowerLimit();
            if (tmpLimit != kFloatMissing)
            {
              unsigned long siili = PrecipitationScale(tmpLimit);
              tmpSelector.ProbabilityLowerLimit(static_cast<float>(siili));
            }
            tmpLimit = theSelector->ProbabilityUpperLimit();
            if (tmpLimit != kFloatMissing)
            {
              unsigned long siili = PrecipitationScale(tmpLimit);
              tmpSelector.ProbabilityUpperLimit(static_cast<float>(siili));
            }
            break;
          }
          default:
            break;
        }

        float returnValue = integrator->CalcResult(tmpSelector);
        if (returnValue == kFloatMissing)
          return kUnsignedLongMissing;
        else
          return static_cast<unsigned long>(returnValue);
      }
    }
    return kUnsignedLongMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSelector Undocumented
 * \return Undocumented
 * \todo Kill the cat
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::IntegratedLongValue(NFmiIntegrationSelector *theSelector)
{
  try
  {
    if (!theSelector)
      return kUnsignedLongMissing;

    NFmiWeatherAndCloudiness newval(itsInfoVersion);
    newval.SubValue(SubValue(kFmiTotalCloudCover, theSelector), kFmiTotalCloudCover);
    newval.SubValue(SubValue(kFmiLowCloudCover, theSelector), kFmiLowCloudCover);
    newval.SubValue(SubValue(kFmiMediumCloudCover, theSelector), kFmiMediumCloudCover);
    newval.SubValue(SubValue(kFmiHighCloudCover, theSelector), kFmiHighCloudCover);
    newval.SubValue(SubValue(kFmiPrecipitation1h, theSelector), kFmiPrecipitation1h);
    newval.SubValue(SubValue(kFmiPrecipitationForm, theSelector), kFmiPrecipitationForm);
    newval.SubValue(SubValue(kFmiPrecipitationType, theSelector), kFmiPrecipitationType);
    newval.SubValue(SubValue(kFmiProbabilityThunderstorm, theSelector),
                    kFmiProbabilityThunderstorm);
    newval.SubValue(SubValue(kFmiFogIntensity, theSelector), kFmiFogIntensity);

    return newval.LongValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSelector Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::IntegratedHsade(NFmiIntegrationSelector *theSelector)
{
  try
  {
    if (!theSelector)
      return kUnsignedLongMissing;

    NFmiWeatherAndCloudiness newval(
        IntegratedLongValue(theSelector), kFmiPackedWeather, kFloatMissing, itsInfoVersion);
    return static_cast<unsigned long>(newval.SubValue(kFmiWeatherSymbol1));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSelector Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::IntegratedHessaa(NFmiIntegrationSelector *theSelector)
{
  try
  {
    if (!theSelector)
      return kUnsignedLongMissing;

    NFmiWeatherAndCloudiness newval(
        IntegratedLongValue(theSelector), kFmiPackedWeather, kFloatMissing, itsInfoVersion);
    return static_cast<unsigned long>(newval.SubValue(kFmiWeatherSymbol3));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param theSelector Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiWeatherAndCloudiness::SubValue(FmiParameterName theParam,
                                          NFmiIntegrationSelector *theSelector)
{
  try
  {
    unsigned long tempValue = 0;
    double returnValue = kFloatMissing;
    bool scaleIt = true;
    if (theSelector && theSelector->Type() == kFmiProb)
      scaleIt = false;
    switch (theParam)
    {
      case kFmiTotalCloudCover:
        if (theSelector)
          returnValue = IntegratedSubValue(theParam, theSelector);
        else
          returnValue = TotalCloudinessValue();
        break;
      case kFmiLowCloudCover:
        if (theSelector)
          tempValue = IntegratedSubValue(theParam, theSelector);
        else
          tempValue = LowClouds();
        if (tempValue != kT4BitMissing && tempValue != kUnsignedLongMissing)
          returnValue = scaleIt ? 10 * tempValue : tempValue;
        break;
      case kFmiMediumCloudCover:
        if (theSelector)
          tempValue = IntegratedSubValue(theParam, theSelector);
        else
          tempValue = MiddleClouds();
        if (tempValue != kT4BitMissing && tempValue != kUnsignedLongMissing)
          returnValue = scaleIt ? 10 * tempValue : tempValue;
        break;
      case kFmiHighCloudCover:
        if (theSelector)
          tempValue = IntegratedSubValue(theParam, theSelector);
        else
        {
          tempValue = HighClouds();
          if (itsInfoVersion >= 7.)
          {
            returnValue = HighCloudsV7(tempValue);
            break;  // versio 7:lle on jo skaalaus tehty
          }
        }
        if (tempValue != kT4BitMissing && tempValue != kUnsignedLongMissing)
          returnValue = scaleIt ? 10 * tempValue : tempValue;
        break;
      case kFmiPrecipitation1h:
        if (theSelector)
          returnValue = IntegratedSubValue(theParam, theSelector);
        else
          returnValue = Precipitation1hValue();
        break;
      case kFmiPrecipitationType:
        if (theSelector)
          tempValue = IntegratedSubValue(theParam, theSelector);
        else
          tempValue = PrecipitationType();
        if (tempValue != kT2BitMissing && tempValue != kUnsignedLongMissing)
          returnValue = tempValue;
        break;
      case kFmiPrecipitationForm:
        if (theSelector)
          tempValue = IntegratedSubValue(theParam, theSelector);
        else
          tempValue = PrecipitationForm();
        if (tempValue != kT3BitMissing && tempValue != kUnsignedLongMissing)
          returnValue = tempValue;
        break;
      case kFmiProbabilityThunderstorm:
        if (theSelector)
          returnValue = IntegratedSubValue(theParam, theSelector);
        else
          returnValue = ThunderProbabilityValue();
        break;
      case kFmiFogIntensity:
        if (theSelector)
          tempValue = IntegratedSubValue(theParam, theSelector);
        else
          tempValue = FogIntensity();
        if (tempValue != kT2BitMissing && tempValue != kUnsignedLongMissing)
          returnValue = tempValue;
        break;
      case kFmiWeatherSymbol3:
        if (theSelector)
          tempValue = IntegratedHessaa(theSelector);
        else
          tempValue = ToHessaa();
        if (tempValue != kUnsignedLongMissing)
          returnValue = tempValue;
        break;
      case kFmiWeatherSymbol1:  // hsade
        if (theSelector)
          tempValue = IntegratedHsade(theSelector);
        else
          tempValue = ToHsade();
        if (tempValue != kUnsignedLongMissing)
          returnValue = tempValue;
        break;
      case kFmiMiddleAndLowCloudCover:  // ala- ja keskipilvet
        if (theSelector)
          tempValue = IntegratedSubValue(theParam, theSelector);
        else
          tempValue = LowAndMiddleClouds();
        if (tempValue != kUnsignedLongMissing)
          returnValue = tempValue;
        break;
      default:
        break;
    }
    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiWeatherAndCloudiness::ThunderProbabilityValue() const
{
  try
  {
    unsigned long tempValue = ThunderProbability();
    if (tempValue != kTThunderProbabilityMissing && tempValue != kUnsignedLongMissing)
      return 10 * tempValue;

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiWeatherAndCloudiness::TotalCloudinessValue() const
{
  try
  {
    unsigned long tempValue = TotalCloudiness();
    if (tempValue != kT4BitMissing && tempValue != kUnsignedLongMissing)
      return 10 * tempValue;

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiWeatherAndCloudiness::Precipitation1hValue() const
{
  try
  {
    unsigned long tempValue = TotalPrecipitation();
    return PrecipitationScale(tempValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiWeatherAndCloudiness::RawSubValue(FmiParameterName theParam)
{
  try
  {
    unsigned long tempValue = 0;
    double returnValue = kFloatMissing;
    switch (theParam)
    {
      case kFmiTotalCloudCover:
        tempValue = TotalCloudiness();
        if (tempValue != kT4BitMissing)
          returnValue = tempValue;
        break;
      case kFmiLowCloudCover:
        tempValue = LowClouds();
        if (tempValue != kT4BitMissing)
          returnValue = tempValue;
        break;
      case kFmiMediumCloudCover:
        tempValue = MiddleClouds();
        if (tempValue != kT4BitMissing)
          returnValue = tempValue;
        break;
      case kFmiHighCloudCover:
        tempValue = HighClouds();
        if (tempValue != HighCloudsMissingValue())
          returnValue = tempValue;
        break;
      case kFmiPrecipitation1h:
        tempValue = TotalPrecipitation();
        if (tempValue != TotalPrecipitationMissingValue())
          returnValue = tempValue;
        break;
      case kFmiPrecipitationType:
        tempValue = PrecipitationType();
        if (tempValue != kT2BitMissing)
          returnValue = tempValue;
        break;
      case kFmiPrecipitationForm:
        tempValue = PrecipitationForm();
        if (tempValue != kT3BitMissing)
          returnValue = tempValue;
        break;
      case kFmiProbabilityThunderstorm:
        tempValue = ThunderProbability();
        if (tempValue != kTThunderProbabilityMissing)
          returnValue = tempValue;
        break;
      case kFmiFogIntensity:
        tempValue = FogIntensity();
        if (tempValue != kT2BitMissing)
          returnValue = tempValue;
        break;
      case kFmiWeatherSymbol3:
        tempValue = ToHessaa();
        if (tempValue != kUnsignedLongMissing)
          returnValue = tempValue;
        break;
      case kFmiWeatherSymbol1:  // hsade
        tempValue = ToHsade();
        if (tempValue != kUnsignedLongMissing)
          returnValue = tempValue;
        break;
      case kFmiMiddleAndLowCloudCover:  // ala- ja keskipilvet
        tempValue = LowAndMiddleClouds();
        if (tempValue != kUnsignedLongMissing)
          returnValue = tempValue;
        break;
      default:
        break;
    }
    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsPrecipitationOk() const
{
  try
  {
    if (!DataOk())
      return false;

    if (TotalPrecipitation() != TotalPrecipitationMissingValue())
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param precForm Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsFair(float precForm)
{
  try
  {
    return precForm == kTNoPrecipitation;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param precForm Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsLiquid(float precForm)
{
  try
  {
    return (precForm == kTRain || precForm == kTDrizzle || precForm == kTFreezingDrizzle ||
            precForm == kTFreezingRain);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param precForm Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsHalfLiquid(float precForm)
{
  try
  {
    return precForm == kTSleet;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param precForm Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsSolid(float precForm)
{
  try
  {
    return precForm == kTSnow || precForm == kTHail;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param precForm Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsFreezing(float precForm)
{
  try
  {
    return precForm == kTFreezingDrizzle || precForm == kTFreezingRain;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsFair() const
{
  try
  {
    return (CalcTotalPrecipitation() == kTNoPrecipitation);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::OnlySomePrecipitation() const
{
  try
  {
    return TotalPrecipitation() == kTSomePrecipitation;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsSnow() const
{
  try
  {
    //  Huom! Tässä tutkitaan vain sateen olomuoto ja sademäärä
    return PrecipitationForm() == kTSnow && !IsFair();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsSleet() const
{
  try
  {
    //  Huom! Tässä tutkitaan vain sateen olomuoto ja sademäärä
    return PrecipitationForm() == kTSleet && !IsFair();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsRain() const
{
  try
  {
    //  Huom! Tässä tutkitaan vain sateen olomuoto ja sademäärä
    return PrecipitationForm() == kTRain && !IsFair();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsDrizzle() const
{
  try
  {
    //  Huom! Tässä tutkitaan vain sateen olomuoto ja sademäärä
    return PrecipitationForm() == kTDrizzle && !IsFair();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsFreezingDrizzle() const
{
  try
  {
    //  Huom! Tässä tutkitaan vain sateen olomuoto ja sademäärä
    return PrecipitationForm() == kTFreezingDrizzle && !IsFair();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsFreezingRain() const
{
  try
  {
    //  Huom! Tässä tutkitaan vain sateen olomuoto ja sademäärä
    return PrecipitationForm() == kTFreezingRain && !IsFair();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsHail() const
{
  try
  {
    //  Huom! Tässä tutkitaan vain sateen olomuoto ja sademäärä
    return PrecipitationForm() == kTHail && !IsFair();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsConvective() const
{
  try
  {
    //  Huom! Tässä tutkitaan vain sateen tyyppi ja sademäärä
    return PrecipitationType() == kTConvectivePrecipitation && !IsFair();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsLargeScale() const
{
  try
  {
    //  Huom! Tässä tutkitaan vain sateen tyyppi ja sademäärä
    return PrecipitationType() == kTLargeScalePrecipitation && !IsFair();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::IsThunder() const
{
  try
  {
    return (ThunderProbability() != kTThunderProbabilityMissing && ThunderProbability() >= 4);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::CalcTotalCloudiness()
{
  try
  {
    unsigned long N = 0;
    if (TotalCloudiness() == kT4BitMissing && MiddleClouds() == kT4BitMissing &&
        HighClouds() == HighCloudsMissingValue() && LowClouds() == kT4BitMissing)
      N = kT4BitMissing;
    else
    {
      if (TotalCloudiness() != kT4BitMissing)
        N = TotalCloudiness();
      if (LowClouds() != kT4BitMissing && N < LowClouds())
        N = LowClouds();
      if (MiddleClouds() != kT4BitMissing && N < MiddleClouds())
        N = MiddleClouds();
      if (HighClouds() != HighCloudsMissingValue() && N < HighClouds())
        N = HighClouds();
    }
    return N;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::PrecipitationScaleV6(double theValue) const
{
  try
  {
    if (theValue == kFloatMissing)
      return kUnsignedLongMissing;
    if (theValue < 0.03)
      return 0;

    // Marko lisäsi tämän, ettei sateet haihdu esim. tasoittaessa tai
    // interpoloitaessa tiheämpään aikaan
    if (theValue >= 0.03 && theValue < gMinimumPrecipitationForV6)
      return 1;

    if (theValue > 100)
      theValue = 100;
    return static_cast<unsigned long>(round(4 * log(theValue) + 10));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiWeatherAndCloudiness::PrecipitationScaleV7(double theValue) const
{
  try
  {
    static const double a = 0.1343;
    static const double b = 20. / log(1. / a + 1.);

    if (theValue == kFloatMissing)
      return kUnsignedLongMissing;
    if (theValue < gMinimumPrecipitationForV7)
      return 0;  // 0.015 on pienin mahd. arvo
    if (theValue > 100)
      theValue = 100;

    return static_cast<unsigned long>(round(b * log(theValue / a + 1)));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiWeatherAndCloudiness::PrecipitationScaleV6(unsigned long theValue) const
{
  try
  {
    if (theValue == TotalPrecipitationMissingValue())
      return kFloatMissing;
    if (theValue < 0.1)
      return 0;
    double temp = exp((static_cast<double>(theValue) - 10) / 4);
    return temp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiWeatherAndCloudiness::PrecipitationScaleV7(unsigned long theValue) const
{
  try
  {
    static const double a = 0.1343;
    static const double b = 20. / log(1. / a + 1.);

    if (theValue == TotalPrecipitationMissingValue())
      return kFloatMissing;
    if (theValue == 0)
      return 0;
    double rr = a * (exp(static_cast<double>(theValue) / b) - 1);
    return rr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::ThunderProbability(unsigned long theValue)
{
  try
  {
    if (theValue != kT4BitMissing)
    {
      if (theValue > 10)
        theValue = 10;
      SetThunderProbability(theValue);
      return true;
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::TotalCloudiness(unsigned long theValue)
{
  try
  {
    if (theValue != kT4BitMissing)
    {
      if (theValue > 10)
        theValue = 10;
      SetTotalCloudiness(theValue);
      if (fAutoUpdate)
      {
        if (TotalCloudiness() == 0)
        {
          SetTotalPrecipitation(kTNoPrecipitation);
          CheckIfPrecipZeroAndCleanTypeAndForm();
        }
        // Marko Lisäsin säännön, että jos asetetaan kokonaispilvisyyttä,
        // mikään osa pilvisyys ei voi olla yli kokonaispilvisyyden
        if (LowClouds() != kT4BitMissing && LowClouds() > theValue)
          SetLowClouds(theValue);
        if (MiddleClouds() != kT4BitMissing && MiddleClouds() > theValue)
          SetMiddleClouds(theValue);
        if (HighClouds() != HighCloudsMissingValue() && HighClouds() > theValue)
          SetHighClouds(theValue);
      }
      return true;
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::LowClouds(unsigned long theValue)
{
  try
  {
    if (theValue != kT4BitMissing)
    {
      if (theValue > 10)
        theValue = 10;
      SetLowClouds(theValue);
      //	  SetTotalCloudiness(0); // tämä aiheutti ongelmi fqgV7ToV6-filtterin kanssa, joten
      // laitoin
      // kaikkien kerrosten asetuksissa N:n nollauksen pois
      SetTotalCloudiness(CalcTotalCloudiness());
      return true;
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::MiddleClouds(unsigned long theValue)
{
  try
  {
    if (theValue != kT4BitMissing)
    {
      if (theValue > 10)
        theValue = 10;
      SetMiddleClouds(theValue);
      //	  SetTotalCloudiness(0); // tämä aiheutti ongelmi fqgV7ToV6-filtterin kanssa, joten
      // laitoin
      // kaikkien kerrosten asetuksissa N:n nollauksen pois
      SetTotalCloudiness(CalcTotalCloudiness());
      return true;
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::HighClouds(unsigned long theValue)
{
  try
  {
    if (theValue != HighCloudsMissingValue())
    {
      unsigned long maxHighCloudValue = (itsInfoVersion >= 7.) ? 6 : 10;
      if (theValue > maxHighCloudValue)
        theValue = maxHighCloudValue;
      SetHighClouds(theValue);
      //	  SetTotalCloudiness(0); // tämä aiheutti ongelmi fqgV7ToV6-filtterin kanssa, joten
      // laitoin
      // kaikkien kerrosten asetuksissa N:n nollauksen pois
      SetTotalCloudiness(CalcTotalCloudiness());
      return true;
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::TotalPrecipitation(unsigned long theValue)
{
  try
  {
    bool newRain = (TotalPrecipitation() == 0) && (theValue > 0) &&
                   (theValue != TotalPrecipitationMissingValue());
    // täällä on käytetty fAutoUpdatea
    if (theValue != TotalPrecipitationMissingValue())
    {
      SetTotalPrecipitation(theValue);
      CheckIfPrecipZeroAndCleanTypeAndForm();  // Onko tämä oikea kohta?????
      if (TotalPrecipitation() == 0 && PrecipitationType() == kT2BitMissing &&
          PrecipitationForm() == kT3BitMissing)  // ei sada
        fDataOk = true;
      else if (TotalPrecipitation() > 0 && TotalCloudiness() > 0 &&
               PrecipitationType() != kT2BitMissing &&
               PrecipitationForm() != kT3BitMissing)  // sataa
        fDataOk = true;                               // autoUpdatea ei tarvita

      else if (fAutoUpdate)  // tarvittaessa päivitetään muita parametreja
      {
        if (TotalPrecipitation() > 0)
        {
          if (TotalCloudiness() == 0)
          {
            // Huom! 1.2.2000/Marko Laitoin säätömahdollisuuden kun luodaan sadetta,
            // tuleeko pilvistä puolipilvistä vai täyspilveä ja tuleeko sateesta kuuroa vai tasaista
            if (PrecipitationType() == kT2BitMissing)  // ei ole tyyppiä
            {
              if (fDefaultPrecipitationTypeIsLargeScale)
                SetPrecipitationType(kTLargeScalePrecipitation);
              else
                SetPrecipitationType(kTConvectivePrecipitation);
            }
            if (PrecipitationForm() == kT3BitMissing)  // ei ole olomuotoa
            {
              if (itsTemperature > 0)  // puuttuva T -> vettä
                SetPrecipitationForm(kTRain);
              if (itsTemperature <= 0)
                SetPrecipitationForm(kTSnow);
            }
            if (fDefaultPrecipitationTypeIsLargeScale)
              SetTotalCloudiness(10);
            else
              SetTotalCloudiness(5);
          }

          if (TotalCloudiness() >= 8)  // täyspilvistä
          {
            if (PrecipitationType() == kT2BitMissing)  // ei ole tyyppiä
              SetPrecipitationType(kTLargeScalePrecipitation);
            if (PrecipitationForm() == kT3BitMissing)  // oletus vesisade
            {
              if (itsTemperature > 0)
                SetPrecipitationForm(kTRain);
              if (itsTemperature <= 0)
                SetPrecipitationForm(kTSnow);
            }
          }

          else if (TotalCloudiness() < 8 && TotalCloudiness() > 0)
          {
            if (PrecipitationType() == kT2BitMissing)  // ei ole tyyppiä
              SetPrecipitationType(kTConvectivePrecipitation);
            if (PrecipitationForm() == kT3BitMissing)  // oletus vesisade
            {
              if (itsTemperature > 0)
                SetPrecipitationForm(kTRain);
              if (itsTemperature <= 0)
                SetPrecipitationForm(kTSnow);
            }

            // Marko Laitoin tämän , jottä pilviä saadaan lisää kun
            // sadetta lisätään jos N alle 50 tai 100% riippuen asetuksesta
            if (newRain)  // HUOM! nyt vain uuden sateen tapuksessa, mutta ei esim. jos orig sade
                          // oli puuttuvaa eli ollaan esim. interpolointia tekemässä
            {
              if (fDefaultPrecipitationTypeIsLargeScale)
                SetTotalCloudiness(FmiMax(10ul, TotalCloudiness()));
              else
                SetTotalCloudiness(FmiMax(5ul, TotalCloudiness()));
            }
          }
        }
        fDataOk = true;
      }
      else
        fDataOk = false;

      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::PrecipitationType(unsigned long theValue)
{
  try
  {
    if (theValue != kT2BitMissing)
    {
      if (theValue < 1)
        theValue = 1;
      else if (theValue > 2)
        theValue = 2;
      SetPrecipitationType(theValue);

      if (TotalPrecipitation() == TotalPrecipitationMissingValue())
      {
        // jos ei ole sateen intensiteettiä
        fDataOk = false;
        return true;
      }
      // on sadetta

      if (fAutoUpdate)
      {
        // oletustyyppi on vettä
        if (TotalCloudiness() > 0 && PrecipitationForm() == kT3BitMissing)
        {
          if (itsTemperature > 0)
            SetPrecipitationForm(kTRain);
          if (itsTemperature <= 0)
            SetPrecipitationForm(kTSnow);
          fDataOk = true;
        }
        else if (TotalCloudiness() == 0)
          fDataOk = false;  // tässä ei arvata pilvien määrää
      }
      else
        fDataOk = false;

      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::PrecipitationForm(unsigned long theValue)
{
  try
  {
    if (theValue != kT3BitMissing)
    {
      if (theValue > 6)
        theValue = 6;
      SetPrecipitationForm(theValue);  // olomuodolla on nyt arvo

      if (TotalPrecipitation() == TotalPrecipitationMissingValue() || TotalPrecipitation() == 0)
      {
        // jos ei ole sateen intensitettiä
        fDataOk = false;
        return true;
      }
      // on sadetta
      if (PrecipitationType() != kT2BitMissing)  // kokeilulisäys
      {
        fDataOk = true;
        return true;
      }

      if (fAutoUpdate)
      {
        if (PrecipitationType() == kT2BitMissing && TotalCloudiness() >= 8)  // täyspilvistä
          SetPrecipitationType(kTLargeScalePrecipitation);
        else if (PrecipitationType() == kT2BitMissing && TotalCloudiness() > 0 &&
                 TotalCloudiness() < 8)  // vain osa taivaasta pilvessä
          SetPrecipitationType(kTConvectivePrecipitation);
        // onko järkevää lisätä pilviä??
        else if (TotalCloudiness() == 0 && PrecipitationType() == kTLargeScalePrecipitation)
          SetTotalCloudiness(10);
        else if (TotalCloudiness() == 0 && PrecipitationType() == kTConvectivePrecipitation)
          SetTotalCloudiness(5);
        else
        {
          fDataOk = false;
          return true;
        }
        fDataOk = true;
      }
      else
        fDataOk = false;

      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::FogIntensity(unsigned long theValue)
{
  try
  {
    if (theValue != kT2BitMissing)
    {
      if (theValue > 2)
        theValue = 2;

      SetFogIntensity(theValue);
      fDataOk = true;
      return true;
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiWeatherAndCloudiness::LongValue(unsigned long theValue)
{
  try
  {
    if (theValue != kUnsignedLongMissing)
    {
      itsData.longType = theValue;
      fDataOk = true;
      return true;
    }

    itsData.longType = kTCombinedWeatherMissing;
    fDataOk = true;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 */
// ----------------------------------------------------------------------

// weatherandcloudiness-luokan ristiin tarkastukset pitäisi laittaa kaikki
// tänne yhteen paikkaa. Parametrina annetaan muokattu parametri ja
// sitten tehdään tarvittavat muutokset muihin parametreihin kollektiivisesti
// täällä. Kutsutaan SubParam()-metodin lopuksi.
// HUOM!! Pitää tehdä sellainen case, missä tehdään kaikki mahdolliset
// tarkastelut (esim. konstruktori kutsuu).
// HUOM!! Ei toimi vielä kuin kokonaispilvisyyden nollaus tapauksissa.
// HUOM!! Kun tämä saadaan kuntoon, voidaan erilliset viritykset
// eri parametrien asetusten yhteydessä poistaa.

void NFmiWeatherAndCloudiness::CrossCheck(FmiParameterName theParam)
{
  try
  {
    switch (theParam)
    {
      case kFmiTotalCloudCover:
      case kFmiLowCloudCover:
      case kFmiMediumCloudCover:
      case kFmiHighCloudCover:
        if (TotalCloudiness() == 0)  // hätä viritys tässä vaiheessa
        {
          SetLowClouds(0);
          SetMiddleClouds(0);
          SetHighClouds(0);
          SetTotalPrecipitation(0);
          CheckIfPrecipZeroAndCleanTypeAndForm();
        }
        break;
      default:
        break;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiWeatherAndCloudiness::CheckIfPrecipZeroAndCleanTypeAndForm()
{
  try
  {
    if (TotalPrecipitation() == 0)
    {
      this->SetPrecipitationType(kT2BitMissing);
      this->SetPrecipitationForm(kT3BitMissing);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiWeatherAndCloudiness::CreateIntegrators()
{
  try
  {
    if (!itsSubParams)
      return;

    itsIntegrators = new NFmiDataModifierCombi *[itsSubParams->GetSize()];

    SetSubIntegrator(kFmiTotalCloudCover, new NFmiDataModifierCombi(11, 100.));
    SetSubIntegrator(kFmiLowCloudCover, new NFmiDataModifierCombi(11, 100.));
    SetSubIntegrator(kFmiMediumCloudCover, new NFmiDataModifierCombi(11, 100));
    SetSubIntegrator(kFmiHighCloudCover, new NFmiDataModifierCombi(11, 100.));
    SetSubIntegrator(kFmiPrecipitation1h, new NFmiDataModifierCombi(31, 100.));
    SetSubIntegrator(kFmiPrecipitationForm, new NFmiDataModifierCombi(6, 100.));
    SetSubIntegrator(kFmiPrecipitationType, new NFmiDataModifierCombi(2, 100.));
    SetSubIntegrator(kFmiProbabilityThunderstorm, new NFmiDataModifierCombi(11, 1000.));
    SetSubIntegrator(kFmiFogIntensity, new NFmiDataModifierCombi(4, 100.));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theInitValue Undocumented
 * \return Undocumented
 * \todo Kill the cat
 */
// ----------------------------------------------------------------------

NFmiCombinedParam *NFmiWeatherAndCloudiness::CreateNew(float theInitValue)
{
  try
  {
    auto *kisu = new NFmiWeatherAndCloudiness(*this);
    kisu->LongValue(ConvertFloatToLong(theInitValue));
    return kisu;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiWeatherAndCloudiness::CreateSubParams()
{
  try
  {
    NFmiDataIdent subDataIdent[9];

    NFmiParam param;
    NFmiProducer producer;

    param = NFmiParam(kFmiTotalCloudCover, "Total Cloud Cover", 0, 100, 2, 1, "%.1f", kLinearly);
    subDataIdent[0] = NFmiDataIdent(param, producer, kContinuousParam);
    param = NFmiParam(kFmiLowCloudCover, "Low Cloud Cover", 0, 100, 2, 1, "%.1f", kLinearly);
    subDataIdent[1] = NFmiDataIdent(param, producer, kContinuousParam);
    param = NFmiParam(kFmiMediumCloudCover, "Medium Cloud Cover", 0, 100, 2, 1, "%.1f", kLinearly);
    subDataIdent[2] = NFmiDataIdent(param, producer, kContinuousParam);
    param = NFmiParam(kFmiHighCloudCover, "High Cloud Cover", 0, 100, 2, 1, "%.1f", kLinearly);
    subDataIdent[3] = NFmiDataIdent(param, producer, kContinuousParam);
    param = NFmiParam(kFmiPrecipitation1h, "Precipitation mm/h", 0, 98, 2, 1, "%.1f", kLinearly);
    subDataIdent[4] = NFmiDataIdent(param, producer, kContinuousParam);
    param =
        NFmiParam(kFmiPrecipitationForm, "Precipitation Form", 0, 4, 2, 1, "%.1f", kNearestPoint);
    subDataIdent[5] = NFmiDataIdent(param, producer, kNumberParam);
    param =
        NFmiParam(kFmiPrecipitationType, "Precipitation Type", 0, 1, 2, 1, "%.1f", kNearestPoint);
    subDataIdent[6] = NFmiDataIdent(param, producer, kNumberParam);
    param = NFmiParam(
        kFmiProbabilityThunderstorm, "Probability of Thunder", 0, 100, 2, 1, "%.1f", kLinearly);
    subDataIdent[7] = NFmiDataIdent(param, producer, kContinuousParam);
    param = NFmiParam(kFmiFogIntensity, "Density of Fog", 0, 2, 2, 1, "%.1f", kNearestPoint);
    subDataIdent[8] = NFmiDataIdent(param, producer, kNumberParam);

    itsSubParams = new NFmiParamBag(subDataIdent, 9);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param firstLimit Undocumented
 * \param secondLimit Undocumented
 */
// ----------------------------------------------------------------------

void NFmiWeatherAndCloudiness::SetPrecipitationProbabilityLimits(float firstLimit,
                                                                 float secondLimit)
{
  try
  {
    itsPrecipitationProbabilityLimit1 = firstLimit;
    itsPrecipitationProbabilityLimit2 = secondLimit;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
