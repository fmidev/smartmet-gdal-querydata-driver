// ======================================================================
/*!
 * \file NFmiInfoAreaMask.cpp
 * \brief Implementation of class NFmiInfoAreaMask
 */
// ======================================================================
/*!
 * \class NFmiInfoAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiInfoAreaMask.h"
#include "NFmiArea.h"
#include "NFmiDataModifierClasses.h"
#include "NFmiFastInfoUtils.h"
#include "NFmiInterpolation.h"
#include "NFmiMetMath.h"
#include "NFmiProducerName.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiSimpleCondition.h"
#include <macgyver/Exception.h>

#include <boost/math/special_functions/round.hpp>
#include <cassert>

// HUOM!!! Jostain syystä kun käytin täällä boost::math::iround -funktiota, se ei mennyt parissa
// VC++ 2013 projektissa linkkerin läpi (Unknown variable blaa blaa). Esim. SmartMet projekti meni
// läpi ilman ongelmia, mutta qdmisc -solutionin qdtempprob- ja modifyLevelData- projektit eivät
// menneet.
// Kyseiset projektit menivät läpi vielä aiemmin, mutta kun lisäsin boostin iround -kutsut, ei enää.
// SIKSI joudun käyttämään FmiRound -funktioita.

void MetaParamDataHolder::initialize(const std::shared_ptr<NFmiFastQueryInfo> &info,
                                     unsigned long possibleMetaParamId)
{
  try
  {
    possibleMetaParamId_ = possibleMetaParamId;
    metaWindParamUsage_ = NFmiFastInfoUtils::CheckMetaWindParamUsage(info);
    checkMetaParamCalculation_ =
        metaWindParamUsage_.ParamNeedsMetaCalculations(possibleMetaParamId_);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool MetaParamDataHolder::isMetaParameterCalculationNeeded() const
{
  try
  {
    if (checkMetaParamCalculation_ &&
        metaWindParamUsage_.ParamNeedsMetaCalculations(possibleMetaParamId_))
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsFindFunction(NFmiAreaMask::FunctionType theFunction)
{
  try
  {
    if (theFunction == NFmiAreaMask::FindH || theFunction == NFmiAreaMask::FindC)
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsSimpleConditionFindFunction(NFmiAreaMask::FunctionType theFunction)
{
  try
  {
    if (theFunction == NFmiAreaMask::FindCountCond || theFunction == NFmiAreaMask::FindHeightCond)
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsFindConditionalFunction(NFmiAreaMask::FunctionType theFunction)
{
  try
  {
    if (theFunction >= NFmiAreaMask::ProbOver && theFunction <= NFmiAreaMask::ProbBetweenEq)
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool CheckProbabilityCondition(NFmiAreaMask::FunctionType condition,
                                      double value,
                                      double limit1,
                                      double limit2)
{
  try
  {
    switch (condition)
    {
      case NFmiAreaMask::ProbOver:
        return value > limit1;
      case NFmiAreaMask::ProbOverEq:
        return value >= limit1;
      case NFmiAreaMask::ProbUnder:
        return value < limit1;
      case NFmiAreaMask::ProbUnderEq:
        return value <= limit1;
      case NFmiAreaMask::ProbEqual:
        return value == limit1;
      case NFmiAreaMask::ProbNotEqual:
        return value != limit1;
      case NFmiAreaMask::ProbBetween:
        return (value > limit1) && (value < limit2);
      case NFmiAreaMask::ProbBetweenEq:
        return (value >= limit1) && (value <= limit2);

      default:
        throw Fmi::Exception(BCP,
                             "Internal error in SmartTool system, probability condition unknown");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Static data member initialization to empty
NFmiInfoAreaMask::MultiSourceDataGetterType NFmiInfoAreaMask::itsMultiSourceDataGetter;

void NFmiInfoAreaMask::SetMultiSourceDataGetterCallback(
    const MultiSourceDataGetterType &theCallbackFunction)
{
  itsMultiSourceDataGetter = theCallbackFunction;
}

std::vector<std::shared_ptr<NFmiFastQueryInfo>> NFmiInfoAreaMask::GetMultiSourceData(
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    std::shared_ptr<NFmiArea> &calculationArea,
    bool getStationarySynopDataOnly)
{
  std::vector<std::shared_ptr<NFmiFastQueryInfo>> infoVector;
  NFmiDataIdent usedDataIdent = theInfo->Param();
  if (getStationarySynopDataOnly)
    usedDataIdent.GetProducer()->SetIdent(NFmiInfoData::kFmiSpSynoXProducer);
  itsMultiSourceDataGetter(
      infoVector, usedDataIdent, *theInfo->Level(), theInfo->DataType(), calculationArea);
  return infoVector;
}

// Nyt synop ja salama datat ovat tälläisiä. Tämä on yritys tehdä vähän optimointia muutenkin jo
// pirun raskaaseen koodiin.
// HUOM! Tämä on riippuvainen NFmiEditMapGeneralDataDoc::MakeDrawedInfoVectorForMapView -metodin
// erikoistapauksista.
bool NFmiInfoAreaMask::IsKnownMultiSourceData(const std::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  if (theInfo)
  {
    if (theInfo->DataType() == NFmiInfoData::kFlashData) return true;
    // HUOM! kaikkien synop datojen käyttö on aivan liian hidasta, käytetään vain primääri synop
    // dataa laskuissa.
    if (theInfo->Producer()->GetIdent() == kFmiSYNOP) return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiInfoAreaMask::~NFmiInfoAreaMask() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiInfoAreaMask::NFmiInfoAreaMask()
    : NFmiAreaMaskImpl(),
      itsInfo(),
      itsDataIdent(),
      itsLevel(),
      fIsTimeIntepolationNeededInValue(false),
      fUsePressureLevelInterpolation(false),
      itsUsedPressureLevelValue(kFloatMissing),
      metaParamDataHolder(),
      itsInfoVector()
{
  try
  {
    DoConstructorInitializations(kFmiBadParameter);
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
 * \param theOperation Undocumented
 * \param theMaskType Undocumented
 * \param theDataType Undocumented
 * \param theInfo Undocumented
 * \param ownsInfo Undocumented
 * \param thePostBinaryOperator Undocumented
 * \param destroySmartInfoData Undocumented
 */
// ----------------------------------------------------------------------

NFmiInfoAreaMask::NFmiInfoAreaMask(const NFmiCalculationCondition &theOperation,
                                   Type theMaskType,
                                   NFmiInfoData::Type theDataType,
                                   const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                   unsigned long thePossibleMetaParamId,
                                   BinaryOperator thePostBinaryOperator)
    : NFmiAreaMaskImpl(theOperation, theMaskType, theDataType, thePostBinaryOperator),
      itsInfo(theInfo),
      itsDataIdent(theInfo ? theInfo->Param() : NFmiDataIdent()),
      itsLevel(NFmiLevel()),
      fIsTimeIntepolationNeededInValue(false),
      fUsePressureLevelInterpolation(false),
      itsUsedPressureLevelValue(kFloatMissing),
      metaParamDataHolder(),
      itsInfoVector()
{
  try
  {
    DoConstructorInitializations(thePossibleMetaParamId);
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
 * \param theInfo Undocumented
 * \param ownsInfo Undocumented
 * \param thePostBinaryOperator Undocumented
 * \param destroySmartInfoData Undocumented
 */
// ----------------------------------------------------------------------

NFmiInfoAreaMask::NFmiInfoAreaMask(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                   unsigned long thePossibleMetaParamId,
                                   BinaryOperator thePostBinaryOperator)
    : NFmiAreaMaskImpl(
          NFmiCalculationCondition(), kInfo, NFmiInfoData::kEditable, thePostBinaryOperator),
      itsInfo(theInfo),
      itsDataIdent(theInfo ? theInfo->Param() : NFmiDataIdent()),
      itsLevel(NFmiLevel()),
      fIsTimeIntepolationNeededInValue(false),
      fUsePressureLevelInterpolation(false),
      itsUsedPressureLevelValue(kFloatMissing),
      metaParamDataHolder(),
      itsInfoVector()
{
  try
  {
    DoConstructorInitializations(thePossibleMetaParamId);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMask::NFmiInfoAreaMask(const NFmiInfoAreaMask &theOther)
    : NFmiAreaMaskImpl(theOther),
      itsInfo(NFmiAreaMask::DoShallowCopy(theOther.itsInfo)),
      itsDataIdent(theOther.itsDataIdent),
      itsLevel(theOther.itsLevel),
      fIsTimeIntepolationNeededInValue(theOther.fIsTimeIntepolationNeededInValue),
      fUsePressureLevelInterpolation(theOther.fUsePressureLevelInterpolation),
      itsUsedPressureLevelValue(theOther.itsUsedPressureLevelValue),
      metaParamDataHolder(theOther.metaParamDataHolder),
      fIsModelClimatologyData(theOther.fIsModelClimatologyData),
      fUseMultiSourceData(theOther.fUseMultiSourceData),
      itsInfoVector(NFmiAreaMask::DoShallowCopy(theOther.itsInfoVector))
{
}

void NFmiInfoAreaMask::DoConstructorInitializations(unsigned long thePossibleMetaParamId)
{
  try
  {
    if (itsInfo)
    {
      metaParamDataHolder.initialize(itsInfo, thePossibleMetaParamId);
      if (itsInfo->Level()) itsLevel = *itsInfo->Level();
      fIsModelClimatologyData = NFmiFastInfoUtils::IsModelClimatologyData(itsInfo);
      fUseMultiSourceData = IsKnownMultiSourceData(itsInfo);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiAreaMask *NFmiInfoAreaMask::Clone() const
{
  return new NFmiInfoAreaMask(*this);
}

void NFmiInfoAreaMask::Initialize()
{
  NFmiAreaMaskImpl::Initialize();
  if (fUseMultiSourceData)
  {
    std::shared_ptr<NFmiArea> dummyArea;
    bool getStationarySynopDataOnly = false;
    itsInfoVector =
        NFmiInfoAreaMask::GetMultiSourceData(itsInfo, dummyArea, getStationarySynopDataOnly);
  }
  else
  {
    if (itsInfo)
    {
      itsInfoVector.push_back(itsInfo);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiInfoAreaMask::Time(const NFmiMetTime &theTime)
{
  try
  {
    if (itsDataType == NFmiInfoData::kStationary)
      return true;

    assert(itsInfo);
    if (itsInfo)
    {
      itsTime = NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(itsInfo, theTime);
      bool status = itsInfo->Time(theTime);
      // Jos tämän jälkeen käytetään samaa aikaa Value-metodissa, ei aikainterpolointia tarvitse
      // tehdä, jos aika löytyi.
      fIsTimeIntepolationNeededInValue = !status;
      return status;
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
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiInfoAreaMask::IsWantedParam(const NFmiDataIdent & /* theParam */,
                                     const NFmiLevel * /* theLevel */) const
{
  try
  {
    return false;
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

const NFmiString NFmiInfoAreaMask::MakeSubMaskString() const
{
  try
  {
    return itsDataIdent.GetParamName();
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

const NFmiDataIdent *NFmiInfoAreaMask::DataIdent() const
{
  try
  {
    return &itsDataIdent;
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

const NFmiParam *NFmiInfoAreaMask::Param() const
{
  try
  {
    return itsDataIdent.GetParam();
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

const NFmiLevel *NFmiInfoAreaMask::Level() const
{
  try
  {
    return &itsLevel;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMask::Level(const NFmiLevel &theLevel)
{
  try
  {
    itsLevel = theLevel;
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

bool NFmiInfoAreaMask::UseLevelInfo() const
{
  try
  {
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMask::GetSearchRadiusInMetres(double observationRadiusInKm)
{
  if (observationRadiusInKm == kFloatMissing)
  {
    // Tämä on rajaton etsintä NFmiFastInfo::NearestLocation metodissa
    return kFloatMissing * 1000.;
  }
  else
    return observationRadiusInKm * 1000.;
}

bool NFmiInfoAreaMask::FindClosestStationData(const NFmiCalculationParams &calculationParams,
                                              size_t &dataIndexOut,
                                              unsigned long &locationIndexOut)
{
  // Huom! Pitää käyttää macroParamin laskentahilan pistettä UsedLatlon(true),
  // jotta itsObservationRadiusInKm juttu otetaan oikein huomioon.
  auto latlon = calculationParams.UsedLatlon(true);
  NFmiLocation wantedLocation(latlon);
  double minDistanceInMetres = 99999999999;
  double searchRadiusInMetres = GetSearchRadiusInMetres(calculationParams.itsObservationRadiusInKm);
  for (size_t dataCounter = 0; dataCounter < itsInfoVector.size(); dataCounter++)
  {
    const auto &info = itsInfoVector[dataCounter];
    if (!NFmiFastInfoUtils::IsInfoShipTypeData(*info) &&
        info->NearestLocation(latlon, searchRadiusInMetres))
    {
      double currentDistanceInMetres = wantedLocation.Distance(info->LatLon());
      if (currentDistanceInMetres < minDistanceInMetres)
      {
        minDistanceInMetres = currentDistanceInMetres;
        dataIndexOut = dataCounter;
        locationIndexOut = info->LocationIndex();
      }
    }
  }
  return minDistanceInMetres <= searchRadiusInMetres;
}

bool NFmiInfoAreaMask::CheckPossibleObservationDistance(
    const NFmiCalculationParams &theCalculationParamsInOut)
{
  theCalculationParamsInOut.itsCurrentMultiInfoData = nullptr;
  if (itsInfo && !itsInfo->IsGrid())
  {
    // Jos ObsRadius:sella on arvo tai on kyse multi-info datasta, pitää etsiä lähin data ja paikka
    // siinä
    if (theCalculationParamsInOut.itsObservationRadiusInKm != kFloatMissing ||
        itsInfoVector.size() > 1)
    {
      size_t dataIndex = 0;
      unsigned long locationIndex = 0;
      if (FindClosestStationData(theCalculationParamsInOut, dataIndex, locationIndex))
      {
        if (itsInfoVector.size() > 1)
        {
          theCalculationParamsInOut.itsCurrentMultiInfoData = itsInfoVector[dataIndex].get();
        }
        return true;
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiInfoAreaMask::CalcValueFromLocation(const NFmiPoint &theLatLon) const
{
  try
  {
    if (fIsTimeIntepolationNeededInValue)
      return itsInfo->InterpolatedValue(theLatLon, itsTime, 360);

    double value = itsInfo->InterpolatedValue(theLatLon);
    if (value == kFloatMissing && itsInfo->DataType() == NFmiInfoData::kScriptVariableData)
      value = itsInfo->FloatValue();

    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiInfoAreaMask::IsTimeInterpolationNeeded(bool fUseTimeInterpolationAlways) const
{
  try
  {
    if ((fUseTimeInterpolationAlways || fIsTimeIntepolationNeededInValue) &&
        !NFmiFastInfoUtils::IsModelClimatologyData(itsInfo))
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
 * \param theLatlon Undocumented
 * \param theTime Undocumented
 * \param theTimeIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// tätä kaytetaan smarttool-modifierin yhteydessä

double NFmiInfoAreaMask::Value(const NFmiCalculationParams &theCalculationParams,
                               bool fUseTimeInterpolationAlways)
{
  try
  {
    if (fIsModelClimatologyData)
    {
      NFmiCalculationParams usedCalculationParams(theCalculationParams);
      usedCalculationParams.itsTime =
          NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(itsInfo, theCalculationParams.itsTime);
      return ValueFinal(usedCalculationParams, fUseTimeInterpolationAlways);
    }
    else
    {
      return ValueFinal(theCalculationParams, fUseTimeInterpolationAlways);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMask::ValueFinal(const NFmiCalculationParams &theCalculationParams,
                                    bool fUseTimeInterpolationAlways)
{
  if (metaParamDataHolder.isMetaParameterCalculationNeeded())
    return CalcMetaParamValue(theCalculationParams);

  double result = kFloatMissing;
  if (UsePressureLevelInterpolation())
  {
    if (Level()->LevelType() == kFmiFlightLevel)
    {
      double P = ::CalcFlightLevelPressure(UsedPressureLevelValue() * 100);
      result = PressureValueStatic(
          P, theCalculationParams);  // Tässä ei saa kutsua virtuaalista PressureValue -metodia!!
    }
    else if (Level()->LevelType() == kFmiHeight)
    {
      result = HeightValueStatic(
          UsedPressureLevelValue(),
          theCalculationParams);  // Tässä ei saa kutsua virtuaalista PressureValue -metodia!!
    }
    else
      result = PressureValueStatic(
          UsedPressureLevelValue(),
          theCalculationParams);  // Tässä ei saa kutsua virtuaalista PressureValue -metodia!!
  }
  else
  {
    auto currentMultiInfoData = theCalculationParams.itsCurrentMultiInfoData;
    if (currentMultiInfoData)
    {
      NFmiFastInfoUtils::QueryInfoTotalStateRestorer queryInfoTotalStateRestorer(
          *currentMultiInfoData);
      currentMultiInfoData->Param(*itsInfo->Param().GetParam());
      result = currentMultiInfoData->InterpolatedValue(theCalculationParams.UsedLatlon(),
                                                       theCalculationParams.itsTime,
                                                       360);  // interpoloidaan ajassa ja paikassa
    }
    else if (IsTimeInterpolationNeeded(fUseTimeInterpolationAlways))
      result = itsInfo->InterpolatedValue(theCalculationParams.UsedLatlon(),
                                          theCalculationParams.itsTime,
                                          360);  // interpoloidaan ajassa ja paikassa
    else
      result = CalcValueFromLocation(
          theCalculationParams.UsedLatlon());  // ollaan jo oikeassa ajassa, ei aikainterpolointia
  }
  return result;
}

float NFmiInfoAreaMask::CalcMetaParamValue(const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    if (UsePressureLevelInterpolation())
    {
      if (Level()->LevelType() == kFmiFlightLevel)
      {
        double P = ::CalcFlightLevelPressure(UsedPressureLevelValue() * 100);
        return CalcMetaParamPressureValue(P, theCalculationParams);
      }
      else if (Level()->LevelType() == kFmiHeight)
      {
        return CalcMetaParamHeightValue(UsedPressureLevelValue(), theCalculationParams);
      }
      else
        return CalcMetaParamPressureValue(UsedPressureLevelValue(), theCalculationParams);
    }
    else
    {
      return CalcMetaParamValueWithFunction(
          [&]()
          {
          return itsInfo->InterpolatedValue(theCalculationParams.UsedLatlon(),
                                              theCalculationParams.itsTime);
          });
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMask::CalcMetaParamHeightValue(double theHeight,
                                                 const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    return CalcMetaParamValueWithFunction(
        [&]()
        {
          return itsInfo->HeightValue(static_cast<float>(theHeight),
                                    theCalculationParams.UsedLatlon(),
                                      theCalculationParams.itsTime);
        });
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMask::CalcMetaParamPressureValue(
    double thePressure, const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    return CalcMetaParamValueWithFunction(
        [&]()
        {
          return itsInfo->PressureLevelValue(static_cast<float>(thePressure),
                                           theCalculationParams.UsedLatlon(),
                                             theCalculationParams.itsTime);
        });
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMask::CalcCachedInterpolation(std::shared_ptr<NFmiFastQueryInfo> &theUsedInfo,
                                                const NFmiLocationCache &theLocationCache,
                                                const NFmiTimeCache *theTimeCache)
{
  try
  {
    if (metaParamDataHolder.isMetaParameterCalculationNeeded())
      return CalcMetaParamCachedInterpolation(theUsedInfo, theLocationCache, theTimeCache);

    if (theTimeCache)
      return theUsedInfo->CachedInterpolation(theLocationCache, *theTimeCache);

    return theUsedInfo->CachedInterpolation(theLocationCache);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMask::CalcMetaParamCachedInterpolation(
    std::shared_ptr<NFmiFastQueryInfo> &theUsedInfo,
    const NFmiLocationCache &theLocationCache,
    const NFmiTimeCache *theTimeCache)
{
  try
  {
    return CalcMetaParamValueWithFunction(
        [&]()
        {
          return theTimeCache ? theUsedInfo->CachedInterpolation(theLocationCache, *theTimeCache)
                              : theUsedInfo->CachedInterpolation(theLocationCache);
        });
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMask::HeightValue(double theHeight,
                                     const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    if (metaParamDataHolder.isMetaParameterCalculationNeeded())
      return CalcMetaParamHeightValue(theHeight, theCalculationParams);

    return itsInfo->HeightValue(static_cast<float>(theHeight),
                                theCalculationParams.UsedLatlon(),
                                theCalculationParams.itsTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMask::HeightValueStatic(double theHeight,
                                           const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    if (metaParamDataHolder.isMetaParameterCalculationNeeded())
      return CalcMetaParamHeightValue(theHeight, theCalculationParams);

    return itsInfo->HeightValue(static_cast<float>(theHeight),
                                theCalculationParams.UsedLatlon(),
                                theCalculationParams.itsTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMask::PressureValue(double thePressure,
                                       const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    if (metaParamDataHolder.isMetaParameterCalculationNeeded())
      return CalcMetaParamPressureValue(thePressure, theCalculationParams);

    return itsInfo->PressureLevelValue(static_cast<float>(thePressure),
                                       theCalculationParams.UsedLatlon(),
                                       theCalculationParams.itsTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMask::PressureValueStatic(double thePressure,
                                             const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    if (metaParamDataHolder.isMetaParameterCalculationNeeded())
      return CalcMetaParamPressureValue(thePressure, theCalculationParams);

    return itsInfo->PressureLevelValue(static_cast<float>(thePressure),
                                       theCalculationParams.UsedLatlon(),
                                       theCalculationParams.itsTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// erikoistapaus optimoituun käyttöön (ei voi käyttää kaikille luokille!!!!)
// sopii mm. infoareamask ja binaryareamask-luokille

bool NFmiInfoAreaMask::IsMasked(int theIndex) const
{
  try
  {
    if (!fEnabled)
      return true;  // jos maski ei ole käytössä, on maski aina 'päällä'

    double testValue = itsInfo->GetFloatValue(itsInfo->Index(
        itsInfo->ParamIndex(), theIndex, itsInfo->LevelIndex(), itsInfo->TimeIndex()));
    return itsMaskCondition.IsMasked(testValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newInfo Undocumented
 * \param ownsInfo Undocumented
 * \param destroySmartInfoData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------
void NFmiInfoAreaMask::Info(const std::shared_ptr<NFmiFastQueryInfo> &newInfo)
{
  try
  {
    itsInfo = newInfo;
    itsDataIdent = itsInfo ? itsInfo->Param() : NFmiDataIdent();
    itsLevel = itsInfo ? *itsInfo->Level() : NFmiLevel();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Tänne pitää antaa halutun infon matala kopio ja se laitetaan itsInfo:on.
// Sitten siihen laitetaan maskissa olevat parametri ja leveli kohdalleen.
void NFmiInfoAreaMask::UpdateInfo(std::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  try
  {
    itsInfo = theInfo;
    if (itsInfo)
    {
      itsInfo->Param(static_cast<FmiParameterName>(itsDataIdent.GetParamIdent()));
      itsInfo->Level(itsLevel);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::shared_ptr<NFmiDataModifier> NFmiInfoAreaMask::CreateIntegrationFuction(
    NFmiAreaMask::FunctionType func)
{
  try
  {
    std::shared_ptr<NFmiDataModifier> modifier;
    // vertCondFunc tapauksia on paljon ja niille ei tehdä integraatiota, joten tein ehdon
    // vähentämään case -tapauksia
    if (!::IsFindConditionalFunction(func))
    {
      switch (func)
      {
        case NFmiAreaMask::Avg:
          modifier = std::shared_ptr<NFmiDataModifier>(new NFmiDataModifierAvg());
          break;
        case NFmiAreaMask::Min:
          modifier = std::shared_ptr<NFmiDataModifier>(new NFmiDataModifierMin());
          break;
        case NFmiAreaMask::Max:
          modifier = std::shared_ptr<NFmiDataModifier>(new NFmiDataModifierMax());
          break;
        case NFmiAreaMask::Sum:
          modifier = std::shared_ptr<NFmiDataModifier>(new NFmiDataModifierSum());
          break;
        case NFmiAreaMask::Med:
          modifier = std::shared_ptr<NFmiDataModifier>(new NFmiDataModifierMedian());
          break;
      case NFmiAreaMask::ModAvg:
        modifier = std::shared_ptr<NFmiDataModifier>(new NFmiDataModifierModAvg());
        break;
      case NFmiAreaMask::ModMin:
        modifier = std::shared_ptr<NFmiDataModifier>(new NFmiDataModifierModMinMax(true));
        break;
      case NFmiAreaMask::ModMax:
        modifier = std::shared_ptr<NFmiDataModifier>(new NFmiDataModifierModMinMax(false));
        break;
        case NFmiAreaMask::Get:
        case NFmiAreaMask::FindH:
        case NFmiAreaMask::FindC:
        case NFmiAreaMask::MinH:
        case NFmiAreaMask::MaxH:
        case NFmiAreaMask::Grad:
        case NFmiAreaMask::FindHeightCond:
        case NFmiAreaMask::FindCountCond:
        case NFmiAreaMask::PeekZ:
          modifier = std::shared_ptr<NFmiDataModifier>();  // get- ja find -tapauksissa
                                                             // palautetaan tyhjä-olio, koska niille
                                                             // ei tarvita erillistä integraattoria
          break;
          // HUOM!!!! Tee WAvg-modifier myös, joka on peritty Avg-modifieristä ja tee joku kerroin
          // juttu painotukseen.
        default:
          throw Fmi::Exception(BCP,
                               "Internal SmartMet error: Smarttool function has unknown "
                               "integration function,\ncan't "
                               "execute the calculations.");
      }
    }
    return modifier;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Etsii halutun aika loopituksen alku- ja loppuaika indeksejä annetusta infosta.
// Jos startOffset ja endOffsetit ovat samoja, käytetään kyseistä aikaa aikainterpolaatiossa.
bool NFmiInfoAreaMask::CalcTimeLoopIndexies(std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                            const NFmiCalculationParams &theCalculationParams,
                                            double theStartTimeOffsetInHours,
                                            double theEndTimeOffsetInHours,
                                            unsigned long *theStartTimeIndexOut,
                                            unsigned long *theEndTimeIndexOut)
{
  try
  {
    if (theInfo)
    {
      NFmiMetTime startTime = theCalculationParams.itsTime;
      startTime.ChangeByMinutes(FmiRound(theStartTimeOffsetInHours * 60));
      NFmiMetTime endTime = theCalculationParams.itsTime;
      endTime.ChangeByMinutes(FmiRound(theEndTimeOffsetInHours * 60));

      return NFmiInfoAreaMask::CalcTimeLoopIndexies(theInfo,
                                                    theCalculationParams,
                                                    startTime,
                                                    endTime,
                                                    theStartTimeIndexOut,
                                                    theEndTimeIndexOut);
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiInfoAreaMask::CalcTimeLoopIndexies(std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                            const NFmiCalculationParams &theCalculationParams,
                                            const NFmiMetTime &theStartTime,
                                            const NFmiMetTime &theEndTime,
                                            unsigned long *theStartTimeIndexOut,
                                            unsigned long *theEndTimeIndexOut)
{
  try
  {
    NFmiMetTime usedStartTime =
      NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(theInfo, theStartTime);
    NFmiMetTime usedEndTime =
      NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(theInfo, theEndTime);

    NFmiTimeDescriptor times = theInfo->TimeDescriptor().GetIntersection(usedStartTime, usedEndTime);
    // Otetaan aikaindeksi talteen, jotta se voidaan lopuksi palauttaa takaisin
    unsigned long origTimeIndex = theInfo->TimeIndex();
    bool status = false;
    if (times.IsEmpty())
    {
      *theStartTimeIndexOut = *theEndTimeIndexOut = gMissingIndex;
      status = false;
    }
    else if (times.Size() == 1)
    {
      theInfo->Time(times.FirstTime());
      *theStartTimeIndexOut = *theEndTimeIndexOut = theInfo->TimeIndex();
      status = true;
    }
    else
    {
      theInfo->Time(times.FirstTime());
      *theStartTimeIndexOut = theInfo->TimeIndex();
      theInfo->Time(times.LastTime());
      *theEndTimeIndexOut = theInfo->TimeIndex();
      status = true;
    }
    theInfo->TimeIndex(origTimeIndex);
    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// min ja max funktioille voidaan laskuissa ottaa interpoloidun arvon sijasta kaikki interpolaatio
// pistettä ympäröivät arvot sellaisenaan. Interpolaatio kun muuttaa min/max arvoja ikävästi
// neljästä luvusta lasketuksi painotetuksi keskiarvoksi.
void NFmiInfoAreaMask::AddExtremeValues(std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                        std::shared_ptr<NFmiDataModifier> &theFunctionModifier,
                                        const NFmiLocationCache &theLocationCache)
{
  try
  {
    if (!theLocationCache.NoValue())
    {
      if (metaParamDataHolder.isMetaParameterCalculationNeeded())
      {
        if (metaParamDataHolder.metaWindParamUsage().HasWsAndWd())
        {
          theInfo->Param(kFmiWindSpeedMS);
          std::array<float, 4> wsValues;
          theInfo->GetCachedValues(theLocationCache, wsValues);
          theInfo->Param(kFmiWindDirection);
          std::array<float, 4> wdValues;
          theInfo->GetCachedValues(theLocationCache, wdValues);
          for (std::size_t index = 0; index < wsValues.size(); index++)
          {
            switch (metaParamDataHolder.possibleMetaParamId())
            {
              case kFmiWindUMS:
              AddValueToModifier(theInfo,
                                 theFunctionModifier,
                    NFmiFastInfoUtils::CalcU(wsValues[index], wdValues[index]));
                break;
              case kFmiWindVMS:
              AddValueToModifier(theInfo,
                                 theFunctionModifier,
                    NFmiFastInfoUtils::CalcV(wsValues[index], wdValues[index]));
                break;
              case kFmiWindVectorMS:
              AddValueToModifier(theInfo,
                                 theFunctionModifier,
                                 NFmiFastInfoUtils::CalcWindVectorFromSpeedAndDirection(
                                     wsValues[index], wdValues[index]));
                break;
            }
          }
        }
        else if (metaParamDataHolder.metaWindParamUsage().HasWindComponents())
        {
          theInfo->Param(kFmiWindUMS);
          std::array<float, 4> uValues;
          theInfo->GetCachedValues(theLocationCache, uValues);
          theInfo->Param(kFmiWindVMS);
          std::array<float, 4> vValues;
          theInfo->GetCachedValues(theLocationCache, vValues);
          for (std::size_t index = 0; index < uValues.size(); index++)
          {
            switch (metaParamDataHolder.possibleMetaParamId())
            {
              case kFmiWindSpeedMS:
              AddValueToModifier(theInfo,
                                 theFunctionModifier,
                    NFmiFastInfoUtils::CalcWS(uValues[index], vValues[index]));
                break;
              case kFmiWindDirection:
              AddValueToModifier(theInfo,
                                 theFunctionModifier,
                    NFmiFastInfoUtils::CalcWD(uValues[index], vValues[index]));
                break;
              case kFmiWindVectorMS:
              AddValueToModifier(theInfo,
                                 theFunctionModifier,
                                 NFmiFastInfoUtils::CalcWindVectorFromWindComponents(
                    uValues[index], vValues[index]));
                break;
            }
          }
        }
      }
      else
      {
        std::array<float, 4> values;
        theInfo->GetCachedValues(theLocationCache, values);
        for (float value : values)
          AddValueToModifier(theInfo, theFunctionModifier, value);
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMask::AddValueToModifier(std::shared_ptr<NFmiFastQueryInfo> & /* theInfo */,
                                          std::shared_ptr<NFmiDataModifier> &theFunctionModifier,
                                          float theValue)
{
  // In this base virtual method the value is just added to the
  // modifier, in child classes there will be overrides that will
  // keep record of the time of extreme value.
  theFunctionModifier->Calculate(theValue);
}

// Method adds values to function modifier. In case of min/max we use
// AddExtremeValues in order to reach real minimum and maximum values from data.
// If values are just interpolated to a point, it will be sort of 4 values average
// that is used in there.
void NFmiInfoAreaMask::AddValuesToFunctionModifier(
    std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    std::shared_ptr<NFmiDataModifier> &theFunctionModifier,
    const NFmiLocationCache &theLocationCache,
    NFmiAreaMask::FunctionType integrationFunction)
{
  try
  {
    if (DoExtremeAddingSpecialCase() &&
        (integrationFunction == NFmiAreaMask::Max || integrationFunction == NFmiAreaMask::Min))
      AddExtremeValues(theInfo, theFunctionModifier, theLocationCache);
    else
      AddValueToModifier(
          theInfo, theFunctionModifier, CalcCachedInterpolation(theInfo, theLocationCache, nullptr));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
// ****** NFmiInfoAreaMask **********************************************
// ======================================================================

// ======================================================================
// ****** NFmiInfoAreaMaskPeekXY ****************************************
// ======================================================================

NFmiInfoAreaMaskPeekXY::~NFmiInfoAreaMaskPeekXY() = default;

NFmiInfoAreaMaskPeekXY::NFmiInfoAreaMaskPeekXY()
    : NFmiInfoAreaMask(),
      itsXOffset(0),
      itsYOffset(0),
      itsGridXDiff(1)  // dummy arvo, en laita 0, että ei tule joskus 0:lla jakoa vahingossa
      ,
      itsGridYDiff(1)  // dummy arvo, en laita 0, että ei tule joskus 0:lla jakoa vahingossa
{
}

NFmiInfoAreaMaskPeekXY::NFmiInfoAreaMaskPeekXY(const NFmiCalculationCondition &theOperation,
                                               Type theMaskType,
                                               NFmiInfoData::Type theDataType,
                                               const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                               int theXOffset,
                                               int theYOffset,
                                               unsigned long thePossibleMetaParamId,
                                               BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMask(theOperation,
                       theMaskType,
                       theDataType,
                       theInfo,
                       thePossibleMetaParamId,
                       thePostBinaryOperator),
      itsXOffset(theXOffset),
      itsYOffset(theYOffset),
      itsGridXDiff(1)  // dummy arvo, en laita 0, että ei tule joskus 0:lla jakoa vahingossa
      ,
      itsGridYDiff(1)  // dummy arvo, en laita 0, että ei tule joskus 0:lla jakoa vahingossa
{
  try
  {
    CalcGridDiffs();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMaskPeekXY::NFmiInfoAreaMaskPeekXY(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                               int theXOffset,
                                               int theYOffset,
                                               unsigned long thePossibleMetaParamId,
                                               BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMask(theInfo, thePossibleMetaParamId, thePostBinaryOperator),
      itsXOffset(theXOffset),
      itsYOffset(theYOffset),
      itsGridXDiff(1)  // dummy arvo, en laita 0, että ei tule joskus 0:lla jakoa vahingossa
      ,
      itsGridYDiff(1)  // dummy arvo, en laita 0, että ei tule joskus 0:lla jakoa vahingossa
{
  try
  {
    CalcGridDiffs();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMaskPeekXY::NFmiInfoAreaMaskPeekXY(const NFmiInfoAreaMaskPeekXY &theOther) = default;

NFmiAreaMask *NFmiInfoAreaMaskPeekXY::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskPeekXY(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskPeekXY::CalcGridDiffs()
{
  try
  {
    // oletetaan, että on hila dataa ja area löytyy
    if (itsInfo->Grid() == nullptr)
      throw Fmi::Exception(
          BCP, "Error in NFmiInfoAreaMaskPeekXY::CalcGridDiffs, given data was not grid form");

    itsGridXDiff = itsInfo->Area()->Width() / (itsInfo->Grid()->XNumber() - 1);
    itsGridYDiff = itsInfo->Area()->Height() / (itsInfo->Grid()->YNumber() - 1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiCalculationParams NFmiInfoAreaMaskPeekXY::MakeModifiedCalculationParams(
    const NFmiCalculationParams &theCalculationParams)
{
  try
  {
  NFmiPoint xyPoint(itsInfo->Area()->ToXY(theCalculationParams.UsedLatlon()));
    xyPoint.X(xyPoint.X() + itsGridXDiff * itsXOffset);
    xyPoint.Y(xyPoint.Y() -
              itsGridYDiff * itsYOffset);  // huom! '-'-merkki, koska arean y-akseli on
    // käänteinen kuin tämä peek-maailma, jossa y
    // kasvaa ylöspäin
    NFmiPoint wantedLatlon(itsInfo->Area()->ToLatLon(xyPoint));
    NFmiCalculationParams modifiedCalculationParams(theCalculationParams);
  modifiedCalculationParams.SetModifiedLatlon(wantedLatlon, true);
    return modifiedCalculationParams;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
// huom! ei tehdää pelkkää peek juttua vaan 'interpoloitu' peek, koska pelkällä peekellä tulee
// rumaa (leegoja) jälkeä.
double NFmiInfoAreaMaskPeekXY::Value(const NFmiCalculationParams &theCalculationParams,
                                     bool fUseTimeInterpolationAlways)
{
  try
  {
    return NFmiInfoAreaMask::Value(MakeModifiedCalculationParams(theCalculationParams),
                                   fUseTimeInterpolationAlways);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMaskPeekXY::PressureValue(double thePressure,
                                             const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    return NFmiInfoAreaMask::PressureValue(thePressure,
                                           MakeModifiedCalculationParams(theCalculationParams));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskPeekXY2    **********************
// **********************************************************

NFmiInfoAreaMaskPeekXY2::~NFmiInfoAreaMaskPeekXY2() = default;

NFmiInfoAreaMaskPeekXY2::NFmiInfoAreaMaskPeekXY2()
    : NFmiInfoAreaMask(), itsXOffset(0), itsYOffset(0), itsEditedInfo()
{
}

NFmiInfoAreaMaskPeekXY2::NFmiInfoAreaMaskPeekXY2(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const std::shared_ptr<NFmiFastQueryInfo> &theEditedInfo,
    int theXOffset,
    int theYOffset,
    unsigned long thePossibleMetaParamId,
    BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMask(theOperation,
                       theMaskType,
                       theDataType,
                       theInfo,
                       thePossibleMetaParamId,
                       thePostBinaryOperator),
      itsXOffset(theXOffset),
      itsYOffset(theYOffset),
      itsEditedInfo(theEditedInfo)
{
}

NFmiInfoAreaMaskPeekXY2::NFmiInfoAreaMaskPeekXY2(const NFmiInfoAreaMaskPeekXY2 &theOther)
    : NFmiInfoAreaMask(theOther),
      itsXOffset(theOther.itsXOffset),
      itsYOffset(theOther.itsYOffset),
      itsEditedInfo(NFmiAreaMask::DoShallowCopy(theOther.itsEditedInfo))
{
}

NFmiAreaMask *NFmiInfoAreaMaskPeekXY2::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskPeekXY2(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiCalculationParams NFmiInfoAreaMaskPeekXY2::MakeModifiedCalculationParams(
    const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    NFmiCalculationParams modifiedCalculationParams(theCalculationParams);
    auto area = itsEditedInfo->Area();
    if (area)
    {
      // worldXy on annettu latlon piste kartta-alueen metrisessä maailmassa
    NFmiPoint worldXyPoint = area->LatLonToWorldXY(modifiedCalculationParams.UsedLatlon());
      // offsetit on annettu kilometreissa, joten ne pitää kertoa 1000:lla
      worldXyPoint.X(worldXyPoint.X() + itsXOffset * 1000.);
      worldXyPoint.Y(worldXyPoint.Y() + itsYOffset * 1000.);
    modifiedCalculationParams.SetModifiedLatlon(area->WorldXYToLatLon(worldXyPoint), true);
    }
    else
    {
    modifiedCalculationParams.SetModifiedLatlon(NFmiPoint::gMissingLatlon, true);
    }
    return modifiedCalculationParams;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaMaskPeekXY2::Value(const NFmiCalculationParams &theCalculationParams,
                                      bool fUseTimeInterpolationAlways)
{
  try
  {
    return NFmiInfoAreaMask::Value(MakeModifiedCalculationParams(theCalculationParams),
                                   fUseTimeInterpolationAlways);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMaskPeekXY2::PressureValue(double thePressure,
                                              const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    return NFmiInfoAreaMask::PressureValue(thePressure,
                                           MakeModifiedCalculationParams(theCalculationParams));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskPeekXY2    **********************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskPeekXY3    **********************
// **********************************************************

NFmiInfoAreaMaskPeekXY3::~NFmiInfoAreaMaskPeekXY3() = default;

NFmiInfoAreaMaskPeekXY3::NFmiInfoAreaMaskPeekXY3()
    : NFmiInfoAreaMask(), itsXOffsetInKM(0), itsYOffsetInKM(0), itsEditedInfo()
{
}

NFmiInfoAreaMaskPeekXY3::NFmiInfoAreaMaskPeekXY3(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const std::shared_ptr<NFmiFastQueryInfo> &theEditedInfo,
    double theXOffsetInKM,
    double theYOffsetInKM,
    unsigned long thePossibleMetaParamId,
    BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMask(theOperation,
                       theMaskType,
                       theDataType,
                       theInfo,
                       thePossibleMetaParamId,
                       thePostBinaryOperator),
      itsXOffsetInKM(theXOffsetInKM),
      itsYOffsetInKM(theYOffsetInKM),
      itsEditedInfo(theEditedInfo)
{
}

NFmiInfoAreaMaskPeekXY3::NFmiInfoAreaMaskPeekXY3(
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const std::shared_ptr<NFmiFastQueryInfo> &theEditedInfo,
    double theXOffsetInKM,
    double theYOffsetInKM,
    unsigned long thePossibleMetaParamId,
    BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMask(theInfo, thePossibleMetaParamId, thePostBinaryOperator),
      itsXOffsetInKM(theXOffsetInKM),
      itsYOffsetInKM(theYOffsetInKM),
      itsEditedInfo(theEditedInfo)
{
}

NFmiInfoAreaMaskPeekXY3::NFmiInfoAreaMaskPeekXY3(const NFmiInfoAreaMaskPeekXY3 &theOther)
    : NFmiInfoAreaMask(theOther),
      itsXOffsetInKM(theOther.itsXOffsetInKM),
      itsYOffsetInKM(theOther.itsYOffsetInKM),
      itsEditedInfo(NFmiAreaMask::DoShallowCopy(theOther.itsEditedInfo))
{
}

NFmiAreaMask *NFmiInfoAreaMaskPeekXY3::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskPeekXY3(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsPacificViewData(std::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  try
  {
    if (theInfo)
    {
      if (theInfo->Grid())  // trajektori datojen pitäisi olla hiladatoja
        return theInfo->Grid()->Area()->PacificView();
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiCalculationParams NFmiInfoAreaMaskPeekXY3::MakeModifiedCalculationParams(
    const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    bool usePacificView = ::IsPacificViewData(itsInfo);
  NFmiLocation loc(theCalculationParams.UsedLatlon());
    // x-suunnassa siirto ei mielestäni toimi oikein vaan piti laittaa positiiviselle ja
    // negatiiviselle tapauksille omat haarat
    if (itsXOffsetInKM > 0)
      loc.SetLocation(90., itsXOffsetInKM * 1000., usePacificView);

    if (itsXOffsetInKM < 0)
      loc.SetLocation(270., itsXOffsetInKM * 1000., usePacificView);
    // y-suunnassa offsetin merkkisyys osaa siirtää pistettä oikein
    if (itsYOffsetInKM != 0)
      loc.SetLocation(360., itsYOffsetInKM * 1000., usePacificView);

    NFmiCalculationParams modifiedCalculationParams(theCalculationParams);
    modifiedCalculationParams.SetModifiedLatlon(loc.GetLocation(), true);
    if (itsFunctionDataTimeOffsetInHours != 0)
    {
      modifiedCalculationParams.itsTime.ChangeByMinutes(
        static_cast<long>(itsFunctionDataTimeOffsetInHours * 60.f));
    }
    return modifiedCalculationParams;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaMaskPeekXY3::Value(const NFmiCalculationParams &theCalculationParams,
                                      bool fUseTimeInterpolationAlways)
{
  try
  {
    if (itsFunctionDataTimeOffsetInHours != 0) 
      fUseTimeInterpolationAlways = true;
    return NFmiInfoAreaMask::Value(MakeModifiedCalculationParams(theCalculationParams),
                                   fUseTimeInterpolationAlways);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMaskPeekXY3::PressureValue(double thePressure,
                                              const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    return NFmiInfoAreaMask::PressureValue(thePressure,
                                           MakeModifiedCalculationParams(theCalculationParams));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskPeekXY3    **********************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskMetFuncBase *********************
// **********************************************************

NFmiInfoAreaMaskMetFuncBase::~NFmiInfoAreaMaskMetFuncBase() = default;

NFmiInfoAreaMaskMetFuncBase::NFmiInfoAreaMaskMetFuncBase(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    bool thePeekAlongTudes,
    MetFunctionDirection theMetFuncDirection,
    unsigned long thePossibleMetaParamId,
    BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMask(theOperation,
                       theMaskType,
                       theDataType,
                       theInfo,
                       thePossibleMetaParamId,
                       thePostBinaryOperator),
      itsTimeCache(),
      itsGridSizeX(0),
      itsGridSizeY(0),
      itsGridPointWidthInMeters(0),
      itsGridPointHeightInMeters(0),
      itsMetFuncDividerX(0)  // laitetaan 0-arvot oletuksena, nämä 'dividerit' pitää alustaa sitten
                             // ennen varsinaisia laskuja!!!
      ,
      itsMetFuncDividerY(0),
      fTotalWindParam(false),
      fPeekAlongTudes(thePeekAlongTudes)
{
  try
  {
    // emon (NFmiAreaMaskImpl) dataosan asetetaan poikkeuksellisesti tässä
    this->itsMetFunctionDirection = theMetFuncDirection;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMaskMetFuncBase::NFmiInfoAreaMaskMetFuncBase(
    const NFmiInfoAreaMaskMetFuncBase &theOther) = default;

void NFmiInfoAreaMaskMetFuncBase::Initialize()
{
  try
  {
    NFmiInfoAreaMask::Initialize();
    SetGridSizeVariables();
    if (itsInfo->Param().GetParamIdent() == kFmiTotalWindMS)
      fTotalWindParam = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiInfoAreaMaskMetFuncBase::Time(const NFmiMetTime &theTime)
{
  try
  {
    bool status = NFmiInfoAreaMask::Time(theTime);
    itsTimeCache = itsInfo->CalcTimeCache(theTime);
    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiLocationCache NFmiInfoAreaMaskMetFuncBase::CalcLocationCache(const NFmiPoint &theLatlon)
{
  try
  {
    return itsInfo->CalcLocationCache(theLatlon, itsGridSizeX, itsGridSizeY);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskMetFuncBase::SetGridSizeVariables()
{
  try
  {
    if (IsDataOperatable(itsInfo) == false)
      throw Fmi::Exception(BCP, "Error: given data for Met-function was not grid-data");

    itsGridSizeX = itsInfo->Grid()->XNumber();
    itsGridSizeY = itsInfo->Grid()->YNumber();
    double worldXYRectWidth = itsInfo->Grid()->Area()->WorldXYWidth();
    double worldXYRectHeight = itsInfo->Grid()->Area()->WorldXYHeight();
    itsGridPointWidthInMeters = static_cast<float>(worldXYRectWidth / (itsGridSizeX - 1.));
    itsGridPointHeightInMeters = static_cast<float>(worldXYRectHeight / (itsGridSizeY - 1.));
    // nyt kun hilaväli tiedetään, voidaan kutsua funktio, jolla lasketaan met-funktio jakajat
    SetDividers();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiInfoAreaMaskMetFuncBase::IsDataOperatable(
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo) const
{
  try
  {
    if (theInfo && theInfo->Grid())
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiLocationCache NFmiInfoAreaMaskMetFuncBase::CalcPeekedLocation(
    const NFmiLocationCache &theLocationCachePoint, int theOffsetX, int theOffsetY)
{
  try
  {
    if (fPeekAlongTudes)
    {  // lasketaan peek-piste leveys- ja pituuspiirejä pitkin
      bool usePacificView = ::IsPacificViewData(itsInfo);
      NFmiLocation loc(itsInfo->Grid()->GridToLatLon(theLocationCachePoint.itsGridPoint));
      // x-suunnassa siirto ei mielestäni toimi oikein vaan piti laittaa positiiviselle ja
      // negatiiviselle tapauksille omat haarat
      if (theOffsetX > 0)
        loc.SetLocation(90., theOffsetX * itsGridPointWidthInMeters, usePacificView);
      if (theOffsetX < 0)
        loc.SetLocation(270., theOffsetX * itsGridPointWidthInMeters, usePacificView);
      // y-suunnassa offsetin merkkisyys osaa siirtää pistettä oikein
      if (theOffsetY != 0)
        loc.SetLocation(360., theOffsetY * itsGridPointWidthInMeters, usePacificView);
      NFmiLocationCache locationCache = CalcLocationCache(loc.GetLocation());
      return locationCache;
    }

    // lasketaan peek-piste datan oman hilan suuntaisesti
    return NFmiLocationCache::MakePeekedLocation(
        theLocationCachePoint, theOffsetX, theOffsetY, itsGridSizeX, itsGridSizeY);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Oletus, kaikki offset-pyynnöt on valideja, joten rajoja ei tarkastella
float NFmiInfoAreaMaskMetFuncBase::Peek(const NFmiLocationCache &theLocationCachePoint,
                                        int theOffsetX,
                                        int theOffsetY)
{
  try
  {
    NFmiLocationCache peekPoint = CalcPeekedLocation(theLocationCachePoint, theOffsetX, theOffsetY);
    float value = kFloatMissing;
    if (UsePressureLevelInterpolation())
    {
      if (Level()->LevelType() == kFmiFlightLevel)
      {
        double P = ::CalcFlightLevelPressure(UsedPressureLevelValue() * 100);
        value = CalcCachedPressureLevelValue(static_cast<float>(P), peekPoint, itsTimeCache);
      }
      else
        value = CalcCachedPressureLevelValue(
            static_cast<float>(UsedPressureLevelValue()), peekPoint, itsTimeCache);
    }
    else
      value = CalcCachedInterpolation(itsInfo, peekPoint, &itsTimeCache);
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMaskMetFuncBase::CalcCachedPressureLevelValue(
    float P, const NFmiLocationCache &theLocationCache, const NFmiTimeCache &theTimeCache)
{
  try
  {
    if (metaParamDataHolder.isMetaParameterCalculationNeeded())
      return CalcMetaParamCachedPressureLevelValue(P, theLocationCache, theTimeCache);

    return itsInfo->CachedPressureLevelValue(P, theLocationCache, theTimeCache);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMaskMetFuncBase::CalcMetaParamCachedPressureLevelValue(
    float P, const NFmiLocationCache &theLocationCache, const NFmiTimeCache &theTimeCache)
{
  try
  {
    return CalcMetaParamValueWithFunction(
        [&]() { return itsInfo->CachedPressureLevelValue(P, theLocationCache, theTimeCache); });
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMaskMetFuncBase::CalcMetFuncWithFactors(
    const NFmiLocationCache &theLocationCachePoint,
    bool fDoX,
    const CalcFactorVector &theCalcFactors)
{
  try
  {
    float gradValue = kFloatMissing;
    float gradValueSum = 0;
    for (const auto &theCalcFactor : theCalcFactors)
    {
      int offsetX = fDoX ? theCalcFactor.first : 0;
      int offsetY = fDoX ? 0 : theCalcFactor.first;
      float value = Peek(theLocationCachePoint, offsetX, offsetY);
      if (value == kFloatMissing)
        return kFloatMissing;  // jos yksikin kohde arvo oli puuttuvaa, koko juttu on puuttuvaa
      gradValueSum += theCalcFactor.second * value;
    }
    gradValue = gradValueSum / (fDoX ? itsMetFuncDividerX : itsMetFuncDividerY);
    return gradValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// jos ollaan tarpeeksi lähellä oikeaa tai ylä reunaa, myös ulkopuolella, oletetaan että ollaan
// tasan reunalla
// Kaava on (x-suunnassa): (1 * peek(-2, 0) - 4 * peek(-1, 0) + 3 * peek(0, 0)) / (2*deltaX)
float NFmiInfoAreaMaskMetFuncBase::CalcUpperEdgeMetFunc(
    const NFmiLocationCache &theLocationCachePoint, bool fDoX)
{
  try
  {
    float metFuncValue = CalcMetFuncWithFactors(theLocationCachePoint, fDoX, UpperEdgeFactors());
    return metFuncValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// jos ollaan tarpeeksi lähellä vasenta tai ala reunaa, myös ulkopuolella, oletetaan että ollaan
// tasan reunalla
// Kaava on (x-suunnassa): (-1 * peek(2, 0) + 4 * peek(1, 0) - 3 * peek(0, 0)) / (2*deltaX)
float NFmiInfoAreaMaskMetFuncBase::CalcLowerEdgeMetFunc(
    const NFmiLocationCache &theLocationCachePoint, bool fDoX)
{
  try
  {
    float metFuncValue = CalcMetFuncWithFactors(theLocationCachePoint, fDoX, LowerEdgeFactors());
    return metFuncValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tämä on tavallinen tapaus, kun ollaan hilan sisällä
// Kaava on (x-suunnassa): (peek(1, 0) - peek(-1, 0)) / (2*deltaX)
float NFmiInfoAreaMaskMetFuncBase::CalcMiddleAreaMetFunc(
    const NFmiLocationCache &theLocationCachePoint, bool fDoX)
{
  try
  {
    float metFuncValue = CalcMetFuncWithFactors(theLocationCachePoint, fDoX, MiddleAreaFactors());
    return metFuncValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const double gGridEpsilon = 0.00001;

float NFmiInfoAreaMaskMetFuncBase::CalcMetFuncComponent(
    const NFmiLocationCache &theLocationCachePoint,
    bool fDoX,
    double theRealGridPointComponent,
    int theGridSizeComponent)
{
  try
  {
    float metFuncComponent = kFloatMissing;
    if (NFmiQueryDataUtil::IsEqualEnough(theRealGridPointComponent, 0., gGridEpsilon))
      metFuncComponent = CalcLowerEdgeMetFunc(
          theLocationCachePoint, fDoX);  // jos ollaan tarpeeksi lähellä vasenta reunaa, myös
                                         // ulkopuolella, oletetaan että ollaan tasan reunalla
    else if (NFmiQueryDataUtil::IsEqualEnough(
                 theRealGridPointComponent, theGridSizeComponent - 1., gGridEpsilon))
      metFuncComponent = CalcUpperEdgeMetFunc(
          theLocationCachePoint, fDoX);  // jos ollaan tarpeeksi lähellä oikeaa reunaa, myös
                                         // ulkopuolella, oletetaan että ollaan tasan reunalla
    else if (theRealGridPointComponent < 0 ||
             theRealGridPointComponent >= theGridSizeComponent - 1.)
      metFuncComponent =
          kFloatMissing;  // jos ollaan (tarpeeksi) hilan reunojen ulkopuolella, arvoksi missing
    else if (theRealGridPointComponent < 1)
    {  // erikoistapaus jossa ollaan vasemmassa reunassa, reunan ja 1. sarakkeen välissä, lasketaan
       // aproksimaatio
      // eli location pyöristetään lähimpään X-hilaan ja katsotaan ollaanko reunalla vai 'keskellä'
      // ja lasketaan sen mukaan
      NFmiLocationCache approximatedLocation = theLocationCachePoint;
      approximatedLocation.SetToNearestGridPoint(fDoX, !fDoX, itsGridSizeX, itsGridSizeY);
      if (approximatedLocation.itsGridPoint.X() == 0)
        metFuncComponent = CalcLowerEdgeMetFunc(approximatedLocation, fDoX);
      else
        metFuncComponent = CalcMiddleAreaMetFunc(approximatedLocation, fDoX);
    }
    else if (theRealGridPointComponent > theGridSizeComponent - 2.)
    {  // erikoistapaus jossa ollaan oikeassa reunassa, reunan ja viimeistä edellisen sarakkeen
       // välissä, lasketaan approksimaatio
      // eli location pyöristetään lähimpään X-hilaan ja katsotaan ollaanko reunalla vai 'keskellä'
      // ja lasketaan sen mukaan.
      NFmiLocationCache approximatedLocation = theLocationCachePoint;
      approximatedLocation.SetToNearestGridPoint(fDoX, !fDoX, itsGridSizeX, itsGridSizeY);
      if (approximatedLocation.itsGridPoint.X() == theGridSizeComponent - 1)
        metFuncComponent = CalcUpperEdgeMetFunc(approximatedLocation, fDoX);
      else
        metFuncComponent = CalcMiddleAreaMetFunc(approximatedLocation, fDoX);
    }
    else
    {  // tämä on tavallinen tapaus, kun ollaan hilan sisällä
      metFuncComponent = CalcMiddleAreaMetFunc(theLocationCachePoint, fDoX);
    }
    return metFuncComponent;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskMetFuncBase *********************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskGrad    *************************
// **********************************************************

bool NFmiInfoAreaMaskGrad::fCalcFactorVectorsInitialized = false;
NFmiInfoAreaMaskGrad::CalcFactorVector NFmiInfoAreaMaskGrad::itsLowerEdgeFactors;
NFmiInfoAreaMaskGrad::CalcFactorVector NFmiInfoAreaMaskGrad::itsUpperEdgeFactors;
NFmiInfoAreaMaskGrad::CalcFactorVector NFmiInfoAreaMaskGrad::itsMiddleAreaFactors;

void NFmiInfoAreaMaskGrad::InitCalcFactorVectors()
{
  try
  {
    if (NFmiInfoAreaMaskGrad::fCalcFactorVectorsInitialized == false)
    {
      NFmiInfoAreaMaskGrad::itsLowerEdgeFactors.push_back(std::make_pair(2, -1.f));
      NFmiInfoAreaMaskGrad::itsLowerEdgeFactors.push_back(std::make_pair(1, 4.f));
      NFmiInfoAreaMaskGrad::itsLowerEdgeFactors.push_back(std::make_pair(0, -3.f));

      NFmiInfoAreaMaskGrad::itsUpperEdgeFactors.push_back(std::make_pair(-2, 1.f));
      NFmiInfoAreaMaskGrad::itsUpperEdgeFactors.push_back(std::make_pair(-1, -4.f));
      NFmiInfoAreaMaskGrad::itsUpperEdgeFactors.push_back(std::make_pair(0, 3.f));

      NFmiInfoAreaMaskGrad::itsMiddleAreaFactors.push_back(std::make_pair(1, 1.f));
      NFmiInfoAreaMaskGrad::itsMiddleAreaFactors.push_back(std::make_pair(-1, -1.f));

      NFmiInfoAreaMaskGrad::fCalcFactorVectorsInitialized = true;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMaskGrad::~NFmiInfoAreaMaskGrad() = default;

NFmiInfoAreaMaskGrad::NFmiInfoAreaMaskGrad(const NFmiCalculationCondition &theOperation,
                                           Type theMaskType,
                                           NFmiInfoData::Type theDataType,
                                           const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                           bool thePeekAlongTudes,
                                           MetFunctionDirection theMetFuncDirection,
                                           unsigned long thePossibleMetaParamId,
                                           BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMaskMetFuncBase(theOperation,
                                  theMaskType,
                                  theDataType,
                                  theInfo,
                                  thePeekAlongTudes,
                                  theMetFuncDirection,
                                  thePossibleMetaParamId,
                                  thePostBinaryOperator),
      fCalculateDivergence(false)
{
  try
  {
    NFmiInfoAreaMaskGrad::InitCalcFactorVectors();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMaskGrad::NFmiInfoAreaMaskGrad(const NFmiInfoAreaMaskGrad &theOther) = default;

NFmiAreaMask *NFmiInfoAreaMaskGrad::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskGrad(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskGrad::SetDividers()
{
  try
  {
    itsMetFuncDividerX = itsGridPointWidthInMeters * 2;
    itsMetFuncDividerY = itsGridPointHeightInMeters * 2;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaMaskGrad::Value(const NFmiCalculationParams &theCalculationParams,
                                   bool /* fUseTimeInterpolationAlways */)
{
  try
  {
    // Laske gradientti haluttuun pisteeseen ja aikaan.
    // 1. Laske latlon-pistettä vastaava 'reaali'-hilapiste.
  NFmiLocationCache locationCache = CalcLocationCache(theCalculationParams.UsedLatlon());
    // 2. Laske hilojen väli metreinä X- ja Y-suunnassa
    // -on jo laskettu konstruktoreissa SetGridSizeVariables-metodilla -> itsGridPointWidthInMeters
    // ja itsGridPointHeightInMeters
    // 3. Laske gradientti käyttämällä 'reaali'-hilapiste kurkkauksia ja hilojen välisiä
    // etäisyyksiä.
    if (fTotalWindParam)
      itsInfo->Param(
          kFmiWindUMS);  // x-komponenttia alskettaessa käytetäänkin tuulen u-komponenttia!
    float gradX = CalcMetFuncComponent(
        locationCache, true, locationCache.itsGridPoint.X(), itsGridSizeX);  // x-komponentti
    if (fTotalWindParam)
      itsInfo->Param(
          kFmiWindVMS);  // y-komponenttia alskettaessa käytetäänkin tuulen v-komponenttia!
    float gradY = CalcMetFuncComponent(
        locationCache, false, locationCache.itsGridPoint.Y(), itsGridSizeY);  // y-komponentti
    if (itsMetFunctionDirection == DirectionXandY && gradX != kFloatMissing &&
        gradY != kFloatMissing)
    {
      if (fCalculateDivergence)
      {
        float divergence = gradX + gradY;  // lasketaan vain komponentit yhteen
        return divergence;
      }
      else
      {
        float gradScalar = ::sqrt((gradX * gradX) + (gradY * gradY));  // lasketaan komponenttien
        // avulla pituus ja käytetään
        // sitä paluuarvona (skalaari
        // arvo, vaikka gradientti on
        // oikeasti vektori suure)
        return gradScalar;
      }
    }
    else if (itsMetFunctionDirection == DirectionX)
      return gradX;  // jos kyse oli X- tai Y-suuntaisesta laskuista, ei tarvitse tarkistella
                     // puuttuvia arvoja, koska arvot palautetaan vain sellaisenaan
    else if (itsMetFunctionDirection == DirectionY)
      return gradY;

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMaskGrad::PressureValue(double thePressure,
                                           const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    // Aseta korkeus jutut päälle, jotka otetaan huomioon NFmiInfoAreaMaskMetFuncBase::Peek
    // -metodissa
    UsePressureLevelInterpolation(true);
    UsedPressureLevelValue(thePressure);

    //  Kutsu sitten lopuksi nomaalia Value-metodia
    return Value(theCalculationParams, true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskGrad    *************************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskAdvection  **********************
// **********************************************************

NFmiInfoAreaMaskAdvection::~NFmiInfoAreaMaskAdvection() = default;

NFmiInfoAreaMaskAdvection::NFmiInfoAreaMaskAdvection(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfoUwind,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfoVwind,
    bool thePeekAlongTudes,
    MetFunctionDirection theMetFuncDirection,
    unsigned long thePossibleMetaParamId,
    BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMaskGrad(theOperation,
                           theMaskType,
                           theDataType,
                           theInfo,
                           thePeekAlongTudes,
                           theMetFuncDirection,
                           thePossibleMetaParamId,
                           thePostBinaryOperator),
      itsInfoUwind(theInfoUwind),
      itsInfoVwind(theInfoVwind)
{
}

NFmiInfoAreaMaskAdvection::NFmiInfoAreaMaskAdvection(const NFmiInfoAreaMaskAdvection &theOther)
    : NFmiInfoAreaMaskGrad(theOther),
      itsInfoUwind(NFmiAreaMask::DoShallowCopy(theOther.itsInfoUwind)),
      itsInfoVwind(NFmiAreaMask::DoShallowCopy(theOther.itsInfoVwind))
{
}

NFmiAreaMask *NFmiInfoAreaMaskAdvection::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskAdvection(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMaskAdvection::Value(const NFmiCalculationParams &theCalculationParams,
                                        bool /* fUseTimeInterpolationAlways */)
{
  try
  {
    // Laske gradientti haluttuun pisteeseen ja aikaan.
    // 1. Laske latlon-pistettä vastaava 'reaali'-hilapiste.
  NFmiLocationCache locationCache = CalcLocationCache(theCalculationParams.UsedLatlon());
    // 2. Laske hilojen väli metreinä X- ja Y-suunnassa
    // -on jo laskettu konstruktoreissa SetGridSizeVariables-metodilla -> itsGridPointWidthInMeters
    // ja itsGridPointHeightInMeters
    // 3. Laske gradientti käyttämällä 'reaali'-hilapiste kurkkauksia ja hilojen välisiä
    // etäisyyksiä.
    float gradX = CalcMetFuncComponent(
        locationCache, true, locationCache.itsGridPoint.X(), itsGridSizeX);  // x-komponentti
    float gradY = CalcMetFuncComponent(
        locationCache, false, locationCache.itsGridPoint.Y(), itsGridSizeY);  // y-komponentti
    if (itsMetFunctionDirection == DirectionXandY && gradX != kFloatMissing &&
        gradY != kFloatMissing)
    {
      float u = CalcCachedInterpolation(itsInfoUwind, locationCache, &itsTimeCache);
      float v = CalcCachedInterpolation(itsInfoVwind, locationCache, &itsTimeCache);
      if (u != kFloatMissing && v != kFloatMissing)
      {
        float advectionValue =
            (gradX * u + gradY * v) *
            -1;  // huom. gradientti osoittaa kasvavaan suuntaan, joten että advektion
                 // (esim. kylmästä lämpimään on negatiivinen advektio ei positiivinen) merkki
                 // saadaan oikein, pitää gradientin merkki vaihtaa
        return advectionValue;
      }
    }
    else if (itsMetFunctionDirection == DirectionX && gradX != kFloatMissing)
    {
      float u = CalcCachedInterpolation(itsInfoUwind, locationCache, &itsTimeCache);
      if (u != kFloatMissing)
      {
        float advectionValue =
            (gradX * u) * -1;  // huom. gradientti osoittaa kasvavaan suuntaan, joten että advektion
        // (esim. kylmästä lämpimään on negatiivinen advektio ei positiivinen) merkki
        // saadaan oikein, pitää gradientin merkki vaihtaa
        return advectionValue;
      }
    }
    else if (itsMetFunctionDirection == DirectionY && gradY != kFloatMissing)
    {
      float v = CalcCachedInterpolation(itsInfoVwind, locationCache, &itsTimeCache);
      if (v != kFloatMissing)
      {
        float advectionValue =
            (gradY * v) * -1;  // huom. gradientti osoittaa kasvavaan suuntaan, joten että advektion
        // (esim. kylmästä lämpimään on negatiivinen advektio ei positiivinen) merkki
        // saadaan oikein, pitää gradientin merkki vaihtaa
        return advectionValue;
      }
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskAdvection  **********************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskLaplace    **********************
// **********************************************************

bool NFmiInfoAreaMaskLaplace::fCalcFactorVectorsInitialized = false;
NFmiInfoAreaMaskLaplace::CalcFactorVector NFmiInfoAreaMaskLaplace::itsLowerEdgeFactors;
NFmiInfoAreaMaskLaplace::CalcFactorVector NFmiInfoAreaMaskLaplace::itsUpperEdgeFactors;
NFmiInfoAreaMaskLaplace::CalcFactorVector NFmiInfoAreaMaskLaplace::itsMiddleAreaFactors;

void NFmiInfoAreaMaskLaplace::InitCalcFactorVectors()
{
  try
  {
    if (NFmiInfoAreaMaskLaplace::fCalcFactorVectorsInitialized == false)
    {
      NFmiInfoAreaMaskLaplace::itsLowerEdgeFactors.push_back(std::make_pair(2, 1.f));
      NFmiInfoAreaMaskLaplace::itsLowerEdgeFactors.push_back(std::make_pair(1, -2.f));
      NFmiInfoAreaMaskLaplace::itsLowerEdgeFactors.push_back(std::make_pair(0, 1.f));

      NFmiInfoAreaMaskLaplace::itsUpperEdgeFactors.push_back(std::make_pair(-2, 1.f));
      NFmiInfoAreaMaskLaplace::itsUpperEdgeFactors.push_back(std::make_pair(-1, -2.f));
      NFmiInfoAreaMaskLaplace::itsUpperEdgeFactors.push_back(std::make_pair(0, 1.f));

      NFmiInfoAreaMaskLaplace::itsMiddleAreaFactors.push_back(std::make_pair(1, 1.f));
      NFmiInfoAreaMaskLaplace::itsMiddleAreaFactors.push_back(std::make_pair(0, -2.f));
      NFmiInfoAreaMaskLaplace::itsMiddleAreaFactors.push_back(std::make_pair(-1, 1.f));

      NFmiInfoAreaMaskLaplace::fCalcFactorVectorsInitialized = true;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMaskLaplace::~NFmiInfoAreaMaskLaplace() = default;

NFmiInfoAreaMaskLaplace::NFmiInfoAreaMaskLaplace(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    bool thePeekAlongTudes,
    MetFunctionDirection theMetFuncDirection,
    unsigned long thePossibleMetaParamId,
    BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMaskMetFuncBase(theOperation,
                                  theMaskType,
                                  theDataType,
                                  theInfo,
                                  thePeekAlongTudes,
                                  theMetFuncDirection,
                                  thePossibleMetaParamId,
                                  thePostBinaryOperator)
{
  try
  {
    NFmiInfoAreaMaskLaplace::InitCalcFactorVectors();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMaskLaplace::NFmiInfoAreaMaskLaplace(const NFmiInfoAreaMaskLaplace &theOther) = default;

NFmiAreaMask *NFmiInfoAreaMaskLaplace::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskLaplace(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskLaplace::SetDividers()
{
  try
  {
    itsMetFuncDividerX = itsGridPointWidthInMeters * itsGridPointWidthInMeters;
    itsMetFuncDividerY = itsGridPointHeightInMeters * itsGridPointHeightInMeters;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaMaskLaplace::Value(const NFmiCalculationParams &theCalculationParams,
                                      bool /* fUseTimeInterpolationAlways */)
{
  try
  {
    // Laske gradientti haluttuun pisteeseen ja aikaan.
    // 1. Laske latlon-pistettä vastaava 'reaali'-hilapiste.
  NFmiLocationCache locationCache = CalcLocationCache(theCalculationParams.UsedLatlon());
    // 2. Laske hilojen väli metreinä X- ja Y-suunnassa
    // -on jo laskettu konstruktoreissa SetGridSizeVariables-metodilla -> itsGridPointWidthInMeters
    // ja itsGridPointHeightInMeters
    // 3. Laske gradientti käyttämällä 'reaali'-hilapiste kurkkauksia ja hilojen välisiä
    // etäisyyksiä.
    if (fTotalWindParam)
      itsInfo->Param(
          kFmiWindUMS);  // x-komponenttia alskettaessa käytetäänkin tuulen u-komponenttia!
    float laplaceX = CalcMetFuncComponent(
        locationCache, true, locationCache.itsGridPoint.X(), itsGridSizeX);  // x-komponentti
    if (fTotalWindParam)
      itsInfo->Param(
          kFmiWindVMS);  // y-komponenttia alskettaessa käytetäänkin tuulen v-komponenttia!
    float laplaceY = CalcMetFuncComponent(
        locationCache, false, locationCache.itsGridPoint.Y(), itsGridSizeY);  // y-komponentti
    if (itsMetFunctionDirection == DirectionXandY && laplaceX != kFloatMissing &&
        laplaceY != kFloatMissing)
    {
      float laplaceScalar = laplaceX + laplaceY;
      return laplaceScalar;
    }
    else if (itsMetFunctionDirection == DirectionX)
      return laplaceX;  // jos kyse oli X- tai Y-suuntaisesta laskuista, ei tarvitse tarkistella
                        // puuttuvia arvoja, koska arvot palautetaan vain sellaisenaan
    else if (itsMetFunctionDirection == DirectionY)
      return laplaceY;

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskLaplace    **********************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskRotor      **********************
// **********************************************************

NFmiInfoAreaMaskRotor::~NFmiInfoAreaMaskRotor() = default;

NFmiInfoAreaMaskRotor::NFmiInfoAreaMaskRotor(const NFmiCalculationCondition &theOperation,
                                             Type theMaskType,
                                             NFmiInfoData::Type theDataType,
                                             const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                             bool thePeekAlongTudes,
                                             MetFunctionDirection theMetFuncDirection,
                                             unsigned long thePossibleMetaParamId,
                                             BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMaskGrad(theOperation,
                           theMaskType,
                           theDataType,
                           theInfo,
                           thePeekAlongTudes,
                           theMetFuncDirection,
                           thePossibleMetaParamId,
                           thePostBinaryOperator)
{
}

NFmiInfoAreaMaskRotor::NFmiInfoAreaMaskRotor(const NFmiInfoAreaMaskRotor &theOther) = default;

NFmiAreaMask *NFmiInfoAreaMaskRotor::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskRotor(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaMaskRotor::Value(const NFmiCalculationParams &theCalculationParams,
                                    bool /* fUseTimeInterpolationAlways */)
{
  try
  {
    // Laske gradientti haluttuun pisteeseen ja aikaan.
    // 1. Laske latlon-pistettä vastaava 'reaali'-hilapiste.
  NFmiLocationCache locationCache = CalcLocationCache(theCalculationParams.UsedLatlon());
    // 2. Laske hilojen väli metreinä X- ja Y-suunnassa
    // -on jo laskettu konstruktoreissa SetGridSizeVariables-metodilla -> itsGridPointWidthInMeters
    // ja itsGridPointHeightInMeters
    // 3. Laske rotor käyttämällä 'reaali'-hilapiste kurkkauksia ja hilojen välisiä etäisyyksiä.
    itsInfo->Param(kFmiWindVMS);  // x-komponenttia alskettaessa käytetäänkin tuulen v-komponenttia!
    float rotX = CalcMetFuncComponent(
        locationCache, true, locationCache.itsGridPoint.X(), itsGridSizeX);  // x-komponentti
    itsInfo->Param(kFmiWindUMS);  // y-komponenttia alskettaessa käytetäänkin tuulen u-komponenttia!
    float rotY = CalcMetFuncComponent(
        locationCache, false, locationCache.itsGridPoint.Y(), itsGridSizeY);  // y-komponentti
    if (itsMetFunctionDirection == DirectionXandY && rotX != kFloatMissing && rotY != kFloatMissing)
    {
      float rotScalar =
          ::sqrt((rotX * rotX) + (rotY * rotY));  // lasketaan komponenttien avulla
                                                  // pituus ja käytetään sitä
                                                  // paluuarvona (skalaari arvo, vaikka
                                                  // rotor on oikeasti vektori suure)
      return rotScalar;
    }
    else if (itsMetFunctionDirection == DirectionX)
      return rotX;  // jos kyse oli X- tai Y-suuntaisesta laskuista, ei tarvitse tarkistella
                    // puuttuvia arvoja, koska arvot palautetaan vain sellaisenaan
    else if (itsMetFunctionDirection == DirectionY)
      return rotY;

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskRotor      **********************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskVertFunc   **********************
// **********************************************************

NFmiInfoAreaMaskVertFunc::~NFmiInfoAreaMaskVertFunc() = default;

NFmiInfoAreaMaskVertFunc::NFmiInfoAreaMaskVertFunc(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType thePrimaryFunc,
    NFmiAreaMask::FunctionType theSecondaryFunc,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskMetFuncBase(theOperation,
                                  theMaskType,
                                  theDataType,
                                  theInfo,
                                  false,
                                  NoDirection,
                                  thePossibleMetaParamId,
                                  kNoValue),
      itsPrimaryFunc(thePrimaryFunc),
      itsSecondaryFunc(theSecondaryFunc),
      itsArgumentVector(),
      itsFunctionModifier(),
      itsStartLevelValue(kFloatMissing),
      itsEndLevelValue(kFloatMissing),
      itsStartLevelIndex(gMissingIndex),
      itsEndLevelIndex(gMissingIndex),
      itsUsedHeightParId(kFmiBadParameter),
      fReturnHeightValue(false),
      itsLevelIncrement(1),
      fReverseLevels(false)
{
  try
  {
    itsFunctionArgumentCount = theArgumentCount;
    if (itsPrimaryFunc == NFmiAreaMask::MinH || itsPrimaryFunc == NFmiAreaMask::MaxH ||
        itsPrimaryFunc == NFmiAreaMask::FindH || itsPrimaryFunc == NFmiAreaMask::FindHeightCond)
      fReturnHeightValue = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMaskVertFunc::NFmiInfoAreaMaskVertFunc(const NFmiInfoAreaMaskVertFunc &theOther)
    : NFmiInfoAreaMaskMetFuncBase(theOther),
      itsPrimaryFunc(theOther.itsPrimaryFunc),
      itsSecondaryFunc(theOther.itsSecondaryFunc),
      itsArgumentVector(theOther.itsArgumentVector),
      itsFunctionModifier(theOther.itsFunctionModifier ? theOther.itsFunctionModifier->Clone()
                                                       : nullptr),
      itsStartLevelValue(theOther.itsStartLevelValue),
      itsEndLevelValue(theOther.itsEndLevelValue),
      itsStartLevelIndex(theOther.itsStartLevelIndex),
      itsEndLevelIndex(theOther.itsEndLevelIndex),
      itsUsedHeightParId(theOther.itsUsedHeightParId),
      fReturnHeightValue(theOther.fReturnHeightValue),
      itsLevelIncrement(theOther.itsLevelIncrement),
      fReverseLevels(theOther.fReverseLevels)
{
}

NFmiAreaMask *NFmiInfoAreaMaskVertFunc::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskVertFunc(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskVertFunc::Initialize()
{
  try
  {
    NFmiInfoAreaMaskMetFuncBase::Initialize();

    itsFunctionModifier = NFmiInfoAreaMask::CreateIntegrationFuction(itsPrimaryFunc);

    switch (itsSecondaryFunc)
    {
      case NFmiAreaMask::VertP:
      case NFmiAreaMask::VertFL:
      case NFmiAreaMask::TimeVertP:
      case NFmiAreaMask::TimeVertFL:
        itsUsedHeightParId = kFmiPressure;
        break;
      case NFmiAreaMask::VertZ:
      case NFmiAreaMask::TimeVertZ:
        itsUsedHeightParId = kFmiGeomHeight;
        break;
      case NFmiAreaMask::VertHyb:
      case NFmiAreaMask::TimeVertHyb:
        itsUsedHeightParId = kFmiModelLevel;
        if (itsInfo->LevelType() != kFmiHybridLevel)
          throw Fmi::Exception(
              BCP,
              "Model-level vertical function has called but data doesn't have model levels,\ncan't "
              "execute the calculations.");
        break;
      default:
        throw Fmi::Exception(BCP,
                             "Internal SmartMet error: Vertical function has unknown height "
                             "parameter,\ncan't execute "
                             "the calculations.");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskVertFunc::SetArguments(std::vector<float> &theArgumentVector)
{
  try
  {
    // jokaiselle pisteelle ja ajanhetkelle annetaan eri argumentit tässä
    itsArgumentVector = theArgumentVector;
    if (static_cast<int>(itsArgumentVector.size()) !=
        itsFunctionArgumentCount - 1)  // -1 tarkoittaa että funktion 1. argumentti tulee suoraan
                                       // itsIfo:sta, eli sitä ei anneta argumentti-listassa
      throw Fmi::Exception(BCP,
                           "Internal SmartMet error: Vertical function was given invalid number of "
                           "arguments, cannot "
                           "calculate the macro.");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsBetweenValues(double value, double value1, double value2)
{
  try
  {
    if (value >= value1 && value <= value2)
      return true;
    if (value >= value2 && value <= value1)
      return true;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskVertFunc::SetLevelIndexies(float theHeightValue)
{
  try
  {
    if (theHeightValue != kFloatMissing)
    {
      if (::IsBetweenValues(theHeightValue, itsStartLevelValue, itsEndLevelValue))
      {  // eli löytyi ei puuttuva paine arvo ja se oli haluttujen paine-rajojen sisällä
        if (itsStartLevelIndex ==
            gMissingIndex)  // jos ei ole vielä alaraja indeksiä, laitetaan se nyt
          itsStartLevelIndex = itsInfo->LevelIndex();
        itsEndLevelIndex = itsInfo->LevelIndex();  // päivitetään ylärajaindeksiä niin kauan kuin
                                                   // ollaan rajojen sisällä
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsInsideLimits(float low, float high, float value)
{
  try
  {
    return (value >= low) && (value <= high);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskVertFunc::SearchLevels(const NFmiLocationCache &theLocationCache)
{
  try
  {
    if (itsInfo)
    {
      if (itsUsedHeightParId == kFmiModelLevel)
      {
        itsInfo->LastLevel();
        float lastExistingLevelValue = itsInfo->Level()->LevelValue();

        itsInfo->FirstLevel();
        float firstExistingLevelValue = itsInfo->Level()->LevelValue();

        if (firstExistingLevelValue > lastExistingLevelValue)
          std::swap(firstExistingLevelValue, lastExistingLevelValue);

        if (itsStartLevelValue > itsEndLevelValue)
          std::swap(itsStartLevelValue, itsEndLevelValue);

        if (itsStartLevelValue < firstExistingLevelValue)
          itsStartLevelValue = firstExistingLevelValue;

        if (itsEndLevelValue > lastExistingLevelValue)
          itsEndLevelValue = lastExistingLevelValue;

        // float closestInsideStartLevelValue = kFloatMissing;
        float minStartDiff = 99999999.f;
        // float closestInsideEndLevelValue = kFloatMissing;
        float minEndDiff = 99999999.f;
        for (itsInfo->ResetLevel(); itsInfo->NextLevel();)
        {
          const NFmiLevel *aLevel = itsInfo->Level();

          if (::IsInsideLimits(itsStartLevelValue, itsEndLevelValue, aLevel->LevelValue()))
          {
            // alarajan etsintä koodia
            if (minStartDiff > 0)  // ei ole vielä löytynyt tarkkaa leveliä
            {
              float currentDiff = ::fabs(itsStartLevelValue - aLevel->LevelValue());
              if (currentDiff < minStartDiff)
              {
                // closestInsideStartLevelValue = aLevel->LevelValue();
                minStartDiff = currentDiff;
                itsStartLevelIndex = itsInfo->LevelIndex();
              }
            }

            // ylärajan etsintä koodia
            if (minEndDiff > 0)  // ei ole vielä löytynyt tarkkaa leveliä
            {
              float currentDiff = ::fabs(itsEndLevelValue - aLevel->LevelValue());
              if (currentDiff < minEndDiff)
              {
                // closestInsideEndLevelValue = aLevel->LevelValue();
                minEndDiff = currentDiff;
                itsEndLevelIndex = itsInfo->LevelIndex();
              }
            }
          }
        }

        if (itsStartLevelIndex > itsEndLevelIndex)
          std::swap(itsStartLevelIndex, itsEndLevelIndex);
      }
      else
      {
        for (itsInfo->ResetLevel(); itsInfo->NextLevel();)
        {
          float Pvalue =
              itsInfo->GetLevelHeightValue(itsUsedHeightParId, theLocationCache, itsTimeCache);
          SetLevelIndexies(Pvalue);
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static float ConvertFL2P(float FL)
{
  try
  {
    if (FL != kFloatMissing)
      return static_cast<float>(::CalcFlightLevelPressure(FL));

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskVertFunc::SetLevelValues()
{
  try
  {
    // otetaan argumentteina annetut arvot haettaviin level-rajoihin
    itsStartLevelValue = itsArgumentVector[0];
    itsEndLevelValue = kFloatMissing;  // Huom! tämä voi olla puuttuva, jos kyse on simppelistä
                                       // get-funktiosta (esim. vertp_get(WS_Hir, pressure))
    if (itsArgumentVector.size() > 1)
      itsEndLevelValue = itsArgumentVector[1];
    // mahdolliset FL:t pitää muuttaa paineeksi [hPa]
    if (itsSecondaryFunc == NFmiAreaMask::VertFL)
    {  // annetut lentopinnat ovat hehto jalkoina, ne pitää muuttaa jaloiksi ennen konversiota
      itsStartLevelValue = ::ConvertFL2P(itsStartLevelValue * 100);
      itsEndLevelValue = ::ConvertFL2P(itsEndLevelValue * 100);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskVertFunc::FindCalculatedLeves(const NFmiLocationCache &theLocationCache)
{
  try
  {
    // nollataan ensin level-indeksit
    itsStartLevelIndex = gMissingIndex;
    itsEndLevelIndex = gMissingIndex;
    itsLevelIncrement = 1;
    fReverseLevels = false;

    SearchLevels(theLocationCache);

    // Nyt on aloitus ja lopetus levelit löydetty. Nyt on vielä pääteltävä mihin suuntaan levelit
    // menevät.
    // Data on tarkoitus käydä läpi mannpinnasta ylöspäin, joten on tehtävä lisä tarkasteluja.
    // Eli jos datan-levelien suunta on alaspäin = avaruudesta kohti maanpintaa, on incrementti -1
    // ja levelIndeksit käännetään.
    if (itsStartLevelIndex != gMissingIndex &&
        itsEndLevelIndex != gMissingIndex)  // lisäksi levelIndeksien pitää olla järkeviä
    {
      if (itsUsedHeightParId == kFmiPressure)
      {
        if (itsInfo->PressureParamIsRising())
          itsLevelIncrement = -1;  // PressureParamIsRising = true -> levelien vertikaali suunta on
                                   // alaspäin (avaruudesta kohti maanpintaa)
      }
      else
      {
        if (itsInfo->HeightParamIsRising() == false)
          itsLevelIncrement = -1;  // HeightParamIsRising = false -> levelien vertikaali suunta on
                                   // alaspäin (avaruudesta kohti maanpintaa)
      }

      if (itsLevelIncrement == -1)
      {
        std::swap(
            itsStartLevelIndex,
            itsEndLevelIndex);  // jos incrementti oli -1, pitää levelIndeksit vaihtaa päittäin
        fReverseLevels = true;
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMaskVertFunc::DoGetFunction(const NFmiLocationCache &theLocationCache,
                                              const NFmiCalculationParams &theCalculationParams,
                                              float theLevelValue)
{
  try
  {
    if (itsSecondaryFunc == NFmiAreaMask::VertZ)
    {
      return static_cast<float>(HeightValue(theLevelValue, theCalculationParams));
    }
    else if (itsSecondaryFunc == NFmiAreaMask::VertP || itsSecondaryFunc == NFmiAreaMask::VertFL)
    {
      return static_cast<float>(PressureValue(theLevelValue, theCalculationParams));
    }
    else
    {  // else hybrid arvo suoraan
      itsInfo->FirstLevel();
      NFmiLevel aLevel(*itsInfo->Level());
      aLevel.LevelValue(theLevelValue);
      itsInfo->Level(aLevel);
      return CalcCachedInterpolation(itsInfo, theLocationCache, &itsTimeCache);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Oletus: annetut currentPressureLevel ja usedDeltaZ ovat jo tarkistettu, että eivät ole missing.
float NFmiInfoAreaMaskVertFunc::CalculateUsedPeekZPressureLevel(float currentPressureLevel,
                                                                float usedDeltaZ)
{
  switch (itsSecondaryFunc)
  {
    case NFmiAreaMask::VertP:
    {
      return currentPressureLevel + usedDeltaZ;
    }
    case NFmiAreaMask::VertFL:
    {
      // Muutetaan ensin perus painepinta flight-leveliksi
      auto usedFLlevel = ::CalcPressureFlightLevel(currentPressureLevel);
      usedFLlevel += usedDeltaZ;
      // Lopuksi muutetaan yhteis flight-level takaisin paineeksi
      return static_cast<float>(::CalcFlightLevelPressure(usedFLlevel * 100));
    }
    case NFmiAreaMask::VertZ:
    {
      auto usedHeightlevel = ::CalcHeightAtPressure(currentPressureLevel) * 1000;
      usedHeightlevel += usedDeltaZ;
      return static_cast<float>(::CalcPressureAtHeight(usedHeightlevel / 1000.));
    }
    default:
      return kFloatMissing;
  }
}

float NFmiInfoAreaMaskVertFunc::DoPeekZFunction(const NFmiCalculationParams &theCalculationParams,
                                                float theDeltaZ)
{
  if (!theCalculationParams.fSpecialCalculationCase)
    throw std::runtime_error("Don't use peekZ functions for non cross-section calculations");
  if (theCalculationParams.itsPressureHeight != kFloatMissing && theDeltaZ != kFloatMissing)
  {
    auto usedPressureLevelValue =
        CalculateUsedPeekZPressureLevel(theCalculationParams.itsPressureHeight, theDeltaZ);

    return static_cast<float>(PressureValue(usedPressureLevelValue, theCalculationParams));
  }
  return kFloatMissing;
}

static bool IsUnder(float theSearchedValue, float theCurrentValue)
{
  try
  {
    // OLETUS! Verrattavat arvot eivät saa olla puuttuvia!!!!
    return theSearchedValue < theCurrentValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

unsigned long NFmiInfoAreaMaskVertFunc::GetNonMissingStartLevelIndex(
    const NFmiLocationCache &theLocationCache, const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    VerticalIterationBreakingData iterationBreakingData(true);
    // Lambda funktio joka annetaan iterointi metodille
    auto findLevelFunction = [&]()
    {
      float value = CalcCachedInterpolation(itsInfo, theLocationCache, &itsTimeCache);
      if (value != kFloatMissing)
      {
        iterationBreakingData.index = itsInfo->LevelIndex();
        iterationBreakingData.stopIteration = true;
      }
    };
    IterateLevelsFromGroundUpward(findLevelFunction,
                                  iterationBreakingData,
                                  static_cast<int>(itsStartLevelIndex),
                                  theCalculationParams);
    return iterationBreakingData.index;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Laskee logaritmisessa asteikossa interpoloidun arvon.
// Käytetään esim. logaritmisen paine asteikon kanssa.
// Palauttaa x:ää vastaavan y:n, kun x1 arvoa vastaa y1 ja x2:n arvoa vastaa y2.
// eli x-parametrit ovat logaritmisella asteikolla (eli paineet ovat x-lukuja).
// Palauttaa x:ää vastaavan luvun y, joka on siis interpoloitu logaritmisesti.
// *********************************************************************************
// HUOM! Tämä on kopioitu suoraan NFmiFastQueryInfo.cpp:stä. Funktio pitäisi laittaa
// johonkin ylaiseen paikkaan molempien käytettäväksi!!!!
// *********************************************************************************
/*
static double CalcLogInterpolatedValue(double x1, double x2, double x, double y1, double y2)
{
    double y = kFloatMissing;
    if(x1 != kFloatMissing && x2 != kFloatMissing && x != kFloatMissing)
    {
        if(x1 == x2)
            y = y1 != kFloatMissing ? y1 : y2;
        else if(y1 != kFloatMissing && y2 != kFloatMissing)
        {
            double w = (::log(x)-::log(x1)) / (::log(x2)-::log(x1));
            y = (1-w)*y1 + w*y2;
        }
        else if(y1 != kFloatMissing) // jos toinen -arvoista puuttuu annetaan arvoksi toinen
            y = y1;
        else if(y2 != kFloatMissing) // jos toinen -arvoista puuttuu annetaan arvoksi toinen
            y = y2;
    }
    return y;
}
*/

float NFmiInfoAreaMaskVertFunc::GetLevelHeightValue(const NFmiLocationCache &theLocationCache)
{
  try
  {
    if (itsUsedHeightParId == kFmiModelLevel)
      return itsInfo->Level()->LevelValue();

    return itsInfo->GetLevelHeightValue(itsUsedHeightParId, theLocationCache, itsTimeCache);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

class SimpleConditionPreviousValueResetter
{
  std::shared_ptr<NFmiSimpleCondition> &simpleCondition_;

 public:
  SimpleConditionPreviousValueResetter(std::shared_ptr<NFmiSimpleCondition> &simpleCondition)
      : simpleCondition_(simpleCondition)
  {
  }
  ~SimpleConditionPreviousValueResetter() { simpleCondition_->ResetPreviousValue(); }
};

// Etsitään ne kohdat missä simple-condition menee päälle.
// Eli ne kohdat missä edellisen kerroksen simple-condition arvo oli false ja nykyisen levelin arvo
// on true. Jos 1. kerroksen simple-condition on heti päällä, lasketaan se 1. kohdaksi.
float NFmiInfoAreaMaskVertFunc::FindHeightForSimpleCondition(
    const NFmiLocationCache &theLocationCache, const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    SimpleConditionPreviousValueResetter simpleConditionPreviousValueResetter(itsSimpleCondition);
    bool findHeight = itsPrimaryFunc == NFmiAreaMask::FindHeightCond;
    // kuinka mones osuma haetaan, 1 on 1. 2 on 2. jne. 0 (tai <0) on viimeinen
    int search_nth_value = findHeight ? static_cast<int>(::round(itsArgumentVector[2])) : 0;
    bool previousLevelCondition = false;
    bool currentLevelCondition = false;
    float previousLevelHeigth = kFloatMissing;
    float currentLevelHeigth = kFloatMissing;
    float foundHeight = kFloatMissing;
    int foundCount = 0;
    unsigned long realStartLevelIndex =
        GetNonMissingStartLevelIndex(theLocationCache, theCalculationParams);
    if (realStartLevelIndex != gMissingIndex)
    {
      // Otetaan 1. levelin arvot previous-muuttujiin, jotta loopitus voidaan alkaa toisesta ilman
      // kikkailuja
      itsInfo->LevelIndex(realStartLevelIndex);
      previousLevelCondition = VertFuncSimpleconditionCheck(theCalculationParams);
      previousLevelHeigth = GetLevelHeightValue(theLocationCache);
      // Jos 1. levelin kohdalla on tilanne jo päällä, laitetaan sen tiedot talteen ja kasvatetaan
      // löytölaskuria
      if (previousLevelCondition && previousLevelHeigth != kFloatMissing)
      {
        foundCount++;
        foundHeight = previousLevelHeigth;
        if (findHeight && search_nth_value == 1) 
          return foundHeight;
      }

      VerticalIterationBreakingData iterationBreaking(true);
      // Lambda funktio joka annetaan iterointi metodille
      auto findValueFunction = [&]()
      {
        currentLevelCondition = VertFuncSimpleconditionCheck(theCalculationParams);
        currentLevelHeigth = GetLevelHeightValue(theLocationCache);
        // simple-condition tila muuttunut niin tilanne on mennyt päälle ja otetaan tilanne talteen
        if (currentLevelHeigth != kFloatMissing)
        {
          if (previousLevelCondition != currentLevelCondition)
          {
            foundCount++;
            // Laitetaan löytökorkeudeksi sen levelin arvo missä simple-condition oli päällä
            foundHeight = currentLevelCondition ? currentLevelHeigth : previousLevelHeigth;
            // Jos etsitään tiettyä esim. 3. löytöä ja foundCount pääsee 3:een, palautetaan
            // välittömästi arvo
            if (search_nth_value > 0 && search_nth_value <= foundCount)
              iterationBreaking.stopIteration = true;
          }
          previousLevelCondition = currentLevelCondition;
          previousLevelHeigth = currentLevelHeigth;
        }
      };
      IterateLevelsFromGroundUpward(findValueFunction,
                                    iterationBreaking,
                                    static_cast<int>(realStartLevelIndex + itsLevelIncrement),
                                    theCalculationParams);
    }
    // Palautetaan viimeisin löytynyt arvo (tai missing, jos ei löytynyt yhtään)
    if (findHeight)
      return foundHeight;

    return static_cast<float>(foundCount);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMaskVertFunc::DoFindFunction(const NFmiLocationCache &theLocationCache,
                                               const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    bool findHeight = itsPrimaryFunc == NFmiAreaMask::FindH;
    if (itsArgumentVector.size() < 3)
      return kFloatMissing;
    float searchedValue = itsArgumentVector[2];
    if (searchedValue == kFloatMissing)
      return kFloatMissing;
    if (findHeight && itsArgumentVector.size() < 4)
      return kFloatMissing;
    // kuinka mones osuma haetaan, 1 on 1. 2 on 2. jne. 0 (tai <0) on viimeinen
    int search_nth_value = findHeight ? static_cast<int>(::round(itsArgumentVector[3])) : 0;

    float value1 = kFloatMissing;  // tässä on tallessa viimeistä edellinen kunnollinen arvo
    float value2 = kFloatMissing;  // tässä on tallessa viimeisin kunnollinen arvo
    float height1 =
        kFloatMissing;  // tässä on tallessa viimeistä edellisen kunnollisen arvon korkeus
    float height2 = kFloatMissing;  // tässä on tallessa viimeisimmän kunnollisen arvon korkeus
    float foundHeight = kFloatMissing;
    int foundCount = 0;
    unsigned long realStartLevelIndex =
        GetNonMissingStartLevelIndex(theLocationCache, theCalculationParams);
    if (realStartLevelIndex != gMissingIndex)
    {
      itsInfo->LevelIndex(realStartLevelIndex);
      value1 = CalcCachedInterpolation(itsInfo, theLocationCache, &itsTimeCache);
      height1 = GetLevelHeightValue(theLocationCache);

      bool isUnder = ::IsUnder(searchedValue, value1);

      VerticalIterationBreakingData iterationBreaking(true);
      // Lambda funktio joka annetaan iterointi metodille
      auto findValueFunction = [&]()
      {
        value2 = CalcCachedInterpolation(itsInfo, theLocationCache, &itsTimeCache);
        if (value2 != kFloatMissing)
        {
          height2 = GetLevelHeightValue(theLocationCache);
          bool isUnder2 = ::IsUnder(searchedValue, value2);
          if (isUnder != isUnder2)
          {
            foundCount++;
            // löytyi etsittävä arvo, lasketaan korkeudelle arvio
            foundHeight = static_cast<float>(
                NFmiInterpolation::Linear(searchedValue, value1, value2, height1, height2));
            // Jos etsitään tiettyä esim. 3. löytöä ja foundCount
            // pääsee 3:een, palautetaan välittömästi arvo
            if (search_nth_value > 0 && search_nth_value <= foundCount)
              iterationBreaking.stopIteration = true;
          }
          value1 = value2;    // huom! vain ei puuttuvia arvoja saa siirtää!
          height1 = height2;  // huom! vain ei puuttuvia arvoja saa siirtää!
          isUnder = isUnder2;
        }
      };
      IterateLevelsFromGroundUpward(findValueFunction,
                                    iterationBreaking,
                                    static_cast<int>(realStartLevelIndex + itsLevelIncrement),
                                    theCalculationParams);
    }
    // Palautetaan viimeisin löytynyt arvo (tai missing, jos ei löytynyt yhtään)
    if (findHeight)
      return foundHeight;

    return static_cast<float>(foundCount);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMaskVertFunc::DoVerticalGrad(const NFmiLocationCache &theLocationCache,
                                               const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    float value1 = DoGetFunction(theLocationCache, theCalculationParams, itsStartLevelValue);
    float value2 = DoGetFunction(theLocationCache, theCalculationParams, itsEndLevelValue);
    if (value1 != kFloatMissing && value2 != kFloatMissing)
    {
      float result = ::fabs(value2 - value1) / ::fabs(itsEndLevelValue - itsStartLevelValue);
      return result;
    }

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaMaskVertFunc::Value(const NFmiCalculationParams &theCalculationParams,
                                       bool /* fUseTimeInterpolationAlways */)
{
  try
  {
  // Pikaviritys poikkileikkausnäytön peek-z funktiolle (pitäisi tehdä oma luokka hanskaamaan)
    if (itsPrimaryFunc == NFmiAreaMask::PeekZ)
      return DoPeekZFunction(theCalculationParams, itsArgumentVector[0]);

    SetLevelValues();
    if (itsStartLevelValue == kFloatMissing)
      return kFloatMissing;  // jos jo alku level arvo on puuttuvaa, ei voi tehdä mitään järkevää
    if (itsEndLevelValue == kFloatMissing &&
      (itsPrimaryFunc != NFmiAreaMask::Get && itsPrimaryFunc != NFmiAreaMask::PeekZ))
      return kFloatMissing;  // jos jo loppu level arvo on puuttuvaa, eikä kyse ollut
                             // get-funktiosta, ei voi tehdä mitään järkevää

    // 1. Laske latlon-pistettä vastaava 'reaali'-hilapiste.
  NFmiLocationCache locationCache = CalcLocationCache(theCalculationParams.UsedLatlon());

    if (itsPrimaryFunc == NFmiAreaMask::Get)
      return DoGetFunction(locationCache, theCalculationParams, itsStartLevelValue);
    else if (itsPrimaryFunc == NFmiAreaMask::Grad)
      return DoVerticalGrad(locationCache, theCalculationParams);

    // 2. Käy läpi haluttu level korkeus/level väli ja laske haluttu operaatio niille
    FindCalculatedLeves(locationCache);

    if (itsStartLevelIndex == gMissingIndex || itsEndLevelIndex == gMissingIndex)
      return kFloatMissing;  // jos jompikumpi level indekseistä osoittaa 'kuuseen', ei kannata
                             // jatkaa

    float value = kFloatMissing;
    if (::IsFindFunction(itsPrimaryFunc))
      value = DoFindFunction(locationCache, theCalculationParams);
    else if (::IsSimpleConditionFindFunction(itsPrimaryFunc))
      value = FindHeightForSimpleCondition(locationCache, theCalculationParams);
    else
      value = DoNormalFunction(locationCache, theCalculationParams);

    if (fReturnHeightValue && itsSecondaryFunc == NFmiAreaMask::VertFL)
      value = static_cast<float>(::CalcPressureFlightLevel(value));

    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

class DoubleValueSearcher
{
 public:
  DoubleValueSearcher(bool doSearchMax)
      : fSearchMax(doSearchMax),
        itsExtremeValue(kFloatMissing),
        itsExtremeSecondaryValue(kFloatMissing)
  {
  }

  void Values(float primaryValue, float secondaryValue)
  {
    try
    {
      if (primaryValue != kFloatMissing && secondaryValue != kFloatMissing)
      {
        if (IsExtreme(primaryValue))
        {
          itsExtremeValue = primaryValue;
          itsExtremeSecondaryValue = secondaryValue;
        }
      }
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }

  float ExtremeValue() const { return itsExtremeValue; }
  float ExtremeSecondaryValue() const { return itsExtremeSecondaryValue; }

 private:
  bool IsExtreme(float primaryValue)
  {
    try
    {
      if (primaryValue != kFloatMissing && itsExtremeValue != kFloatMissing)
      {
        if (fSearchMax)
          return primaryValue > itsExtremeValue;
        else
          return primaryValue < itsExtremeValue;
      }
      else if (primaryValue != kFloatMissing)
        return true;

      return false;
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }

  bool fSearchMax;                 // jos tämä on true, etsii maksimi arvoa, muuten minimiä
  float itsExtremeValue;           // tähän talletetaan min/max arvo
  float itsExtremeSecondaryValue;  // tähän talletetaan toisio arvo, mitä loppujen lopuksi kaivataan
                                   // (esim. jonkin parametrin max-arvon korkeutta)
};

float NFmiInfoAreaMaskVertFunc::DoNormalFunction(const NFmiLocationCache &theLocationCache,
                                                 const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    if (fReturnHeightValue)
    {
      DoubleValueSearcher valueSearcher(itsPrimaryFunc == NFmiAreaMask::MaxH);
      // Lambda funktio joka annetaan iterointi metodille
      auto heightSeekerFunction = [&]()
      {
        float primaryValue = CalcCachedInterpolation(itsInfo, theLocationCache, &itsTimeCache);
        float heightValue = GetLevelHeightValue(theLocationCache);
        valueSearcher.Values(primaryValue, heightValue);
      };
      VerticalIterationBreakingData noIterationBreaking;
      IterateLevelsFromGroundUpward(heightSeekerFunction,
                                    noIterationBreaking,
                                    static_cast<int>(itsStartLevelIndex),
                                    theCalculationParams);
      return valueSearcher.ExtremeSecondaryValue();
    }
    else
    {
      itsFunctionModifier->Clear();
      // Lambda funktio joka annetaan iterointi metodille
      auto valueFunction = [&]()
      {
        itsFunctionModifier->Calculate(
            CalcCachedInterpolation(itsInfo, theLocationCache, &itsTimeCache));
      };
      VerticalIterationBreakingData noIterationBreaking;
      IterateLevelsFromGroundUpward(valueFunction,
                                    noIterationBreaking,
                                    static_cast<int>(itsStartLevelIndex),
                                    theCalculationParams);
      return itsFunctionModifier->CalculationResult();
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiInfoAreaMaskVertFunc::VertFuncSimpleconditionCheck(
    const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    if (itsSimpleCondition)
    {
    float pressure = itsInfo->GetCurrentLevelPressure(theCalculationParams.UsedLatlon(),
                                                        theCalculationParams.itsTime);
      return itsSimpleCondition->CheckPressureCondition(pressure, theCalculationParams);
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskVertFunc   **********************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskVertConditionalFunc  ************
// **********************************************************

NFmiInfoAreaMaskVertConditionalFunc::~NFmiInfoAreaMaskVertConditionalFunc() = default;

NFmiInfoAreaMaskVertConditionalFunc::NFmiInfoAreaMaskVertConditionalFunc(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType thePrimaryFunc,
    NFmiAreaMask::FunctionType theSecondaryFunc,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskVertFunc(theOperation,
                               theMaskType,
                               theDataType,
                               theInfo,
                               thePrimaryFunc,
                               theSecondaryFunc,
                               theArgumentCount,
                               thePossibleMetaParamId),
      itsLimit1(kFloatMissing),
      itsLimit2(kFloatMissing)
{
  try
  {
    if (::IsFindConditionalFunction(itsPrimaryFunc))
      fReturnHeightValue = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMaskVertConditionalFunc::NFmiInfoAreaMaskVertConditionalFunc(
    const NFmiInfoAreaMaskVertConditionalFunc &theOther) = default;

NFmiAreaMask *NFmiInfoAreaMaskVertConditionalFunc::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskVertConditionalFunc(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskVertConditionalFunc::Initialize()
{
  try
  {
    NFmiInfoAreaMaskVertFunc::Initialize();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Tätä kutsutaan jokaiselle erillis pistelaskulle erikseen value-funktiossa.
bool NFmiInfoAreaMaskVertConditionalFunc::InitializeFromArguments()
{
  try
  {
    itsStartLevelValue = itsArgumentVector[0];
    itsEndLevelValue = itsArgumentVector[1];
    itsLimit1 = itsArgumentVector[2];
    itsLimit2 = kFloatMissing;  // Huom! tämä voi olla puuttuva, jos kyse on simppelistä
                                // get-funktiosta (esim. vertp_get(WS_Hir, pressure))
    if (itsArgumentVector.size() > 4)
      itsLimit2 = itsArgumentVector[4];

    if (itsStartLevelValue == kFloatMissing || itsEndLevelValue == kFloatMissing)
      return false;  // jos alku/loppu level arvo on puuttuvaa, ei voi tehdä mitään järkevää

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoAreaMaskVertConditionalFunc::Value(const NFmiCalculationParams &theCalculationParams,
                                                  bool fUseTimeInterpolationAlways)
{
  try
  {
    float value = kFloatMissing;
    if (InitializeFromArguments())
    {
      SetLevelValues();
      // 1. Laske latlon-pistettä vastaava 'reaali'-hilapiste.
    NFmiLocationCache locationCache = CalcLocationCache(theCalculationParams.UsedLatlon());

      // 2. Käy läpi haluttu level korkeus/level väli ja laske haluttu operaatio niille
      FindCalculatedLeves(locationCache);

      if (itsStartLevelIndex != gMissingIndex && itsEndLevelIndex != gMissingIndex)
      {
        value = DoFindConditionalFunction(locationCache, theCalculationParams);
        if (fReturnHeightValue && itsSecondaryFunc == NFmiAreaMask::VertFL)
          value = static_cast<float>(::CalcPressureFlightLevel(value));
      }
    }
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiInfoAreaMaskVertConditionalFunc::CheckProbabilityCondition(double value)
{
  try
  {
    return ::CheckProbabilityCondition(itsPrimaryFunc, value, itsLimit1, itsLimit2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMaskVertConditionalFunc::DoFindConditionalFunction(
    const NFmiLocationCache &theLocationCache, const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    if (fReturnHeightValue)
    {
      VerticalIterationBreakingData iterationBreakingData(true);
      // Lambda funktio joka annetaan iterointi metodille
      auto findValueHeightFunction = [&]()
      {
        float value = CalcCachedInterpolation(itsInfo, theLocationCache, &itsTimeCache);
        if (CheckProbabilityCondition(value))
        {
          iterationBreakingData.value = GetLevelHeightValue(theLocationCache);
          iterationBreakingData.stopIteration = true;
        }
      };
      IterateLevelsFromGroundUpward(findValueHeightFunction,
                                    iterationBreakingData,
                                    static_cast<int>(itsStartLevelIndex),
                                    theCalculationParams);
      return iterationBreakingData.value;
    }

    throw Fmi::Exception(BCP,
                         "Error in program's logic: vertical conditional find function didn't "
                         "return height value...");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskVertConditionalFunc  ************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskTimeVertFunc  *******************
// **********************************************************
NFmiInfoAreaMaskTimeVertFunc::~NFmiInfoAreaMaskTimeVertFunc() = default;

NFmiInfoAreaMaskTimeVertFunc::NFmiInfoAreaMaskTimeVertFunc(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType thePrimaryFunc,
    NFmiAreaMask::FunctionType theSecondaryFunc,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskVertFunc(theOperation,
                               theMaskType,
                               theDataType,
                               theInfo,
                               thePrimaryFunc,
                               theSecondaryFunc,
                               theArgumentCount,
                               thePossibleMetaParamId),
      itsStartTimeOffsetInHours(0),
      itsEndTimeOffsetInHours(0)
{
}

NFmiInfoAreaMaskTimeVertFunc::NFmiInfoAreaMaskTimeVertFunc(
    const NFmiInfoAreaMaskTimeVertFunc &theOther) = default;

NFmiAreaMask *NFmiInfoAreaMaskTimeVertFunc::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskTimeVertFunc(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskTimeVertFunc::SetRangeValuesFromArguments()
{
  try
  {
    itsStartTimeOffsetInHours = itsArgumentVector[0];
    itsEndTimeOffsetInHours = itsArgumentVector[1];
    // otetaan argumentteina annetut arvot haettaviin level-rajoihin
    itsStartLevelValue = itsArgumentVector[2];
    itsEndLevelValue = itsArgumentVector[3];

    // mahdolliset FL:t pitää muuttaa paineeksi [hPa]
    if (itsSecondaryFunc == NFmiAreaMask::TimeVertFL)
    {  // annetut lentopinnat ovat hehto jalkoina, ne pitää muuttaa jaloiksi ennen konversiota
      itsStartLevelValue = ::ConvertFL2P(itsStartLevelValue * 100);
      itsEndLevelValue = ::ConvertFL2P(itsEndLevelValue * 100);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaMaskTimeVertFunc::Value(const NFmiCalculationParams &theCalculationParams,
                                           bool /* fUseTimeInterpolationAlways */)
{
  try
  {
    SetRangeValuesFromArguments();
    if (itsStartLevelValue == kFloatMissing)
      return kFloatMissing;  // jos jo alku level arvo on puuttuvaa, ei voi tehdä mitään järkevää
    if (itsEndLevelValue == kFloatMissing)
      return kFloatMissing;  // jos jo loppu level arvo on puuttuvaa, eikä kyse ollut
                             // get-funktiosta, ei voi tehdä mitään järkevää

    // 1. Laske latlon-pistettä vastaava 'reaali'-hilapiste.
    NFmiLocationCache locationCache = CalcLocationCache(theCalculationParams.UsedLatlon());
    if (locationCache.NoValue())
      return kFloatMissing;

    // 2. Käy läpi haluttu level korkeus/level väli ja laske haluttu operaatio niille
    FindCalculatedLeves(locationCache);

    if (itsStartLevelIndex == gMissingIndex || itsEndLevelIndex == gMissingIndex)
      return kFloatMissing;  // jos jompikumpi level indekseistä osoittaa 'kuuseen', ei kannata
                             // jatkaa

    itsFunctionModifier->Clear();

    // Lasketaan aikaloopitus rajat
    unsigned long origTimeIndex = itsInfo->TimeIndex();  // Otetaan aikaindeksi talteen, jotta se
                                                         // voidaan lopuksi palauttaa takaisin
    unsigned long startTimeIndex = origTimeIndex;
    unsigned long endTimeIndex = origTimeIndex;
    if (NFmiInfoAreaMask::CalcTimeLoopIndexies(itsInfo,
                                               theCalculationParams,
                                               itsStartTimeOffsetInHours,
                                               itsEndTimeOffsetInHours,
                                               &startTimeIndex,
                                               &endTimeIndex))
    {
      NFmiCalculationParams usedCalculationParams(theCalculationParams);
      for (unsigned long timeIndex = startTimeIndex; timeIndex <= endTimeIndex; timeIndex++)
      {
        itsInfo->TimeIndex(timeIndex);
        if (itsSimpleCondition)
          usedCalculationParams.itsTime = itsInfo->Time();
        // Lambda funktio joka annetaan iterointi metodille
        auto modifierFunction = [&]()
        {
          NFmiInfoAreaMask::AddValuesToFunctionModifier(
              itsInfo, itsFunctionModifier, locationCache, itsPrimaryFunc);
        };
        VerticalIterationBreakingData noIterationBreaking;
        NFmiInfoAreaMaskVertFunc::IterateLevelsFromGroundUpward(
            modifierFunction,
            noIterationBreaking,
            static_cast<int>(itsStartLevelIndex),
            usedCalculationParams);
      }
      itsInfo->TimeIndex(origTimeIndex);
    }

    return itsFunctionModifier->CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskTimeVertFunc  *******************
// **********************************************************

// **********************************************************
// *****    NFmiInfoAreaMaskProbFunc   **********************
// **********************************************************

NFmiInfoAreaMaskProbFunc::~NFmiInfoAreaMaskProbFunc() = default;

NFmiInfoAreaMaskProbFunc::NFmiInfoAreaMaskProbFunc(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType thePrimaryFunc,
    NFmiAreaMask::FunctionType theSecondaryFunc,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMask(
          theOperation, theMaskType, theDataType, theInfo, thePossibleMetaParamId, kNoValue),
      itsPrimaryFunc(thePrimaryFunc),
      itsSecondaryFunc(theSecondaryFunc),
      itsArgumentVector(),
      itsSearchRangeInKM(0),
      itsLimit1(kFloatMissing),
      itsLimit2(kFloatMissing),
      itsStartTimeOffsetInHours(0),
      itsEndTimeOffsetInHours(0),
      itsGridPointRectSizeX(0),
      itsGridPointRectSizeY(0),
      itsConditionFullfilledGridPointCount(0),
      itsTotalCalculatedGridPoints(0)
{
  try
  {
    itsFunctionArgumentCount = theArgumentCount;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaMaskProbFunc::NFmiInfoAreaMaskProbFunc(const NFmiInfoAreaMaskProbFunc &theOther) =
    default;

NFmiAreaMask *NFmiInfoAreaMaskProbFunc::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskProbFunc(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Tätä kutsutaan jokaiselle erillis pistelaskulle erikseen value-funktiossa.
void NFmiInfoAreaMaskProbFunc::InitializeFromArguments()
{
  try
  {
    itsSearchRangeInKM = itsArgumentVector[0];
    itsStartTimeOffsetInHours = itsArgumentVector[1];
    itsEndTimeOffsetInHours = itsArgumentVector[2];
    // Limit1/2 membereitä ei alusteta, jos kyse area-integraatio funktioista lapsiluokissa
    itsLimit1 = kFloatMissing;
    if (itsArgumentVector.size() > 3)
      itsLimit1 = itsArgumentVector[3];
    // Huom! tämä voi olla puuttuva, jos kyse on simppelistä
    // get-funktiosta (esim. vertp_get(WS_Hir, pressure))
    itsLimit2 = kFloatMissing;
    if (itsArgumentVector.size() > 4)
      itsLimit2 = itsArgumentVector[4];

    itsGridPointRectSizeX = 0;
    itsGridPointRectSizeY = 0;
    // Laske mikä on annetun infon maailmassa annetun search-rangen mukaiset
    // itsGridPointRectSizeX/Y:t
    if (itsSearchRangeInKM > 0 && itsSearchRangeInKM != kFloatMissing && itsInfo)
    {
      const NFmiGrid *grid = itsInfo->Grid();
      if (grid)
      {
        double gridSizeXInKM = grid->Area()->WorldXYWidth() / (grid->XNumber() - 1) * 0.001;
        itsGridPointRectSizeX = FmiRound(2. * itsSearchRangeInKM / gridSizeXInKM);
        itsGridPointRectSizeX =
            std::max(itsGridPointRectSizeX, 2);  // 2 on minimi hakulaatikon koko

        double gridSizeYInKM = grid->Area()->WorldXYHeight() / (grid->YNumber() - 1) * 0.001;
        itsGridPointRectSizeY = FmiRound(2. * itsSearchRangeInKM / gridSizeYInKM);
        itsGridPointRectSizeY =
            std::max(itsGridPointRectSizeY, 2);  // 2 on minimi hakulaatikon koko
      }
      else
      {
        // pitäisikö asemadatoille tehdä omat laskut perustuen pelkästään etäisyyksiin asemille?
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskProbFunc::SetArguments(std::vector<float> &theArgumentVector)
{
  try
  {
    // jokaiselle pisteelle ja ajanhetkelle annetaan eri argumentit tässä
    itsArgumentVector = theArgumentVector;
    if (static_cast<int>(itsArgumentVector.size()) !=
        itsFunctionArgumentCount - 1)  // -1 tarkoittaa että funktion 1. argumentti tulee suoraan
                                       // itsIfo:sta, eli sitä ei anneta argumentti-listassa
      throw Fmi::Exception(
          BCP,
          "Internal SmartMet error: Probability function was given invalid number of arguments, "
          "cannot calculate the macro.");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsOdd(int value)
{
  return value % 2 == 1;
}

static void CalcPeekLoopLimits(int rectSize, double interpolationPoint, int &limit1, int &limit2)
{
  try
  {
    if (::IsOdd(rectSize))
    {
      // Pariton on helppo tapaus, looppi alkaa -x:sta ja menee x:ään ja x = (int)(theRectSize/2)
      limit2 = static_cast<int>(rectSize / 2);
      limit1 = -limit2;
    }
    else
    {
      // Parillinen tapaus on hankalampi. Pitää katsoa millä puolella rajaa ollaan lähimmästä
      // hilapisteestä pisteestä (= kokonaisluvusta) ja sen mukaan laskea eri arvot limiteille.
      if (interpolationPoint > FmiRound(interpolationPoint))
      {
        // Interpolointi piste on suurempi kuin nearest-point arvo, tällöin limit2 saa yhden
        // suuremman (itseis)arvon
        limit2 = static_cast<int>(rectSize / 2);
        limit1 = -(limit2 - 1);
      }
      else
      {
        // Interpolointi piste on pienempi kuin nearest-point arvo, tällöin limit1 saa yhden
        // suuremman (itseis)arvon
        limit1 = static_cast<int>(-rectSize / 2);
        limit2 = -limit1 - 1;
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiInfoAreaMaskProbFunc::CheckProbabilityCondition(double value)
{
  try
  {
    return ::CheckProbabilityCondition(itsPrimaryFunc, value, itsLimit1, itsLimit2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Etsii halutun aika loopituksen alku- ja loppuaika indeksejä annetusta infosta.
// Palauttaa mahdollisen interpolaatio ajan, jos startOffset ja endOffsetit ovat samoja, käytetään
// kyseistä aikaa aikainterpolaatiossa.
NFmiMetTime NFmiInfoAreaMaskProbFunc::CalcTimeLoopLimits(
    std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const NFmiCalculationParams &theCalculationParams,
    double theStartTimeOffsetInHours,
    double theEndTimeOffsetInHours,
    unsigned long *theStartTimeIndexOut,
    unsigned long *theEndTimeIndexOut,
    bool *doSpecialCalculation,
    bool allowInterpolation)
{
  try
  {
    NFmiMetTime interpolationTime = theCalculationParams.itsTime;
    *doSpecialCalculation = false;
    if (theInfo)
    {
      NFmiMetTime startTime = theCalculationParams.itsTime;
      startTime.ChangeByMinutes(FmiRound(theStartTimeOffsetInHours * 60));

      if (allowInterpolation && theStartTimeOffsetInHours == theEndTimeOffsetInHours)
        interpolationTime = startTime;
      else
      {
        auto usedTimeRangeInMinutes = static_cast<unsigned long>(
            std::abs(theEndTimeOffsetInHours - theStartTimeOffsetInHours) * 60);
        bool foundStartTime = theInfo->FindNearestTime(startTime, kForward, usedTimeRangeInMinutes);
        *theStartTimeIndexOut = theInfo->TimeIndex();

        NFmiMetTime endTime = theCalculationParams.itsTime;
        endTime.ChangeByMinutes(FmiRound(theEndTimeOffsetInHours * 60));
        bool foundEndTime = theInfo->FindNearestTime(endTime, kBackward, usedTimeRangeInMinutes);
        *theEndTimeIndexOut = theInfo->TimeIndex();

        if (foundStartTime && !foundEndTime)
        {
          *theEndTimeIndexOut = *theStartTimeIndexOut;
          *doSpecialCalculation = true;
        }
        else if (!foundStartTime && foundEndTime)
        {
          *theStartTimeIndexOut = *theEndTimeIndexOut;
          *doSpecialCalculation = true;
        }
        else if (*theStartTimeIndexOut != gMissingIndex && *theEndTimeIndexOut == gMissingIndex)
        {  // alkuaika on datan sisällä, mutta loppuaika ei, laitetaan loppuaika viimeiseen
           // aika-askeleeseen
          *theEndTimeIndexOut = theInfo->SizeTimes() - 1;
          *doSpecialCalculation = true;
        }
      }
    }
    return interpolationTime;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static void SetSimpleConditionCalculationTime(
    NFmiCalculationParams &simpleConditionCalculationPointParams,
    bool useInterpolatedTime,
    const NFmiMetTime &interpolationTime,
    std::shared_ptr<NFmiFastQueryInfo> &info)
{
  try
  {
    // Jos tarvitaan simple-condition laskuja, pitää niitä varten olla aika tallessa
    if (useInterpolatedTime)
      simpleConditionCalculationPointParams.itsTime = interpolationTime;
    else
      simpleConditionCalculationPointParams.itsTime = info->Time();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaMaskProbFunc::Value(const NFmiCalculationParams &theCalculationParams,
                                       bool /* fUseTimeInterpolationAlways */)
{
  try
  {
    InitializeFromArguments();
    InitializeIntegrationValues();
    if (itsGridPointRectSizeX && itsGridPointRectSizeY)
    {
      NFmiLocationCache locCache = itsInfo->CalcLocationCache(theCalculationParams.UsedLatlon());
      if (!locCache.NoValue())
      {
      NFmiLocation location(theCalculationParams.UsedLatlon());

        // Lasketaan laatikon loopitus rajat, x1 on mistä x-suuntainen peek-indeksi
        // alkaa ja x2 mihin se loppuu (esim. -2 ja 2, jos laatkion x-koko on 5).
        // Sama periaate y1 ja y2:lla.
        int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        ::CalcPeekLoopLimits(itsGridPointRectSizeX, locCache.itsGridPoint.X(), x1, x2);
        ::CalcPeekLoopLimits(itsGridPointRectSizeY, locCache.itsGridPoint.Y(), y1, y2);

        // Lasketaan aikaloopitus rajat
        unsigned long origTimeIndex =
            itsInfo->TimeIndex();  // Otetaan aikaindeksi talteen, jotta se
                                   // voidaan lopuksi palauttaa takaisin
        unsigned long startTimeIndex = origTimeIndex;
        unsigned long endTimeIndex = origTimeIndex;
        bool doSpecialCalculation = false;
        NFmiMetTime interpolationTime =
            NFmiInfoAreaMaskProbFunc::CalcTimeLoopLimits(itsInfo,
                                                         theCalculationParams,
                                                         itsStartTimeOffsetInHours,
                                                         itsEndTimeOffsetInHours,
                                                         &startTimeIndex,
                                                         &endTimeIndex,
                                                         &doSpecialCalculation,
                                                         true);
        NFmiCalculationParams simpleConditionCalculationPointParams(theCalculationParams);

        for (unsigned long timeIndex = startTimeIndex; timeIndex <= endTimeIndex; timeIndex++)
        {
          itsInfo->TimeIndex(timeIndex);
          // Jos käydään läpi yhtä ajan hetkeä, tehdään interpolaatio, ja käytetään originaali
          // laskenta aikaa. Jos käydään läpi aikajakso datan originaali aikaresoluutiossa,
          // käytetään infoon asetettua aikaa
          bool useInterpolatedTime = (startTimeIndex == endTimeIndex);
          if (doSpecialCalculation)
            useInterpolatedTime = false;
          ::SetSimpleConditionCalculationTime(simpleConditionCalculationPointParams,
                                              useInterpolatedTime,
                                              interpolationTime,
                                              itsInfo);
          DoSubgridCalculations(location,
                                x1,
                                x2,
                                y1,
                                y2,
                                interpolationTime,
                                useInterpolatedTime,
                                simpleConditionCalculationPointParams);
          if (NFmiInfoAreaMaskProbFunc::CheckTimeIndicesForLoopBreak(startTimeIndex, endTimeIndex))
            break;
        }
        itsInfo->TimeIndex(origTimeIndex);
      }
      return CalcAreaProbability();
      }
    else
    {
      return DoObservationAreaMaskCalculations(theCalculationParams);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsSynopDataCase(const std::shared_ptr<NFmiFastQueryInfo> &info)
{
  if (info)
  {
    return info->Producer()->GetIdent() == kFmiSYNOP;
  }
  return false;
}

double NFmiInfoAreaMaskProbFunc::DoObservationAreaMaskCalculations(
    const NFmiCalculationParams &theCalculationParams)
{
  std::shared_ptr<NFmiArea> emptyAreaPtr;
  auto infoVector = NFmiInfoAreaMask::GetMultiSourceData(
      itsInfo, emptyAreaPtr, ::IsSynopDataCase(itsInfo));
  for (auto &info : infoVector)
  {
    NFmiCalculationParams simpleConditionCalculationPointParams(theCalculationParams);
    if (infoVector.size() > 1)
    {
      simpleConditionCalculationPointParams.itsCurrentMultiInfoData = info.get();
    }
    unsigned long startTimeIndex = 0;
    unsigned long endTimeIndex = 0;
    // Havaintojen kanssa ei tehdä aikainterpolaatioita koskaan!
    bool allowTimeInterpolation = false;
    bool doSpecialCalculation = false;
    NFmiMetTime interpolationTime =
        NFmiInfoAreaMaskProbFunc::CalcTimeLoopLimits(info,
                                                     theCalculationParams,
                                                     itsStartTimeOffsetInHours,
                                                     itsEndTimeOffsetInHours,
                                                     &startTimeIndex,
                                                     &endTimeIndex,
                                                     &doSpecialCalculation,
                                                     allowTimeInterpolation);
    int maxWantedLocations = 1000;
    if (startTimeIndex != gMissingIndex && endTimeIndex != gMissingIndex)
    {
      auto searchRangeLocations = info->NearestLocations(
          theCalculationParams.UsedLatlon(), maxWantedLocations, itsSearchRangeInKM * 1000.);
      for (const auto &locationIndexAndDistanceValue : searchRangeLocations)
      {
        info->LocationIndex(locationIndexAndDistanceValue.first);
        // Jos tarvitaan simple-condition laskuja, pitää niitä varten olla paikka tallessa
        simpleConditionCalculationPointParams.SetModifiedLatlon(info->LatLon(), true);

        for (unsigned long timeIndex = startTimeIndex; timeIndex <= endTimeIndex; timeIndex++)
        {
          info->TimeIndex(timeIndex);
          ::SetSimpleConditionCalculationTime(simpleConditionCalculationPointParams,
                                              allowTimeInterpolation,
                                              interpolationTime,
                                              info);
          if (simpleConditionCalculationPointParams.UsedLatlon() != NFmiPoint::gMissingLatlon)
          {
            float value = CalculationPointValueForObservation(info);
            if (value != kFloatMissing)
            {
              // Jos kyse prob laskuista, joissa käytetään simple-condition ehtoa,
              // laskuria pitää kasvattaa tässä, en ole varma sotkeeko tämä jotain muuta tapausta.
              // value != missing on huono tarkastus, koska simple-condition laskuissa voi olla
              // muita parametreja käytössä, mutta en tiedä mitä muuta tässä voisi tarkastella.
              itsTotalCalculatedGridPoints++;
            }
            if (SimpleConditionCheck(simpleConditionCalculationPointParams))
              DoIntegrationCalculations(value);
          }
        }
        if (NFmiInfoAreaMaskProbFunc::CheckTimeIndicesForLoopBreak(startTimeIndex, endTimeIndex))
          break;
      }
    }
  }
  return CalcAreaProbability();
}

double NFmiInfoAreaMaskProbFunc::CalcAreaProbability()
{
  if (itsTotalCalculatedGridPoints)
  {
    // Kerrotaan 100:lla, jotta saadaan prosentteja.
    // Em. luku on double, jotta jakolaskusta ei tulisi integer jakoa.
    return (100. * itsConditionFullfilledGridPointCount) / itsTotalCalculatedGridPoints;
  }

  return kFloatMissing;
}

float NFmiInfoAreaMaskProbFunc::CalculationPointValueForObservation(
    const std::shared_ptr<NFmiFastQueryInfo> &info)
{
  if (metaParamDataHolder.isMetaParameterCalculationNeeded())
  {
    return CalcMetaParamValueWithFunction([&]() { return info->FloatValue(); });
  }
  else
  {
    return info->FloatValue();
  }
}

void NFmiInfoAreaMaskProbFunc::DoSubgridCalculations(
    const NFmiLocation &theCalculationPointLocation,
    int leftSubGridOffset,
    int rightSubGridOffset,
    int bottomSubGridOffset,
    int topSubGridOffset,
    const NFmiMetTime &theInterpolationTime,
    bool useInterpolatedTime,
    NFmiCalculationParams &theSimpleConditionCalculationPointParams)
{
  try
  {
    for (int offsetY = bottomSubGridOffset; offsetY <= topSubGridOffset; offsetY++)
    {
      for (int offsetX = leftSubGridOffset; offsetX <= rightSubGridOffset; offsetX++)
      {
        if (!IsCalculationPointInsideCircle(theCalculationPointLocation, offsetX, offsetY))
        {
          continue;  // kyseinen piste oli ympyrän ulkopuolella
        }
        // Jos tarvitaan simple-condition laskuja, pitää niitä varten olla paikka tallessa
        theSimpleConditionCalculationPointParams.SetModifiedLatlon(
          itsInfo->PeekLocationLatLon(offsetX, offsetY), true);
        if (theSimpleConditionCalculationPointParams.UsedLatlon() != NFmiPoint::gMissingLatlon)
        {
          float value =
              CalculationPointValue(offsetX, offsetY, theInterpolationTime, useInterpolatedTime);
          if (SimpleConditionCheck(theSimpleConditionCalculationPointParams))
            DoIntegrationCalculations(value);
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Time-looppi voi mennä gMissingIndex => gMissingIndex, jolloin ++-operaatio
// veisi luvun takaisin 0:aan, siksi tämä ehto ja loopin breikki.
bool NFmiInfoAreaMaskProbFunc::CheckTimeIndicesForLoopBreak(unsigned long theStartTimeIndex,
                                                            unsigned long theEndTimeIndex)
{
  try
  {
    return (theStartTimeIndex == gMissingIndex || theEndTimeIndex == gMissingIndex ||
            theStartTimeIndex > theEndTimeIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiInfoAreaMaskProbFunc::IsCalculationPointInsideCircle(
    const NFmiLocation &theCalculationPointLocation, int theOffsetX, int theOffsetY)
{
  try
  {
    if (itsSecondaryFunc == NFmiAreaMask::AreaCircle ||
      itsSecondaryFunc == NFmiAreaMask::Occurrence || itsSecondaryFunc == NFmiAreaMask::Occurrence2)
    {
      // ympyrä tapauksessa tarkastetaan jokainen piste erikseen, onko se halutun säteisen ympyrän
      // sisällä
      double distanceInKM = theCalculationPointLocation.Distance(
                                itsInfo->PeekLocationLatLon(theOffsetX, theOffsetY)) *
                            0.001;
      return distanceInKM < itsSearchRangeInKM;
    }

    // Rect-laskuilla ollaan aina laatikon sisällä
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMaskProbFunc::CalculationPointValue(int theOffsetX,
                                                      int theOffsetY,
                                                      const NFmiMetTime &theInterpolationTime,
                                                      bool useInterpolatedTime)
{
  try
  {
    if (metaParamDataHolder.isMetaParameterCalculationNeeded())
      return CalcMetaParamCalculationPointValue(
          theOffsetX, theOffsetY, theInterpolationTime, useInterpolatedTime);

    if (useInterpolatedTime)
      return itsInfo->PeekLocationValue(theOffsetX, theOffsetY, theInterpolationTime);

    return itsInfo->PeekLocationValue(theOffsetX, theOffsetY);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMaskProbFunc::CalcMetaParamCalculationPointValue(
    int theOffsetX,
    int theOffsetY,
    const NFmiMetTime &theInterpolationTime,
    bool useInterpolatedTime)
{
  try
  {
    return CalcMetaParamValueWithFunction(
        [&]()
        {
          return useInterpolatedTime
                     ? itsInfo->PeekLocationValue(theOffsetX, theOffsetY, theInterpolationTime)
                     : itsInfo->PeekLocationValue(theOffsetX, theOffsetY);
        });
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskProbFunc::DoIntegrationCalculations(float value)
{
  try
  {
    if (value != kFloatMissing)
    {
      itsTotalCalculatedGridPoints++;
      if (CheckProbabilityCondition(value))
        itsConditionFullfilledGridPointCount++;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskProbFunc::InitializeIntegrationValues()
{
  try
  {
    itsTotalCalculatedGridPoints = 0;
    itsConditionFullfilledGridPointCount = 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoAreaMaskProbFunc   **********************
// **********************************************************

// **********************************************************
// *****    NFmiInfoTimeIntegrator   ************************
// **********************************************************

NFmiInfoTimeIntegrator::~NFmiInfoTimeIntegrator() = default;

NFmiInfoTimeIntegrator::NFmiInfoTimeIntegrator(const NFmiCalculationCondition &theOperation,
                                               Type theMaskType,
                                               NFmiInfoData::Type theDataType,
                                               const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                               NFmiAreaMask::FunctionType theIntegrationFunc,
                                               int theStartTimeOffset,
                                               int theEndTimeOffset,
                                               unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskMetFuncBase(theOperation,
                                  theMaskType,
                                  theDataType,
                                  theInfo,
                                  false,
                                  NoDirection,
                                  thePossibleMetaParamId,
                                  kNoValue),
      itsIntegrationFunc(theIntegrationFunc),
      itsFunctionModifier(),
      itsStartTimeOffset(theStartTimeOffset),
      itsEndTimeOffset(theEndTimeOffset)
{
  try
  {
    itsFunctionModifier = NFmiInfoAreaMask::CreateIntegrationFuction(itsIntegrationFunc);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoTimeIntegrator::NFmiInfoTimeIntegrator(const NFmiInfoTimeIntegrator &theOther)
    : NFmiInfoAreaMaskMetFuncBase(theOther),
      itsIntegrationFunc(theOther.itsIntegrationFunc),
      itsFunctionModifier(theOther.itsFunctionModifier ? theOther.itsFunctionModifier->Clone()
                                                       : nullptr),
      itsStartTimeOffset(theOther.itsStartTimeOffset),
      itsEndTimeOffset(theOther.itsEndTimeOffset)
{
}

NFmiAreaMask *NFmiInfoTimeIntegrator::Clone() const
{
  try
  {
    return new NFmiInfoTimeIntegrator(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoTimeIntegrator::Value(const NFmiCalculationParams &theCalculationParams,
                                     bool /* fUseTimeInterpolationAlways */)
{
  try
  {
    if (itsTimeCache.NoValue())
      return kFloatMissing;  // jos mentiin jo originaalisti datan aikojen ulkopuolelle, ei voi
                             // mitään

    // 1. Laske latlon-pistettä vastaava 'reaali'-hilapiste.
    NFmiLocationCache locationCache = CalcLocationCache(theCalculationParams.UsedLatlon());
    if (locationCache.NoValue())
      return kFloatMissing;  // jos mentiin datan alueen ulkopuolelle, palautetaan puuttuvaa

    unsigned long oldTimeIndex = itsInfo->TimeIndex();
    itsFunctionModifier->Clear();

    unsigned long startTimeIndex =
        itsTimeCache.itsOffset <= 0.5
            ? itsTimeCache.itsTimeIndex1
            : itsTimeCache.itsTimeIndex2;  // haetaan lähempi timeindeksi 'nolla'-kohdaksi
    int usedStartIndex = startTimeIndex + itsStartTimeOffset;
    int usedEndIndex = startTimeIndex + itsEndTimeOffset;
    for (int i = usedStartIndex; i <= usedEndIndex; i++)
    {
      itsInfo->TimeIndex(i);
      float tmpValue = CalcCachedInterpolation(itsInfo, locationCache, nullptr);
      itsFunctionModifier->Calculate(tmpValue);
    }
    itsInfo->TimeIndex(oldTimeIndex);
    return itsFunctionModifier->CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiInfoTimeIntegrator   ************************
// **********************************************************

// **********************************************************
// *****  NFmiInfoRectAreaIntegrator ************************
// **********************************************************

NFmiInfoRectAreaIntegrator::~NFmiInfoRectAreaIntegrator() = default;

NFmiInfoRectAreaIntegrator::NFmiInfoRectAreaIntegrator(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType theIntegrationFunc,
    int theStartXOffset,
    int theEndXOffset,
    int theStartYOffset,
    int theEndYOffset,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskMetFuncBase(theOperation,
                                  theMaskType,
                                  theDataType,
                                  theInfo,
                                  false,
                                  NoDirection,
                                  thePossibleMetaParamId,
                                  kNoValue),
      itsIntegrationFunc(theIntegrationFunc),
      itsFunctionModifier(),
      itsStartXOffset(theStartXOffset),
      itsEndXOffset(theEndXOffset),
      itsStartYOffset(theStartYOffset),
      itsEndYOffset(theEndYOffset)
{
  try
  {
    itsFunctionModifier = NFmiInfoAreaMask::CreateIntegrationFuction(itsIntegrationFunc);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoRectAreaIntegrator::NFmiInfoRectAreaIntegrator(const NFmiInfoRectAreaIntegrator &theOther)
    : NFmiInfoAreaMaskMetFuncBase(theOther),
      itsIntegrationFunc(theOther.itsIntegrationFunc),
      itsFunctionModifier(theOther.itsFunctionModifier ? theOther.itsFunctionModifier->Clone()
                                                       : nullptr),
      itsStartXOffset(theOther.itsStartXOffset),
      itsEndXOffset(theOther.itsEndXOffset),
      itsStartYOffset(theOther.itsStartYOffset),
      itsEndYOffset(theOther.itsEndYOffset)
{
}

NFmiAreaMask *NFmiInfoRectAreaIntegrator::Clone() const
{
  try
  {
    return new NFmiInfoRectAreaIntegrator(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiInfoRectAreaIntegrator::Value(const NFmiCalculationParams &theCalculationParams,
                                         bool /* fUseTimeInterpolationAlways */)
{
  try
  {
    if (itsTimeCache.NoValue())
      return kFloatMissing;  // jos mentiin originaalisti datan aikojen ulkopuolelle, ei voi mitään

    // 1. Laske latlon-pistettä vastaava 'reaali'-hilapiste.
    NFmiLocationCache locationCache = CalcLocationCache(theCalculationParams.UsedLatlon());
    if (locationCache.NoValue())
      return kFloatMissing;  // jos mentiin datan alueen ulkopuolelle, palautetaan puuttuvaa

    itsFunctionModifier->Clear();

    for (int j = itsStartYOffset; j <= itsEndYOffset; j++)
    {
      for (int i = itsStartXOffset; i <= itsEndXOffset; i++)
      {
        itsFunctionModifier->Calculate(Peek(locationCache, i, j));
      }
    }
    return itsFunctionModifier->CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****  NFmiInfoRectAreaIntegrator ************************
// **********************************************************

// **********************************************************
// *****  NFmiInfoAreaIntegrationFunc ***********************
// **********************************************************

NFmiInfoAreaIntegrationFunc::~NFmiInfoAreaIntegrationFunc() = default;

NFmiInfoAreaIntegrationFunc::NFmiInfoAreaIntegrationFunc(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType thePrimaryFunc,
    NFmiAreaMask::FunctionType theSecondaryFunc,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskProbFunc(theOperation,
                               theMaskType,
                               theDataType,
                               theInfo,
                               thePrimaryFunc,
                               theSecondaryFunc,
                               theArgumentCount,
                               thePossibleMetaParamId),
      itsIntegrationFunc(thePrimaryFunc),
      itsFunctionModifier()
{
  try
  {
    itsFunctionModifier = NFmiInfoAreaMask::CreateIntegrationFuction(itsIntegrationFunc);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiInfoAreaIntegrationFunc::NFmiInfoAreaIntegrationFunc(
    const NFmiInfoAreaIntegrationFunc &theOther)
    : NFmiInfoAreaMaskProbFunc(theOther),
      itsIntegrationFunc(theOther.itsIntegrationFunc),
      itsFunctionModifier(theOther.itsFunctionModifier ? theOther.itsFunctionModifier->Clone()
                                                       : nullptr)
{
}

NFmiAreaMask *NFmiInfoAreaIntegrationFunc::Clone() const
{
  try
  {
    return new NFmiInfoAreaIntegrationFunc(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tätä kaytetaan smarttool-modifierin yhteydessä
double NFmiInfoAreaIntegrationFunc::Value(const NFmiCalculationParams &theCalculationParams,
                                          bool fUseTimeInterpolationAlways)
{
  try
  {
    // This makes all the integration calculations over time range and wanted area
    NFmiInfoAreaMaskProbFunc::Value(theCalculationParams, fUseTimeInterpolationAlways);

    // Oletus: itsFunctionModifier:in olemassaolo on jo tarkastettu InitializeIntegrationValues
    // metodissa.
    return itsFunctionModifier->CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Oletus: itsFunctionModifier:in olemassaolo on jo tarkastettu InitializeIntegrationValues
// metodissa.
void NFmiInfoAreaIntegrationFunc::DoIntegrationCalculations(float value)
{
  try
  {
    itsFunctionModifier->Calculate(value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaIntegrationFunc::InitializeIntegrationValues()
{
  try
  {
    NFmiInfoAreaMaskProbFunc::InitializeIntegrationValues();
    if (itsFunctionModifier)
      itsFunctionModifier->Clear();
    else
      throw Fmi::Exception(
          BCP,
          std::string("Internal error in ") + __FUNCTION__ + ", integration modifier was nullptr");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****  NFmiInfoAreaIntegrationFunc ***********************
// **********************************************************

// **********************************************************
// *****  NFmiInfoAreaMaskAreaProbFunc **********************
// **********************************************************

NFmiInfoAreaMaskAreaProbFunc::~NFmiInfoAreaMaskAreaProbFunc() = default;

NFmiInfoAreaMaskAreaProbFunc::NFmiInfoAreaMaskAreaProbFunc(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiAreaMask::FunctionType thePrimaryFunc,
    NFmiAreaMask::FunctionType theSecondaryFunc,
    int theArgumentCount,
    unsigned long thePossibleMetaParamId)
    : NFmiInfoAreaMaskProbFunc(theOperation,
                               theMaskType,
                               theDataType,
                               theInfo,
                               thePrimaryFunc,
                               theSecondaryFunc,
                               theArgumentCount,
                               thePossibleMetaParamId)
{
}

NFmiInfoAreaMaskAreaProbFunc::NFmiInfoAreaMaskAreaProbFunc(
    const NFmiInfoAreaIntegrationFunc &theOther)
    : NFmiInfoAreaMaskProbFunc(theOther)
{
}

NFmiAreaMask *NFmiInfoAreaMaskAreaProbFunc::Clone() const
{
  try
  {
    return new NFmiInfoAreaMaskAreaProbFunc(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiInfoAreaMaskAreaProbFunc::CalculationPointValue(int theOffsetX,
                                                          int theOffsetY,
                                                          const NFmiMetTime &theInterpolationTime,
                                                          bool useInterpolatedTime)
{
  try
  {
    // Jos päästään tänne asti kasvatetaan itsTotalCalculatedGridPoints -laskuri.
    // Tänne pääsy tarkoittaa että laskenta piste on datan alueen sisällä ja laskenta ympyrän
    // alueella
    itsTotalCalculatedGridPoints++;
    return NFmiInfoAreaMaskProbFunc::CalculationPointValue(
        theOffsetX, theOffsetY, theInterpolationTime, useInterpolatedTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiInfoAreaMaskAreaProbFunc::DoIntegrationCalculations(float value)
{
  try
  {
    // Jos päästään tänne asti kasvatetaan vain itsConditionFullfilledGridPointCount -laskuria,
    // tänne pääsy tarkoittaa että laskenta piste on datan alueen ja laskenta alihilan alueella ja
    // lisäksi simple-condition on päästänyt läpi (joiden osumia tässä etsitäänkin).
    itsConditionFullfilledGridPointCount++;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****  NFmiInfoAreaMaskAreaProbFunc **********************
// **********************************************************

// **********************************************************
// *****  NFmiTimeShiftedInfoAreaMask ***********************
// **********************************************************

NFmiTimeShiftedInfoAreaMask::~NFmiTimeShiftedInfoAreaMask() = default;

NFmiTimeShiftedInfoAreaMask::NFmiTimeShiftedInfoAreaMask()
    : NFmiInfoAreaMask(), itsTimeOffsetInHours(0), itsChangeByMinutesValue(0)
{
}

NFmiTimeShiftedInfoAreaMask::NFmiTimeShiftedInfoAreaMask(
    const NFmiCalculationCondition &theOperation,
    Type theMaskType,
    NFmiInfoData::Type theDataType,
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
    float theTimeOffsetInHours,
    unsigned long thePossibleMetaParamId,
    BinaryOperator thePostBinaryOperator)
    : NFmiInfoAreaMask(theOperation,
                       theMaskType,
                       theDataType,
                       theInfo,
                       thePossibleMetaParamId,
                       thePostBinaryOperator),
      itsTimeOffsetInHours(theTimeOffsetInHours),
      itsChangeByMinutesValue(boost::math::lround(theTimeOffsetInHours * 60.f))
{
}

NFmiTimeShiftedInfoAreaMask::NFmiTimeShiftedInfoAreaMask(
    const NFmiTimeShiftedInfoAreaMask &theOther)
    : NFmiInfoAreaMask(theOther),
      itsTimeOffsetInHours(theOther.itsTimeOffsetInHours),
      itsChangeByMinutesValue(theOther.itsChangeByMinutesValue)
{
}

NFmiAreaMask *NFmiTimeShiftedInfoAreaMask::Clone() const
{
  try
  {
    return new NFmiTimeShiftedInfoAreaMask(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiTimeShiftedInfoAreaMask::Value(const NFmiCalculationParams &theCalculationParams,
                                          bool fUseTimeInterpolationAlways)
{
  try
  {
    if (IsTimeInterpolationNeeded(fUseTimeInterpolationAlways))
    {
      return NFmiInfoAreaMask::Value(GetUsedCalculationParams(theCalculationParams),
                                     fUseTimeInterpolationAlways);
    }
    else
    {
      // Data on jo asetettu haluttuun offset aikaan, kutsutaan vain emon metodia
      return NFmiInfoAreaMask::Value(theCalculationParams, fUseTimeInterpolationAlways);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiTimeShiftedInfoAreaMask::HeightValue(double theHeight,
                                                const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    return NFmiInfoAreaMask::HeightValue(theHeight, GetUsedCalculationParams(theCalculationParams));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiTimeShiftedInfoAreaMask::PressureValue(double thePressure,
                                                  const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    return NFmiInfoAreaMask::PressureValue(thePressure,
                                           GetUsedCalculationParams(theCalculationParams));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiTimeShiftedInfoAreaMask::Time(const NFmiMetTime &theTime)
{
  try
  {
    auto usedTime = theTime;
    usedTime.ChangeByMinutes(itsChangeByMinutesValue);
    return NFmiInfoAreaMask::Time(usedTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiCalculationParams NFmiTimeShiftedInfoAreaMask::GetUsedCalculationParams(
    const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    NFmiCalculationParams usedCalculationParams = theCalculationParams;
    usedCalculationParams.itsTime.ChangeByMinutes(itsChangeByMinutesValue);
    return usedCalculationParams;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****  NFmiTimeShiftedInfoAreaMask ***********************
// **********************************************************
