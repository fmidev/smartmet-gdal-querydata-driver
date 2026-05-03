// ======================================================================
/*!
 * \file NFmiWeatherAndCloudiness.h
 * \brief Interface of class NFmiWeatherAndCloudiness
 */
// ======================================================================

#pragma once

#include "NFmiCombinedParam.h"

#include <cmath>
extern "C"
{
#if defined(_MSC_VER) && \
    (_MSC_VER <          \
     1600)  // stdint.h -headeria ei ole MSVC++ kääntäjissä ennen VC++ 2010 eli _MSC_VER < 1600
  typedef unsigned long uint32_t;
#else
#include <stdint.h>
#endif
}

class NFmiDataIdent;
class NFmiParamBag;
class NFmiVoidPtrList;
class NFmiPoint;
class NFmiQueryInfo;
class NFmiIntegrationSelector;

//! Undocumented
class NFmiWeatherAndCloudiness : public NFmiCombinedParam
{
 public:
  virtual ~NFmiWeatherAndCloudiness();
  NFmiWeatherAndCloudiness();
  NFmiWeatherAndCloudiness(double theInfoVersion);
  NFmiWeatherAndCloudiness(const NFmiWeatherAndCloudiness& theWeather);
  NFmiWeatherAndCloudiness& operator = (const NFmiWeatherAndCloudiness&);

  NFmiWeatherAndCloudiness(unsigned long theValue,
                           FmiWeatherParamType theParamType,
                           float theTemperature = kFloatMissing,
                           double theInfoVersion = 7.);

  NFmiWeatherAndCloudiness(float theValue,
                           FmiWeatherParamType theParamType,
                           float theTemperature = kFloatMissing,
                           double theInfoVersion = 7.);

  NFmiWeatherAndCloudiness(unsigned long theCloudValue,
                           unsigned long theWeatherValue,
                           float theTemperature = kFloatMissing,
                           double theInfoVersion = 7.);

  NFmiWeatherAndCloudiness(unsigned long theSynopCloudValue,
                           unsigned long theSynopWeatherValue,
                           float theHumidity,
                           float theTemperature,
                           double theInfoVersion = 7.);

  NFmiWeatherAndCloudiness(float theCloudSymbol,
                           float theHSade,
                           float theTemperature,
                           float theTotalCloudiness,
                           float theFogSymbol,
                           float thePrecipitation = kFloatMissing,
                           double theInfoVersion = 7.);

  virtual NFmiCombinedParam* Clone() const;

  static bool IsFair(float precForm);
  static bool IsLiquid(float precForm);
  static bool IsHalfLiquid(float precForm);
  static bool IsSolid(float precForm);
  static bool IsFreezing(float precForm);

  bool IsLargeScale() const;
  bool IsConvective() const;
  bool IsRain() const;
  bool IsSleet() const;
  bool IsSnow() const;
  bool IsDrizzle() const;
  bool IsFreezingDrizzle() const;
  bool IsFreezingRain() const;
  bool IsHail() const;
  bool OnlySomePrecipitation() const;
  bool IsFair() const;
  bool IsThunder() const;
  bool IsPrecipitationOk() const;
  virtual bool IsMemberParam(FmiParameterName type) const;

  bool SubValue(double theValue, FmiParameterName theParam);
  double SubValue(FmiParameterName theParam, NFmiIntegrationSelector* theSelector = 0);
  virtual double RawSubValue(FmiParameterName theParam);

  virtual bool LongValue(unsigned long theValue);
  virtual unsigned long LongValue() const;

  NFmiDataIdent* CreateParam(const NFmiProducer& theProducer = NFmiProducer(),
                             NFmiVoidPtrList* theSecondaryProducerList = 0) const;

  float MeanFloatValue(
      FmiParameterName theParam,
      bool isTimePeriod,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness1,
      float factor1,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness2,
      float factor2,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness3 = NFmiWeatherAndCloudiness(),
      float factor3 = kFloatMissing,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness4 = NFmiWeatherAndCloudiness(),
      float factor4 = kFloatMissing) const;

  virtual bool SetToWeightedMean(NFmiCombinedParam* theWeatherAndCloudiness1,
                                 float fac1,
                                 NFmiCombinedParam* theWeatherAndCloudiness2,
                                 float fac2,
                                 NFmiCombinedParam* theWeatherAndCloudiness3 = 0,
                                 float fac3 = kFloatMissing,
                                 NFmiCombinedParam* theWeatherAndCloudiness4 = 0,
                                 float fac4 = kFloatMissing);

  const NFmiWeatherAndCloudiness MeanWeather(
      bool isTimePeriod,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness1,
      float fac1,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness2,
      float fac2,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness3 = NFmiWeatherAndCloudiness(),
      float fac3 = kFloatMissing,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness4 = NFmiWeatherAndCloudiness(),
      float fac4 = kFloatMissing) const;

  virtual bool SetToWeightedPeriod(NFmiQueryInfo* info,
                                   const NFmiPoint& theLonLat,
                                   unsigned long period,
                                   bool considerPrecipitationMax,
                                   float factor1 = 1.,
                                   float factor2 = 1.,
                                   float factor3 = 1.);

  virtual bool SetToWeightedPeriod(NFmiQueryInfo* info,
                                   const NFmiPoint& theLonLat,
                                   const NFmiMetTime& startTime,
                                   const NFmiMetTime& endTime,
                                   bool considerPrecipitationMax,
                                   float factor1 = 1.,
                                   float factor2 = 1.,
                                   float factor3 = 1.);

  int CrossCheckIndex() const;

  bool DefaultPrecipitationTypeIsLargeScale() const;

  // NOTE NOTE NOTE NOTE!!!!!!!!!
  // This method is not thread safe since it modifies a global variable
  void DefaultPrecipitationTypeIsLargeScale(bool newState);

 protected:
  virtual void CreateSubParams();

  unsigned long PrecipitationFormFromWeightTable(double* formTable) const;
  unsigned long PrecipitationFormWeightedMean(unsigned long form1,
                                              unsigned long amount1,
                                              float factor1,
                                              unsigned long form2,
                                              unsigned long amount2,
                                              float factor2,
                                              unsigned long form3 = 0,
                                              unsigned long amount3 = 0,
                                              float factor3 = kFloatMissing,
                                              unsigned long form4 = 0,
                                              unsigned long amount4 = 0,
                                              float factor4 = kFloatMissing) const;

  float PrecipitationWeightedMean(unsigned long missingValue,
                                  bool considerMax,
                                  unsigned long value1,
                                  float factor1,
                                  unsigned long value2,
                                  float factor2,
                                  unsigned long value3 = kUnsignedLongMissing,
                                  float factor3 = kFloatMissing,
                                  unsigned long value4 = kUnsignedLongMissing,
                                  float factor4 = kFloatMissing) const;

  int WeightedMean(int missingValue,
                   int value1,
                   float factor1,
                   int value2,
                   float factor2,
                   int value3 = kShortMissing,
                   float factor3 = kFloatMissing,
                   int value4 = kShortMissing,
                   float factor4 = kFloatMissing) const;

  virtual void CreateIntegrators();

 public:
  void SetPrecipitationProbabilityLimits(float firstLimit = -1, float secondLimit = -1);
  virtual NFmiCombinedParam* CreateNew(float theInitValue);

  unsigned long TimeIntegratedHessaa(
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness1,
      float factor1,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness2,
      float factor2,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness3 = NFmiWeatherAndCloudiness(),
      float factor3 = kFloatMissing,
      const NFmiWeatherAndCloudiness& theWeatherAndCloudiness4 = NFmiWeatherAndCloudiness(),
      float factor4 = kFloatMissing);

  unsigned long CalcTotalPrecipitation() const;
  unsigned long CalcPrecipitationFromHessaa(unsigned long theValue);

  unsigned long LowAndMiddleClouds() const;
  unsigned long CloudsToEighths(unsigned long tenProsents) const;
  unsigned long ToHsade() const;
  bool DataOk() const;
  void Temperature(float theTemperature);
  float Temperature() const;

  unsigned long PrecipitationScale(double theValue) const;
  unsigned long PrecipitationScaleV6(double theValue) const;
  unsigned long PrecipitationScaleV7(double theValue) const;

  double PrecipitationScale(unsigned long theValue) const;
  double PrecipitationScaleV6(unsigned long theValue) const;
  double PrecipitationScaleV7(unsigned long theValue) const;

 private:
  double ThunderProbabilityValue() const;
  double TotalCloudinessValue() const;
  double Precipitation1hValue() const;
  void CheckIfPrecipZeroAndCleanTypeAndForm();
  void CrossCheck(FmiParameterName theParam);
  unsigned long CalcTotalCloudiness();

  bool TotalCloudiness(unsigned long theValue);
  bool LowClouds(unsigned long theValue);
  bool MiddleClouds(unsigned long theValue);
  bool HighClouds(unsigned long theValue);
  bool TotalPrecipitation(unsigned long theValue);
  bool PrecipitationType(unsigned long theValue);
  bool PrecipitationForm(unsigned long theValue);
  bool ThunderProbability(unsigned long theValue);
  bool FogIntensity(unsigned long theValue);

  bool CheckHessaa(unsigned long theValue) const;
  bool FromHessaa(unsigned long theValue);
  unsigned long ThunderProbabilityFromHessaa(unsigned long theValue);
  unsigned long PrecipitationFormFromHessaa(unsigned long theValue);
  unsigned long PrecipitationTypeFromHessaa(unsigned long theValue);
  unsigned long TotalPrecipitationFromHessaa(unsigned long theValue);
  unsigned long TotalCloudinessFromHessaa(unsigned long theValue);

  unsigned long ToHessaa();
  unsigned long TotalCloudinessToHessaa() const;
  unsigned long ThunderToHessaa() const;
  unsigned long TotalPrecipitationToHessaa() const;
  unsigned long PartlyCloudyHessaa();
  unsigned long HalfCloudyHessaa();
  unsigned long OverCastHessaa();

  unsigned long CalcSynopPrecipitationAmount(unsigned theSynopWeatherCode) const;
  unsigned long CalcSynopThunderProb(unsigned long theSynopWeatherCode) const;
  unsigned long CalcSynopPrecipitationForm(unsigned long theSynopWeatherCode,
                                           unsigned long theTotalCloudiness) const;

  unsigned long CalcSynopPrecipitationForm(unsigned long theSynopWeatherCode,
                                           unsigned long theTotalCloudiness,
                                           float theHumidity,
                                           float theTemperature) const;
  unsigned long CalcPrecipitationForm(unsigned long theSynopWeatherCode) const;
  unsigned long CalcSynopPrecipitationType(unsigned long theSynopWeatherCode,
                                           unsigned long theTotalCloudiness) const;
  unsigned long CalcPrecipitationType(unsigned long theSynopWeatherCode) const;
  unsigned long CalcSynopFogIntensity(unsigned long theSynopWeatherCode) const;
  unsigned long CalcFogIntensity(unsigned long theFogSymbol) const;

  void CalcClouds(unsigned long theCloudSymbol);

  //! HUOM!! Tämä debuggaus strukti ei toimi kuin little-endian koneissa ja infoversiolle 7
  struct FmiWeatherStruct
  {
    unsigned long TotalCloudiness : 4;  // 0...10 (10%), 15 = missing
    unsigned long LowClouds : 4;        // 0...10 (10%), 15 = missing
    unsigned long MiddleClouds : 4;     // 0...10 (10%), 15 = missing
    unsigned long HighClouds : 3;       // Marko testaa
    ///////bitti lisää
    unsigned long TotalPrecipitation : 6;  // Marko testaa
    // lauran ehdotus: y=0.6*value^1.5
    unsigned long PrecipitationType : 2;  // 1=LargeScale, 2=Convective, (3 = missing?)
    unsigned long PrecipitationForm : 3;  // 0=drizzle, 1=rain, 2=sleet, 3=snow, 4=freezing drizzle,
                                          // 5=freezing rain, 6=hail, 7=no rain
    unsigned long FogIntensity : 2;       // 0=noFog, 1=moderateFog, 2=denseFog, 3=missing

    // ukkosen missing-arvo on 14 eikä 15 siksi, että kaikki bitit
    // eivät saa olla ykkösiä long->float muunnoksissa, tämän parametrin
    // on oltava viimeisenä tai muuten puuttuvat tiedot käsitellään väärin.

    unsigned long ThunderProbability : 4;  // 0...10 (10%), 14 = missing

  };  // struct FmiWeatherStruct

  // TÄMÄ pitäisi olla private, mutta GSM-kirjasto käyttää 4.9.2000/Marko

 public:
  unsigned long TotalCloudiness() const;
  unsigned long IntegratedSubValue(FmiParameterName theParam,
                                   NFmiIntegrationSelector* theSelector = 0) const;
  unsigned long IntegratedLongValue(NFmiIntegrationSelector* theSelector = 0);
  unsigned long IntegratedHsade(NFmiIntegrationSelector* theSelector = 0);
  unsigned long IntegratedHessaa(NFmiIntegrationSelector* theSelector = 0);

 private:
  unsigned long LowClouds() const;
  unsigned long MiddleClouds() const;
  unsigned long HighClouds() const;

  unsigned long HighCloudsV6() const;
  unsigned long HighCloudsV7() const;
  unsigned long TotalPrecipitation() const;

  unsigned long TotalPrecipitationV6() const;
  unsigned long TotalPrecipitationV7() const;
  unsigned long PrecipitationType() const;
  unsigned long PrecipitationForm() const;
  unsigned long FogIntensity() const;
  unsigned long ThunderProbability() const;

  // puuttuvan arvon testit on syytä tehdä näiden funktioiden kautta muuttuneen version myötä
  // sateelle ja yläpilville

  unsigned long TotalPrecipitationMissingValue() const;
  unsigned long HighCloudsMissingValue() const;

  unsigned long HighCloudsV7(double theValue) const;
  double HighCloudsV7(unsigned long theValue) const;

  void SetTotalCloudiness(unsigned long theValue);
  void SetLowClouds(unsigned long theValue);
  void SetMiddleClouds(unsigned long theValue);
  void SetHighClouds(unsigned long theValue);
  void SetHighCloudsV6(unsigned long theValue);
  void SetHighCloudsV7(unsigned long theValue);
  void SetTotalPrecipitation(unsigned long theValue);
  void SetTotalPrecipitationV6(unsigned long theValue);
  void SetTotalPrecipitationV7(unsigned long theValue);
  void SetPrecipitationType(unsigned long theValue);
  void SetPrecipitationForm(unsigned long theValue);
  void SetFogIntensity(unsigned long theValue);
  void SetThunderProbability(unsigned long theValue);

  //! Tämän structin avulla debuggaus on helpompaa (toimii ainoastaan little-endian koneissa)
  union FmiWeatherUnion
  {
    uint32_t longType;
    float floatType;
#ifndef UNIX
#ifndef NDEBUG
    FmiWeatherStruct structType;
#endif
#endif
  } itsData;

  // kertoo, saako esim sateen tyypille tai olomuodolle antaa arvon, jos
  // intensiteettiä kasvatetaan 0:sta. Laura lisäsi 17.3
  // bool fAutoUpdate;

  float itsTemperature;  // sateen olomuodon päättelyyn

  int itsCrossCheckIndex;  // Marko lisäsi myöhempään tarpeeseen, pitää sisällään tiedon, mitä
                           // CrossCheck-funktio muutti (jos muutti mitään)

  // staattista muuttujaa käytetään kun TotalPrecipitation():ssa arvotaan pilviä ja sateen tyyppiä
  // kun lisätään sadetta 'tyhjään' paikkaan, tällä määrätään tuleeko sateesta kuuroa vai tasaista
  // ja tuleeko kokonaispilvisyydestä 50 vai 100 prosenttista.
  // 27.2.2001/Marko Jouduin siirtämään parametrin cpp-tiedostoon DLL ongelmien takia
  //	static bool fDefaultPrecipitationTypeIsLargeScale;

 protected:
  float itsPrecipitationProbabilityLimit1;
  float itsPrecipitationProbabilityLimit2;

};  // class NFmiWeatherAndCloudiness

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an auto_ptr
 */
// ----------------------------------------------------------------------

inline NFmiCombinedParam* NFmiWeatherAndCloudiness::Clone() const
{
  return new NFmiWeatherAndCloudiness(*this);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::LongValue() const
{
  return itsData.longType;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiWeatherAndCloudiness::CrossCheckIndex() const
{
  return itsCrossCheckIndex;
}
// ----------------------------------------------------------------------
/*!
 * \param tenProsents Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::CloudsToEighths(unsigned long tenProsents) const
{
  return static_cast<unsigned long>(4.0 * static_cast<float>(tenProsents) / 5);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiWeatherAndCloudiness::DataOk() const
{
  return fDataOk;
}
// ----------------------------------------------------------------------
/*!
 * \param theTemperature Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::Temperature(float theTemperature)
{
  itsTemperature = theTemperature;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiWeatherAndCloudiness::Temperature() const
{
  return itsTemperature;
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::PrecipitationScale(double theValue) const
{
  if (itsInfoVersion >= 7.)
    return PrecipitationScaleV7(theValue);
  else
    return PrecipitationScaleV6(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiWeatherAndCloudiness::PrecipitationScale(unsigned long theValue) const
{
  if (itsInfoVersion >= 7.)
    return PrecipitationScaleV7(theValue);
  else
    return PrecipitationScaleV6(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::TotalCloudiness() const
{
  return (itsData.longType & 0xF);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::LowClouds() const
{
  return (itsData.longType >> 4 & 0xF);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::MiddleClouds() const
{
  return (itsData.longType >> 8 & 0xF);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::HighCloudsV6() const
{
  return (itsData.longType >> 12 & 0xF);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::HighCloudsV7() const
{
  return (itsData.longType >> 12 & 0x7);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::HighClouds() const
{
  if (itsInfoVersion >= 7.)
    return HighCloudsV7();
  else
    return HighCloudsV6();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::TotalPrecipitationV6() const
{
  return (itsData.longType >> 16 & 0x1F);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::TotalPrecipitationV7() const
{
  return (itsData.longType >> 15 & 0x3F);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::TotalPrecipitation() const
{
  if (itsInfoVersion >= 7.)
    return TotalPrecipitationV7();
  else
    return TotalPrecipitationV6();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::PrecipitationType() const
{
  return TotalPrecipitation() > 0 ? (itsData.longType >> 21 & 0x3) : kT2BitMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::PrecipitationForm() const
{
  return TotalPrecipitation() > 0 ? (itsData.longType >> 23 & 0x7) : kT3BitMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::FogIntensity() const
{
  return (itsData.longType >> 26 & 0x3);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::ThunderProbability() const
{
  return (itsData.longType >> 28 & 0xF);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::TotalPrecipitationMissingValue() const
{
  return (itsInfoVersion >= 7.) ? kT6BitMissing : kT5BitMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWeatherAndCloudiness::HighCloudsMissingValue() const
{
  return (itsInfoVersion >= 7.) ? kT3BitMissing : kT4BitMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetTotalCloudiness(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0xF << 0))) | ((theValue & 0xF) << 0));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetLowClouds(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0xF << 4))) | ((theValue & 0xF) << 4));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetMiddleClouds(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0xF << 8))) | ((theValue & 0xF) << 8));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetHighCloudsV6(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0xF << 12))) | ((theValue & 0xF) << 12));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetHighCloudsV7(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0x7 << 12))) | ((theValue & 0x7) << 12));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetHighClouds(unsigned long theValue)
{
  if (itsInfoVersion >= 7.)
    SetHighCloudsV7(theValue);
  else
    SetHighCloudsV6(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetTotalPrecipitationV6(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0x1F << 16))) | ((theValue & 0x1F) << 16));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetTotalPrecipitationV7(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0x3F << 15))) | ((theValue & 0x3F) << 15));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetTotalPrecipitation(unsigned long theValue)
{
  if (itsInfoVersion >= 7.)
    SetTotalPrecipitationV7(theValue);
  else
    SetTotalPrecipitationV6(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetPrecipitationType(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0x3 << 21))) | ((theValue & 0x3) << 21));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetPrecipitationForm(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0x7 << 23))) | ((theValue & 0x7) << 23));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetFogIntensity(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0x3 << 26))) | ((theValue & 0x3) << 26));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiWeatherAndCloudiness::SetThunderProbability(unsigned long theValue)
{
  itsData.longType =
      static_cast<uint32_t>((itsData.longType & (~(0xF << 28))) | ((theValue & 0xF) << 28));
}

// ======================================================================
