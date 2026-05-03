// ======================================================================
/*!
 * \file NFmiQueryDataUtil.h
 * \brief Interface of class NFmiQueryDataUtil
 */
// ======================================================================

#pragma once

#include "NFmiArea.h"
#include "NFmiDataIdent.h"
#include "NFmiDataMatrix.h"
#include "NFmiDef.h"
#include "NFmiGlobals.h"
#include "NFmiGrid.h"
#include "NFmiLevel.h"
#include "NFmiMetTime.h"
#include <memory>

#ifndef BOOST_DISABLE_THREADS

#ifdef _MSC_VER
#pragma warning( \
    disable : 4244 4267 4512)  // boost:in thread kirjastosta tulee ikävästi 4244 varoituksia
#endif
#include <boost/thread.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4244 4267 4512)  // laitetaan 4244 takaisin päälle, koska se on tärkeä
                                           // (esim. double -> int auto castaus varoitus)
#endif

#endif  // BOOST_DISABLE_THREADS

#include <set>
#include <string>
#include <vector>

class NFmiFastQueryInfo;
class NFmiQueryInfo;
class NFmiGrid;
class NFmiQueryData;
class NFmiTimeDescriptor;
class NFmiTimeBag;
class NFmiParamBag;
class NFmiPoint;
class NFmiString;
class NFmiRect;
class NFmiDataModifier;
class NFmiCalculationCondition;
class NFmiLocationBag;
class NFmiArea;
class NFmiLogger;

/*!
 * \class MyGrid
 * Tässä pari apu luokkaa useiden hilojen yhdistämistä varten.
 * En käyttänyt NFmiGrid-luokkaa, koska sillä ja sen emolla ei
 * ollut kopi-konstruktoria ei sijoitus operaattoreita (enkä jaksanut tehdä).
 * Käytetään apuna mm. kun grib:eistä tehdään qdataa ja kun yhdistellään
 * monista qdatoista yksi uusi qdata.
 */
class MyGrid
{
 public:
  MyGrid() : itsArea(0), itsNX(0), itsNY(0) {}
  MyGrid(const NFmiArea *theArea, int theNX, int theNY)
      : itsArea(theArea ? theArea->Clone() : 0), itsNX(theNX), itsNY(theNY)
  {
  }

  MyGrid(const MyGrid &theGrid)
      : itsArea(theGrid.itsArea ? theGrid.itsArea->Clone() : 0),
        itsNX(theGrid.itsNX),
        itsNY(theGrid.itsNY)
  {
  }

  MyGrid(const NFmiGrid &theGrid)
      : itsArea(theGrid.Area() ? theGrid.Area()->Clone() : 0),
        itsNX(theGrid.XNumber()),
        itsNY(theGrid.YNumber())
  {
  }

  ~MyGrid() { delete itsArea; }
  MyGrid &operator=(const MyGrid &theGrid)
  {
    if (this != &theGrid)
    {
      delete itsArea;
      itsArea = theGrid.itsArea ? theGrid.itsArea->Clone() : 0;
      itsNX = theGrid.itsNX;
      itsNY = theGrid.itsNY;
    }
    return *this;
  }

  MyGrid &operator=(const NFmiGrid &theGrid)
  {
    delete itsArea;
    itsArea = theGrid.Area() ? theGrid.Area()->Clone() : 0;
    itsNX = theGrid.XNumber();
    itsNY = theGrid.YNumber();
    return *this;
  }
  bool operator==(const MyGrid &theGrid) const
  {
    if (itsNX == theGrid.itsNX)
      if (itsNY == theGrid.itsNY)
      {
        if (itsArea == 0 && theGrid.itsArea == 0)
          return true;
        else if (itsArea && theGrid.itsArea)
        {
          if (itsArea->ClassId() == theGrid.itsArea->ClassId())
            if (*itsArea == *(theGrid.itsArea))
              return true;
        }
      }
    return false;
  }

  bool operator<(const MyGrid &theGrid) const;

  bool operator==(const NFmiGrid &theGrid) const
  {
    MyGrid tmpGrid(theGrid);
    return (*this == tmpGrid);
  }

  // private:
  NFmiArea *itsArea;
  unsigned long itsNX;
  unsigned long itsNY;
};

struct CombinedParamStruct
{  // tämä structi piti tehdä, että sain vähennettyä boost::in Thread:ille annettujen parametrin
  // määrää (max 9 parametria)
  CombinedParamStruct() : weather1(false), weather2(false), wind1(false), wind2(false), wind3(false)
  {
  }

  bool weather1;  // tehdään weatherandcloudiness seuraavista: kFmiWeatherSymbol1,
                  // kFmiTotalCloudCover, kFmiCloudSymbol ja kFmiFogSymbol
  bool weather2;  // tehdään weatherandcloudiness seuraavista: kFmiWeatherSymbol3
  bool wind1;     // tehdään totalwind seuraavista: kFmiWindVectorMS
  bool wind2;     // tehdään totalwind seuraavista: kFmiWindDirection ja kFmiWindSpeedMS
  bool wind3;     // tehdään totalwind seuraavista: kFmiWindUMS ja kFmiWindVMS
};

static const NFmiRect gMissingCropRect(kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing);

// Tähän rakenteeseen saadaan kaikki tieto mistä tahansa hilasta, että
// voidaan tehdä yhdistelyjä.
// Käytetään apuna mm. kun grib:eistä tehdään qdataa ja kun yhdistellään
// monista qdatoista yksi uusi qdata.
struct GridRecordData
{
  GridRecordData();
  void ChangeParam(const NFmiParam &theWantedParam);

  NFmiMetTime itsOrigTime;
  NFmiMetTime itsValidTime;
  NFmiDataIdent itsParam;
  NFmiLevel itsLevel;
  MyGrid itsOrigGrid;  // tämä on datan originaali gridi
  MyGrid itsGrid;  // tämä on haluttu datan gridi, joka yleensa sama kuin orig, mutta jos on haluttu
                   // cropata dataa, tässä croppi hila
  bool fDoProjectionConversion;
  NFmiDataMatrix<float> itsGridData;
  NFmiRect itsLatlonCropRect;        // tähän talletetaan mahdollinen croppi recti
  NFmiPoint itsGridPointCropOffset;  // tähän talletetaan mahdollinen cropin offset originaali
                                     // gridin hila yksikössä
  double itsMissingValue;
  bool fParamChanged;
  NFmiParam itsOrigParam;
};

class NFmiStopFunctor
{
 public:
  NFmiStopFunctor() : fStop(false) {}
  ~NFmiStopFunctor() {}
  bool Stop() const { return fStop; }
  void Stop(bool newValue) { fStop = newValue; }

 private:
  bool fStop;
};

class NFmiOperationProgress
{
 public:
  virtual ~NFmiOperationProgress() {}
  virtual void StepIt() = 0;
  virtual void SetRange(int low, int high, int stepCount) = 0;
  virtual void AddRange(int value) = 0;
  virtual bool DoPostMessage(unsigned int message, unsigned int wParam = 0, long lParam = 0) = 0;
  virtual bool WaitUntilInitialized() = 0;
};

class NFmiThreadCallBacks
{
 public:
  NFmiThreadCallBacks(NFmiStopFunctor *theStopper = 0, NFmiOperationProgress *theProgress = 0)
      : itsStopper(theStopper), itsProgress(theProgress)
  {
  }

  bool Stop() const;
  void Stop(bool newValue);
  void StepIt();
  void SetRange(int low, int high, int stepCount);
  void AddRange(int value);
  bool DoPostMessage(unsigned int message, unsigned int wParam = 0, long lParam = 0);
  void CheckIfStopped();  // heittää poikkeuksen, jos lopetetaan
  bool WaitUntilInitialized();

 private:
  NFmiStopFunctor *itsStopper;
  NFmiOperationProgress *itsProgress;
};

class NFmiStopThreadException
{
 public:
  NFmiStopThreadException() {}
  ~NFmiStopThreadException() {}
};

// Tämä luokka laskee worker-threadi parvelle aina kulloisenkin 'vapaan' laskettavan timeIndeksin.
// Tältä siis pyydetään seuraav työstettävä aika-indeksi mm.
// FillSingleTimeGridDataInThread-funktiossa.
class NFmiTimeIndexCalculator
{
 public:
  typedef boost::shared_mutex MutexType;
  typedef boost::shared_lock<MutexType>
      ReadLock;  // Read-lockia ei oikeasti tarvita, mutta laitan sen tähän, jos joskus tarvitaankin
  typedef boost::unique_lock<MutexType> WriteLock;

  NFmiTimeIndexCalculator(unsigned long theTimeSize);
  NFmiTimeIndexCalculator(unsigned long theStartTimeIndex, unsigned long theEndTimeIndex);
  bool GetCurrentTimeIndex(unsigned long &theTimeIndexOut);

 private:
  unsigned long itsStartTimeIndex;
  unsigned long itsEndTimeIndex;
  unsigned long itsCurrentTimeIndex;
  MutexType itsMutex;
  bool fNoMoreWork;

  NFmiTimeIndexCalculator(const NFmiTimeIndexCalculator &);  // ei toteuteta kopio konstruktoria
  NFmiTimeIndexCalculator &operator=(
      NFmiTimeIndexCalculator const &);  // ei toteuteta sijoitus operaattoria
};

// Tämä luokka laskee worker-threadi parvelle aina sopivan locationIndex-välin laskettavaksi.
// Idea on siis se että hila laskut jaetaan sopivan mittäisiin pätkiin esim. 50 pituisiin
// ja kukin worker threadi aina vuorollaan kysisi, minkä pätkän nyt voisi tehdä.
class NFmiLocationIndexRangeCalculator
{
 public:
  typedef boost::shared_mutex MutexType;
  typedef boost::shared_lock<MutexType>
      ReadLock;  // Read-lockia ei oikeasti tarvita, mutta laitan sen tähän, jos joskus tarvitaankin
  typedef boost::unique_lock<MutexType> WriteLock;

  NFmiLocationIndexRangeCalculator(unsigned long theLocationSize, unsigned long theChunkSize);
  NFmiLocationIndexRangeCalculator(unsigned long theStartLocationIndex,
                                   unsigned long theEndLocationIndex,
                                   unsigned long theChunkSize);

  bool GetCurrentLocationRange(unsigned long &theStartIndexOut, unsigned long &theEndIndexOut);

 private:
  unsigned long itsStartLocationIndex;
  unsigned long itsEndLocationIndex;
  unsigned long itsCurrentLocationIndex;
  unsigned long itsChunkSize;
  MutexType itsMutex;
  bool fNoMoreWork;

  // ei toteuteta kopio konstruktoria
  NFmiLocationIndexRangeCalculator(const NFmiLocationIndexRangeCalculator &);
  // ei toteuteta sijoitus operaattoria
  NFmiLocationIndexRangeCalculator &operator=(NFmiLocationIndexRangeCalculator const &);
};

//! Undocumented
class NFmiQueryDataUtil
{
 public:
  typedef std::vector<GridRecordData *> GridDataVector;
  using SoundingLevelContainer = std::vector<unsigned long>;
  using SignificantSoundingLevels = std::unique_ptr<SoundingLevelContainer>;
  static SignificantSoundingLevels GetSignificantSoundingLevelIndices(NFmiFastQueryInfo &theInfo);

  static void DestroyGridRecordData(GridDataVector &theGridRecordDatas);  // apufunktio, joka tuhoaa
                                                                          // dynaamisen datan ja
                                                                          // tyhjentää vektorin

  /*!
   *  \class LimitChecker
   *  Tarkistaa ettei jokin laskettu arvo mene yli alle säädettyjen rajojen.
   *  Tarpeen mm. Lagrange interpoloinnissa, jossa voi syntyä uusia minimi ja maksimi
   *  arvoja. Tällöin esim. tuulen suunnaksi voi tulla negatiivinen arvo tai yli 360 astetta.
   *  Halutessa myös pyöräyttää ympäri eli jos arvoksi olisi tuulen suunnalle tulossa
   *  370, osaa laittaa arvoksi 10 eikä leikkaa 360:een. Jos taas kyseessä vaikka
   *  kosteusprosentti, pitää yli sadan mene arvo leikata 100:n.
   */
  class LimitChecker
  {
   public:
    LimitChecker(float theLowerLimit, float theUpperLimit, bool theCircularValue = false);
    float GetInsideLimitsValue(float theValue) const;

    inline float LowerLimit() const { return itsLowerLimit; }
    inline void LowerLimit(float newValue)
    {
      itsLowerLimit = newValue;
      Update();
    }
    inline float UpperLimit() const { return itsUpperLimit; }
    inline void UpperLimit(float newValue)
    {
      itsUpperLimit = newValue;
      Update();
    }
    inline bool CircularValue() const { return fCircularValue; }
    inline void CircularValue(bool newValue)
    {
      fCircularValue = newValue;
      Update();
    }

   private:
    void Update();

    float itsLowerLimit;
    float itsUpperLimit;
    bool fCircularValue;       // esim tuulen suunta menee ympäri rajojen
    float itsLimitDifference;  // ympyrä laskuja varten
  };                           // class LimitChecker

 public:
  static void CheckIfStopped(NFmiStopFunctor *theStopFunctor);
  static void CheckIfStopped(NFmiThreadCallBacks *theThreadCallBacks);
  static void DoStepIt(NFmiThreadCallBacks *theThreadCallBacks);
  static void SetRange(NFmiThreadCallBacks *theThreadCallBacks, int low, int high, int stepCount);
  static void AddRange(NFmiThreadCallBacks *theThreadCallBacks, int value);

  template <typename T>
  static bool IsEqualEnough(T value1, T value2, T usedEpsilon)
  {
    if (::fabs(static_cast<double>(value1 - value2)) < usedEpsilon)
      return true;
    return false;
  }

  static NFmiQueryData *ReadNewestData(const std::string &theFileFilter);
  static bool AreAreasEqual(const NFmiArea *theArea1, const NFmiArea *theArea2);
  static bool AreGridsEqual(const NFmiGrid *theGrid1, const NFmiGrid *theGrid2);
  static bool AreAreasSameKind(const NFmiArea *theArea1, const NFmiArea *theArea2);

  static NFmiQueryData *GridQD2NewGridQD(NFmiQueryData *theSourceData,
                                         NFmiGrid *theWantedGridFormat);

  static NFmiQueryData *ExtractParams(NFmiQueryData *theSourceData,
                                      const NFmiParamBag &theWantedParams);

  static NFmiQueryData *ExtractLocations(NFmiQueryData *theSourceData,
                                         const NFmiLocationBag &theWantedLocations,
                                         FmiInterpolationMethod theInterpolationMethod);

  static NFmiQueryData *CombineParams(NFmiQueryData *theSourceData1, NFmiQueryData *theSourceData2);

  static NFmiQueryData *CombineLocations(NFmiQueryData *theSourceData1,
                                         NFmiQueryData *theSourceData2,
                                         int thePriorisedDataNumber);

  static NFmiQueryData *Interpolate2OtherGrid(NFmiQueryData *theSourceData,
                                              const NFmiGrid *theWantedGrid,
                                              NFmiLogger *theLogger = 0);

  static NFmiQueryData *Interpolate2OtherGrid(NFmiQueryData *theSourceData,
                                              const NFmiGrid *theWantedGrid,
                                              NFmiLogger *theLogger,
                                              int theMaxThreadCount);

  static void FillGridData(NFmiQueryData *theSource,
                           NFmiQueryData *theTarget,
                           unsigned long theStartTimeIndex = gMissingIndex,
                           unsigned long theEndTimeIndex = gMissingIndex,
                           NFmiLogger *theLogger = 0,
                           bool fUseOnlyOneThread = true);

  static void FillGridDataFullMT(NFmiQueryData *theSource,
                                 NFmiQueryData *theTarget,
                                 unsigned long theStartTimeIndex = gMissingIndex,
                                 unsigned long theEndTimeIndex = gMissingIndex,
                                 unsigned int usedThreadCount = 0,
                                 NFmiLogger *theDebugLogger = 0);

  static NFmiQueryData *ExtractTimes(const NFmiQueryData *theSourceData,
                                     const NFmiTimeDescriptor &theWantedTimeDesc);

  static NFmiQueryData *InterpolateTimes(NFmiQueryData *theSourceData,
                                         int theTimeResolutionInMinutes,
                                         int theStartTimeResolutionInMinutes,
                                         NFmiTimeBag *thePossibleReferenceTimeBag = 0,
                                         int theMaxTimeSearchRangeInMinutes = 360,
                                         FmiInterpolationMethod theInterpolationMethod = kLinearly);

  static NFmiQueryData *Area1QDOverArea2QD(NFmiQueryData *areaData1, NFmiQueryData *areaData2);

  static NFmiTimeDescriptor NewTimeDescriptor(NFmiQueryData *theSourceData,
                                              int theTimeResolutionInMinutes);

  //! Aikaresoluution pitää olla sama
  static bool OverWriteTimesFromFile(NFmiFastQueryInfo &theLongerInfo,
                                     NFmiFastQueryInfo &theShorterInfo);

  static NFmiQueryData *SpaceOutData(NFmiFastQueryInfo &theSourceInfo,
                                     const NFmiPoint &newGridSize,
                                     NFmiDataModifier *theModifier,
                                     int theModifierId,
                                     double theAreaFactor,
                                     double theRange,
                                     const NFmiRect &theDataGatheringRange,
                                     const NFmiCalculationCondition &theMaskCondition);

  static NFmiQueryData *CombineTimes(NFmiFastQueryInfo &theSourceInfo1,
                                     NFmiFastQueryInfo &theSourceInfo2,
                                     int theMaxTimeBagSizeInNewData,
                                     int theStartTimeFunction,
                                     int theEndTimeFunction,
                                     int theOriginTimeFunction,
                                     bool fMakeChangingTimeResolution);

  static NFmiQueryData *CombineTimes(std::vector<std::string> &theFileNames,
                                     int theMaxTimesInNewData);

  static NFmiQueryData *MakeCombineParams(NFmiFastQueryInfo &theSourceInfo,
                                          double theWantedInfoVersion,
                                          bool fKeepCloudSymbolParameter,
                                          bool fDoTotalWind,
                                          bool fDoWeatherAndCloudiness,
                                          FmiParameterName theWindGustParId,
                                          bool fAllowLessParamsWhenCreatingWeather,
                                          int theMaxUsedThreadCount,
                                          bool fDoaccuratePrecip,
                                          bool fForceTimeBag = false);

  static NFmiQueryData *MakeCombineParams(NFmiFastQueryInfo &theSourceInfo,
                                          double theWantedInfoVersion,
                                          bool fKeepCloudSymbolParameter,
                                          bool fDoTotalWind,
                                          bool fDoWeatherAndCloudiness,
                                          FmiParameterName theWindGustParId,
                                          const std::vector<int> &thePrecipFormParIds,
                                          bool fAllowLessParamsWhenCreatingWeather,
                                          int theMaxUsedThreadCount,
                                          bool fDoaccuratePrecip,
                                          bool fForceTimeBag = false);

  static NFmiQueryData *MakeCombineParams(NFmiFastQueryInfo &theSourceInfo,
                                          double theWantedInfoVersion,
                                          bool fKeepCloudSymbolParameter,
                                          bool fDoTotalWind,
                                          bool fDoWeatherAndCloudiness,
                                          FmiParameterName theWindGustParId,
                                          const std::vector<int> &thePrecipFormParIds,
                                          const std::vector<int> &theFogParIds,
                                          const std::vector<int> &thePotParIds,
                                          bool fAllowLessParamsWhenCreatingWeather,
                                          int theMaxUsedThreadCount,
                                          bool fDoaccuratePrecip,
                                          bool fForceTimeBag = false);

  static NFmiQueryData *CreateEmptyData(NFmiQueryInfo &srcInfo);
  static NFmiQueryData *CreateEmptyData(NFmiQueryInfo &srcInfo,
                                        const std::string &theFilename,
                                        bool fInitialize);

  //@{ \name datan konversiot 6:sta 7:aan tai päinvastoin -funktiot
  static NFmiQueryData *FqdV6ToV7(NFmiFastQueryInfo &theSourceInfo);
  static NFmiQueryData *FqdV7ToV6(NFmiFastQueryInfo &theSourceInfo);
  //@}

  static NFmiQueryData *DoTimeFiltering(NFmiQueryData *theSourceData,
                                        int theFilterId,
                                        int theTimeOffsetStartInMinutes,
                                        int theTimeOffsetEndInMinutes,
                                        double theAdditionalParam1 = 1,
                                        double theAdditionalParam2 = 1);

  static NFmiQueryData *DoAreaFiltering(NFmiQueryData *theSourceData,
                                        int theFilterId,
                                        int theAreaType,
                                        double theAdditionalParam1 = 1,
                                        double theAdditionalParam2 = 1);

  // ********************************************************************************
  // ***************  NowcastFilter osuus on nyt täällä!  ***************************
  // ********************************************************************************
  static bool DoNowCastFiltering(const NFmiString &theData1FileName,
                                 const NFmiString &theData2FileName,
                                 const NFmiString &theOutputFileName,
                                 int theTimeRangeForPureData);

  static NFmiQueryData *DoNowCastFiltering(NFmiFastQueryInfo &theInfo1,
                                           NFmiFastQueryInfo &theInfo2,
                                           int theTimeRangeForPureData);

  // ********************************************************************************
  // ***************  NowcastFilter osuus on nyt täällä!  ***************************
  // ********************************************************************************

  static const NFmiString &GetOfficialQueryDataProdIdsKey();

  static NFmiQueryData *QDCrop(
      NFmiFastQueryInfo &theInfo, int theLeft, int theTop, int theRight, int theBottom);

  static std::string GetFileFilterDirectory(const std::string &theFileFilter);
  using LoggingFunction = std::function<void(const std::string &)>;
  static NFmiQueryData *CombineQueryDatas(
      bool fDoRebuild,
      std::shared_ptr<NFmiQueryData> &theBaseQData,
      std::vector<std::shared_ptr<NFmiQueryData>> &theQDataVector,
      bool fDoTimeStepCombine,
      int theMaxTimeStepsInData = 0,
      NFmiStopFunctor *theStopFunctor = nullptr,
      LoggingFunction *loggingFunction = nullptr,
      const std::string *theFileFilterPtr = nullptr);
  static NFmiQueryData *CombineQueryDatas(bool fDoRebuildCheck,
                                          const std::string &theBaseDataFileFilter,
                                          const std::string &theFileFilter,
                                          bool fDoTimeStepCombine,
                                          int theMaxTimeStepsInData = 0,
                                          NFmiStopFunctor *theStopFunctor = nullptr,
                                          LoggingFunction *loggingFunction = nullptr);
  static int CalcOptimalThreadCount(int maxAvailableThreads, int separateTaskCount);
  static unsigned int GetReasonableWorkingThreadCount(double wantedHardwareThreadPercent = 50.,
                                                      unsigned int separateTaskCount = 0);
  static std::vector<std::string> GetFileNamesForCombinationWork(const std::string &theFileFilter);
  static std::shared_ptr<NFmiQueryData> GetNewestQueryData(const std::string &theFileFilter);
  static std::vector<std::shared_ptr<NFmiQueryData>> ReadQueryDataFilesForCombinationWork(
      std::shared_ptr<NFmiQueryData> theBaseQData,
      const std::string &theDirName,
      std::vector<std::string> &theFilesIn,
      NFmiStopFunctor *theStopFunctor,
      bool fDoRebuildCheck);
  static NFmiQueryData *CombineAcceptedTimeStepQueryData(
      bool fDoRebuild,
      std::shared_ptr<NFmiQueryData> &theBaseQData,
      std::vector<std::shared_ptr<NFmiQueryData>> &theQDataVector,
      const std::vector<NFmiMetTime> &theAcceptedTimes,
      NFmiStopFunctor *theStopFunctor = 0);
  static std::vector<NFmiMetTime> MakeValidTimesList(
      std::vector<std::shared_ptr<NFmiFastQueryInfo>> &theFastInfoVector,
      int theMaxTimeStepsInData);
  static std::vector<std::shared_ptr<NFmiFastQueryInfo>> MakeTotalFastInfoVector(
      std::vector<std::shared_ptr<NFmiQueryData>> &theQDataVector,
      std::shared_ptr<NFmiQueryData> &theBaseQData,
      bool fDoRebuild);

};  // class NFmiQueryDataUtil

// ======================================================================
