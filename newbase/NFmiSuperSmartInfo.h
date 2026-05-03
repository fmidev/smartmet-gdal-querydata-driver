// ======================================================================
/*!
 * \file NFmiSuperSmartInfo.h
 * \brief Interface of class NFmiSuperSmartInfo
 */
// ======================================================================

#pragma once

#ifdef _MSC_VER
#pragma warning(disable : 4251)  // poistaa VC++ kääntäjän varoituksen dll käännöksissä
#endif

#include "NFmiFastQueryInfo.h"
#include "NFmiRect.h"

class NFmiCalculator;
class NFmiAreaMask;
class NFmiDataModifierDescriptor;
class NFmiBitmapAreaMask;

//! Undocumented
class NFmiSuperSmartInfo : public NFmiFastQueryInfo
{
 public:
  // Accesslevel määrää millä tasolla dataa haetaan kulloisellakin hetkellä SSInfosta.
  enum DataAccessLevel
  {
    kNormalAccess = 0,     // == 'raaka' yhteys dataan eli käytä suoraan fastinfon FloatValue:ta
    kVarianceAccess = 1,   // varianssi tason käyttö
    kVariationAccess = 2,  // epävarmuus tason käyttö
    kTimeIntegrationAccess = 3,  // aikaintegraatio tason käyttö esim. sateen 12h yhteenlaskeminen
    kCalculatorAccess = 4        // korkeimman tason yhteys dataan (== laskee mukaan mahdollisesti
                                 // calculatorin ja todennäköisyyden ja varianssin, jos ne ovat
                                 // olemassa ja käytössä)
  };

  ~NFmiSuperSmartInfo();

  NFmiSuperSmartInfo();
  NFmiSuperSmartInfo(const NFmiQueryInfo& theInfo);
  NFmiSuperSmartInfo(const NFmiSuperSmartInfo& theInfo);

  NFmiSuperSmartInfo(const NFmiParamDescriptor& theParamDescriptor,
                     const NFmiTimeDescriptor& theTimeDescriptor,
                     const NFmiHPlaceDescriptor& theHPlaceDescriptor = NFmiHPlaceDescriptor(),
                     const NFmiVPlaceDescriptor& theVPlaceDescriptor = NFmiVPlaceDescriptor());

  NFmiSuperSmartInfo(NFmiQueryData* data,
                     NFmiParamDescriptor* theParamDescriptor = 0,
                     NFmiTimeDescriptor* theTimeDescriptor = 0,
                     NFmiHPlaceDescriptor* theHPlaceDescriptor = 0,
                     NFmiVPlaceDescriptor* theVPlaceDescriptor = 0);

  // paikan-asetus-funktiot pitää kirjoittaa uudestaan areamaskien takia, koska
  // nyt liikutaan maskeja käytettäessä vain maskattuihin kohtiin ja muut hypätään yli!!!!

  bool FirstLocation();
  bool NextLocation();
  bool PreviousLocation();
  using NFmiFastQueryInfo::NearestLocation;
  bool NearestLocation(const NFmiLocation& theLocation,
                       double theMaxDistance = kFloatMissing * 1000.);

  bool NearestPoint(const NFmiPoint& theLatLonPoint);

  // Parametrin-asetus-funktiot pitää kirjoittaa uudestaan varianssi-calculaattorien takia!!!!!

  bool FirstParam(bool fIgnoreSubParam = true);
  bool NextParam(bool fIgnoreSubParam = true);
  bool PreviousParam(bool fIgnoreSubParam = true);
  bool LastParam(bool fIgnoreSubParam = true);

#ifndef UNIX
  using NFmiFastQueryInfo::Param;  // nyt voi käyttää suoraan SSInfosta muitakin Param-funktioita
#else
  // oli pakko kirjoittaa, koska muuten en voi kutsua SSInfo:lta tätä,
  // koska muitakin Param-metodeja on peittämässä tätä
  NFmiDataIdent& Param() const { return NFmiFastQueryInfo::Param(); };
#endif

  bool Param(const NFmiParam& theParam);
  bool Param(const NFmiParam& theParam, const NFmiParam& theSubParam);
  bool Param(const NFmiDataIdent& theDataIdent);
  bool Param(FmiParameterName theParam);

  // ajan-asetus-funktiot pitää kirjoittaa uudestaan areamaskin takia,
  // jos sattuu olemaan dynaamisia ajassa muuttuvia!!!!

  bool FirstTime();
  bool NextTime();
  bool LastTime();
  bool PreviousTime();

#ifndef UNIX
  using NFmiFastQueryInfo::Time;  // nyt voi käyttää suoraan SSInfosta muitakin Time-funktioita
  using NFmiFastQueryInfo::TimeIndex;  // nyt voi käyttää suoraan SSInfosta muitakin
                                       // TimeIndex-funktioita
#else
  const NFmiMetTime& Time() const { return NFmiFastQueryInfo::Time(); }
  unsigned long TimeIndex() const { return NFmiFastQueryInfo::TimeIndex(); }
#endif
  bool Time(const NFmiMetTime& theTime);
  bool TimeIndex(unsigned long theIndex);

  unsigned long SizeLocations() const;
  unsigned long SizeActiveLocations() const;

  // QueryData Read
  float FloatValue() const;

  using NFmiQueryInfo::FloatValue;

  float FloatValue(bool doVariationCalculation,
                   bool doCalculation,
                   bool doTimeIntegration,
                   bool doVarianceCalculation);

  NFmiCombinedParam* CombinedValue();
  NFmiCombinedParam* CombinedValue(bool doVariationCalculation,
                                   bool doCalculation,
                                   bool doTimeIntegration,
                                   bool doVarianceCalculation);

  using NFmiFastQueryInfo::PeekLocationValue;
  float PeekLocationValue(int theXOffset, int theYOffset) const;
  float PeekTimeValue(int theTimeOffset);
  using NFmiQueryInfo::PeekValue;
  float PeekValue(int theTimeOffset, int theXOffset, int theYOffset);

  // ClassIdent
  unsigned long ClassId() const;
  const char* ClassName() const;

  NFmiQueryInfo* Clone() const;
  std::ostream& Write(std::ostream& file) const;
  std::istream& Read(std::istream& file);

  // operator
  NFmiSuperSmartInfo& operator=(const NFmiSuperSmartInfo& theInfo);

  void FloatValueAccessLevel(DataAccessLevel value);
  DataAccessLevel FloatValueAccessLevel() const;
  void UseCalculator(bool value);
  bool UseCalculator() const;
  void UseVarianceCalculator(bool value);
  bool UseVarianceCalculator() const;
  void UseVariationCalculator(bool value);
  bool UseVariationCalculator() const;
  bool UseTimeIntegrationCalculator();
  void UseTimeIntegrationCalculator(bool value);
  void SetCalculator(NFmiCalculator* value, bool useCalculator = true);
  void SetDataModifierDescriptor(NFmiDataModifierDescriptor* value);
  NFmiDataModifierDescriptor* DataModifierDescriptor();
  NFmiCalculator* Calculator() const;
  NFmiCalculator* CurrentVarianceCalculator() const;
  void AreaMask(NFmiAreaMask* value);
  NFmiAreaMask* AreaMask() const;
  void UseAreaMask(bool value);
  bool UseAreaMask() const;
  bool UseHelperBinaryMasks() const;
  void UseHelperBinaryMasks(bool newStatus);
  const NFmiBitmapAreaMask* CurrentHelperBinaryMask() const;
  NFmiCalculator* VariationCalculator();
  void VariationCalculator(NFmiCalculator* newCalculator);
  NFmiCalculator* TimeIntegrationCalculator();
  void TimeIntegrationCalculator(NFmiCalculator* newCalculator);
  NFmiString HelperBinaryMaskTestString(int theUsedVariationFactor);  // testi funktio
  bool LocationIndex2XYPosition(
      unsigned long theIndex,
      int* theXOffset,
      int* theYOffset);            // tämä kannattaa testien jälkeen laittaa privateksi
  void UpdateHelperBinaryMasks();  // kutsu tätä kun haluat päivittää apuharvennusmaskeja
  void InitEmptyAreaMask();
  bool SetNearestPointMask(const NFmiPoint& theLatLonPoint, bool newValue, bool fClearFirst);

 protected:
  void Destroy();

  virtual size_t Index() const;
  size_t Index(unsigned long theParamIndex,
               unsigned long theLocationIndex,
               unsigned long theLevelIndex,
               unsigned long theTimeIndex) const
  {
    return NFmiFastQueryInfo::Index(theParamIndex, theLocationIndex, theLevelIndex, theTimeIndex);
  }

  bool IsLocationMasked(unsigned long theLocationIndex) const;

  float VarianceFloatValue() const;
  float VariationFloatValue() const;
  float TimeIntegrationFloatValue() const;
  float RawFloatValue() const;
  float CalculationFloatValue() const;
  void UpdateVarianceCalculator();
  void UpdateAreaMaskTime();
  unsigned long CalcPeekLocationIndex(unsigned long currentIndex,
                                      int theXOffset,
                                      int theYOffset) const;
  NFmiCombinedParam* VariationCombinedValue();
  NFmiCombinedParam* CalculationCombinedValue();

 private:
  // kutsu tätä aina kun aika muuttuu SSInfossa!!!!
  void TimeChanged(unsigned long theOldTimeIndex);

  // kopioi helperbinarymaskit omaan käyttöönsä
  void CopyHelperBinaryMasksAndInfo(const NFmiSuperSmartInfo& theSSInfo);

  // alustaa kaikki tarvittavat harvennusapu maskit(eli kaikille epävarmuuksille 0-n)
  bool InitHelperBinaryMasks();

  // luo aina tietylle epävarmuuskertoimelle harvennusmaskin
  NFmiBitmapAreaMask* CreateHelperBinaryMask(int theUsedVariationFactor);

  NFmiBitmapAreaMask* CreateZeroVariationHelperBinaryMask();

  // tuhoaa maskit
  void ClearHelperBinaryMasks();

  // laskee käytetyn maskin xy-indeksi-boundingboxin
  void CalcXYMaskBoundingBox();

  NFmiBitmapAreaMask* HelperBinaryMask(int theUsedVariationFactor);
  void SetCurrentHelperBinaryMask();

  // Vaikuttaa mm. FloatValue-metodin toimintaan. Millä tasolla dataa haetaan:
  // raakana, varianssia, todennäköisyyttä, vai Calculaattori tasolla laskettua.
  // 0 = raaka data käyttö eli sama kuin FastInfo:n FloatValue.
  mutable DataAccessLevel itsFloatValueAccessLevel;

  // Käytetäänkö calculaattoria laskuissa. Jos ei ole kyseistä calculaattoria, arvon pitää olla
  // false.
  bool fUseCalculator;

  // Käytetäänkö varianssi calculaattoria laskuissa. Jos ei ole kyseistä calculaattoria, arvon pitää
  // olla false.
  bool fUseVarianceCalculator;
  bool fUseVariationCalculator;
  bool fUseTimeIntegrationCalculator;

  // tähän laitetaan mm. integraatio calculaattorit // ei omista, ei tuhoa!!!
  NFmiCalculator* itsCalculator;

  // epävarmuuslaatikko calculaattori // ei omista, ei tuhoaa!!!
  NFmiCalculator* itsVariationCalculator;

  // tämä laskee vesiputouksen 3. portaassa halutunlaisen aikaintegroinnin, esim. 12h
  // sade summa. ei omista, ei tuhoa!!!
  NFmiCalculator* itsTimeIntegrationCalculator;

  // Tähän SSInfo hakee aina kulloisenkin parametrin varianssi-modifierin
  // parametrikohtaisesta varianssi-modifier-listasta, jonka käyttäjä on
  // antanut. omistaa, tuhoaa!!!
  NFmiCalculator* itsCurrentVarianceCalculator;

  // parametrikohtaiset varianssi ja muut datamodifierit, jos jonkun parametrin
  // v-calculator puuttuu, ei lasketa varianssia! ei omista, ei tuhoa!!!
  NFmiDataModifierDescriptor* itsDataModifierDescriptor;

  // tämä vaikuttaa SSInfon paikka juoksutukseen, vain maskatut paikat
  // käydään läpi!!! // ei omista, ei tuhoa!!!
  NFmiAreaMask* itsAreaMask;

  bool fUseAreaMask;

  // Tämä on optimointi vipu: Harvennetaan käytäviä maskattuja pisteitä tämän
  // avulla, jotta epävarmuusalue ei hidasta liikaa laskentoja.
  bool fUseHelperBinaryMasks;

  // Optimoinnissa käytetty binääri maski. vectorista haetaan aina tähän
  // sopiva maski(ei omista ei tuhoa)
  NFmiBitmapAreaMask* itsCurrentHelperBinaryMask;

  // tätä käytetään apuna laskettaessa harvennusmaskeja
  NFmiRect itsXYMaskBoundingBox;

  // tähän talletetaan eri epävarmuus kertoimilla lasketut apumaskit (omistaa, tuhoaa)
  std::vector<NFmiBitmapAreaMask*> itsHelperBinaryMaskList;

  // Tämä osaa luoda käyttäjän pyytämiä speciaali maskeja ja osaa mm.
  // hoitaa optimoinnin binaarimaskin rakentamisen.

};  // class NFmiSuperSmartInfo

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::Param(FmiParameterName theParam)
{
  return Param(NFmiParam(theParam));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiSuperSmartInfo::ClassId() const
{
  return kNFmiQueryInfo;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiSuperSmartInfo::ClassName() const
{
  return "NFmiSuperSmartInfo";
}
// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::FloatValueAccessLevel(DataAccessLevel value)
{
  itsFloatValueAccessLevel = value;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiSuperSmartInfo::DataAccessLevel NFmiSuperSmartInfo::FloatValueAccessLevel() const
{
  return itsFloatValueAccessLevel;
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::UseCalculator(bool value)
{
  fUseCalculator = value;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseCalculator() const
{
  return fUseCalculator;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseVarianceCalculator() const
{
  return fUseVarianceCalculator;
}
// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::UseVariationCalculator(bool value)
{
  fUseVariationCalculator = value;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseVariationCalculator() const
{
  return fUseVariationCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseTimeIntegrationCalculator()
{
  return fUseTimeIntegrationCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::UseTimeIntegrationCalculator(bool value)
{
  fUseTimeIntegrationCalculator = value;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiDataModifierDescriptor* NFmiSuperSmartInfo::DataModifierDescriptor()
{
  return itsDataModifierDescriptor;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiCalculator* NFmiSuperSmartInfo::Calculator() const
{
  return itsCalculator;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// huom! ei asetus funktiota, koska tämä haetaan aina parametrikohtaisesta listasta!!!
inline NFmiCalculator* NFmiSuperSmartInfo::CurrentVarianceCalculator() const
{
  return itsCurrentVarianceCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::AreaMask(NFmiAreaMask* value)
{
  itsAreaMask = value;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMask* NFmiSuperSmartInfo::AreaMask() const
{
  return itsAreaMask;
}
// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::UseAreaMask(bool value)
{
  fUseAreaMask = value;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseAreaMask() const
{
  return fUseAreaMask;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseHelperBinaryMasks() const
{
  return fUseHelperBinaryMasks;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiBitmapAreaMask* NFmiSuperSmartInfo::CurrentHelperBinaryMask() const
{
  return itsCurrentHelperBinaryMask;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiCalculator* NFmiSuperSmartInfo::VariationCalculator()
{
  return itsVariationCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \param newCalculator Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::VariationCalculator(NFmiCalculator* newCalculator)
{
  itsVariationCalculator = newCalculator;
  TimeChanged(static_cast<unsigned long>(-1));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiCalculator* NFmiSuperSmartInfo::TimeIntegrationCalculator()
{
  return itsTimeIntegrationCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \param newCalculator Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::TimeIntegrationCalculator(NFmiCalculator* newCalculator)
{
  itsTimeIntegrationCalculator = newCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiSuperSmartInfo::RawFloatValue() const
{
  return NFmiFastQueryInfo::FloatValue();
}

// ======================================================================
