// ======================================================================
/*!
 * \file NFmiInfoAreaMask.h
 * \brief Interface of class NFmiInfoAreaMask
 */
// ======================================================================

#pragma once

#include "NFmiAreaMaskImpl.h"
#include "NFmiDataIdent.h"
#include "NFmiFastInfoUtils.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiGrid.h"  // täältä tulee NFmiTimeCache-luokka (tiedän, pitäisi jakaa newbase:a osiin)
#include "NFmiLevel.h"
#include "NFmiMetTime.h"

class NFmiDataModifier;

class MetaParamDataHolder
{
  NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage_;
  unsigned long possibleMetaParamId_ = kFmiBadParameter;
  bool checkMetaParamCalculation_ = false;

 public:
  MetaParamDataHolder() = default;
  void initialize(const std::shared_ptr<NFmiFastQueryInfo> &info,
                  unsigned long possibleMetaParamId);
  bool isMetaParameterCalculationNeeded() const;
  unsigned long possibleMetaParamId() const { return possibleMetaParamId_; }
  const NFmiFastInfoUtils::MetaWindParamUsage &metaWindParamUsage() const
  {
    return metaWindParamUsage_;
  }
  bool checkMetaParamCalculation() const { return checkMetaParamCalculation_; }
  void setCheckMetaParamCalculation(bool checkMetaParamCalculation)
  {
    checkMetaParamCalculation_ = checkMetaParamCalculation;
  }
};

class MetaParamDataHolderDoCheckStateRestorer
{
  MetaParamDataHolder &metaParamDataHolder_;
  bool originalCheckMetaParamCalculationState_;

 public:
  MetaParamDataHolderDoCheckStateRestorer(MetaParamDataHolder &metaParamDataHolder,
                                          bool wantedCheckMetaParamCalculationState)
      : metaParamDataHolder_(metaParamDataHolder),
        originalCheckMetaParamCalculationState_(metaParamDataHolder.checkMetaParamCalculation())
  {
    metaParamDataHolder_.setCheckMetaParamCalculation(wantedCheckMetaParamCalculationState);
  }
  ~MetaParamDataHolderDoCheckStateRestorer()
  {
    metaParamDataHolder_.setCheckMetaParamCalculation(originalCheckMetaParamCalculationState_);
  }
};

class NFmiInfoAreaMask : public NFmiAreaMaskImpl
{
 public:
  using MultiSourceDataGetterType =
      std::function<void(std::vector<std::shared_ptr<NFmiFastQueryInfo>> &,
                         const NFmiDataIdent &,
                         const NFmiLevel &,
                         NFmiInfoData::Type,
                         const std::shared_ptr<NFmiArea> &)>;

  virtual ~NFmiInfoAreaMask();
  NFmiInfoAreaMask();
  NFmiInfoAreaMask(const NFmiCalculationCondition &theOperation,
                   Type theMaskType,
                   NFmiInfoData::Type theDataType,
                   const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                   unsigned long thePossibleMetaParamId,
                   BinaryOperator thePostBinaryOperator);

  NFmiInfoAreaMask(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                   unsigned long thePossibleMetaParamId,
                   BinaryOperator thePostBinaryOperator);
  NFmiInfoAreaMask(const NFmiInfoAreaMask &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMask &operator=(const NFmiInfoAreaMask &theMask) = delete;

  static void SetMultiSourceDataGetterCallback(
      const MultiSourceDataGetterType &theCallbackFunction);
  static MultiSourceDataGetterType &GetMultiSourceDataGetterCallback()
  {
    return itsMultiSourceDataGetter;
  }
  // Nyt ainakin synop ja salama datat ovat tälläisiä
  static bool IsKnownMultiSourceData(const std::shared_ptr<NFmiFastQueryInfo> &theInfo);
  static std::vector<std::shared_ptr<NFmiFastQueryInfo>> GetMultiSourceData(
      const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
      std::shared_ptr<NFmiArea> &calculationArea,
      bool getStationarySynopDataOnly);

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double ValueFinal(const NFmiCalculationParams &theCalculationParams,
                    bool fUseTimeInterpolationAlways);
  double HeightValue(double theHeight, const NFmiCalculationParams &theCalculationParams) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;

  // Pitää olla ei virtuaalinen versio PressureValue-metodista, jotta tietyissä tilanteissa estetään
  // mahdollisen lapsiluokan metodin virtuaalinen kutsu
  double PressureValueStatic(double thePressure, const NFmiCalculationParams &theCalculationParams);
  double HeightValueStatic(double theHeight, const NFmiCalculationParams &theCalculationParams);

  // erikoistapaus optimoituun käyttöön (ei voi käyttää kaikille luokille!!!!)
  bool IsMasked(int theIndex) const override;
  bool IsMasked(const NFmiPoint &theLatLon) const override
  {
    return NFmiAreaMaskImpl::IsMasked(theLatLon);
  }
  bool Time(const NFmiMetTime &theTime) override;
  bool IsWantedParam(const NFmiDataIdent &theParam, const NFmiLevel *theLevel = 0) const override;
  std::shared_ptr<NFmiFastQueryInfo> Info() override { return itsInfo; };
  void Info(const std::shared_ptr<NFmiFastQueryInfo> &newInfo);
  void UpdateInfo(std::shared_ptr<NFmiFastQueryInfo> &theInfo) override;

  // tehty virtuaaliseksi, koska perusluokassa ei ole valmiuksia paaluttaa arvoa tähän
  const NFmiDataIdent *DataIdent() const override;

  // tehty virtuaaliseksi, koska perusluokassa ei ole valmiuksia paaluttaa arvoa tähän
  const NFmiParam *Param() const override;

  // tehty virtuaaliseksi, koska perusluokassa ei ole valmiuksia paaluttaa arvoa tähän
  const NFmiLevel *Level() const override;
  void Level(const NFmiLevel &theLevel) override;

  // tehty virtuaaliseksi, koska perusluokassa ei ole valmiuksia paaluttaa arvoa tähän
  bool UseLevelInfo() const override;
  bool UsePressureLevelInterpolation() const override { return fUsePressureLevelInterpolation; }
  void UsePressureLevelInterpolation(bool newValue) override
  {
    fUsePressureLevelInterpolation = newValue;
  }
  double UsedPressureLevelValue() const override { return itsUsedPressureLevelValue; }
  void UsedPressureLevelValue(double newValue) override { itsUsedPressureLevelValue = newValue; }
  bool CheckPossibleObservationDistance(
      const NFmiCalculationParams &theCalculationParamsInOut) override;
  void Initialize() override;

  static std::shared_ptr<NFmiDataModifier> CreateIntegrationFuction(
      NFmiAreaMask::FunctionType func);
  static bool CalcTimeLoopIndexies(std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                   const NFmiCalculationParams &theCalculationParams,
                                   double theStartTimeOffsetInHours,
                                   double theEndTimeOffsetInHours,
                                   unsigned long *theStartTimeIndexOut,
                                   unsigned long *theEndTimeIndexOut);
  static bool CalcTimeLoopIndexies(std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                   const NFmiCalculationParams &theCalculationParams,
                                   const NFmiMetTime &theStartTime,
                                   const NFmiMetTime &theEndTime,
                                   unsigned long *theStartTimeIndexOut,
                                   unsigned long *theEndTimeIndexOut);
  void AddValuesToFunctionModifier(std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                   std::shared_ptr<NFmiDataModifier> &theFunctionModifier,
                                   const NFmiLocationCache &theLocationCache,
                                   NFmiAreaMask::FunctionType integrationFunction);

 protected:
  double CalcValueFromLocation(const NFmiPoint &theLatLon) const override;
  const NFmiString MakeSubMaskString() const override;
  bool IsTimeInterpolationNeeded(bool fUseTimeInterpolationAlways) const;
  float CalcMetaParamValue(const NFmiCalculationParams &theCalculationParams);
  float CalcMetaParamHeightValue(double theHeight,
                                 const NFmiCalculationParams &theCalculationParams);
  float CalcMetaParamPressureValue(double thePressure,
                                   const NFmiCalculationParams &theCalculationParams);
  float CalcCachedInterpolation(std::shared_ptr<NFmiFastQueryInfo> &theUsedInfo,
                                const NFmiLocationCache &theLocationCache,
                                const NFmiTimeCache *theTimeCache);
  float CalcMetaParamCachedInterpolation(std::shared_ptr<NFmiFastQueryInfo> &theUsedInfo,
                                         const NFmiLocationCache &theLocationCache,
                                         const NFmiTimeCache *theTimeCache);
  void AddExtremeValues(std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                        std::shared_ptr<NFmiDataModifier> &theFunctionModifier,
                        const NFmiLocationCache &theLocationCache);
  virtual bool DoExtremeAddingSpecialCase() const { return true; }
  virtual void AddValueToModifier(std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                  std::shared_ptr<NFmiDataModifier> &theFunctionModifier,
                                  float theValue);
  bool FindClosestStationData(const NFmiCalculationParams &calculationParams,
                              size_t &dataIndexOut,
                              unsigned long &locationIndexOut);
  double GetSearchRadiusInMetres(double observationRadiusInKm);

  void DoConstructorInitializations(unsigned long thePossibleMetaParamId);

 protected:
  std::shared_ptr<NFmiFastQueryInfo> itsInfo;
  NFmiDataIdent itsDataIdent;
  NFmiLevel itsLevel;
  // jos vanhassa maski jutussa tarvitaan aikainterpolointia, tässä on interpoloitava aika
  NFmiMetTime itsTime;
  // erikois optimointia Value-metodin ja Time-metodin käytössä
  bool fIsTimeIntepolationNeededInValue;
  // tämä asetetaan trueksi jos tarvitaan tehdä lennossa painepinta interpolointeja
  bool fUsePressureLevelInterpolation;
  // jos fUsePressureLevelInterpolation on true, käytetään laskuissa tätä painepintaa
  double itsUsedPressureLevelValue;
  MetaParamDataHolder metaParamDataHolder;
  bool fIsModelClimatologyData = false;
  // Datan tuottajasta päätellään, voiko tuottajalla olla useita datoja (synop + salamat)
  bool fUseMultiSourceData = false;
  // Tähän laitetaan havainto laskuissa käytettävät datat, joko se joko on jo emoluokassa
  // oleva itsInfo, tai multisource tapauksissa joukko datoja
  std::vector<std::shared_ptr<NFmiFastQueryInfo>> itsInfoVector;
  static MultiSourceDataGetterType itsMultiSourceDataGetter;

  template <typename GetFunction>
  float CalcMetaParamValueWithFunction(GetFunction getFunction)
  {
    // Tästä funktiosta kutsutuissa Value -metodeissa ei ole tarkoitus tehdä metaparam tarkastusta,
    // siksi se laitetaan väliaikaisesti pois päältä
    MetaParamDataHolderDoCheckStateRestorer metaParamDataHolderDoCheckStateRestorer(
        metaParamDataHolder, false);

    if (metaParamDataHolder.metaWindParamUsage().HasWsAndWd())
    {
      itsInfo->Param(kFmiWindSpeedMS);
      float WS = getFunction();
      itsInfo->Param(kFmiWindDirection);
      float WD = getFunction();
      switch (metaParamDataHolder.possibleMetaParamId())
      {
        case kFmiWindUMS:
          return NFmiFastInfoUtils::CalcU(WS, WD);
        case kFmiWindVMS:
          return NFmiFastInfoUtils::CalcV(WS, WD);
        case kFmiWindVectorMS:
          return NFmiFastInfoUtils::CalcWindVectorFromSpeedAndDirection(WS, WD);
        default:
          return kFloatMissing;
      }
    }
    else if (metaParamDataHolder.metaWindParamUsage().HasWindComponents())
    {
      itsInfo->Param(kFmiWindUMS);
      float u = getFunction();
      itsInfo->Param(kFmiWindVMS);
      float v = getFunction();
      switch (metaParamDataHolder.possibleMetaParamId())
      {
        case kFmiWindDirection:
          return NFmiFastInfoUtils::CalcWD(u, v);
        case kFmiWindSpeedMS:
          return NFmiFastInfoUtils::CalcWS(u, v);
        case kFmiWindVectorMS:
          return NFmiFastInfoUtils::CalcWindVectorFromWindComponents(u, v);
        default:
          return kFloatMissing;
      }
    }

    return kFloatMissing;
  }

};  // class NFmiInfoAreaMask

//! Tämä luokka toimii kuten NFmiInfoAreaMask mutta kurkkaa halutun x-y hila pisteen yli arvoa
class NFmiInfoAreaMaskPeekXY : public NFmiInfoAreaMask
{
 public:
  virtual ~NFmiInfoAreaMaskPeekXY();
  NFmiInfoAreaMaskPeekXY();
  NFmiInfoAreaMaskPeekXY(const NFmiCalculationCondition &theOperation,
                         Type theMaskType,
                         NFmiInfoData::Type theDataType,
                         const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                         int theXOffset,
                         int theYOffset,
                         unsigned long thePossibleMetaParamId,
                         BinaryOperator thePostBinaryOperator);

  NFmiInfoAreaMaskPeekXY(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                         int theXOffset,
                         int theYOffset,
                         unsigned long thePossibleMetaParamId,
                         BinaryOperator thePostBinaryOperator);
  NFmiInfoAreaMaskPeekXY(const NFmiInfoAreaMaskPeekXY &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMaskPeekXY &operator=(const NFmiInfoAreaMaskPeekXY &theMask) = delete;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;

 protected:
  NFmiCalculationParams MakeModifiedCalculationParams(
      const NFmiCalculationParams &theCalculationParams);

  int itsXOffset;  // kuinka monen hilapisteen yli kurkataan x-suunnassa
  int itsYOffset;  // kuinka monen hilapisteen yli kurkataan y-suunnassa
 private:
  void CalcGridDiffs();

  double itsGridXDiff;  // mikä on x-hilaväli xy-koordinaatistossa
  double itsGridYDiff;  // mikä on y-hilaväli xy-koordinaatistossa

};  // class NFmiInfoAreaMaskPeekXY

//! Tämä luokka toimii kuten NFmiInfoAreaMaskPeekXY mutta käyttää kurkkaukseen editoritavan-datan
//! tai macroParam-datan karttaprojektiota ja X/Y kurkkaukset tehdään siinä kilometreissa. Tämä
//! kurkkaus ei välttämättä mene lat-lon suuntaisesti kuin ehkä jos pohjana on latlon area.
class NFmiInfoAreaMaskPeekXY2 : public NFmiInfoAreaMask
{
 public:
  virtual ~NFmiInfoAreaMaskPeekXY2();
  NFmiInfoAreaMaskPeekXY2();
  NFmiInfoAreaMaskPeekXY2(const NFmiCalculationCondition &theOperation,
                          Type theMaskType,
                          NFmiInfoData::Type theDataType,
                          const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          const std::shared_ptr<NFmiFastQueryInfo> &theEditedInfo,
                          int theXOffset,
                          int theYOffset,
                          unsigned long thePossibleMetaParamId,
                          BinaryOperator thePostBinaryOperator);

  NFmiInfoAreaMaskPeekXY2(const NFmiInfoAreaMaskPeekXY2 &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMaskPeekXY2 &operator=(const NFmiInfoAreaMaskPeekXY2 &theMask) = delete;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;

 protected:
  NFmiCalculationParams MakeModifiedCalculationParams(
      const NFmiCalculationParams &theCalculationParams);

  int itsXOffset;  // kuinka monen hilapisteen yli kurkataan x-suunnassa
  int itsYOffset;  // kuinka monen hilapisteen yli kurkataan y-suunnassa
 private:
  std::shared_ptr<NFmiFastQueryInfo>
      itsEditedInfo;  // Tästä katsotaan vain haluttu siirtymä matka

};  // class NFmiInfoAreaMaskPeekXY2

//! Tämä luokka toimii kuten NFmiInfoAreaMaskPeekXY ja  käyttää kurkkaukseen haluttuja kilometri
//! etäisyyksiä
class NFmiInfoAreaMaskPeekXY3 : public NFmiInfoAreaMask
{
 public:
  virtual ~NFmiInfoAreaMaskPeekXY3();
  NFmiInfoAreaMaskPeekXY3();
  NFmiInfoAreaMaskPeekXY3(const NFmiCalculationCondition &theOperation,
                          Type theMaskType,
                          NFmiInfoData::Type theDataType,
                          const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          const std::shared_ptr<NFmiFastQueryInfo> &theEditedInfo,
                          double theXOffsetInKM,
                          double theYOffsetInKM,
                          unsigned long thePossibleMetaParamId,
                          BinaryOperator thePostBinaryOperator);

  NFmiInfoAreaMaskPeekXY3(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          const std::shared_ptr<NFmiFastQueryInfo> &theEditedInfo,
                          double theXOffsetInKM,
                          double theYOffsetInKM,
                          unsigned long thePossibleMetaParamId,
                          BinaryOperator thePostBinaryOperator);
  NFmiInfoAreaMaskPeekXY3(const NFmiInfoAreaMaskPeekXY3 &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMaskPeekXY3 &operator=(const NFmiInfoAreaMaskPeekXY2 &theMask) = delete;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;

 protected:
  NFmiCalculationParams MakeModifiedCalculationParams(
      const NFmiCalculationParams &theCalculationParams);

  double itsXOffsetInKM;  // kuinka monen kilometrin yli kurkataan x-suunnassa
  double itsYOffsetInKM;  // kuinka monen kilometrin yli kurkataan y-suunnassa
 private:
  std::shared_ptr<NFmiFastQueryInfo>
      itsEditedInfo;  // Tästä katsotaan vain haluttu siirtymä matka

};  // class NFmiInfoAreaMaskPeekXY3

class NFmiInfoAreaMaskMetFuncBase : public NFmiInfoAreaMask
{
 public:
  ~NFmiInfoAreaMaskMetFuncBase();
  NFmiInfoAreaMaskMetFuncBase(const NFmiCalculationCondition &theOperation,
                              Type theMaskType,
                              NFmiInfoData::Type theDataType,
                              const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                              bool thePeekAlongTudes,
                              MetFunctionDirection theMetFuncDirection,
                              unsigned long thePossibleMetaParamId,
                              BinaryOperator thePostBinaryOperator);
  NFmiInfoAreaMaskMetFuncBase(const NFmiInfoAreaMaskMetFuncBase &theOther);
  NFmiInfoAreaMaskMetFuncBase &operator=(const NFmiInfoAreaMaskMetFuncBase &theMask) = delete;
  void Initialize() override;

  bool Time(const NFmiMetTime &theTime) override;

 protected:
  typedef std::vector<std::pair<int, float>> CalcFactorVector;

  virtual void SetDividers() = 0;
  virtual void InitCalcFactorVectors() = 0;
  virtual const CalcFactorVector &LowerEdgeFactors() const = 0;
  virtual const CalcFactorVector &UpperEdgeFactors() const = 0;
  virtual const CalcFactorVector &MiddleAreaFactors() const = 0;

  NFmiLocationCache CalcLocationCache(const NFmiPoint &theLatlon);
  void SetGridSizeVariables();
  bool IsDataOperatable(const std::shared_ptr<NFmiFastQueryInfo> &theInfo) const;
  float Peek(const NFmiLocationCache &theLocationCachePoint, int offsetX, int offsetY);
  NFmiLocationCache CalcPeekedLocation(const NFmiLocationCache &theLocationCachePoint,
                                       int theOffsetX,
                                       int theOffsetY);
  float CalcMetFuncWithFactors(const NFmiLocationCache &theLocationCachePoint,
                               bool fDoX,
                               const CalcFactorVector &theCalcFactors);
  float CalcUpperEdgeMetFunc(const NFmiLocationCache &theLocationCachePoint, bool fDoX);
  float CalcLowerEdgeMetFunc(const NFmiLocationCache &theLocationCachePoint, bool fDoX);
  float CalcMiddleAreaMetFunc(const NFmiLocationCache &theLocationCachePoint, bool fDoX);
  float CalcMetFuncComponent(const NFmiLocationCache &theLocationCachePoint,
                             bool fDoX,
                             double theRealGridPointComponent,
                             int theGridSizeComponent);
  float CalcCachedPressureLevelValue(float P,
                                     const NFmiLocationCache &theLocationCache,
                                     const NFmiTimeCache &theTimeCache);
  float CalcMetaParamCachedPressureLevelValue(float P,
                                              const NFmiLocationCache &theLocationCache,
                                              const NFmiTimeCache &theTimeCache);

  NFmiTimeCache
      itsTimeCache;  // tähän lasketaan time-metodissa optimoinnissa käytetty aika-cache laskuri
  int itsGridSizeX;
  int itsGridSizeY;
  float itsGridPointWidthInMeters;
  float itsGridPointHeightInMeters;
  float itsMetFuncDividerX;  // tähän lasketaan luku, millä jaetaan laskut X-suunnassa
  float itsMetFuncDividerY;  // tähän lasketaan luku, millä jaetaan laskut Y-suunnassa
  bool fTotalWindParam;  // jos parametrina on total wind- itää se ottaa tietyissä tapauksissa
                         // erikseen huomioon laskuissa
  bool fPeekAlongTudes;  // tehdäänkö xy-suuntaiset kurkkaukset datan oman hilan suunnassa (false)
                         // vaiko pitkin latitude ja longitude linjoja (true)
};

//! Tämä luokka laskee gradientin tai divergenssin.
class NFmiInfoAreaMaskGrad : public NFmiInfoAreaMaskMetFuncBase
{
 public:
  ~NFmiInfoAreaMaskGrad();
  NFmiInfoAreaMaskGrad(const NFmiCalculationCondition &theOperation,
                       Type theMaskType,
                       NFmiInfoData::Type theDataType,
                       const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                       bool thePeekAlongTudes,
                       MetFunctionDirection theMetFuncDirection,
                       unsigned long thePossibleMetaParamId,
                       BinaryOperator thePostBinaryOperator);
  NFmiInfoAreaMaskGrad(const NFmiInfoAreaMaskGrad &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMaskGrad &operator=(const NFmiInfoAreaMaskGrad &theMask) = delete;

  bool CalculateDivergence() const { return fCalculateDivergence; }
  void CalculateDivergence(bool newValue) { fCalculateDivergence = newValue; }
  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;
  void InitCalcFactorVectors() override;

 protected:
  void SetDividers() override;
  const CalcFactorVector &LowerEdgeFactors() const override { return itsLowerEdgeFactors; }
  const CalcFactorVector &UpperEdgeFactors() const override { return itsUpperEdgeFactors; }
  const CalcFactorVector &MiddleAreaFactors() const override { return itsMiddleAreaFactors; }

 private:
  static CalcFactorVector itsLowerEdgeFactors;
  static CalcFactorVector itsUpperEdgeFactors;
  static CalcFactorVector itsMiddleAreaFactors;
  bool fCalculateDivergence;  // tämä muuttuja määrää lasketaanko gradientti (oletus) vai
                              // divergenssi. Laskenta on muuten sama,
                              // mutta x- ja y-componentit lasketaan vain erilailla lopuksi yhteen

  static bool fCalcFactorVectorsInitialized;

};  // class NFmiInfoAreaMaskGrad

class NFmiInfoAreaMaskAdvection : public NFmiInfoAreaMaskGrad
{
 public:
  ~NFmiInfoAreaMaskAdvection();
  NFmiInfoAreaMaskAdvection(const NFmiCalculationCondition &theOperation,
                            Type theMaskType,
                            NFmiInfoData::Type theDataType,
                            const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                            const std::shared_ptr<NFmiFastQueryInfo> &theInfoUwind,
                            const std::shared_ptr<NFmiFastQueryInfo> &theInfoVwind,
                            bool thePeekAlongTudes,
                            MetFunctionDirection theMetFuncDirection,
                            unsigned long thePossibleMetaParamId,
                            BinaryOperator thePostBinaryOperator);
  NFmiInfoAreaMaskAdvection(const NFmiInfoAreaMaskAdvection &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 private:
  void SetupWindComponents();

  std::shared_ptr<NFmiFastQueryInfo> itsInfoUwind;  // Tästä katsotaan tuulen u-komponentti
  std::shared_ptr<NFmiFastQueryInfo> itsInfoVwind;  // Tästä katsotaan tuulen v-komponentti
};

//! Tämä luokka laskee laplacen (eli nablan?).
class NFmiInfoAreaMaskLaplace : public NFmiInfoAreaMaskMetFuncBase
{
 public:
  ~NFmiInfoAreaMaskLaplace();
  NFmiInfoAreaMaskLaplace(const NFmiCalculationCondition &theOperation,
                          Type theMaskType,
                          NFmiInfoData::Type theDataType,
                          const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          bool thePeekAlongTudes,
                          MetFunctionDirection theMetFuncDirection,
                          unsigned long thePossibleMetaParamId,
                          BinaryOperator thePostBinaryOperator);
  NFmiInfoAreaMaskLaplace(const NFmiInfoAreaMaskLaplace &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMaskLaplace &operator=(const NFmiInfoAreaMaskLaplace &theMask) = delete;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  void InitCalcFactorVectors() override;

 protected:
  void SetDividers() override;
  float CalcLaplaceX(const NFmiLocationCache &theLocationCachePoint);
  float CalcLaplaceY(const NFmiLocationCache &theLocationCachePoint);

  const CalcFactorVector &LowerEdgeFactors() const override { return itsLowerEdgeFactors; }
  const CalcFactorVector &UpperEdgeFactors() const override { return itsUpperEdgeFactors; }
  const CalcFactorVector &MiddleAreaFactors() const override { return itsMiddleAreaFactors; }

 private:
  static CalcFactorVector itsLowerEdgeFactors;
  static CalcFactorVector itsUpperEdgeFactors;
  static CalcFactorVector itsMiddleAreaFactors;
  static bool fCalcFactorVectorsInitialized;

};  // class NFmiInfoAreaMaskLaplace

class NFmiInfoAreaMaskRotor : public NFmiInfoAreaMaskGrad
{
 public:
  ~NFmiInfoAreaMaskRotor();
  NFmiInfoAreaMaskRotor(const NFmiCalculationCondition &theOperation,
                        Type theMaskType,
                        NFmiInfoData::Type theDataType,
                        const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                        bool thePeekAlongTudes,
                        MetFunctionDirection theMetFuncDirection,
                        unsigned long thePossibleMetaParamId,
                        BinaryOperator thePostBinaryOperator);
  NFmiInfoAreaMaskRotor(const NFmiInfoAreaMaskRotor &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
};

// Tämän luokan avulla voidaan iterointi halutessa lopettaa
// NFmiInfoAreaMaskVertFunc::IterateLevelsFromGroundUpward metodissa.
// Tähän voi tallettaa arvon tai indeksin, jonka em. metodia kutsuva taho voi sitten palauttaa.
// Jos tätä dataa ei haluta käyttää, niin iterointi tehdään aina alusta loppuun ja paluu arvot
// voi palauttaa muita keinoja käyttämällä.
class VerticalIterationBreakingData
{
 public:
  VerticalIterationBreakingData(bool useData = false)
      : useBreakingData(useData), stopIteration(false), value(kFloatMissing), index(gMissingIndex)
  {
  }

  bool BreakIteration() const { return useBreakingData && stopIteration; }

  bool useBreakingData;  // Käytetäänkö loopissa tätä datapakettia
  bool stopIteration;
  float value;          // arvo joka halutaan loopituksen lopuksi palauttaa
  unsigned long index;  // indeksi joka halutaan loppituksen lopuksi palauttaa
};

class NFmiInfoAreaMaskVertFunc : public NFmiInfoAreaMaskMetFuncBase
{
 public:
  ~NFmiInfoAreaMaskVertFunc();
  NFmiInfoAreaMaskVertFunc(const NFmiCalculationCondition &theOperation,
                           Type theMaskType,
                           NFmiInfoData::Type theDataType,
                           const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                           NFmiAreaMask::FunctionType thePrimaryFunc,
                           NFmiAreaMask::FunctionType theSecondaryFunc,
                           int theArgumentCount,
                           unsigned long thePossibleMetaParamId);
  NFmiInfoAreaMaskVertFunc(const NFmiInfoAreaMaskVertFunc &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMaskVertFunc &operator=(const NFmiInfoAreaMaskVertFunc &theMask) = delete;
  void Initialize() override;
  void SetArguments(std::vector<float> &theArgumentVector) override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  // Nämä virtuaali funktiot on toteutettava, vaikka niillä ei ole taas käyttöä täällä
  void SetDividers() override {}
  void InitCalcFactorVectors() override {}
  const CalcFactorVector &LowerEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &UpperEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &MiddleAreaFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }

  void FindCalculatedLeves(const NFmiLocationCache &theLocationCache);
  void SearchLevels(const NFmiLocationCache &theLocationCache);
  void SetLevelValues();
  float DoGetFunction(const NFmiLocationCache &theLocationCache,
                      const NFmiCalculationParams &theCalculationParams,
                      float theLevelValue);
  float DoPeekZFunction(const NFmiCalculationParams &theCalculationParams, float theDeltaZ);
  float DoVerticalGrad(const NFmiLocationCache &theLocationCache,
                       const NFmiCalculationParams &theCalculationParams);
  float DoFindFunction(const NFmiLocationCache &theLocationCache,
                       const NFmiCalculationParams &theCalculationParams);
  float FindHeightForSimpleCondition(const NFmiLocationCache &theLocationCache,
                                     const NFmiCalculationParams &theCalculationParams);
  void SetLevelIndexies(float theHeightValue);
  unsigned long GetNonMissingStartLevelIndex(const NFmiLocationCache &theLocationCache,
                                             const NFmiCalculationParams &theCalculationParams);
  float GetLevelHeightValue(const NFmiLocationCache &theLocationCache);
  float DoNormalFunction(const NFmiLocationCache &theLocationCache,
                         const NFmiCalculationParams &theCalculationParams);
  float CalculateUsedPeekZPressureLevel(float currentPressureLevel, float usedDeltaZ);

  // Vertikaali levelien läpikäynti oikeassa järjestyksessä. Tehty template funktioksi
  // jotta sitä voidaan käyttää erilaisten lambda funktioiden avulla eri paikoissa.
  template <typename FunctionObject>
  void IterateLevelsFromGroundUpward(FunctionObject &functionObject,
                                     VerticalIterationBreakingData &iterationBreakingData,
                                     int usedStartLevelIndex,
                                     const NFmiCalculationParams &theCalculationParams)
  {
    for (auto levelIndex = usedStartLevelIndex;
         fReverseLevels ? levelIndex >= static_cast<int>(itsEndLevelIndex)
                        : levelIndex <= static_cast<int>(itsEndLevelIndex);
         levelIndex += itsLevelIncrement)
    {
      if (itsInfo->LevelIndex(levelIndex))  // pitäisi olla aina totta
      {
        functionObject();
        if (iterationBreakingData.BreakIteration()) break;
      }
    }
  }

  bool VertFuncSimpleconditionCheck(const NFmiCalculationParams &theCalculationParams);

  // Esim. Avg, Max, Get, Find, jne.
  NFmiAreaMask::FunctionType itsPrimaryFunc;
  // Esim. VertP, VertZ, jne.
  NFmiAreaMask::FunctionType itsSecondaryFunc;
  std::vector<float> itsArgumentVector;
  // tämä luodaan itsPrimaryFunc-dataosan mukaan
  std::shared_ptr<NFmiDataModifier> itsFunctionModifier;
  // Tähän otetaan annetusta argumentti listasta aloitus korkeus (missä yksikössä onkaan)
  float itsStartLevelValue;
  // tähän otetaan annetusta argumentti listasta lopetus korkeus (missä
  // yksikössä onkaan), PAITSI, jos kyse on get-funktiosta, jolloin tämä on
  // puuttuva
  float itsEndLevelValue;
  unsigned long itsStartLevelIndex;  // dataa käydään läpi alkaen tästä levelistä
  unsigned long itsEndLevelIndex;    // dataa käydään läpi tähän leveliin asti

  FmiParameterName itsUsedHeightParId;
  bool fReturnHeightValue;
  // Kun ollaan päätelty mihin suuntaan leveldata menee (ylös maanpinnasta vai alas avaruudesta)
  // tähän on tarkoitus laskea for-looppeja varten level incrementti (joko 1 tai -1)
  // Jos datan levelien suunta on maanpinnasta ylöspäin, on incrementti 1 ja käydään levelit
  // normaali järjestyksessä läpi. Jos datan levelien suuntä on avaruudesta maanpintaa kohden, on
  // incrementti -1 ja levelit käydään käänteisessä järjestyksessä.
  int itsLevelIncrement;
  // Jos itsLevelIncrement on -1, tämä on true, jolloin for-loopitus tehdään
  // käänteisessä järjestyksessä
  bool fReverseLevels;
};

class NFmiInfoAreaMaskVertConditionalFunc : public NFmiInfoAreaMaskVertFunc
{
 public:
  ~NFmiInfoAreaMaskVertConditionalFunc();
  NFmiInfoAreaMaskVertConditionalFunc(const NFmiCalculationCondition &theOperation,
                                      Type theMaskType,
                                      NFmiInfoData::Type theDataType,
                                      const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                      NFmiAreaMask::FunctionType thePrimaryFunc,
                                      NFmiAreaMask::FunctionType theSecondaryFunc,
                                      int theArgumentCount,
                                      unsigned long thePossibleMetaParamId);
  NFmiInfoAreaMaskVertConditionalFunc(const NFmiInfoAreaMaskVertConditionalFunc &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMaskVertConditionalFunc &operator=(
      const NFmiInfoAreaMaskVertConditionalFunc &theMask) = delete;
  void Initialize() override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  bool InitializeFromArguments();
  bool CheckProbabilityCondition(double value);
  float DoFindConditionalFunction(const NFmiLocationCache &theLocationCache,
                                  const NFmiCalculationParams &theCalculationParams);

  double itsLimit1;
  double itsLimit2;
};

class NFmiInfoAreaMaskTimeVertFunc : public NFmiInfoAreaMaskVertFunc
{
 public:
  ~NFmiInfoAreaMaskTimeVertFunc();
  NFmiInfoAreaMaskTimeVertFunc(const NFmiCalculationCondition &theOperation,
                               Type theMaskType,
                               NFmiInfoData::Type theDataType,
                               const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                               NFmiAreaMask::FunctionType thePrimaryFunc,
                               NFmiAreaMask::FunctionType theSecondaryFunc,
                               int theArgumentCount,
                               unsigned long thePossibleMetaParamId);
  NFmiInfoAreaMaskTimeVertFunc(const NFmiInfoAreaMaskTimeVertFunc &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMaskTimeVertFunc &operator=(const NFmiInfoAreaMaskTimeVertFunc &theMask) = delete;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  void SetRangeValuesFromArguments();

  double itsStartTimeOffsetInHours;  // kuinka monta tuntia aikaiteroinnin alkuaika poikkeaa
                                     // current-timesta
  double itsEndTimeOffsetInHours;    // kuinka monta tuntia aikaiteroinnin loppuaika poikkeaa
                                     // current-timesta
};

class NFmiInfoAreaMaskProbFunc : public NFmiInfoAreaMask
{
 public:
  ~NFmiInfoAreaMaskProbFunc();
  NFmiInfoAreaMaskProbFunc(const NFmiCalculationCondition &theOperation,
                           Type theMaskType,
                           NFmiInfoData::Type theDataType,
                           const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                           NFmiAreaMask::FunctionType thePrimaryFunc,
                           NFmiAreaMask::FunctionType theSecondaryFunc,
                           int theArgumentCount,
                           unsigned long thePossibleMetaParamId);
  NFmiInfoAreaMaskProbFunc(const NFmiInfoAreaMaskProbFunc &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMaskProbFunc &operator=(const NFmiInfoAreaMaskProbFunc &theMask) = delete;
  void SetArguments(std::vector<float> &theArgumentVector) override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  static NFmiMetTime CalcTimeLoopLimits(std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                        const NFmiCalculationParams &theCalculationParams,
                                        double theStartTimeOffsetInHours,
                                        double theEndTimeOffsetInHours,
                                        unsigned long *theStartTimeIndexOut,
                                        unsigned long *theEndTimeIndexOut,
                                        bool *doSpecialCalculation,
                                        bool allowInterpolation);
  static bool CheckTimeIndicesForLoopBreak(unsigned long theStartTimeIndex,
                                           unsigned long theEndTimeIndex);

 protected:
  void InitializeFromArguments();
  bool CheckProbabilityCondition(double value);
  bool IsCalculationPointInsideCircle(const NFmiLocation &theCalculationPointLocation,
                                      int theOffsetX,
                                      int theOffsetY);
  void DoSubgridCalculations(const NFmiLocation &theCalculationPointLocation,
                             int leftSubGridOffset,
                             int rightSubGridOffset,
                             int bottomSubGridOffset,
                             int topSubGridOffset,
                             const NFmiMetTime &theInterpolationTime,
                             bool useInterpolatedTime,
                             NFmiCalculationParams &theSimpleConditionCalculationPointParams);

  // Seuraavat virtuaali funktiot liittyvät Simplecondition laskuihin ja ne on tarkoitus siirtää
  // myöhemmin ylemmäs perintäpuussa esim. juureen eli NFmiAreaMask interface luokkaan asti.
  void DoIntegrationCalculations(float value) override;
  void InitializeIntegrationValues() override;
  float CalculationPointValue(int theOffsetX,
                              int theOffsetY,
                              const NFmiMetTime &theInterpolationTime,
                              bool useInterpolatedTime) override;
  float CalcMetaParamCalculationPointValue(int theOffsetX,
                                           int theOffsetY,
                                           const NFmiMetTime &theInterpolationTime,
                                           bool useInterpolatedTime);
  double DoObservationAreaMaskCalculations(const NFmiCalculationParams &theCalculationParams);
  double CalcAreaProbability();
  float CalculationPointValueForObservation(const std::shared_ptr<NFmiFastQueryInfo> &info);

  // Esim. Over, Under, Between, Equal
  NFmiAreaMask::FunctionType itsPrimaryFunc;
  // esim. ProbRect, ProbCircle
  NFmiAreaMask::FunctionType itsSecondaryFunc;

  // tähän lasketaan lennossa laskuissa tarvittavat argumentit (alueen säde ja raja(t))
  std::vector<float> itsArgumentVector;

  // minkä säteisen alueen halutaan käyvän läpi originaali datassa
  double itsSearchRangeInKM;
  double itsLimit1;
  double itsLimit2;
  // kuinka monta tuntia aikaiteroinnin alkuaika poikkeaa current-timesta
  double itsStartTimeOffsetInHours;

  // kuinka monta tuntia aikaiteroinnin loppuaika poikkeaa current-timesta
  double itsEndTimeOffsetInHours;
  // käydään läpi ali hila, joka on tämän kokoinen x-suunnassa (min 2)
  int itsGridPointRectSizeX;
  // käydään läpi ali hila, joka on tämän kokoinen y-suunnassa (min 2)
  int itsGridPointRectSizeY;
  // Kaikki hilapisteet, missä todennäköisyys ehto piti paikkaansa (tämä otetaan talteen Value
  // -metodissa, jotta NFmiInfoAreaMaskOccurrance -lapsiluokka voi käyttää tämän luokan laskuja
  // hyväkseen hiladatan tapauksessa)
  int itsConditionFullfilledGridPointCount;
  // Kaikkien laskuissaa läpikäytyjen hilapisteiden määrä, aika- ja area-loopeissa yhteensä,
  // joissa oli ei-puuttuva arvo.
  int itsTotalCalculatedGridPoints;
};

class NFmiInfoTimeIntegrator : public NFmiInfoAreaMaskMetFuncBase
{
 public:
  ~NFmiInfoTimeIntegrator();
  NFmiInfoTimeIntegrator(const NFmiCalculationCondition &theOperation,
                         Type theMaskType,
                         NFmiInfoData::Type theDataType,
                         const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                         NFmiAreaMask::FunctionType theIntegrationFunc,
                         int theStartTimeOffset,
                         int theEndTimeOffset,
                         unsigned long thePossibleMetaParamId);
  NFmiInfoTimeIntegrator(const NFmiInfoTimeIntegrator &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  // Nämä virtuaali funktiot on toteutettava, vaikka niillä ei ole taas käyttöä täällä
  void SetDividers() override {}
  void InitCalcFactorVectors() override {}
  const CalcFactorVector &LowerEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &UpperEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &MiddleAreaFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }

  NFmiAreaMask::FunctionType itsIntegrationFunc;  // esim. Avg, Max, Min, Sum, jne.

  std::shared_ptr<NFmiDataModifier>
      itsFunctionModifier;  // tämä luodaan itsIntegrationFunc-dataosan mukaan
  int itsStartTimeOffset;   // dataa käydään läpi alkaen tästä suhteellisesta ajasta
  int itsEndTimeOffset;     // dataa käydään läpi päätyen tähän suhteelliseen aikaan
};

// Luokka tekee yksinkertaisen laatikko integraation halutulla funktiolla.
// Tarkoittaen esim. MIN(T -1 -1 1 1) tyyppisiä funktioita, joissa käydään läpi
// originaali dataa vakio hilalaatikon puitteissa.
class NFmiInfoRectAreaIntegrator : public NFmiInfoAreaMaskMetFuncBase
{
 public:
  ~NFmiInfoRectAreaIntegrator();
  NFmiInfoRectAreaIntegrator(const NFmiCalculationCondition &theOperation,
                             Type theMaskType,
                             NFmiInfoData::Type theDataType,
                             const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                             NFmiAreaMask::FunctionType theIntegrationFunc,
                             int theStartXOffset,
                             int theEndXOffset,
                             int theStartYOffset,
                             int theEndYOffset,
                             unsigned long thePossibleMetaParamId);
  NFmiInfoRectAreaIntegrator(const NFmiInfoRectAreaIntegrator &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  // Nämä virtuaali funktiot on toteutettava, vaikka niillä ei ole taas käyttöä täällä
  void SetDividers() override {}
  void InitCalcFactorVectors() override {}
  const CalcFactorVector &LowerEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &UpperEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &MiddleAreaFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }

  NFmiAreaMask::FunctionType itsIntegrationFunc;  // esim. Avg, Max, Min, Sum, jne.

  std::shared_ptr<NFmiDataModifier>
      itsFunctionModifier;  // tämä luodaan itsIntegrationFunc-dataosan mukaan
  int itsStartXOffset;
  int itsEndXOffset;
  int itsStartYOffset;
  int itsEndYOffset;
};

// Luokka tekee alueellisia integraatio laskuja halutulla funktiolla.
// Laskenta-alue on halutun säteinen [km] ympyrä kulloisen laskenta pisteen
// ympärillä datan originaali hilassa.
class NFmiInfoAreaIntegrationFunc : public NFmiInfoAreaMaskProbFunc
{
 public:
  ~NFmiInfoAreaIntegrationFunc();
  NFmiInfoAreaIntegrationFunc(const NFmiCalculationCondition &theOperation,
                              Type theMaskType,
                              NFmiInfoData::Type theDataType,
                              const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                              NFmiAreaMask::FunctionType thePrimaryFunc,
                              NFmiAreaMask::FunctionType theSecondaryFunc,
                              int theArgumentCount,
                              unsigned long thePossibleMetaParamId);
  NFmiInfoAreaIntegrationFunc(const NFmiInfoAreaIntegrationFunc &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaIntegrationFunc &operator=(const NFmiInfoAreaIntegrationFunc &theMask) = delete;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  void DoIntegrationCalculations(float value) override;
  void InitializeIntegrationValues() override;

  // Esim. Avg, Max, Min, Sum, Med, jne.
  NFmiAreaMask::FunctionType itsIntegrationFunc;
  // Tämä luodaan itsIntegrationFunc-dataosan mukaan
  std::shared_ptr<NFmiDataModifier> itsFunctionModifier;
};

// Luokka joka laskee todennäköisyyksiä datan omassa hilassa ynmpyrä alueen sisällä olevien
// pisteiden kanssa, MUTTA niin että laskuissa käytetään simple-condition laskuja.
class NFmiInfoAreaMaskAreaProbFunc : public NFmiInfoAreaMaskProbFunc
{
 public:
  ~NFmiInfoAreaMaskAreaProbFunc();
  NFmiInfoAreaMaskAreaProbFunc(const NFmiCalculationCondition &theOperation,
                               Type theMaskType,
                               NFmiInfoData::Type theDataType,
                               const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                               NFmiAreaMask::FunctionType thePrimaryFunc,
                               NFmiAreaMask::FunctionType theSecondaryFunc,
                               int theArgumentCount,
                               unsigned long thePossibleMetaParamId);
  NFmiInfoAreaMaskAreaProbFunc(const NFmiInfoAreaIntegrationFunc &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMaskAreaProbFunc &operator=(const NFmiInfoAreaMaskAreaProbFunc &theMask) = delete;

 protected:
  void DoIntegrationCalculations(float value) override;
  float CalculationPointValue(int theOffsetX,
                              int theOffsetY,
                              const NFmiMetTime &theInterpolationTime,
                              bool useInterpolatedTime) override;
};

// Datalle tehdään laskuissa aina haluttu aikasiirto.
class NFmiTimeShiftedInfoAreaMask : public NFmiInfoAreaMask
{
 public:
  virtual ~NFmiTimeShiftedInfoAreaMask();
  NFmiTimeShiftedInfoAreaMask();
  NFmiTimeShiftedInfoAreaMask(const NFmiCalculationCondition &theOperation,
                              Type theMaskType,
                              NFmiInfoData::Type theDataType,
                              const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                              float theTimeOffsetInHours,
                              unsigned long thePossibleMetaParamId,
                              BinaryOperator thePostBinaryOperator);
  NFmiTimeShiftedInfoAreaMask(const NFmiTimeShiftedInfoAreaMask &theOther);
  NFmiAreaMask *Clone() const override;
  NFmiInfoAreaMask &operator=(const NFmiInfoAreaMask &theMask) = delete;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double HeightValue(double theHeight, const NFmiCalculationParams &theCalculationParams) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;

  bool Time(const NFmiMetTime &theTime) override;

 protected:
  NFmiCalculationParams GetUsedCalculationParams(const NFmiCalculationParams &theCalculationParams);

  float itsTimeOffsetInHours;
  long itsChangeByMinutesValue;
};

// ======================================================================
