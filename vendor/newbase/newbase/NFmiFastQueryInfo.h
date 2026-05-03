// ======================================================================
/*!
 * \file NFmiFastQueryInfo.h
 * \brief Interface of class NFmiFastQueryInfo
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiQueryInfo.h"
#include <boost/unordered_map.hpp>
#include <memory>

class NFmiRect;

namespace Fmi
{
class CoordinateMatrix;
class SpatialReference;
}  // namespace Fmi

//! Undocumented
class NFmiFastQueryInfo : public NFmiQueryInfo
{
 public:
  ~NFmiFastQueryInfo();

  NFmiFastQueryInfo(const NFmiFastQueryInfo &theInfo);

  NFmiFastQueryInfo(const NFmiQueryInfo &theInfo);

  NFmiFastQueryInfo(double theInfoVersion = 7.);

  NFmiFastQueryInfo(const NFmiParamDescriptor &theParamDescriptor,
                    const NFmiTimeDescriptor &theTimeDescriptor,
                    const NFmiHPlaceDescriptor &theHPlaceDescriptor = NFmiHPlaceDescriptor(),
                    const NFmiVPlaceDescriptor &theVPlaceDescriptor = NFmiVPlaceDescriptor(),
                    double theInfoVersion = 7.);

  NFmiFastQueryInfo(NFmiQueryData *data,
                    NFmiParamDescriptor *theParamDescriptor = 0,
                    NFmiTimeDescriptor *theTimeDescriptor = 0,
                    NFmiHPlaceDescriptor *theHPlaceDescriptor = 0,
                    NFmiVPlaceDescriptor *theVPlaceDescriptor = 0);

  bool NextActiveParam();
  bool NextActiveParam(bool fIgnoreSubParam);  // = true);
  bool IsActiveParam();
  bool ActivateParam(bool newState, bool fIgnoreSubParam);  // = true);
  bool ActivateParam(bool newState);

  void Reset();
  void ResetParam();
  void ResetLocation();
  void ResetLevel();
  void ResetTime();

  bool First();

  // miksi first-metodit palauttavat booleanin?? (nfmiqueryinfon peruja), voidaanko muuttaa???

  bool FirstParam(bool fIgnoreSubParam = true);
  bool FirstLocation();
  bool FirstLevel();
  bool FirstTime();

  bool NextParam(bool fIgnoreSubParam = true);
  bool NextLocation();

  bool NextLevel();
  bool NextTime();

  // miksi last-metodit palauttavat booleanin?? (nfmiqueryinfon peruja), voidaanko muuttaa???

  bool LastParam(bool fIgnoreSubParam = true);
  bool LastLocation();
  bool LastLevel();
  virtual bool LastTime();

  bool PreviousParam(bool fIgnoreSubParam = true);
  bool PreviousLocation();
  bool PreviousLevel();
  bool PreviousTime();

  // HUOM!!! Nämä seuraavat metodit ovat todellisuudessa samaa luokkaa nopeudessa
  // kuin NFmiQueryInfo:nkin vastaavat metodit.

  bool Param(const NFmiParam &theParam);
  bool Param(const NFmiParam &theParam, const NFmiParam &theSubParam);
  bool Param(const NFmiDataIdent &theDataIdent);
  bool Param(FmiParameterName theParam);

  bool Location(const NFmiLocation &theLocation);
  bool Location(const unsigned long &theIdent);
  bool Location(const NFmiString &theLocationName);
  bool Location(const NFmiPoint &theLonLatPoint, NFmiPoint *theGridPoint = 0);
  bool NearestLocation(const NFmiLocation &theLocation,
                       double theMaxDistance = kFloatMissing * 1000.);
  bool NearestLocation(const NFmiLocation &theLocation,
                       const NFmiArea *theArea,
                       double theMaxDistance = kFloatMissing * 1000.);
  bool NearestPoint(const NFmiPoint &theLatLonPoint);
  bool Level(const NFmiLevel &theLevelValue);
  bool Time(const NFmiMetTime &theTime);

  const NFmiLocation *Location() const;
  NFmiPoint LatLon() const;
  NFmiPoint LatLon(unsigned long index) const;

  const NFmiPoint RelativePoint() const;
  const NFmiLevel *Level() const;
  FmiLevelType LevelType() const;
  NFmiProducer *Producer();
  const NFmiProducer &FirstParamProducer();

  NFmiPoint WorldXY() const;  // actual metric coordinate, or latlon for point data
  NFmiPoint WorldXY(
      unsigned long index) const;  // actual metric coordinate, or latlon for point data

  const Fmi::SpatialReference &SpatialReference() const;
  Fmi::CoordinateMatrix CoordinateMatrix(bool wrapped = false) const;
  bool NeedsGlobeWrap() const;

  NFmiDataIdent &Param() const;
  // *** vastaavan nopeuksiset loppuvat tähän *********************

  // HUOM!!! Nämä seuraavat metodit ovat todellisuudessa hitaita ja
  // niiden käyttöä pitäisi välttää!
  // Näistä voisi varmaan tehdä nopeampia!!!!

  const NFmiMetTime &Time() const;
  bool TimeToNearestStep(const NFmiMetTime &theTime,
                         FmiDirection theDirection,
                         long theTimeRangeInMinutes = kLongMissing);
  const NFmiMetTime &ValidTime() const;
  using NFmiQueryInfo::OriginTime;
  const NFmiMetTime &OriginTime() const;
  // ****** hitaat loppuvat tähän ********************************

  unsigned long TimeResolution();

  // ******* Halutaanko manipuloida infoa suoraan indeksien avulla ????? ******************

  unsigned long ParamIndex() const;
  unsigned long LocationIndex() const;
  unsigned long LevelIndex() const;
  unsigned long TimeIndex() const;
  bool ParamIndex(unsigned long theIndex);
  bool LocationIndex(unsigned long theIndex);
  bool LevelIndex(unsigned long theIndex);
  bool TimeIndex(unsigned long theIndex);
  // ******* Halutaanko manipuloida infoa suoraan indeksien avulla ????? ******************

  unsigned long SizeParams() const;
  unsigned long SizeLocations() const;
  unsigned long SizeLevels() const;
  unsigned long SizeTimes() const;

  bool IsParamUsable() const;
  bool IsLocationUsable() const;
  bool IsLevelUsable() const;
  bool IsTimeUsable() const;

  unsigned long PeekLocationIndex(int theXOffset, int theYOffset) const;
  float PeekLocationValue(int theXOffset, int theYOffset) const;  // lähinnä gridi-datan tutkimiseen
  float PeekLocationValue(int theXOffset, int theYOffset, const NFmiMetTime &theTime);
  const NFmiPoint PeekLocationLatLon(int theXOffset, int theYOffset);
  float PeekTimeValue(int theTimeIndexOffset);

  using NFmiQueryInfo::PeekValue;
  float PeekValue(int theTimeOffset, int theXOffset, int theYOffset);

  NFmiDataMatrix<float> Values() const;

  // 12.09.2013 Anssi.R changed method to virtual to be able to override in NFmiMultiQueryInfo
  NFmiDataMatrix<float> Values(const NFmiMetTime &theInterpolatedTime);
  // Hakee annetun time-rangen sisällä olevan kentän interpoloituna. Jos time-range menee toiselta
  // aikasuunnalta yli, mutta toiselta ei, palautetaan nearesr, jos niin on määrätty
  // (doNearestTimeIfPossible = true).
  virtual NFmiDataMatrix<float> Values(const NFmiMetTime &theInterpolatedTime,
                                       long theTimeRangeInMinutes,
                                       bool doNearestTimeIfPossible = false);

  // 31.5.2017 Tavi high performance bulk query
  bool GetValues(size_t startIndex, size_t step, size_t count, std::vector<float> &values) const;
  bool SetValues(size_t startIndex, size_t step, size_t count, const std::vector<float> &values);
  bool GetValuesPartial(size_t startIndex,
                        size_t rowCount,
                        size_t rowStep,
                        size_t columnCount,
                        size_t columnStep,
                        std::vector<float> &values) const;
  bool GetLevelToVec(std::vector<float> &values);
  bool SetLevelFromVec(const std::vector<float> &values);
  bool GetLevelToVecPartial(size_t x1, size_t y1, size_t x2, size_t y2, std::vector<float> &values);
  bool GetCube(std::vector<float> &values);
  void DoSubParamConversions(std::vector<float> &values, std::string callingFunctionName) const;

  bool GetInterpolatedLevel(std::vector<float> &values, const NFmiMetTime &time);
  bool GetInterpolatedCube(std::vector<float> &values, const NFmiMetTime &t);

  NFmiDataMatrix<float> Values(NFmiDataModifier *theFunction,
                               const NFmiMetTime &theTime,
                               int theBackwardOffsetInMinutes,
                               int theForwardOffsetInMinutes);

  virtual NFmiDataMatrix<float> Values(const Fmi::CoordinateMatrix &theLatlonMatrix,
                                       float P = kFloatMissing,
                                       float H = kFloatMissing);

  virtual NFmiDataMatrix<float> Values(const Fmi::CoordinateMatrix &theLatlonMatrix,
                                       const NFmiMetTime &theTime,
                                       float P = kFloatMissing,
                                       float H = kFloatMissing);
  virtual NFmiDataMatrix<float> Values(const Fmi::CoordinateMatrix &theLatlonMatrix,
                                       const NFmiMetTime &theTime,
                                       float P,
                                       float H,
                                       long theTimeRangeInMinutes,
                                       bool doNearestTimeIfPossible = false);

  NFmiDataMatrix<float> CroppedValues(int x1, int y1, int x2, int y2) const;
  NFmiDataMatrix<float> CroppedValues(
      const NFmiMetTime &theInterpolatedTime, int x1, int y1, int x2, int y2);
  NFmiDataMatrix<float> CroppedValues(const NFmiMetTime &theInterpolatedTime,
                                      int x1,
                                      int y1,
                                      int x2,
                                      int y2,
                                      long theTimeRangeInMinutes,
                                      bool doNearestTimeIfPossible = false);
  bool SetValues(const NFmiDataMatrix<float> &theMatrix);

  Fmi::CoordinateMatrix LocationsWorldXY(const NFmiArea &theArea) const;
  Fmi::CoordinateMatrix LocationsXY(const NFmiArea &theArea) const;

  // Tähän tulee joukko funktioita, jotka palauttavat aktiivisen parametrin
  // arvon haluttuun korkeuteen [m]. Metodeja on neljä, jotka tekevät erilaisia
  // interpolointeja parametreista riippuen.
  float HeightValue(float theHeight);
  float HeightValue(float theHeight, const NFmiPoint &theLatlon);
  float HeightValue(float theHeight, const NFmiPoint &theLatlon, const NFmiMetTime &theTime);
  float HeightValue(float theHeight,
                    const NFmiPoint &theLatlon,
                    const NFmiMetTime &theTime,
                    unsigned long theTimeRangeInMinutes);
  float HeightValue(float theHeight, const NFmiMetTime &theTime);

  // Tähän tulee joukko funktioita, jotka palauttavat aktiivisen parametrin
  // arvon haluttuun paine pintaan interpoloituna (log(p) interpolointi).
  // Metodeja on neljä, jotka tekevät erilaisia interpolointeja parametreista riippuen.
  float PressureLevelValue(float P);
  float PressureLevelValue(float P, const NFmiPoint &theLatlon);
  float PressureLevelValue(float P, const NFmiPoint &theLatlon, const NFmiMetTime &theTime);
  float PressureLevelValue(float P,
                           const NFmiPoint &theLatlon,
                           const NFmiMetTime &theTime,
                           unsigned long theTimeRangeInMinutes);
  float PressureLevelValue(float P, const NFmiMetTime &theTime);

  // Tässä metodit millä saadaan dataan nopeita osoitus indeksejä, mitä voidaan hyödyntää kun
  // tehdään
  // Nopeita 4D interpolointeja (useita samaan paikkaan ja aikaa ja korkeuteen).
  // Kaikki double & parametrit (out-parametreja) ovat reaali lukuja joiden avulla voidaan päätellä
  // fastqueryinfosta
  // nopeasti laskuissa halutut arvot. Palauttaa false, jos ollaan datan ulkopuolella.
  bool GetLocationIndex(const NFmiPoint &theLatlon, double &xInd, double &yInd);
  bool GetTimeIndex(const NFmiMetTime &theTime, double &tInd);
  bool GetLevelIndex(const NFmiPoint &theLatlon,
                     const NFmiMetTime &theTime,
                     double pressure,
                     double &pInd);
  bool GetFixedPressureLevelIndex(const NFmiPoint &theLatlon,
                                  const NFmiMetTime &theTime,
                                  double &pressure,
                                  double &pInd);
  float FastPressureLevelValue(double xInd, double yInd, double tInd, double pInd);
  float FastPressureLevelValue(double xInd, double yInd, double pInd);
  float FastPressureLevelValue(double xInd, double yInd);

  static NFmiDataMatrix<float> CalcCrossSectionLeveldata(NFmiFastQueryInfo &theInfo,
                                                         const std::vector<NFmiPoint> &thePoints,
                                                         const NFmiMetTime &theTime);
  static NFmiDataMatrix<float> CalcTimeCrossSectionLeveldata(NFmiFastQueryInfo &theInfo,
                                                             const NFmiPoint &thePoint,
                                                             NFmiTimeBag &theTimes);
  static NFmiDataMatrix<float> CalcRouteCrossSectionLeveldata(
      NFmiFastQueryInfo &theInfo,
      const std::vector<NFmiPoint> &theLatlonPoints,
      const std::vector<NFmiMetTime> &thePointTimes);

  NFmiDataMatrix<float> CrossSectionValues(const NFmiMetTime &theInterpolatedTime,
                                           const std::vector<float> &theHeights,
                                           const std::vector<NFmiPoint> &theLatlonPoints);
  NFmiDataMatrix<float> TimeCrossSectionValues(std::vector<float> &theHeights,
                                               const NFmiPoint &thePoint,
                                               NFmiTimeBag &theWantedTimes);
  NFmiDataMatrix<float> RouteCrossSectionValues(const std::vector<float> &theHeights,
                                                const std::vector<NFmiPoint> &theLatlonPoints,
                                                const std::vector<NFmiMetTime> &thePointTimes);
  std::vector<float> ConvertPressuresToHeights(const std::vector<float> &thePressures);

  NFmiDataMatrix<float> CrossSectionValuesLogP(const NFmiMetTime &theInterpolatedTime,
                                               const std::vector<float> &thePressures,
                                               const std::vector<NFmiPoint> &theLatlonPoints);
  NFmiDataMatrix<float> TimeCrossSectionValuesLogP(std::vector<float> &thePressures,
                                                   const NFmiPoint &thePoint,
                                                   NFmiTimeBag &theWantedTimes,
                                                   unsigned int theStartTimeIndex = 0);
  NFmiDataMatrix<float> RouteCrossSectionValuesLogP(const std::vector<float> &thePressures,
                                                    const std::vector<NFmiPoint> &theLatlonPoints,
                                                    const std::vector<NFmiMetTime> &thePointTimes);

  NFmiDataMatrix<float> CrossSectionValuesHybrid(const NFmiMetTime &theInterpolatedTime,
                                                 const std::vector<NFmiLevel> &theLevels,
                                                 const std::vector<NFmiPoint> &theLatlonPoints);
  NFmiDataMatrix<float> TimeCrossSectionValuesHybrid(const std::vector<NFmiLevel> &theLevels,
                                                     const NFmiPoint &thePoint,
                                                     NFmiTimeBag &theWantedTimes);
  NFmiDataMatrix<float> RouteCrossSectionValuesHybrid(
      const std::vector<NFmiLevel> &theLevels,
      const std::vector<NFmiPoint> &theLatlonPoints,
      const std::vector<NFmiMetTime> &thePointTimes);

  NFmiDataMatrix<float> FlightRouteValues(const std::vector<float> &theHeights,
                                          const std::vector<NFmiPoint> &theLatlonPoints,
                                          const std::vector<NFmiMetTime> &thePointTimes);
  NFmiDataMatrix<float> FlightRouteValuesHybrid(const std::vector<NFmiLevel> &theLevels,
                                                const std::vector<NFmiPoint> &theLatlonPoints,
                                                const std::vector<NFmiMetTime> &thePointTimes);
  NFmiDataMatrix<float> FlightRouteValuesLogP(const std::vector<float> &thePressures,
                                              const std::vector<NFmiPoint> &theLatlonPoints,
                                              const std::vector<NFmiMetTime> &thePointTimes);

  // Hakee haluttuun hilaan interpoloitua dataa halutulta ajalta
  NFmiDataMatrix<float> GridValues(const NFmiGrid &theWantedGrid,
                                   const NFmiMetTime &theInterpolatedTime);
  NFmiDataMatrix<float> GridValues(const NFmiGrid &theWantedGrid,
                                   const NFmiMetTime &theInterpolatedTime,
                                   bool relative_uv);

  // 12.09.2013 Anssi.R changed methods to virtual to be able to override in NFmiMultiQueryInfo
  // Tämä hakee hilan sellaisenaan (datan originaali hila ja alue) halutulle painepinnalle.
  virtual NFmiDataMatrix<float> PressureValues(const NFmiMetTime &theInterpolatedTime,
                                               float wantedPressureLevel);
  // Tämä hakee haluttuun hilaan ja alueeseen dataa.
  virtual NFmiDataMatrix<float> PressureValues(const NFmiGrid &theWantedGrid,
                                               const NFmiMetTime &theInterpolatedTime,
                                               float wantedPressureLevel);
  NFmiDataMatrix<float> PressureValues(const NFmiGrid &theWantedGrid,
                                       const NFmiMetTime &theInterpolatedTime,
                                       float wantedPressureLevel,
                                       bool relative_uv);
  // Tämä hakee hilan sellaisenaan (datan originaali hila ja alue) halutulle korkeudelle [m].
  // Jos haluat lentopinnoille dataa (Flight Level) on lentopinta -> metri kerroin = 30.5
  // eli esim. lentopinta 50 saadaan laskulla 50 * 30.5 eli 1525 [m].
  NFmiDataMatrix<float> HeightValues(const NFmiMetTime &theInterpolatedTime,
                                     float wantedHeightLevel);
  // Sama korkeus haku, mutta haluttuun hilaan ja projektioon.
  NFmiDataMatrix<float> HeightValues(const NFmiGrid &theWantedGrid,
                                     const NFmiMetTime &theInterpolatedTime,
                                     float wantedHeightLevel);
  NFmiDataMatrix<float> HeightValues(const NFmiGrid &theWantedGrid,
                                     const NFmiMetTime &theInterpolatedTime,
                                     float wantedHeightLevel,
                                     bool relative_uv);

  bool HeightDataAvailable() const
  {
    return fHeightValueAvailable || fHeightLevelDataAvailable;
  }  // tämä on uusi yleisfunktio, jota pitäisi käyttää kun halutaan tietää onko heightlevel-dataa
     // käytössä
  bool HeightValueAvailable() const { return fHeightValueAvailable; }
  bool HeightLevelDataAvailable() const { return fHeightLevelDataAvailable; }
  bool HeightParamIsRising() const { return fHeightParamIsRising; }
  unsigned long HeightParamIndex() const { return itsHeightParamIndex; }
  unsigned long PressureParamIndex() const { return itsPressureParamIndex; }
  bool PressureDataAvailable() const
  {
    return fPressureValueAvailable || fPressureLevelDataAvailable;
  }  // tämä on uusi yleisfunktio, jota pitäisi käyttää kun halutaan tietää onko painelevel-dataa
  // käytössä
  bool PressureValueAvailable() const
  {
    return fPressureValueAvailable;
  }  // Katso voiko PressureDataAvailable -funktiota käyttää mieluummin
  bool PressureLevelDataAvailable() const
  {
    return fPressureLevelDataAvailable;
  }  // Katso voiko PressureDataAvailable -funktiota käyttää mieluummin
  bool PressureParamIsRising() const { return fPressureParamIsRising; }
  bool FindNearestTime(const NFmiMetTime &theTime,
                       FmiDirection theDirection = kCenter,
                       unsigned long theTimeRangeInMinutes = kUnsignedLongMissing);

  NFmiGrid *GridData(bool fUseExisting = false);
  NFmiQueryInfo *Clone() const;
  NFmiFastQueryInfo &operator=(const NFmiFastQueryInfo &theInfo);
  bool SetDescriptors(NFmiQueryInfo *theQueryInfo, bool fIgnoreLevel = true);
  NFmiQueryInfo *CreateCombinedInfo(NFmiQueryInfo *theOtherInfo);
  bool IsSubParamUsed() const;

  // ***** 14.5.2001/MArko Uusia metodeja hilassa liikkumiseen ***************
  // HUOM!! liikkumista ei voi sallia laatikon ulkopuolelle kuten esim. PeekValue-metodissa voi!!!
  // Move*** ovat liikkumista varten

  bool MoveUp(int moveBy = 1);     // toimii vain gridi datalle oikein!!!
  bool MoveDown(int moveBy = 1);   // toimii vain gridi datalle oikein!!!
  bool MoveLeft(int moveBy = 1);   // toimii vain gridi datalle oikein!!!
  bool MoveRight(int moveBy = 1);  // toimii vain gridi datalle oikein!!!

  // näillä asetetaan paikka suoraan johonkin laitaan (ei 'laatikon' ulkopuolelle!)

  bool Top();     // toimii vain gridi datalle oikein!!!
  bool Bottom();  // toimii vain gridi datalle oikein!!!
  bool Left();    // toimii vain gridi datalle oikein!!!
  bool Right();   // toimii vain gridi datalle oikein!!!
  // ***** 14.5.2001/MArko Uusia metodeja hilassa liikkumiseen ***************

  size_t Index(unsigned long theParamIndex,
               unsigned long theLocationIndex,
               unsigned long theLevelIndex,
               unsigned long theTimeIndex) const;

  virtual float GetFloatValue(size_t theIndex) const;

  // HUOM! tämä on viritys funktio, joka toimii oikeasti vain NFmiFastQueryInfo:ssa
  // Täälläkin tämän käyttö on vaarallista, ellei tiedä mitä tekee.
  // Käytetty InterpolatedValue-metodien virittämiseen W&C parametrin ali parametrien kanssa.
  void SetIsSubParamUsed(bool newState) { fUseSubParam = newState; }
  bool HasLatlonInfoInData() const;
  NFmiPoint GetLatlonFromData();  // jos datassa on kFmiLongitude ja kFmiLatitude parametrit,
                                  // lasketaan sijanti asetetun ajan ja paikan suhteen niiden
                                  // avulla
  float PeekParamValue(unsigned long theParamIndex);  // jos tiedetään jonkin parametrin indeksi,
                                                      // tämän avulla voidaan kurkata
  // sen parametrin arvoa (aika,paikka ja leveli jo asetettuja)
  // HUOM! Ei toimi aliparametrien (TotalWind ja WeatherAndCloudiness) kanssa!!!!

  // Nämä metodit määrätään fastInfossa koska lapsiluokissa käytetään näitä sitten.
  virtual const std::string &DataFileName() const
  {
    static std::string dummy;
    return dummy;
  }
  virtual void DataFileName(const std::string & /* theDataFileName */) {}
  virtual const std::string &DataFilePattern() const
  {
    static std::string dummy;
    return dummy;
  }
  virtual void DataFilePattern(const std::string & /* theDataFilePattern */) {}
  virtual std::shared_ptr<NFmiQueryData> DataReference()
  {
    return std::shared_ptr<NFmiQueryData>();
  }  // HUOM! ei saa palauttaa itsRefQueryData -data osaa shared-pointterissa, koska se deletoisi
     // lopuksi datan (TÄMÄ siis overridataan lapsessa!)
  NFmiInfoData::Type DataType() const { return itsDataType; };
  void DataType(NFmiInfoData::Type newType) { itsDataType = newType; };
  // ElapsedTimeFromLoadInSeconds toteutetaan vasta lapsiluokassa
  // (SmartMet workstation koodeissa), negatiivisia arvoja pidetään puuttuvina
  virtual double ElapsedTimeFromLoadInSeconds() const { return -1; }
  // Näillä Start/Restore -funktioilla otetaan nykyinen parametri tila talteen ja otetaan käyttöön
  // 'erikois' korkeus-parametri. Ja käytön jälkeen palautus.
  // HUOM! Jos Start-funktio palauttaa true:n, on kyseisen korkeus parametrin käyttö mahdollista ja
  // tällöin pitää kutsua jossain vaiheessa vastaavaa Restore-funktiota!
  bool StartUsingPressureParam();
  void RestoreAfterUsingPressureParam();
  bool StartUsingHeightParam();
  void RestoreAfterUsingHeightParam();
  // Universaali funktio, jolla saa halutunlaisen korkeus parametrin kyseiseltä leveliltä haluttuun
  // kohtaan ja aikaa interpoloituna
  float GetLevelHeightValue(FmiParameterName theParId,
                            const NFmiPoint &theLatlon,
                            const NFmiMetTime &theTime);
  float GetLevelHeightValue(FmiParameterName theParId,
                            const NFmiLocationCache &theLocationCache,
                            const NFmiTimeCache &theTimeCache);

  float GetCurrentLevelPressure();
  float GetCurrentLevelPressure(const NFmiPoint &theLatlon);
  float GetCurrentLevelPressure(const NFmiPoint &theLatlon, const NFmiMetTime &theTime);

  size_t Index() const;

  float IndexFloatValue(size_t theIndex) const;
  bool IndexFloatValue(size_t theIndex, float theValue);

  bool AreIndividualIndexiesInside(const unsigned long &theParamIndex,
                                   const unsigned long &theLocationIndex,
                                   const unsigned long &theLevelIndex,
                                   const unsigned long &theTimeIndex) const;
  void InitFastInfo();
  std::vector<float> CalcPressureLevelDataPressures();
  std::vector<float> CalcHeightLevelDataHeights();
  float FindFirstPressureValue();
  float FindFirstHeightValue();
  void DoWindComponentFix(const NFmiGrid &usedGrid,
                          float u,
                          float v,
                          FmiParameterName id,
                          NFmiDataMatrix<float> &theValues);

  unsigned long itsParamIndex;  // indeksit 'resetoidaan' antamalla arvoksi -1, joka on isoin
                                // unsigned long, arvo on 4294967295
  unsigned long
      itsLocationIndex;  // toivotaan, että datan todellinen koko ei tule koskaan lähelle tätä arvoa
  unsigned long itsLevelIndex;  // tee ensin unsigned long versio (queryinfon peruja), ja yritä
                                // sitten konvertoida nämä pelkiksi int:eiksi
  unsigned long itsTimeIndex;
  unsigned long itsParamSize;
  unsigned long itsLocationSize;
  unsigned long itsLevelSize;
  unsigned long itsTimeSize;
  //  unsigned long itsDataArraySize;
  //  float * itsDataArray; // luokka ei omista tätä, taulun osoite on otettu suoraan queryinfon
  //  datapoolilta (oletus: data on float arvoida)
  NFmiGrid *itsTemporaryGridData;  // jos käyttäjä pyytää gridi dataa, se talletetaan tähän

  // This declaration is needed or NFmiMultiQueryInfo::TimeIndex will not compile
  friend class NFmiMultiQueryInfo;
  size_t itsLocLevTimSize;  // location*level*time size (käytetään FastIndex-metodissa)
  size_t itsLevTimSize;     // level*time size (käytetään FastIndex-metodissa)

  bool fUseSubParam;  // käytetäänkö parhaillaan yhdistelmä parametrin aliparametria

  // Nämä datat ovat tarkoitettu HeightValue-metodien optimointia varten.
  // Nämä alustetaan InitFastInfo-metodissa.
  bool fHeightValueAvailable;  // voidaanko datalta pyytää arvoja HeightValue-metodeilla (leveleitä
                               // pitää olla useita ja pitää olla korkeus parametri datassa)
  bool fHeightLevelDataAvailable;  // Jos level datassa ei ollut height parametria, oliko se kenties
                                   // heightlevel dataa, jolloin eri kerrosten korkeus tiedot
                                   // saadaan levelbagistä
  std::vector<float> itsHeightLevelDataHeights;  // optimointia kun data on heightlevel dataa, tähän
                                                 // on talletettu eri korkeustasot
  bool fHeightParamIsRising;  // kun mennään leveleitä NextLevel:eillä läpi, nouseeko vai laskeeko
                              // korkeus arvot
  unsigned long itsHeightParamIndex;  // Optimointia kun asetetaan välillä height parametri päälle,
                                      // kun tehdään korkeus interpolointia.
  // Nämä datat ovat tarkoitettu PressureLevelValue-metodien optimointia varten.(ja niihin liittyvät
  // poikkileikkaus funktiot)
  // Nämä alustetaan InitFastInfo-metodissa.
  bool fPressureValueAvailable;  // voidaanko datalta pyytää arvoja PressureLevelValue-metodeilla
  // (leveleitä pitää olla useita ja pitää olla korkeus parametri
  // datassa)
  bool fPressureLevelDataAvailable;  // Jos level datassa ei ollut paine parametria, oliko se
                                     // kenties painepinta dataa, jolloin eri kerrosten paine tiedot
                                     // saadaan levelbagistä
  bool fPressureParamIsRising;  // kun mennään leveleitä NextLevel:eillä läpi, nouseeko vai laskeeko
                                // korkeus arvot
  unsigned long itsPressureParamIndex;  // Optimointia kun asetetaan välillä height parametri
                                        // päälle, kun tehdään korkeus interpolointia.
  unsigned long itsOldParamIndex;  // nämä Old-valuet ovat tarpeen kun käytetään välillä eri korkeus
                                   // parametreja StartUsingPressureParam- ja Height -funktioita.
  bool fOldFSubParamUsed;  // Näistä palautetaan arvot sitten vastaavissa RestoreUsingPressureParam-
                           // ja Height -funktioissa.
  std::vector<float> itsPressureLevelDataPressures;  // optimointia kun data on painepinta dataa,
                                                     // tähän on talletettu eri painepinnat
  unsigned long itsLongitudeParamIndex;  // Optimointia GetLatlonFromData-metodin varalle.
  unsigned long itsLatitudeParamIndex;   // Optimointia GetLatlonFromData-metodin varalle.

  // Tähän tulee optimointi apua location ID hakuja varten. Eli kun haetaan havainto-aseman id:n
  // avulla paikkoja.
  // unorderd-mappiin laitetaan asema id avaimeksi ja location index arvoksi.
  typedef boost::unordered_map<unsigned long, unsigned long> StationIdSeekContainer;
  StationIdSeekContainer itsFastStationIdSeekList;
  NFmiInfoData::Type itsDataType;  // data tyyppiä käytetään lähinnä SmartMet koodeissa (esim.
                                   // 2=malli-pinta, 5=havainto, 15=mallihybridipinta), jouduin
                                   // pudottamaan sen fastInfon lapsi-luokalta (NFmiOwnerInfo -
                                   // smarttools-kirjasto)

  NFmiMetTime itsDummyTime;  // kun halutaan palauttaa const& time jota ei ole, palautetaan tämä,
                             // asetettu aikaan 1900.01.01: 1.1.1

};  // class NFmiFastQueryInfo

// ----------------------------------------------------------------------
/*!
 * \param theParamIndex Undocumented
 * \param theLocationIndex Undocumented
 * \param theLevelIndex Undocumented
 * \param theTimeIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::AreIndividualIndexiesInside(const unsigned long &theParamIndex,
                                                           const unsigned long &theLocationIndex,
                                                           const unsigned long &theLevelIndex,
                                                           const unsigned long &theTimeIndex) const
{
  //  if(!IsLocationMasked(theLocationIndex))
  //	return false;

  return ((theParamIndex < itsParamSize) && (theLocationIndex < itsLocationSize) &&
          (theLevelIndex < itsLevelSize) && (theTimeIndex < itsTimeSize));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumenetd
 */
// ----------------------------------------------------------------------

inline size_t NFmiFastQueryInfo::Index() const
{
  if (AreIndividualIndexiesInside(itsParamIndex, itsLocationIndex, itsLevelIndex, itsTimeIndex))
    return (itsParamIndex * itsLocLevTimSize + itsLocationIndex * itsLevTimSize +
            itsLevelIndex * itsTimeSize + itsTimeIndex);
  else
    return static_cast<size_t>(-1);
}

// ----------------------------------------------------------------------
/*!
 * \param theParamIndex Undocumented
 * \param theLocationIndex Undocumented
 * \param theLevelIndex Undocumented
 * \param theTimeIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline size_t NFmiFastQueryInfo::Index(unsigned long theParamIndex,
                                       unsigned long theLocationIndex,
                                       unsigned long theLevelIndex,
                                       unsigned long theTimeIndex) const
{
  if (AreIndividualIndexiesInside(theParamIndex, theLocationIndex, theLevelIndex, theTimeIndex))
  {
    size_t idx = (theParamIndex * itsLocLevTimSize + theLocationIndex * itsLevTimSize +
                  theLevelIndex * itsTimeSize + theTimeIndex);
    return idx;
  }
  else
    return static_cast<size_t>(-1);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::IsSubParamUsed() const
{
  return fUseSubParam;
}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiFastQueryInfo::ResetParam()
{
  itsParamIndex = static_cast<unsigned long>(-1);
  fUseSubParam = false;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiFastQueryInfo::ResetLocation()
{
  itsLocationIndex = static_cast<unsigned long>(-1);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiFastQueryInfo::ResetLevel()
{
  itsLevelIndex = static_cast<unsigned long>(-1);
}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiFastQueryInfo::ResetTime()
{
  itsTimeIndex = static_cast<unsigned long>(-1);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::FirstParam(bool fIgnoreSubParam)
{
  ResetParam();
  return NextParam(fIgnoreSubParam);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::NextLocation()
{
  itsLocationIndex++;
  if (itsLocationIndex < itsLocationSize)
    return true;
  else
  {
    itsLocationIndex = itsLocationSize;
    return false;
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::FirstLocation()
{
  ResetLocation();
  return NextLocation();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::FirstLevel()
{
  ResetLevel();
  return NextLevel();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::NextTime()
{
  itsTimeIndex++;
  if (itsTimeIndex < itsTimeSize)
    return true;
  else
  {
    itsTimeIndex = itsTimeSize;
    return false;
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::FirstTime()
{
  ResetTime();
  return NextTime();
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiFastQueryInfo::IndexFloatValue(size_t theIndex) const
{
  return itsRefRawData ? itsRefRawData->GetValue(theIndex) : kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::LastLocation()
{
  itsLocationIndex = itsLocationSize - 1;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::LastLevel()
{
  itsLevelIndex = itsLevelSize - 1;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::LastTime()
{
  itsTimeIndex = itsTimeSize - 1;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::Param(FmiParameterName theParam)
{
  if (NFmiQueryInfo::Param(theParam))
  {
    itsParamIndex = NFmiQueryInfo::ParamIndex();
    fUseSubParam = itsParamDescriptor->IsSubParamUsed();
    return true;
  }
  else
  {
    // reset index if the param was not found
    itsParamIndex = static_cast<unsigned long>(-1);
    fUseSubParam = false;
    return false;
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::ParamIndex() const
{
  return itsParamIndex;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::LocationIndex() const
{
  return itsLocationIndex;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::LevelIndex() const
{
  return itsLevelIndex;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::TimeIndex() const
{
  return itsTimeIndex;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::SizeParams() const
{
  return itsParamSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::SizeLocations() const
{
  return itsLocationSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::SizeLevels() const
{
  return itsLevelSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::SizeTimes() const
{
  return itsTimeSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::IsParamUsable() const
{
  return itsParamIndex < itsParamSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::IsLocationUsable() const
{
  return itsLocationIndex < itsLocationSize;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::IsLevelUsable() const
{
  return itsLevelIndex < itsLevelSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::IsTimeUsable() const
{
  return itsTimeIndex < itsTimeSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::TimeResolution()
{
  // piti overidata täällä, koska timeindex pitää asettaa
  // ennen Resolution kyselyä
  itsTimeDescriptor->Time(itsTimeIndex);
  return itsTimeDescriptor->Resolution();
}

// ----------------------------------------------------------------------
/*!
 * \brief Return current WorldXY coordinate
 */
// ----------------------------------------------------------------------

inline NFmiPoint NFmiFastQueryInfo::WorldXY() const
{
  return WorldXY(itsLocationIndex);
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa nykyisen indeksin koordinaatit. Koordinaatticache sijaitsee
 * QueryData-luokan cachessa
 *
 * \return Nykyisen pisteen koordinaatit
 */
// ----------------------------------------------------------------------

inline NFmiPoint NFmiFastQueryInfo::LatLon() const
{
  return LatLon(itsLocationIndex);
}

// ----------------------------------------------------------------------
/*!
 * \brief Location index for peek position
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::PeekLocationIndex(int theXOffset, int theYOffset) const
{
  if (!IsGrid())
    return itsLocationIndex + theXOffset;

  long currentXIndex = (itsLocationIndex % itsGridXNumber) + theXOffset;
  long currentYIndex = (itsLocationIndex / itsGridXNumber) + theYOffset;

  // voiko tämän seuraavan tarkistuksen poistaa, kun indeksi tarkistetaan kuitenkin
  // Index-metodissa?? x- ja y-indeksien pitää pysyä gridin sisällä offsettien kera!
  if (currentXIndex >= 0 && currentXIndex < static_cast<long>(itsGridXNumber) &&
      currentYIndex >= 0 && currentYIndex < static_cast<long>(itsGridYNumber))
    return itsLocationIndex + theYOffset * itsGridXNumber + theXOffset;

  return static_cast<unsigned long>(-1);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
inline const Fmi::SpatialReference &NFmiFastQueryInfo::SpatialReference() const
{
  return itsHPlaceDescriptor->SpatialReference();
}
