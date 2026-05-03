// ======================================================================
/*!
 * \file NFmiQueryInfo.h
 * \brief Interface of class NFmiQueryInfo
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiGrid.h"
#include "NFmiHPlaceDescriptor.h"
#include "NFmiInfoData.h"
#include "NFmiLocationBag.h"
#include "NFmiParamDescriptor.h"
#include "NFmiRawData.h"
#include "NFmiSaveBaseFactory.h"
#include "NFmiTimeBag.h"
#include "NFmiTimeDescriptor.h"
#include "NFmiVPlaceDescriptor.h"
#include "NFmiVersion.h"
#include <memory>

class NFmiQueryData;
class NFmiCombinedParam;
class NFmiStringList;
class NFmiDataModifier;
class NFmiDataModifierExtreme;
class NFmiDataModifierExtremePlace;
class NFmiBitMask;
class NFmiStation;

//! Undocumented, should be a const int
#define HEADERMAX 3  // Poistetaan

//! Undocumented
class NFmiQueryInfo
{
 public:
  virtual ~NFmiQueryInfo();

  NFmiQueryInfo(double theInfoVersion = DefaultFmiInfoVersion);

  NFmiQueryInfo(const NFmiQueryInfo &theInfo);

  NFmiQueryInfo(const NFmiParamDescriptor &theParamDescriptor,
                const NFmiTimeDescriptor &theTimeDescriptor,
                const NFmiHPlaceDescriptor &theHPlaceDescriptor = NFmiHPlaceDescriptor(),
                const NFmiVPlaceDescriptor &theVPlaceDescriptor = NFmiVPlaceDescriptor(),
                double theInfoVersion = DefaultFmiInfoVersion);

  NFmiQueryInfo(NFmiQueryData *data,
                NFmiParamDescriptor *theParamDescriptor = 0,
                NFmiTimeDescriptor *theTimeDescriptor = 0,
                NFmiHPlaceDescriptor *theHPlaceDescriptor = 0,
                NFmiVPlaceDescriptor *theVPlaceDescriptor = 0);

  NFmiQueryInfo(const std::string &filename);

  virtual void Reset();
  virtual void ResetParam();
  virtual void ResetLocation();
  virtual void ResetLevel();
  virtual void ResetTime();
  void ResetHeader();

  virtual bool First();
  virtual bool FirstParam(bool fIgnoreSubParam = true);
  virtual bool FirstLocation();
  virtual bool FirstLevel();
  virtual bool FirstTime();
  bool FirstHeader();

  virtual bool NextParam(bool fIgnoreSubParam = true);
  virtual bool NextLocation();
  virtual bool NextLevel();
  virtual bool NextTime();
  bool NextHeader();

  virtual bool PreviousParam(bool fIgnoreSubParam = true);
  virtual bool PreviousTime();
  virtual bool PreviousLocation();
  virtual bool PreviousLevel();

  virtual bool NextActiveParam();
  bool NextActiveLocation();
  bool NextActiveLevel();
  bool NextActiveTime();

  bool PreviousActiveLevel();
  bool PreviousActiveTime();

  virtual bool IsActiveParam();
  virtual bool ActivateParam(bool newState);
  bool ActivateLocation(bool newState);
  bool ActivateLevel(bool newState);
  bool ActivateTime(bool newState);
  bool ActivateTimePeriod(bool newState, const NFmiTimeBag &period);
  const NFmiTimeBag GetActiveTimePeriod();

  bool IsLocation() const;
  bool IsLevel() const;
  bool IsArea() const;
  bool IsGrid() const;
  bool IsValidTime() const;
  bool IsOriginTime() const;
  bool IsHeader() const;

  virtual bool Param(const NFmiDataIdent &theDataIdent);
  virtual bool Param(const NFmiParam &theParam);
  virtual bool Param(const NFmiParam &theParam, const NFmiParam &theSubParam);
  virtual bool Param(FmiParameterName theParam);
  virtual bool Location(const NFmiLocation &theLocation);
  virtual bool Location(const unsigned long &theIdent);
  virtual bool Location(const NFmiString &theLocationName);
  virtual bool Location(const NFmiPoint &theLonLatPoint, NFmiPoint *theGridPoint = 0);
  virtual bool NearestLocation(const NFmiLocation &theLocation,
                               double theMaxDistance = kFloatMissing * 1000.);
  virtual bool NearestLocation(const NFmiLocation &theLocation,
                               const NFmiArea *theArea,
                               double theMaxDistance = kFloatMissing * 1000.);
  virtual bool NearestPoint(const NFmiPoint &theLatLonPoint);

  //! Hakee listan paikkaindeksi/etäisyys metreinä pareja. Listaan haetaan annettua paikkaa lähimmat
  //! datapisteet.
  std::vector<std::pair<int, double> > NearestLocations(
      const NFmiLocation &theLocation,
      int theMaxWantedLocations,
      double theMaxDistance = kFloatMissing) const;

  std::vector<std::pair<int, double> > NearestLocations(
      const NFmiPoint &theLatLonPoint,
      int theMaxWantedLocations,
      double theMaxDistance = kFloatMissing) const;

  virtual bool Level(const NFmiLevel &theLevelValue);
  virtual bool Time(const NFmiMetTime &theTime);
  virtual bool TimeToNearestStep(const NFmiMetTime &theTime,
                                 FmiDirection = kForward,
                                 long theTimeRangeInMinutes = kLongMissing);

  const NFmiArea *Area() const;
  const NFmiGrid *Grid() const;

  virtual const NFmiLocation *Location() const;
  virtual NFmiPoint LatLon() const;
  virtual const NFmiPoint RelativePoint()
      const;  // Palauttaa currentin paikan suhteellisen sijainnin (esim. gridin areassa)
  virtual const NFmiLevel *Level() const;
  virtual NFmiProducer *Producer();

  // vaihtaa tuottajan kaikkiin parametreihin niin infossa kuin querydatassa
  void SetProducer(const NFmiProducer &newProducer);

  virtual NFmiDataIdent &Param() const;
  virtual const NFmiMetTime &Time() const;
  virtual const NFmiMetTime &ValidTime() const;
  virtual const NFmiMetTime &OriginTime() const;
  virtual void OriginTime(const NFmiMetTime &newTime);

  // HUOM! nämä on toistaiseksi tehty toimimaan vain NFmiFastQueryInfo:on (Marko)
  virtual bool IsParamUsable() const;
  virtual bool IsLocationUsable() const;
  virtual bool IsLevelUsable() const;
  virtual bool IsTimeUsable() const;
  // HUOM! nämä on toistaiseksi tehty toimimaan vain NFmiFastQueryInfo:on (Marko)

  const NFmiTimeBag &ValidTimes() const;
  const NFmiParamBag &ParamBag() const;

  const NFmiHPlaceDescriptor &HPlaceDescriptor() const;
  const NFmiVPlaceDescriptor &VPlaceDescriptor() const;
  const NFmiTimeDescriptor &TimeDescriptor() const;
  const NFmiParamDescriptor &ParamDescriptor() const;

  // ****** HUOM!!!! *******************************************
  // Descriptorien asetus funktiot ovat spesiaali funktioita
  // Niillä on tarkoitus muuttaa iteraattori-infon aikoja/
  // parametreja jne. hetkellisesti ja vain siten, että datan
  // koko ei kasva. Jos alkuperäisessä paramDescriptorissa
  // oli esim. 2 parametria ja yrittää asettaa paramDescriptoria
  // missä on kolme parametriä., lentää POIKKEUS!
  // Itse (Marko) käytän tätä systeemiä Metkun editorissa
  // 'huijaamaan' qinfoa, että voin laskea makroparametreja
  // karttanäytölle (smarttool-skriptejä, joita visualisoidaan).
  void SetHPlaceDescriptor(const NFmiHPlaceDescriptor &newDesc);
  void SetVPlaceDescriptor(const NFmiVPlaceDescriptor &newDesc);
  void SetTimeDescriptor(const NFmiTimeDescriptor &newDesc);
  void SetParamDescriptor(const NFmiParamDescriptor &newDesc);
  // ****** HUOM!!!! *******************************************

  // ****** Cached interpolation methods ***********************

  virtual float CachedInterpolation(const NFmiLocationCache &theLocationCache,
                                    const NFmiTimeCache &theTimeCache);
  float CachedInterpolation(const NFmiLocationCache &theLocationCache);
  float CachedInterpolation(const NFmiTimeCache &theTimeCache);
  void GetCachedValues(const NFmiLocationCache &theLocationCache,
                       const NFmiTimeCache &theTimeCache,
                       std::array<float, 4> &theValues1,
                       std::array<float, 4> &theValues2);
  void GetCachedValues(const NFmiLocationCache &theLocationCache, std::array<float, 4> &theValues);
  void GetCachedValues(const NFmiTimeCache &theTimeCache, std::array<float, 2> &theValues);
  float CachedTimeInterpolatedValue(float theValue1,
                                    float theValue2,
                                    const NFmiTimeCache &theTimeCache,
                                    FmiInterpolationMethod theInterpolatioMethod,
                                    FmiParameterName theParId);
  float CachedLocationInterpolatedValue(std::array<float, 4> &theValues,
                                        const NFmiLocationCache &theLocationCache,
                                        FmiInterpolationMethod theInterpolatioMethod,
                                        FmiParameterName theParId);
  float CachedPressureLevelValue(float P, const NFmiLocationCache &theLocationCache);
  float CachedPressureLevelValue(float P,
                                 const NFmiLocationCache &theLocationCache,
                                 const NFmiTimeCache &theTimeCache);
  float CachedPressureLevelValue(float P, const NFmiTimeCache &theTimeCache);
  void GetCachedPressureLevelValues(float P,
                                    const NFmiLocationCache &theLocationCache,
                                    std::array<float, 4> &theValues);
  void GetCachedPressureLevelValues(float P,
                                    const NFmiTimeCache &theTimeCache,
                                    std::array<float, 2> &theValues);
  void GetCachedPressureLevelValues(float P,
                                    const NFmiLocationCache &theLocationCache,
                                    const NFmiTimeCache &theTimeCache,
                                    std::array<float, 4> &theValues1,
                                    std::array<float, 4> &theValues2);
  // ****** Cached interpolation methods ***********************

  void SetLocalTimes(const float theLongitude);  // Muuttaa ajan iteroinnin paikalliseksi
  bool IsLocalTime() const;
  unsigned long ForecastPeriod();

  // Header method
  const NFmiString Owner();
  const NFmiString TimeStamp();
  const NFmiString Leader();

  void Header(NFmiStringList *theHeader);
  NFmiStringList *Header() const;

  bool ResetText();
  bool NextText();
  const NFmiString Text() const;

  // PostProc method
  void PostProcText(NFmiStringList *thePostProc);
  NFmiStringList *PostProcText() const;

  bool ResetPostProc();
  bool NextPostProc();
  const NFmiString PostProc() const;

  size_t Size() const;

  // Descriptor Sizes
  virtual unsigned long SizeParams() const;
  virtual unsigned long SizeLocations() const;
  virtual unsigned long SizeLevels() const;
  virtual unsigned long SizeTimes() const;

  // Desctiptor Active Sizes
  virtual unsigned long SizeActiveParams() const;
  virtual unsigned long SizeActiveLocations() const;
  virtual unsigned long SizeActiveLevels() const;
  virtual unsigned long SizeActiveTimes() const;

  //  Edit, hakee vielä muotoaan
  NFmiDataIdent &EditParam();
  const NFmiLocation &EditLocation();
  NFmiStation &EditStation();
  NFmiLevel &EditLevel();

  // QueryData Read
  virtual float FloatValue() const;
  virtual float InterpolatedValue(const NFmiPoint &theLatLonPoint);

  // Mikael lisäsi seuraavan metodin 4.10.1999, testikäytössä
  virtual float InterpolatedValue(const NFmiMetTime &theTime, int theMaxMinuteRange = 0);
  virtual float InterpolatedValue(const NFmiPoint &theLatLonPoint,
                                  const NFmiMetTime &theTime,
                                  int theMaxMinuteRange = 0);

  virtual float PeekLocationValue(int theXOffset, int theYOffset) const;
  // en toteuta tätä ainakaan vielä qinfoon, vaan vain fastinfoon
  virtual float PeekLocationValue(int theXOffset, int theYOffset, const NFmiMetTime &theTime);
  virtual const NFmiPoint PeekLocationLatLon(int theXOffset, int theYOffset);
  virtual float PeekTimeValue(int theTimeOffset);
  virtual float PeekValue(int theTimeOffset, int theXOffset, int theYOffset);
  virtual bool SetDescriptors(NFmiQueryInfo *theQueryInfo, bool fIgnoreLevel = true);
  //  void * VoidValue();
  virtual const NFmiGrid GridValue();
  virtual bool Grid2Info(NFmiGrid &theSource);

  // QueryData Write
  virtual bool FloatValue(float theFloatData);

  // Tähän tulee joukko funktioita, jotka palauttavat aktiivisen parametrin
  // arvon haluttuun korkeuteen [m]. Metodeja on nejä, jotka tekevät erilaisia
  // interpolointeja parametreista riippuen.
  // HUOM! Nämä NFmiQueryInfo-metodit eivät palauta kuin puuttuvaa, mutta lapsi-
  // luokka NFmiFastQueryInfo:ssa metodit toimivat oikein. En aio toteuttaa metodeja tässä luokassa.
  virtual float HeightValue(float /* theHeight */) { return kFloatMissing; };
  virtual float HeightValue(float /* theHeight */, const NFmiPoint & /* theLatlon */)
  {
    return kFloatMissing;
  };
  virtual float HeightValue(float /* theHeight */,
                            const NFmiPoint & /* theLatlon */,
                            const NFmiMetTime & /* theTime */)
  {
    return kFloatMissing;
  };
  virtual float HeightValue(float /* theHeight */, const NFmiMetTime & /* theTime */)
  {
    return kFloatMissing;
  };

  // Tähän tulee joukko funktioita, jotka palauttavat aktiivisen parametrin
  // arvon haluttuun paine pintaan interpoloituna (log(p) interpolointi).
  // Metodeja on neljä, jotka tekevät erilaisia interpolointeja parametreista riippuen.
  // HUOM! Nämä NFmiQueryInfo-metodit eivät palauta kuin puuttuvaa, mutta lapsi-
  // luokka NFmiFastQueryInfo:ssa metodit toimivat oikein. En aio toteuttaa metodeja tässä luokassa.
  virtual float PressureLevelValue(float /* P */) { return kFloatMissing; };
  virtual float PressureLevelValue(float /* P */, const NFmiPoint & /* theLatlon */)
  {
    return kFloatMissing;
  };
  virtual float PressureLevelValue(float /* P */,
                                   const NFmiPoint & /* theLatlon */,
                                   const NFmiMetTime & /* theTime */)
  {
    return kFloatMissing;
  };
  virtual float PressureLevelValue(float /* P */, const NFmiMetTime & /* theTime */)
  {
    return kFloatMissing;
  };

  virtual bool FindNearestTime(const NFmiMetTime &theTime,
                               FmiDirection theDirection = kCenter,
                               unsigned long theTimeRangeInMinutes = kUnsignedLongMissing);

  // ClassIdent
  virtual unsigned long ClassId() const;
  unsigned long NewClassId();
  virtual const char *ClassName() const;

  virtual NFmiQueryInfo *Clone() const;
  virtual NFmiQueryInfo Combine(const NFmiQueryInfo &theCombine);
  virtual NFmiQueryInfo *CreateCombinedInfo(NFmiQueryInfo *theOtherInfo);
  virtual NFmiQueryData *CreateCombinedData(NFmiQueryInfo *theOtherInfo);

  // ************************************************************
  // Poistin CreateSubQueryData-metodin, KÄYTÄ QDPUTKET-FILTTEREITÄ!!!!!!!!
  // ************************************************************
  //  virtual NFmiQueryData * CreateSubQueryData(NFmiQueryInfo * theSubInfo);

  bool ReplaceData(NFmiQueryInfo *theReplaceData, NFmiTimeBag *TheTimeBag = 0);
  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  // operator
  NFmiQueryInfo &operator=(const NFmiQueryInfo &theInfo);

  const NFmiRawData *RefRawData() const;
  const NFmiQueryData *RefQueryData() const;

  bool UseSubParam();
  const NFmiCombinedParam *CombinedParam();
  float TimePeriodFloatValue(unsigned long period,
                             float startWeight = 1.,
                             float centreWeight = 1.,
                             float endWeight = 1.);

  float TimePeriodFloatValue(const NFmiMetTime &firstTime,
                             const NFmiMetTime &lastTime,
                             float startWeight = 1.,
                             float centreWeight = 1.,
                             float endWeight = 1.);

  float InterpolatedTimePeriodFloatValue(const NFmiPoint &theLonLat,
                                         const NFmiMetTime &theStartTime,
                                         const NFmiMetTime &theEndTime,
                                         float startWeight = 1.,
                                         float centreWeight = 1.,
                                         float endWeight = 1.);

  void CalcLevelData(NFmiDataModifier *theModifier);
  void CalcLocationData(NFmiDataModifier *theModifier,
                        NFmiArea *theArea = 0,
                        NFmiLocationBag *theLocationBag = 0);
  void CalcLocationDataWithExtremePlace(NFmiDataModifierExtremePlace *theModifier);
  void CalcTimeData(NFmiDataModifier *theModifier, NFmiTimeBag *theTimeBag = 0);
  void CalcTimeData(NFmiDataModifier *theModifier, NFmiTimeDescriptor &theTimeDesc);
  void CalcTimeData(NFmiDataModifier *theModifier,
                    const NFmiMetTime &startTime,
                    const NFmiMetTime &endTime);
  void CalcTimeDataWithExtremeTime(NFmiDataModifierExtreme *theModifier,
                                   const NFmiMetTime &startTime,
                                   const NFmiMetTime &endTime);
  void CalcInterpolatedTimeData(NFmiDataModifier *theModifier,
                                const NFmiMetTime &startTime,
                                const NFmiMetTime &endTime,
                                const NFmiPoint &lonlat);
  void CalcInterpolatedTimeData(NFmiDataModifier *theModifier,
                                NFmiTimeDescriptor &theTimeDesc,
                                const NFmiPoint &lonLat);
  void ModifyLocationData(NFmiDataModifier *theModifier,
                          NFmiQueryInfo *theQueryInfoCopy = 0,
                          NFmiArea *theArea = 0,
                          NFmiLocationBag *theLocationBag = 0);
  void ModifyLevelData(NFmiDataModifier *theModifier);
  void ModifyTimeData(NFmiDataModifier *theModifier, NFmiTimeBag *theTimeBag = 0);
  void ModifyTimesLocationData(NFmiDataModifier *theModifier,
                               NFmiQueryInfo *theQueryInfoCopy,
                               NFmiTimeBag *theTimeBag = 0);
  void ModifyTimesLocationData(NFmiDataModifier *theModifier,
                               NFmiQueryInfo *theQueryInfoCopy,
                               NFmiTimeDescriptor &theTimeDescriptor);
  void ModifyTimesLocationData_FullMT(NFmiDataModifier *theModifier,
                                      NFmiTimeDescriptor &theTimeDescriptor);

  virtual unsigned long TimeResolution();

  // Marko laittoi eri index jutut
  virtual unsigned long ParamIndex() const;
  virtual unsigned long LocationIndex() const;
  virtual unsigned long LevelIndex() const;
  virtual unsigned long TimeIndex() const;
  virtual bool ParamIndex(unsigned long theIndex);
  virtual bool LocationIndex(unsigned long theIndex);
  virtual bool LevelIndex(unsigned long theIndex);
  virtual bool TimeIndex(unsigned long theIndex);

  // Persa lisäsi joukon uusia metodeja 11022000

  long CalcTimeUnCertainty();
  long CalcAreaUnCertainty();
  long TimeUnCertaintyStart();
  void TimeUnCertaintyStart(long newValue);
  long TimeUnCertaintyEnd();
  void TimeUnCertaintyEnd(long newValue);
  long AreaUnCertaintyStart();
  void AreaUnCertaintyStart(long newValue);
  long AreaUnCertaintyEnd();
  void AreaUnCertaintyEnd(long newValue);

  // ***** 14.5.2001/MArko Uusia metodeja hilassa liikkumiseen ***************
  // *** HUOM!!! NÄMÄ EIVÄT TOIMI VIELÄ NFmiQueryInfo:ssa, vain NFmiFastQueryInfo:ssa!!!!
  // ****************
  // HUOM!! liikkumista ei voi sallia laatikon ulkopuolelle kuten esim. PeekValue-metodissa voi!!!
  // Move*** ovat liikkumista varten

  virtual bool MoveUp(int moveBy = 1);     // toimii vain gridi datalle oikein!!!
  virtual bool MoveDown(int moveBy = 1);   // toimii vain gridi datalle oikein!!!
  virtual bool MoveLeft(int moveBy = 1);   // toimii vain gridi datalle oikein!!!
  virtual bool MoveRight(int moveBy = 1);  // toimii vain gridi datalle oikein!!!

  // näillä asetetaan paikka suoraan johonkin laitaan (ei 'laatikon' ulkopuolelle!)
  virtual bool Top();     // toimii vain gridi datalle oikein!!!
  virtual bool Bottom();  // toimii vain gridi datalle oikein!!!
  virtual bool Left();    // toimii vain gridi datalle oikein!!!
  virtual bool Right();   // toimii vain gridi datalle oikein!!!
  // *** HUOM!!! NÄMÄ EIVÄT TOIMI VIELÄ NFmiQueryInfo:ssa, vain NFmiFastQueryInfo:ssa!!!!
  // ****************
  // ***** 14.5.2001/MArko Uusia metodeja hilassa liikkumiseen ***************

  // Avain systeemi sijoitetaan NFmiQueryData:n omistamaan NFmiQueryInfo:on.
  // Jos QueryInfo on iteraattori, välittää info käskyt osoittamaansa dataa, joka
  // taas välittää käskyt datan omistamaan infoon. Näin avaimet jäävät 'dataan' talteen.
  // Infoss on itsHeaderText, johon talletetaan avaimet. Avain on FMI_xxxx.
  // Avaimen arvo on jokin teksti esim. "marko" tai "555". Avain ja sen arvo
  // talletetaan itsHeaderText:iin yhdeksi NFmiStringiksi. Muoto on seuraava:
  // FMI_AVAIN=ARVO. Kun käyttäjä etsii tai asettaa avaimia, ei pidä käyttää
  // FMI_ -etuliitettä. Koodi lisää ne sinne itse. Eli jos asetat avaimeksi
  // FMI_AVAIN ja sen arvoksi ARVO, tulee avaimeksi FMI_FMI_AVAIN.

  bool FindFirstKey(const NFmiString &theKey);
  bool FindNextKey(const NFmiString &theKey);
  bool AddKey(const NFmiString &theKey,
              const NFmiString &theValue,
              bool fClearDuplicatesFirst = false);
  bool SetCurrentKeyValue(const NFmiString &newValue);
  const NFmiString GetCurrentKeyValue();
  bool RemoveCurrentKey();
  bool RemoveAllKeys();
  bool RemoveAllKeys(const NFmiString &theKey);
  const NFmiStringList GetAllKeys(bool fRemoveDuplicates = false);
  const NFmiString GetCurrentKey();
  bool DoEndianByteSwap();

  double InfoVersion() const;
  void InfoVersion(double newValue) const;

  // data tyyppien get- ja set-metodit liittyvät mm. maskeihin ja QInfon lapsi luokkiin.
  virtual NFmiInfoData::Type DataType() const { return NFmiInfoData::kNoDataType; }
  virtual void DataType(NFmiInfoData::Type /* newType */){};
  // Lapsi luokat tarvitsevat tälläiset maski-metodit, tässä toteutetaan vain dummyna.
  virtual void MaskType(unsigned long /* theMaskType */){};
  virtual unsigned long MaskType() { return 0; };

  float CalcTimeOffsetToLastTime(const NFmiMetTime &theTime,
                                 const NFmiMetTime &time1,
                                 const NFmiMetTime &time2);
  bool CalcLatlonCachePoints(NFmiQueryInfo &theTargetInfo,
                             NFmiDataMatrix<NFmiLocationCache> &theLocationCache);
  NFmiLocationCache CalcLocationCache(const NFmiPoint &theLatlon,
                                      unsigned long thePossibleSourceSizeX = gMissingIndex,
                                      unsigned long thePossibleSourceSizeY = gMissingIndex);
  bool CalcTimeCache(NFmiQueryInfo &theTargetInfo, std::vector<NFmiTimeCache> &theTimeCache);

  virtual NFmiTimeCache CalcTimeCache(const NFmiMetTime &theTime);

  bool HasNonFiniteValueSet() const { return fHasNonFiniteValueSet; }
  void HasNonFiniteValueSet(bool newValue) { fHasNonFiniteValueSet = newValue; }
  bool IsInside(const NFmiPoint &theLatLon, double theRadius) const;
  virtual bool IsInside(const NFmiMetTime &theTime) const;
  unsigned long GridXNumber() const { return itsGridXNumber; }
  unsigned long GridYNumber() const { return itsGridYNumber; }
  std::size_t GridHashValue() const;

 private:
  const NFmiString MakeKeyValueString(const NFmiString &theKey, const NFmiString &theValue);
  bool FindFirstAnyKey();
  bool FindNextAnyKey();
  bool IsCurrentAnyKey();
  bool IsCurrentKey(const NFmiString &theKey);
  bool ReplaceCurrentKeyValue(const NFmiString &newValue);
  const NFmiString ExtractCurrentKeyValue();
  const NFmiString ExtractCurrentKey();

 protected:
  float InterpolatedValueFromTimeBag(const NFmiMetTime &theTime, int theMaxMinuteRange);
  float InterpolatedValueFromTimeList(const NFmiMetTime &theTime, int theMaxMinuteRange);
  float FindNearestNonMissingValueFromTimeList(const NFmiMetTime &theTime,
                                               int theMaxMinuteRange,
                                               int &theTimeIndexInOut,
                                               NFmiMetTime &theFoundTime,
                                               FmiDirection theDirection);

  float InterpolatedValueForCombinedParam(const NFmiPoint &theGridPoint);
  float InterpolatedSubValueForTotalWind(const NFmiPoint &theGridPoint,
                                         int theXShift,
                                         int theYShift);
  float InterpolatedValueForWeatherAndCloudiness(const NFmiPoint &theGridPoint,
                                                 int theXShift,
                                                 int theYShift);
  float InterpolatedValueForWeatherAndCloudiness(const NFmiMetTime &theTime,
                                                 int theMaxMinuteRange,
                                                 const NFmiPoint &theGridPoint,
                                                 int theXShift,
                                                 int theYShift);
  float InterpolatedValueForTotalWind(const NFmiPoint &theGridPoint, int theXShift, int theYShift);
  float InterpolatedValueForTotalWind(const NFmiMetTime &theTime,
                                      int theMaxMinuteRange,
                                      const NFmiPoint &theGridPoint,
                                      int theXShift,
                                      int theYShift);

  float TimePeriodValue(const NFmiPoint &theLonLat,
                        unsigned long period,
                        float factor1 = 1.,
                        float factor2 = 1.,
                        float factor3 = 1.);
  float TimePeriodValue(const NFmiPoint &theLonLat,
                        const NFmiMetTime &startTime,
                        const NFmiMetTime &endTime,
                        float factor1 = 1.,
                        float factor2 = 1.,
                        float factor3 = 1.);
  void Destroy();
  virtual size_t Index() const;
  virtual size_t Index(unsigned long theParamIndex,
                       unsigned long theLocationIndex,
                       unsigned long theLevelIndex,
                       unsigned long theTimeIndex) const;
  float Interpolate(const NFmiDataIdent &theDataIdent,
                    const NFmiMetTime &theTime,
                    const NFmiMetTime &theTime1,
                    const NFmiMetTime &theTime2,
                    float theValue1,
                    float theValue2);

  // 1999.08.20/Marko
  virtual float SubParamFloatValue() const;
  virtual bool SubParamFloatValue(float theFloatData);

  // Muuntaa floatin subvalueksi
  float SubValueFromFloat(float fValue) const;

  virtual float IndexFloatValue(
      size_t theIndex) const;  // palauttaa suoraan arvon ilman aliparametri tarkasteluja
  virtual bool IndexFloatValue(
      size_t theIndex, float theValue);  // asettaa suoraan arvon ilman aliparametri tarkasteluja
  virtual float PeekValue(size_t theIndex) const;  // käytetään mm. PeekLocationValue():ssä
  virtual bool PokeValue(size_t theIndex, float theFloatData);  // 25.9.2001/Marko Mahd. nopea arvon
                                                                // asetus (jos indeksi on voitu
                                                                // laskea toisaalla)
  virtual float SubParamFloatValue(size_t theIndex) const;      // käytetään ed. metodissa
  bool FindSubParam(const NFmiDataIdent &theParam);
  bool FindSubParam(const NFmiParam &theParam);
  bool ChangeCombinedParamParser(const NFmiDataIdent &theParam);
  void CalcLocationData2(NFmiDataModifier *theModifier);
  void CalcLocationData2(NFmiDataModifier *theModifier, NFmiArea *theArea);
  void CalcLocationData2(NFmiDataModifier *theModifier, NFmiLocationBag *theLocationBag);
  void ModifyLocationData2(NFmiDataModifier *theModifier, NFmiQueryInfo *theQueryInfoCopy);
  void ModifyLocationData2(NFmiDataModifier *theModifier,
                           NFmiQueryInfo *theQueryInfoCopy,
                           NFmiArea *theArea);
  void ModifyLocationData2(NFmiDataModifier *theModifier,
                           NFmiQueryInfo *theQueryInfoCopy,
                           NFmiLocationBag *theLocationBag);

  virtual bool IsSubParamUsed() const;
  // HUOM! tämä on viritys funktio, joka toimii oikeasti vasta NFmiFastQueryInfo:ssa
  // Sielläkin tämän käyttö on vaarallista, ellei tiedä mitä tekee.
  virtual void SetIsSubParamUsed(bool /* newState */){};

  // QueryDataReference

  NFmiRawData *itsRefRawData;
  NFmiQueryData *itsRefQueryData;

 protected:
  /* // 2012.1.19 Marko Poistetaan käytöstä itsStaticDataMask -luokka, koska se on ilmeisesti turhaa
    (SuperSmartInfo käyttää tätä)
    bool IsLocationMasked(unsigned long theLocationIndex) const;
    NFmiBitMask * itsStaticDataMask;
  */
  // data
  std::unique_ptr<NFmiParamDescriptor> itsParamDescriptor;
  std::unique_ptr<NFmiHPlaceDescriptor> itsHPlaceDescriptor;
  std::unique_ptr<NFmiVPlaceDescriptor> itsVPlaceDescriptor;
  std::unique_ptr<NFmiTimeDescriptor> itsTimeDescriptor;

  std::unique_ptr<NFmiStringList> itsHeaderText;
  std::unique_ptr<NFmiStringList> itsPostProc;

  unsigned long itsNewClassIdent;

  // jos käytetään aliparametria, tämä purkaa datan haluttuun aliparametriin
  NFmiCombinedParam *itsCombinedParamParser;

  // tähän talletetaan tiedostosta luettu info version luku
  // (käytetään v. 6:n ja 7:n välillä, mm. weatherandcloudiness ja totalwind
  // tarvitsevat niitä)
  // qdatan sisäinen info pitää versio numeronsa ja iteraattori info pyytää
  // sitä qdatalta, joka pyytää sitä sisäiseltä infolta.
  // talletettiin varmuuden vuoksi double:na, jos haluaa käyttää desimaaleja joskus

  mutable double itsInfoVersion;

  unsigned long itsGridXNumber;  // mahdollisen gridi datan x-dimensio
  unsigned long itsGridYNumber;  // mahdollisen gridi datan y-dimensio
  bool fIsStrechableGlobalGrid;  // Jos kyse globaali datasta, jossa viimeinen sarake ei mene
                                 // maailman reunaan, koska sen arvot ovat samoja kuin 1.
                                 // sarakkeessa, tällöin tämä on true.
  bool DoGlobalWrapFix(const NFmiPoint &theGridPoint) const;

  // friends
  friend class NFmiDB;
  friend class NFmiPointDB;
  friend class NFmiBufrDB;
  friend class NFmiTimeBufrDB;
  friend class NFmiTempDB;
  friend class NFmiModelDB;
  friend class NFmiPreviDB;
  friend class NFmiTimeSeriesModelDB;
  friend class NFmiRadarAreaDB;
  friend class NFmiQueryData;
  friend class NFmiDBServer;

 private:
  //  bool fUseStaticDataMask; // 2012.1.19 Marko Poistetaan käytöstä itsStaticDataMask -luokka,
  //  koska se on ilmeisesti turhaa (SuperSmartInfo käyttää tätä)
  long itsTimeUnCertaintyStart;
  long itsTimeUnCertaintyEnd;
  long itsAreaUnCertaintyStart;
  long itsAreaUnCertaintyEnd;

  // Kun luetaan infoa qdata-tiedostosta, voidaan päätellä, pitääkö binääri datalle
  // tehdä byte swap. Tieto talletetaan tähän ja sitä käytetään kun datapool lukee
  // binääri datan. querydta-luokka välittää tiedon qinfolta datapoolille read-metodissa.

  bool fDoEndianByteSwap;

  // NFmiQueryInfo::FloatValue(value) -metodi suojataan +-inf ja nan arvoilta, että sellaisia ei voi
  // enää asettaa queryDataan.
  // Jos sellainen arvo annetaan, asetetaan tämä lippu pääle, eikä arvoa aseteta.
  bool fHasNonFiniteValueSet;

};  // class NFmiQueryInfo

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiQueryInfo::ResetParam()
{
  itsParamDescriptor->Reset();
}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiQueryInfo::ResetLocation()
{
  itsHPlaceDescriptor->Reset();
}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiQueryInfo::ResetLevel()
{
  if (itsVPlaceDescriptor)
    itsVPlaceDescriptor->Reset();
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiQueryInfo::ResetTime()
{
  itsTimeDescriptor->Reset();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsLocation() const
{
  return (itsHPlaceDescriptor->IsLocation());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsLevel() const
{
  return (itsVPlaceDescriptor ? itsVPlaceDescriptor->IsLevel() : false);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsArea() const
{
  return (itsHPlaceDescriptor->IsArea());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsGrid() const
{
  return (itsHPlaceDescriptor->IsGrid());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsValidTime() const
{
  return (itsTimeDescriptor->IsValidTime());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsOriginTime() const
{
  return (itsTimeDescriptor->IsOriginTime());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiArea *NFmiQueryInfo::Area() const
{
  return (itsHPlaceDescriptor->Area());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiGrid *NFmiQueryInfo::Grid() const
{
  return (itsHPlaceDescriptor->Grid());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::NextLocation()
{
  return (itsHPlaceDescriptor->Next());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::PreviousLocation()
{
  return (itsHPlaceDescriptor->Previous());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::NextLevel()
{
  return (itsVPlaceDescriptor ? itsVPlaceDescriptor->Next() : false);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::PreviousLevel()
{
  return (itsVPlaceDescriptor ? itsVPlaceDescriptor->Previous() : false);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::NextTime()
{
  return (itsTimeDescriptor->Next());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::PreviousTime()
{
  return (itsTimeDescriptor->Previous());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::PreviousActiveTime()
{
  return (itsTimeDescriptor->PreviousActive());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::NextActiveParam()
{
  return (itsParamDescriptor->NextActive());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::NextActiveLocation()
{
  return (itsHPlaceDescriptor->NextActive());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::NextActiveLevel()
{
  return (itsVPlaceDescriptor ? itsVPlaceDescriptor->Next() : false);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::PreviousActiveLevel()
{
  return (itsVPlaceDescriptor ? itsVPlaceDescriptor->Previous() : false);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::NextActiveTime()
{
  return (itsTimeDescriptor->NextActive());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsActiveParam()
{
  return itsParamDescriptor->IsActive();
}
// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::Location(const NFmiLocation &theLocation)
{
  return itsHPlaceDescriptor->Location(theLocation);
}

// ----------------------------------------------------------------------
/*!
 * \param theIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::Location(const unsigned long &theIdent)
{
  return itsHPlaceDescriptor->Location(theIdent);
}

// ----------------------------------------------------------------------
/*!
 * \param theLocationName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::Location(const NFmiString &theLocationName)
{
  return itsHPlaceDescriptor->Location(theLocationName);
}

// ----------------------------------------------------------------------
/*!
 * \param theLonLatPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::Location(const NFmiPoint &theLonLatPoint, NFmiPoint *theGridPoint)
{
  return itsHPlaceDescriptor->Location(theLonLatPoint, theGridPoint);
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \param theMaxDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::NearestLocation(const NFmiLocation &theLocation, double theMaxDistance)
{
  return itsHPlaceDescriptor->NearestLocation(theLocation, theMaxDistance);
}

inline bool NFmiQueryInfo::NearestLocation(const NFmiLocation &theLocation,
                                           const NFmiArea *theArea,
                                           double theMaxDistance)
{
  return itsHPlaceDescriptor->NearestLocation(theLocation, theArea, theMaxDistance);
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::NearestPoint(const NFmiPoint &theLatLonPoint)
{
  return itsHPlaceDescriptor->NearestPoint(theLatLonPoint);
}

// ----------------------------------------------------------------------
/*!
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::Level(const NFmiLevel &theLevel)
{
  return itsVPlaceDescriptor ? itsVPlaceDescriptor->Level(theLevel) : false;
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::Time(const NFmiMetTime &theTime)
{
  return itsTimeDescriptor->Time(theTime);
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \param theDirection Undocumented
 * \param theTimeRangeInMinutes Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::TimeToNearestStep(const NFmiMetTime &theTime,
                                             FmiDirection theDirection,
                                             long theTimeRangeInMinutes)
{
  return itsTimeDescriptor->TimeToNearestStep(theTime, theDirection, theTimeRangeInMinutes);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::ForecastPeriod()
{
  return itsTimeDescriptor->ForecastPeriod();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiLocation *NFmiQueryInfo::Location() const
{
  return (itsHPlaceDescriptor->Location());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiLevel *NFmiQueryInfo::Level() const
{
  return (itsVPlaceDescriptor ? itsVPlaceDescriptor->Level() : 0);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiMetTime &NFmiQueryInfo::OriginTime() const
{
  return (itsTimeDescriptor->OriginTime());
}

inline void NFmiQueryInfo::OriginTime(const NFmiMetTime &newTime)
{
  itsTimeDescriptor->OriginTime(newTime);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiMetTime &NFmiQueryInfo::ValidTime() const
{
  return (itsTimeDescriptor->ValidTime());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiMetTime &NFmiQueryInfo::Time() const
{
  return (IsValidTime() ? ValidTime() : OriginTime());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsLocalTime() const
{
  return (itsTimeDescriptor ? (itsTimeDescriptor->TimeLocalzation() != kUTC) : false);
}

// ----------------------------------------------------------------------
/*!
 * \param theLongitude Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryInfo::SetLocalTimes(const float theLongitude)
{
  itsTimeDescriptor->SetLocalTimes(theLongitude);
}

// ----------------------------------------------------------------------
/*!
 * \param newState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::ActivateParam(bool newState)
{
  return itsParamDescriptor->SetActivity(newState);
}

// ----------------------------------------------------------------------
/*!
 * \param newState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::ActivateLocation(bool newState)
{
  return itsHPlaceDescriptor->SetActivity(newState);
}

// ----------------------------------------------------------------------
/*!
 * \param newState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::ActivateLevel(bool newState)
{
  return itsVPlaceDescriptor ? itsVPlaceDescriptor->SetActivity(newState) : false;
}

// ----------------------------------------------------------------------
/*!
 * \param newState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::ActivateTime(bool newState)
{
  return itsTimeDescriptor->SetActivity(newState);
}

// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \param thePeriod Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::ActivateTimePeriod(bool theActivityState, const NFmiTimeBag &thePeriod)
{
  return itsTimeDescriptor->SetActivePeriod(theActivityState, thePeriod);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiTimeBag NFmiQueryInfo::GetActiveTimePeriod()
{
  return itsTimeDescriptor->GetActivePeriod();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::ParamIndex() const
{
  return itsParamDescriptor->Index();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::LocationIndex() const
{
  return itsHPlaceDescriptor->Index();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::LevelIndex() const
{
  return itsVPlaceDescriptor->Index();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::TimeIndex() const
{
  return itsTimeDescriptor->Index();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiPoint NFmiQueryInfo::LatLon() const
{
  return itsHPlaceDescriptor->LatLon();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiQueryInfo::RelativePoint() const
{
  return itsHPlaceDescriptor->RelativePoint();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline size_t NFmiQueryInfo::Size() const
{
  if (!itsParamDescriptor || !itsHPlaceDescriptor || !itsTimeDescriptor || !itsVPlaceDescriptor)
    return 0;

  return (itsParamDescriptor->Size() * itsHPlaceDescriptor->Size() * itsTimeDescriptor->Size() *
          itsVPlaceDescriptor->Size());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::SizeParams() const
{
  return itsParamDescriptor->Size();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::SizeLocations() const
{
  return itsHPlaceDescriptor->Size();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::SizeLevels() const
{
  return itsVPlaceDescriptor->Size();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::SizeTimes() const
{
  return itsTimeDescriptor->Size();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::SizeActiveParams() const
{
  return itsParamDescriptor->SizeActive();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::SizeActiveTimes() const
{
  return itsTimeDescriptor ? itsTimeDescriptor->SizeActive() : 0;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \bug The pointer may be constant, casting it to a reference
 *      may crash.
 */
// ----------------------------------------------------------------------

inline const NFmiLocation &NFmiQueryInfo::EditLocation()
{
  return *(itsHPlaceDescriptor->Location());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiStation &NFmiQueryInfo::EditStation()
{
  return *(const_cast<NFmiStation *>(
      reinterpret_cast<const NFmiStation *>(itsHPlaceDescriptor->Location())));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel &NFmiQueryInfo::EditLevel()
{
  return *(itsVPlaceDescriptor->Level());
}
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------
// asettaa suoraan arvon ilman aliparametri tarkasteluja

inline bool NFmiQueryInfo::IndexFloatValue(size_t theIndex, float theValue)
{
  return itsRefRawData ? itsRefRawData->SetValue(theIndex, theValue) : false;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiQueryInfo::IndexFloatValue(size_t theIndex) const
{
  return itsRefRawData ? itsRefRawData->GetValue(theIndex) : kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsSubParamUsed() const
{
  return itsParamDescriptor->IsSubParamUsed();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiQueryInfo::FloatValue() const
{
  return IsSubParamUsed() ? SubParamFloatValue() : IndexFloatValue(Index());
}

// ----------------------------------------------------------------------
/*!
 * \param theFloatData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::FloatValue(float theFloatData)
{
  if (::FmiIsValidNumber(theFloatData) == false)
  {
    fHasNonFiniteValueSet = true;
    return false;
  }
  else
    return IsSubParamUsed() ? SubParamFloatValue(theFloatData)
                            : IndexFloatValue(Index(), theFloatData);
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiQueryInfo::PeekValue(size_t theIndex) const
{
  return IsSubParamUsed() ? SubParamFloatValue(theIndex) : IndexFloatValue(theIndex);
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param theFloatData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::PokeValue(size_t theIndex, float theFloatData)
{
  return IsSubParamUsed() ? SubParamFloatValue(theFloatData)
                          : IndexFloatValue(theIndex, theFloatData);
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::Param(FmiParameterName theParam)
{
  if (itsParamDescriptor->Param(theParam))
  {
    if (itsParamDescriptor->IsSubParamUsed())
    {
      ChangeCombinedParamParser(itsParamDescriptor->Param());
    }
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsParamUsable() const
{
  return true;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsLocationUsable() const
{
  return true;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsLevelUsable() const
{
  return true;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::IsTimeUsable() const
{
  return true;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiHPlaceDescriptor &NFmiQueryInfo::HPlaceDescriptor() const
{
  return *itsHPlaceDescriptor;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiVPlaceDescriptor &NFmiQueryInfo::VPlaceDescriptor() const
{
  return *itsVPlaceDescriptor;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiTimeDescriptor &NFmiQueryInfo::TimeDescriptor() const
{
  return *itsTimeDescriptor;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiParamDescriptor &NFmiQueryInfo::ParamDescriptor() const
{
  return *itsParamDescriptor;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should be const
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::ClassId() const
{
  return kNFmiQueryInfo;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should be const
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::NewClassId()
{
  return itsNewClassIdent;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should be const
 */
// ----------------------------------------------------------------------

inline const char *NFmiQueryInfo::ClassName() const
{
  return "NFmiQueryInfo";
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRawData *NFmiQueryInfo::RefRawData() const
{
  return itsRefRawData;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiQueryData *NFmiQueryInfo::RefQueryData() const
{
  return itsRefQueryData;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::UseSubParam()
{
  return IsSubParamUsed();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiCombinedParam *NFmiQueryInfo::CombinedParam()
{
  return itsCombinedParamParser;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfo::TimeResolution()
{
  return itsTimeDescriptor->Resolution();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiQueryInfo::TimeUnCertaintyStart()
{
  return itsTimeUnCertaintyStart;
}
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryInfo::TimeUnCertaintyStart(long newValue)
{
  itsTimeUnCertaintyStart = newValue;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiQueryInfo::TimeUnCertaintyEnd()
{
  return itsTimeUnCertaintyEnd;
}
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryInfo::TimeUnCertaintyEnd(long newValue)
{
  itsTimeUnCertaintyEnd = newValue;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiQueryInfo::AreaUnCertaintyStart()
{
  return itsAreaUnCertaintyStart;
}
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryInfo::AreaUnCertaintyStart(long newValue)
{
  itsAreaUnCertaintyStart = newValue;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiQueryInfo::AreaUnCertaintyEnd()
{
  return itsAreaUnCertaintyEnd;
}
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryInfo::AreaUnCertaintyEnd(long newValue)
{
  itsAreaUnCertaintyEnd = newValue;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryInfo::DoEndianByteSwap()
{
  return fDoEndianByteSwap;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiQueryInfo::InfoVersion() const
{
  return itsInfoVersion;
}
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryInfo::InfoVersion(double newValue) const
{
  itsInfoVersion = newValue;
}
// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \param ob The NFmiQueryInfo object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, NFmiQueryInfo &ob)
{
  return ob.Write(file);
}
// ----------------------------------------------------------------------
/*!
 * \param file The input stream to read from
 * \param ob The NFmiQueryInfo object to read into
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiQueryInfo &ob)
{
  return ob.Read(file);
}

// ======================================================================
