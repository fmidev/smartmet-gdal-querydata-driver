// ======================================================================
/*!
 * \file NFmiTotalWind.cpp
 * \brief Implementation of class NFmiTotalWind
 */
// ======================================================================
/*!
 * \class NFmiTotalWind
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiTotalWind.h"
#include "NFmiAngle.h"
#include "NFmiDataIdent.h"
#include "NFmiInterpolation.h"
#include "NFmiParamBag.h"
#include <macgyver/Exception.h>

#include <algorithm>
#include <cmath>  // Mika: abs()

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------
NFmiTotalWind::~NFmiTotalWind() = default;
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theWind The object being copied
 */
// ----------------------------------------------------------------------

NFmiTotalWind::NFmiTotalWind(const NFmiTotalWind &theWind)
    : NFmiCombinedParam(theWind), itsData(theWind.itsData), itsPrecision()
{
}

NFmiTotalWind& NFmiTotalWind::operator = (const NFmiTotalWind&) = default;

// ----------------------------------------------------------------------
/*!
 * Apufunktio laskemaan 2-4 luvun painotettu keskiarvo
 *
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

static double WeightedMean(double missingValue,
                           double value1,
                           float factor1,
                           double value2,
                           float factor2,
                           double value3,
                           float factor3,
                           double value4,
                           float factor4)
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
      return sum / facSum;

    return 0;
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
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiTotalWind::NFmiTotalWind(double theInfoVersion)
    : NFmiCombinedParam(theInfoVersion), itsData(), itsPrecision()
{
  try
  {
    itsData.longType = kTCombinedWeatherMissing;
    SetWindDirection(kT6BitMissing);
    SetWindSpeed(WindSpeedMissingValue());  // Marko testi, toimii sekä versiolle 6 että 7!!!
    SetWindGust(WindGustMissingValue());
    fDataOk = true;
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
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiTotalWind::NFmiTotalWind(unsigned long theValue,
                             FmiWindParamType theParamType,
                             double theInfoVersion,
                             float theWindGustValue)
    : NFmiCombinedParam(theInfoVersion), itsData(), itsPrecision()
{
  try
  {
    itsData.longType = kTCombinedWeatherMissing;
    if (theParamType == kFmiPackedWind)
    {
      // huom! puuskaa ei aseteta tässä tapauksessa, koska se tulee pakattuna arvona!!!
      itsData.longType = theValue;
      fDataOk = true;
    }
    else
    {
      FromWindVector(theValue);
      SetWindGust(WindGustVx(theWindGustValue));  // jos parametri on windvector, laitetaan mahd.
                                                  // puuska arvo myös kohdalleen
    }
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
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiTotalWind::NFmiTotalWind(float theValue,
                             FmiWindParamType theParamType,
                             double theInfoVersion,
                             float theWindGustValue)
    : NFmiCombinedParam(theInfoVersion), itsData(), itsPrecision()
{
  try
  {
    itsData.longType = kTCombinedWeatherMissing;
    if (theParamType == kFmiPackedWind)
    {
      // huom! puuskaa ei aseteta tässä tapauksessa, koska se tulee pakattuna arvona!!!
      itsData.longType = ConvertFloatToLong(theValue);
      fDataOk = true;
    }
    else
    {
      FromWindVector(static_cast<long unsigned int>(theValue));
      SetWindGust(WindGustVx(theWindGustValue));  // jos parametri on windvector, laitetaan mahd.
                                                  // puuska arvo myös kohdalleen
    }
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
 * \param theFirstValue Undocumented
 * \param theSecondValue Undocumented
 * \param theParamType Undocumented
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiTotalWind::NFmiTotalWind(float theFirstValue,
                             float theSecondValue,
                             FmiWindParamType theParamType,
                             double theInfoVersion,
                             float theWindGustValue)
    : NFmiCombinedParam(theInfoVersion), itsData(), itsPrecision()
{
  try
  {
    if (theParamType == kFmiDirectionAndSpeed)
      SetFromDirectionAndSpeed(theFirstValue, theSecondValue);
    else if (theParamType == kFmiUVComponents)
      SetFromUVComponents(
          theFirstValue, theSecondValue, true);  // true=aseta mm. myös gustindex puuttuvaksi
    SetWindGust(WindGustVx(theWindGustValue));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theDirection Undocumented
 * \param theSpeed Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTotalWind::SetFromDirectionAndSpeed(float theDirection, float theSpeed)
{
  try
  {
    itsData.longType = kTCombinedWeatherMissing;
    fDataOk = true;  // jos molemmat puuttuu
    if (theDirection != kFloatMissing)
    {
      if (theDirection == 999)
        SetWindDirection(kTVariableWind);
      else
        SetWindDirection(static_cast<int>(round(theDirection / 10.0)) % 36);

      if (theSpeed != kFloatMissing)
        SetWindSpeedFromRealValue(theSpeed);
      else
        fDataOk = false;  // vaan toinen on puuttuva
    }
    else if (theSpeed != kFloatMissing)
    {
      SetWindSpeedFromRealValue(theSpeed);  // Marko testi
      fDataOk = false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theUValue Undocumented
 * \param theVValue Undocumented
 * \param fSetValuesMissingFirst Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTotalWind::SetFromUVComponents(float theUValue,
                                        float theVValue,
                                        bool fSetValuesMissingFirst)
{
  try
  {
    if (fSetValuesMissingFirst)
      itsData.longType = kTCombinedWeatherMissing;
    NFmiWindDirection theDirection(theUValue, theVValue);
    if (theDirection.Value() != kFloatMissing)
    {
      SetWindDirection(static_cast<unsigned long>(round(theDirection.Value() / 10.0)));
      SetWindSpeedFromRealValue(sqrt(theUValue * theUValue + theVValue * theVValue));

      fDataOk = true;
    }
    else
    {
      SetWindDirection(kT6BitMissing);
      SetWindSpeed(WindSpeedMissingValue());
      fDataOk = false;
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

bool NFmiTotalWind::CheckWindVector(unsigned long theValue)
{
  try
  {
    bool retVal = false;
    if (theValue != kUnsignedLongMissing && theValue != kFloatMissing)
    {
      unsigned long maxWindSpeedValue = (itsInfoVersion >= 7.) ? 4094 : 500;
      if (theValue % 100 <= 36)
      {
        if (theValue / 100 <= maxWindSpeedValue)  // nopeus
          retVal = true;
      }
      else if (theValue % 100 == 62 && theValue / 100 <= maxWindSpeedValue)  // vaihtelevaa tuulta
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
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTotalWind::FromWindVector(unsigned long theValue)
{
  try
  {
    if (!CheckWindVector(theValue))
    {
      itsData.longType = kTCombinedWeatherMissing;
      fDataOk = true;
    }
    else
    {
      itsData.longType = kTCombinedWeatherMissing;
      unsigned int speed = WindSpeedFromWindVector(theValue);
      unsigned int direction = WindDirectionFromWindVector(theValue);
      SetWindSpeedFromRealValue(speed);
      SetWindDirection(direction);
      if ((speed != WindSpeedMissingValue() && direction != kT6BitMissing) ||
          (speed == WindSpeedMissingValue() && direction == kT6BitMissing))
        fDataOk = true;
      else
        fDataOk = false;
    }
    return fDataOk;
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

unsigned long NFmiTotalWind::ToWindVector()
{
  try
  {
    unsigned long windVector = kUnsignedLongMissing;
    if (itsData.longType == kTCombinedWeatherMissing)
      return windVector;
    auto speed = static_cast<unsigned long>(round(WindSpeedVx(WindSpeed())));
    unsigned long dir = WindDirection();
    if (speed != kFloatMissing && dir != kT6BitMissing)
      windVector = speed * 100 + (dir);
    return windVector;
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

unsigned long NFmiTotalWind::WindSpeedFromWindVector(unsigned long theValue)
{
  try
  {
    int speed = WindSpeedMissingValue();
    if (theValue != kUnsignedLongMissing && theValue != kFloatMissing)
      speed = (static_cast<int>(theValue)) / 100;

    return speed;
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

bool NFmiTotalWind::IsMemberParam(FmiParameterName type) const
{
  try
  {
    switch (type)
    {
      // onko tässä kaikki
      case kFmiWindDirection:
      case kFmiWindSpeedMS:
      case kFmiWindVectorMS:
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
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiTotalWind::WindDirectionFromWindVector(unsigned long theValue)
{
  try
  {
    int dir = kT6BitMissing;
    if (theValue != kUnsignedLongMissing && theValue != kFloatMissing && theValue != 999)
    {
      dir = (static_cast<int>(theValue)) % 100;
    }
    else if (theValue == 999)
      dir = kTVariableWind;

    return dir;
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

bool NFmiTotalWind::SubValue(double theValue, FmiParameterName theParam)
{
  try
  {
    bool returnVal = false;
    switch (theParam)
    {
      case kFmiWindDirection:
        // alle 0 ja yli 360 arvot huolehditaan WindDirection(double):ssa
        if (theValue == 999)
          returnVal = WindDirection(kTVariableWind);
        else if (theValue == kFloatMissing)
          returnVal = WindDirection(static_cast<unsigned long>(theValue));
        else
          returnVal = WindDirection(static_cast<unsigned long>(round(theValue / 10.0)));
        break;
      case kFmiWindSpeedMS:
      {
        double maxWindSpeedValue = (itsInfoVersion >= 7.) ? 409.4 : 500.;
        if (theValue != kFloatMissing)
          theValue = FmiMax(0., FmiMin(maxWindSpeedValue, theValue));
        returnVal = WindSpeed(WindSpeedVx(theValue));
        break;
      }
      case kFmiHourlyMaximumGust:
      {
        double maxGustSpeedValue = (itsInfoVersion >= 7.) ? 409.4 : 14.;
        if (theValue != kFloatMissing)
          theValue = FmiMax(0., FmiMin(maxGustSpeedValue, theValue));
        returnVal = WindGust(WindGustVx(theValue));
        break;
      }
      case kFmiWindVectorMS:
        returnVal = FromWindVector(static_cast<unsigned long>(FmiMax(0.0, theValue)));
        break;
      default:
        break;
    }
    return returnVal;
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

double NFmiTotalWind::SubValue(FmiParameterName theParam,
                               NFmiIntegrationSelector * /* theSelector */)
{
  try
  {
    // palauttaa kFloatMissing, jos arvo on puuttuva
    double returnVal = kFloatMissing;
    unsigned long tempValue = 0;
    switch (theParam)
    {
      case kFmiWindDirection:
        returnVal = WindDirectionValue();
        break;
      case kFmiWindSpeedMS:
        returnVal = WindSpeedVx(WindSpeed());
        break;
      case kFmiHourlyMaximumGust:
        returnVal = WindGustVx(WindGust());
        break;
      case kFmiWindVectorMS:
        tempValue = ToWindVector();
        if (tempValue != kUnsignedLongMissing)
          returnVal = tempValue;
        break;
      case kFmiWindUMS:
        returnVal = CalcU();
        break;
      case kFmiWindVMS:
        returnVal = CalcV();
        break;
      default:
        break;
    }
    return returnVal;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Laskee raaka biteistä suuntaa ja konvertoi lopulliseen aste arvoon
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiTotalWind::WindDirectionValue()
{
  try
  {
    double returnVal = kFloatMissing;
    unsigned long tempValue = WindDirection();
    if (tempValue != kT6BitMissing && tempValue != kTVariableWind)
    {
      if (tempValue > 36)
        tempValue = tempValue % 36;
      returnVal = tempValue * 10;
      if (returnVal == 0 && WindSpeed() != 0)
        returnVal = 360;
    }
    else if (tempValue == kTVariableWind)
      returnVal = 999;  // tuuli on vaihtelevaa
    return returnVal;
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

double NFmiTotalWind::CalcU()
{
  try
  {
    double value = kFloatMissing;
    double WS = WindSpeedVx(WindSpeed());
    auto WD = static_cast<int>(WindDirectionValue());
    // jos tuulensuunta on vaihtelevaa (999), palautetaan 0 arvo (voisi olla myös puuttuvaa)
    if (WD == 999)
      return 0;

    if (WS != kFloatMissing && WD != kFloatMissing)
    {
      // huom! tuulen suunta pitää ensin kääntää 180 astetta ja sitten
      // muuttaa radiaaneiksi kulma/360 * 2*pii
      value = WS * sin((((180 + WD) % 360) / 360.) * (2. * kPii));
    }
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Lasketaan myös tuuli komponentit u ja v
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiTotalWind::CalcV()
{
  try
  {
    double value = kFloatMissing;
    double WS = WindSpeedVx(WindSpeed());
    auto WD = static_cast<int>(WindDirectionValue());
    // jos tuulensuunta on vaihtelevaa (999), palautetaan 0 arvo (voisi olla myös puuttuvaa)
    if (WD == 999)
      return 0;

    if (WS != kFloatMissing && WD != kFloatMissing)
    {
      // huom! tuulen suunta pitää ensin kääntää 180 astetta ja sitten
      // muuttaa radiaaneiksi kulma/360 * 2*pii
      value = WS * cos((((180 + WD) % 360) / 360.) * (2. * kPii));
    }
    return value;
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

double NFmiTotalWind::RawSubValue(FmiParameterName theParam)
{
  try
  {
    // Persa lisäsi tämän, vaan ei kirjoittanut vielä sisältöä
    // Tällä hetkellä toimii aivan samoin kuin SubValue

    // palauttaa kFloatMissing, jos arvo on puuttuva
    double returnVal = kFloatMissing;
    unsigned long tempValue = 0;
    switch (theParam)
    {
      case kFmiWindDirection:
        tempValue = WindDirection();
        if (tempValue != kT6BitMissing && tempValue != kTVariableWind)
        {
          if (tempValue > 36)
            tempValue = tempValue % 36;
          returnVal = tempValue * 10;
          if (returnVal == 0 && WindSpeed() != 0)
            returnVal = 360;
        }
        else if (tempValue == kTVariableWind)
          returnVal = 999;  // tuuli on vaihtelevaa
        break;
      case kFmiWindSpeedMS:
        tempValue = WindSpeed();
        if (tempValue != WindSpeedMissingValue())
          returnVal = tempValue;
        break;
      case kFmiHourlyMaximumGust:
        tempValue = WindGust();
        if (tempValue != WindGustMissingValue())
          returnVal = tempValue;
        break;
      case kFmiWindVectorMS:
        tempValue = ToWindVector();
        if (tempValue != kUnsignedLongMissing)
          returnVal = tempValue;
        break;
      default:
        break;
    }
    return returnVal;
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

NFmiDataIdent *NFmiTotalWind::CreateParam(const NFmiProducer &theProducer,
                                          NFmiVoidPtrList *theSecondaryProducerList)
{
  try
  {
    NFmiParam param;
    NFmiParamBag subParamBag;

    // lisää tänne gustit ym!
    unsigned long maxWindSpeedValue = (itsInfoVersion >= 7.) ? 4094 : 500;
    unsigned long maxGustSpeedValue = (itsInfoVersion >= 7.) ? 4094 : 14;
    param = NFmiParam(kFmiWindSpeedMS, "Wind speed", 0, maxWindSpeedValue, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param = NFmiParam(kFmiWindDirection, "Wind dir", 0, 360, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param = NFmiParam(
        kFmiWindVectorMS, "Wind vector", kFloatMissing, kFloatMissing, 1, 0, "%.1f", kNearestPoint);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kNumberParam));

    param = NFmiParam(
        kFmiHourlyMaximumGust, "MaxGustMS", 0, maxGustSpeedValue, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param = NFmiParam(kFmiWindUMS, "U", 0, 300, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));
    param = NFmiParam(kFmiWindVMS, "V", 0, 300, 1, 0, "%.1f", kLinearly);
    subParamBag.Add(NFmiDataIdent(param, theProducer, kContinuousParam));

    param = NFmiParam(
        kFmiTotalWindMS, "Wind", kFloatMissing, kFloatMissing, 1, 0, "%.1f", kByCombinedParam);
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
 * \param value1 Undocumented
 * \param value2 Undocumented
 * \param factor1 Undocumented
 * \param factor2 Undocumented
 * \param missingValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

int NFmiTotalWind::PartSum(int value1, int value2, float factor1, float factor2, int missingValue)
{
  try
  {
    int retValue = missingValue;
    if (factor1 == 0 && factor2 != 0)
      retValue = value2;
    else if (factor2 == 0 && factor1 != 0)
      retValue = value1;

    // molemmilla factor>0
    else if (value1 != missingValue && value2 != missingValue)
      retValue = static_cast<unsigned long>(value1 * factor1 + value2 * factor2);
    else if (value1 != missingValue && value2 == missingValue)
      retValue = value2;
    else if (value1 == missingValue && value2 != missingValue)
      retValue = value1;

    return retValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param dir1 Undocumented
 * \param dir2 Undocumented
 * \param factor Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiTotalWind::WindDirectionMean(unsigned long dir1, unsigned long dir2, float factor)
{
  try
  {
    double meanValue = 0.f;

    short lkm1 = 0;  // alle 180 astetta
    short lkm2 = 0;  // yli 180 astetta

    unsigned long dirTable[2];
    dirTable[0] = dir1;
    dirTable[1] = dir2;

    if (dir1 == kTVariableWind)  // vaihtelevaa tuulta
    {
      if (factor == 0)
        return kTVariableWind;
      else
        return dir2;  // ei tarvitse laskea mitään
    }
    else if (dir2 == kTVariableWind)
    {
      if (factor == 1)
        return kTVariableWind;
      else
        return dir1;  // ei tarvitse laskea mitään
    }
    if (dir1 == kT6BitMissing)
    {
      if (factor == 0)
        return kT6BitMissing;  // palautetaan puuttuva tieto
      else
        return dir2;  // ei tarvitse laskea mitään
    }

    for (unsigned long i : dirTable)  // katsotaan kummalla puolella 180:aa astetta arvot ovat
    {
      if (i != kT6BitMissing)
      {
        if (i < 18)
          lkm1++;
        else if (i >= 18)
          lkm2++;
      }
      else
        return kT6BitMissing;  // ei voi laskea keskiarvoa, kun vaan toinen on olemassa
    }
    if (lkm1 == 0 && lkm2 == 0)
      return kT6BitMissing;
    if (lkm1 == 0 || lkm2 == 0)  // molemmat samalla puolella
      meanValue = dirTable[0] * (1 - factor) + dirTable[1] * factor;
    else  // 360 astetta ylittyy
    {
      // Mika: cast välttämätön g++ kääntäjällä
      if (abs(static_cast<int>(dirTable[0]) - static_cast<int>(dirTable[1])) > 18)
      {
        if (dirTable[0] < dirTable[1])
          meanValue = ((dirTable[0] + 36) * (1 - factor) + dirTable[1] * factor);
        else
          meanValue = ((dirTable[1] + 36) * (factor) + dirTable[0] * (1 - factor));
        if (meanValue > 36)
          meanValue = meanValue - 36;
      }
      else
        meanValue = dirTable[0] * (1 - factor) + dirTable[1] * factor;
    }

    return static_cast<unsigned long>(meanValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTotalWind Undocumented
 * \param factor Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiTotalWind *NFmiTotalWind::Mean(const NFmiTotalWind &theTotalWind, float factor)
{
  try
  {
    this->SetWindDirection(
        WindDirectionMean(this->WindDirection(), theTotalWind.WindDirection(), factor));
    this->SetWindSpeed(PartSum(this->WindSpeed(),
                               theTotalWind.WindSpeed(),
                               (1 - factor),
                               factor,
                               WindSpeedMissingValue()));
    this->SetWindGust(kT4BitMissing);
    return this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Lasketaan painotettu keskiarvo käyttäen U- ja V-komponetteja.
 *
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

bool NFmiTotalWind::SetToWeightedMean(NFmiCombinedParam *theCombinedParam1,
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
    auto *theWind1 = static_cast<NFmiTotalWind *>(theCombinedParam1);
    auto *theWind2 = static_cast<NFmiTotalWind *>(theCombinedParam2);
    auto *theWind3 = static_cast<NFmiTotalWind *>(theCombinedParam3);
    auto *theWind4 = static_cast<NFmiTotalWind *>(theCombinedParam4);

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

    NFmiInterpolation::WindInterpolator windInterpolator;
    windInterpolator.operator()(
        theWind1->SubValue(kFmiWindSpeedMS), theWind1->SubValue(kFmiWindDirection), factor1);
    windInterpolator.operator()(
        theWind2->SubValue(kFmiWindSpeedMS), theWind2->SubValue(kFmiWindDirection), factor2);
    windInterpolator.operator()(
        theWind3->SubValue(kFmiWindSpeedMS), theWind3->SubValue(kFmiWindDirection), factor3);
    windInterpolator.operator()(
        theWind4->SubValue(kFmiWindSpeedMS), theWind4->SubValue(kFmiWindDirection), factor4);

    // Muutin tuulen interpolointia siten, että tuulen nopeus lasketaan skalaarisesti
    // ja suunta ottaa huomioon jatkuvuuden 360 kohdalla (ennen laskettiin u- ja v-komponenttien
    // avulla)
    SetFromDirectionAndSpeed(static_cast<float>(windInterpolator.Direction()),
                             static_cast<float>(windInterpolator.Speed()));

    double gust = WeightedMean(kFloatMissing,
                               theWind1->SubValue(kFmiHourlyMaximumGust),
                               factor1,
                               theWind2->SubValue(kFmiHourlyMaximumGust),
                               factor2,
                               theWind3->SubValue(kFmiHourlyMaximumGust),
                               factor3,
                               theWind4->SubValue(kFmiHourlyMaximumGust),
                               factor4);
    SetWindGust(WindGustVx(gust));

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param info Undocumented, unused
 * \param theLonLat Undocumented, unused
 * \param period Undocumented, unused
 * \param considerPrecipitationMax Undocumented, unused
 * \param factor1 Undocumented, unused
 * \param factor2 Undocumented, unused
 * \param factor3 Undocumented, unused
 * \return Undocumented, always true
 * \todo Implement it or delete it
 */
// ----------------------------------------------------------------------

bool NFmiTotalWind::SetToWeightedPeriod(NFmiQueryInfo * /* info */,
                                        const NFmiPoint & /* theLonLat */,
                                        unsigned long /* period */,
                                        bool /* considerPrecipitationMax */,
                                        float /* factor1 */,
                                        float /* factor2 */,
                                        float /* factor3 */)
{
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param info Undocumented, unused
 * \param theLonLat Undocumented, unused
 * \param startTime Undocumented, unused
 * \param endTime Undocumented, unused
 * \param considerPrecipitationMax Undocumented, unused
 * \param factor1 Undocumented, unused
 * \param factor2 Undocumented, unused
 * \param factor3 Undocumented, unused
 * \return Undocumented, always true
 * \todo Implement it or delete it
 */
// ----------------------------------------------------------------------

bool NFmiTotalWind::SetToWeightedPeriod(NFmiQueryInfo * /* info */,
                                        const NFmiPoint & /* theLonLat */,
                                        const NFmiMetTime & /* startTime */,
                                        const NFmiMetTime & /* endTime */,
                                        bool /* considerPrecipitationMax */,
                                        float /* factor1 */,
                                        float /* factor2 */,
                                        float /* factor3 */)
{
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTotalWind::WindDirection(unsigned long theValue)
{
  try
  {
    int integerValue = theValue;
    if (theValue != kFloatMissing)  // vai float???
    {
      if (integerValue < 0)  // integerValue tarvitaan, että nähtäisiin alle nollan arvot
      {
        theValue = 36 + integerValue % 36;
      }
      else if (theValue > 36 && theValue != kT6BitMissing && theValue != kTVariableWind)
      {
        theValue = theValue % 36;
      }
      SetWindDirection(theValue);
      if (integerValue == 0 && WindSpeed() != 0)
        SetWindDirection(36);
      if (WindSpeed() == WindSpeedMissingValue())
        fDataOk = false;
      else
        fDataOk = true;

      return true;
    }

    SetWindDirection(kT6BitMissing);
    if (WindSpeed() == WindSpeedMissingValue())
      fDataOk = false;
    else
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
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTotalWind::WindSpeed(unsigned long theValue)
{
  try
  {
    if (theValue != kFloatMissing)
    {
      unsigned long maxWindSpeedValue = (itsInfoVersion >= 7.) ? 4094 : 500;
      if (theValue > maxWindSpeedValue && theValue != WindSpeedMissingValue())  // Marko testi
        theValue = maxWindSpeedValue;  // suurin mahd. int luku (= 409.4 m/s) tai (V6 500 = 500 m/s)
      SetWindSpeed(theValue);
      if (theValue == 0)
        SetWindDirection(0);  // tyyni

      if (WindDirection() == kT6BitMissing)
        fDataOk = false;
      else
        fDataOk = true;

      return true;
    }

    SetWindSpeed(WindSpeedMissingValue());
    if (WindDirection() == kT6BitMissing)
      fDataOk = false;
    else
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
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTotalWind::WindGust(unsigned long theValue)
{
  try
  {
    SetWindGust(theValue);
    return true;
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

bool NFmiTotalWind::LongValue(unsigned long theValue)
{
  try
  {
    //  if(theValue != kUnsignedLongMissing && theValue != kFloatMissing)
    if (theValue != kTCombinedWeatherMissing && theValue != kFloatMissing)
    {
      itsData.longType = theValue;
      fDataOk = true;
      return true;
    }
    else
    {
      itsData.longType = kTCombinedWeatherMissing;
      fDataOk = true;
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

double NFmiTotalWind::WindSpeedV7(unsigned long theValue)
{
  try
  {
    if (theValue == kT12BitMissing)
      return kFloatMissing;

    return theValue / 10.0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiTotalWind::WindGustV7(unsigned long theValue)
{
  try
  {
    if (theValue == kWindGustV7Missing || theValue == kT12BitMissing)
      return kFloatMissing;

    return theValue / 10.0;
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

unsigned long NFmiTotalWind::WindSpeedV7(double theValue)
{
  try
  {
    if (theValue == kFloatMissing)
      return kT12BitMissing;

    return static_cast<unsigned long>(round((theValue * 10.)));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

unsigned long NFmiTotalWind::WindGustV7(double theValue)
{
  try
  {
    if (theValue == kFloatMissing)
      return kWindGustV7Missing;

    return static_cast<unsigned long>(round((theValue * 10.)));
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

double NFmiTotalWind::WindSpeedV6(unsigned long theValue)
{
  try
  {
    if (theValue == kT9BitMissing)
      return kFloatMissing;

    return theValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiTotalWind::WindGustV6(unsigned long theValue)
{
  try
  {
    if (theValue == kT4BitMissing)
      return kFloatMissing;

    return theValue;
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

unsigned long NFmiTotalWind::WindSpeedV6(double theValue)
{
  try
  {
    if (theValue == kFloatMissing)
      return kT9BitMissing;

    return static_cast<unsigned long>(round(theValue));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

unsigned long NFmiTotalWind::WindGustV6(double theValue)
{
  try
  {
    if (theValue == kFloatMissing)
      return kT4BitMissing;

    return static_cast<unsigned long>(round(theValue));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
