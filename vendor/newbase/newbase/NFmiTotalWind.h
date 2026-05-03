// ======================================================================
/*!
 * \file NFmiTotalWind.h
 * \brief Interface of class NFmiTotalWind
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
class NFmiIntegrationSelector;

//! Undocumented
class NFmiTotalWind : public NFmiCombinedParam
{
 public:
  virtual ~NFmiTotalWind();
  NFmiTotalWind(const NFmiTotalWind& theWind);
  NFmiTotalWind(double theInfoVersion = 7.);
  NFmiTotalWind(unsigned long theValue,
                FmiWindParamType theParamType = kFmiPackedWind,
                double theInfoVersion = 7.,
                float theWindGustValue = kFloatMissing);
  NFmiTotalWind(float theValue,
                FmiWindParamType theParamType = kFmiPackedWind,
                double theInfoVersion = 7.,
                float theWindGustValue = kFloatMissing);
  NFmiTotalWind(float theFirstValue,
                float theSecondValue,
                FmiWindParamType theParamType,
                double theInfoVersion = 7.,
                float theWindGustValue = kFloatMissing);
  NFmiTotalWind& operator = (const NFmiTotalWind&);
  virtual NFmiCombinedParam* Clone() const;

  bool CheckWindVector(unsigned long theValue);
  virtual bool IsMemberParam(FmiParameterName type) const;

  virtual unsigned long LongValue() const;
  virtual bool LongValue(unsigned long theValue);

  double CalcU();  // lasketaan myös tuuli komponentit u ja v
  double CalcV();  // lasketaan myös tuuli komponentit u ja v
  bool SubValue(double theValue, FmiParameterName theParam);
  double SubValue(FmiParameterName theParam, NFmiIntegrationSelector* theSelector = 0);
  double RawSubValue(FmiParameterName theParam);

  NFmiDataIdent* CreateParam(const NFmiProducer& theProducer = NFmiProducer(),
                             NFmiVoidPtrList* theSecondaryProducerList = 0);
  NFmiTotalWind* Mean(const NFmiTotalWind& theTotalWind, float factor = 1);

  bool SetToWeightedMean(NFmiCombinedParam* theCombinedParam1,
                         float fac1,
                         NFmiCombinedParam* theCombinedParam2,
                         float fac2,
                         NFmiCombinedParam* theCombinedParam3 = 0,
                         float fac3 = kFloatMissing,
                         NFmiCombinedParam* theCombinedParam4 = 0,
                         float fac4 = kFloatMissing);

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

 private:
  void SetFromDirectionAndSpeed(float theDirection, float theSpeed);
  void SetFromUVComponents(float theUValue, float theVValue, bool fSetValuesMissingFirst);

  unsigned long WindDirectionMean(unsigned long dir1, unsigned long dir2, float factor);
  int PartSum(int value1, int value2, float factor1, float factor2, int missingValue);
  bool FromWindVector(unsigned long theValue);
  unsigned long WindSpeedFromWindVector(unsigned long theValue);
  unsigned long WindDirectionFromWindVector(unsigned long theValue);
  unsigned long ToWindVector();

  // puuttuvan arvon testit on syytä tehdä tämän funktion kautta muuttuneen version myötä
  // tuulennopeudelle
  unsigned long WindSpeedMissingValue() const;
  unsigned long WindGustMissingValue() const;

  double WindDirectionValue();
  double WindSpeedVx(unsigned long theValue);
  double WindGustVx(unsigned long theValue);
  unsigned long WindSpeedVx(double theValue);
  unsigned long WindGustVx(double theValue);

  double WindSpeedV6(unsigned long theValue);
  unsigned long WindSpeedV6(double theValue);
  double WindSpeedV7(unsigned long theValue);
  unsigned long WindSpeedV7(double theValue);

  double WindGustV6(unsigned long theValue);
  unsigned long WindGustV6(double theValue);
  double WindGustV7(unsigned long theValue);
  unsigned long WindGustV7(double theValue);

  inline bool WindDirection(unsigned long theValue);
  inline bool WindSpeed(unsigned long theValue);
  inline bool WindGust(unsigned long theValue);

  //! HUOM!! Tämä debuggaus strukti ei toimi kuin little-endian koneissa ja versiolle 7
  struct FmiWindStruct
  {
    unsigned long WindDirection : 6;  // 0...36 (10 degrees), 63 = missing
    unsigned long WindSpeedMS : 12;   // WS 0..409.3 m/s kymmenyksen tarkkuudella
    unsigned long WindGustMS : 12;    // GustSpeed 0..409.3 m/s kymmenyksen tarkkuudella
  };

  unsigned long WindDirection() const;
  unsigned long WindSpeed() const;
  unsigned long WindSpeedV6() const;
  unsigned long WindSpeedV7() const;
  unsigned long WindGust() const;
  unsigned long WindGustV6() const;
  unsigned long WindGustV7() const;
  void SetWindDirection(unsigned long theValue);

  // todellisesta tuulennopeudesta muunnetaan uint:ksi ja talletetaan oikean version bittimuotoon
  void SetWindSpeedFromRealValue(double theValue);
  void SetWindSpeed(unsigned long theValue);
  void SetWindSpeedV6(unsigned long theValue);
  void SetWindSpeedV7(unsigned long theValue);
  void SetWindGust(unsigned long theValue);
  void SetWindGustV6(unsigned long theValue);
  void SetWindGustV7(unsigned long theValue);

  //! Undocumented
  union FmiWindUnion
  {
    uint32_t longType;
    float floatType;
#ifndef UNIX
#ifndef NDEBUG
    FmiWindStruct structType;
#endif
#endif
  } itsData;

 protected:
  int itsPrecision;

};  // class NFmiTotalWind

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should use auto_ptr
 */
// ----------------------------------------------------------------------

inline NFmiCombinedParam* NFmiTotalWind::Clone() const
{
  return new NFmiTotalWind(*this);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTotalWind::LongValue() const
{
  return itsData.longType;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTotalWind::WindSpeedMissingValue() const
{
  return (itsInfoVersion >= 7.) ? kT12BitMissing : kT9BitMissing;
}

inline unsigned long NFmiTotalWind::WindGustMissingValue() const
{
  return (itsInfoVersion >= 7.) ? kT12BitMissing : kT4BitMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiTotalWind::WindSpeedVx(unsigned long theValue)
{
  if (itsInfoVersion >= 7.)
    return WindSpeedV7(theValue);
  else
    return WindSpeedV6(theValue);
}

inline double NFmiTotalWind::WindGustVx(unsigned long theValue)
{
  if (itsInfoVersion >= 7.)
    return WindGustV7(theValue);
  else
    return WindGustV6(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTotalWind::WindSpeedVx(double theValue)
{
  if (itsInfoVersion >= 7.)
    return WindSpeedV7(theValue);
  else
    return WindSpeedV6(theValue);
}

inline unsigned long NFmiTotalWind::WindGustVx(double theValue)
{
  if (itsInfoVersion >= 7.)
    return WindGustV7(theValue);
  else
    return WindGustV6(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTotalWind::WindDirection() const
{
  return itsData.longType & 0x3F;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTotalWind::WindSpeedV6() const
{
  return (itsData.longType >> 6) & 0x01FF;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTotalWind::WindSpeedV7() const
{
  return (itsData.longType >> 6) & 0x0FFF;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTotalWind::WindSpeed() const
{
  if (itsInfoVersion >= 7.)
    return WindSpeedV7();
  else
    return WindSpeedV6();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTotalWind::WindGustV6() const
{
  return (itsData.longType >> 15) & 0xF;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTotalWind::WindGustV7() const
{
  return (itsData.longType >> 18) & 0xFFF;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiTotalWind::WindGust() const
{
  if (itsInfoVersion >= 7.)
    return WindGustV7();
  else
    return WindGustV6();
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTotalWind::SetWindDirection(unsigned long theValue)
{
  // Set 0x3F bits, preserving others
  itsData.longType = (itsData.longType & (~(0x3F << 0))) | ((theValue & 0x3F) << 0);

  // AKa 14-Aug-09: Added parantheses to allow '-Wall' on Ubuntu. Was:
  // itsData.longType = itsData.longType&(~(0x3F<<0))|((theValue&0x3F)<<0);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTotalWind::SetWindSpeedV6(unsigned long theValue)
{
  // Set 0x1FF<<6 bits, preserving others
  //
  itsData.longType = (itsData.longType & (~(0x1FF << 6))) | ((theValue & 0x1FF) << 6);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTotalWind::SetWindSpeedV7(unsigned long theValue)
{
  // Set 0xFFF<<6 bits, preserving others
  //
  itsData.longType = (itsData.longType & (~(0xFFF << 6))) | ((theValue & 0xFFF) << 6);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTotalWind::SetWindSpeed(unsigned long theValue)
{
  if (itsInfoVersion >= 7.)
    SetWindSpeedV7(theValue);
  else
    SetWindSpeedV6(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTotalWind::SetWindSpeedFromRealValue(double theValue)
{
  SetWindSpeed(WindSpeedVx(theValue));
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTotalWind::SetWindGustV6(unsigned long theValue)
{
  itsData.longType = (itsData.longType & (~(0xF << 15))) | ((theValue & 0xF) << 15);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTotalWind::SetWindGustV7(unsigned long theValue)
{
  itsData.longType = (itsData.longType & (~(0xFFF << 18))) | ((theValue & 0xFFF) << 18);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiTotalWind::SetWindGust(unsigned long theValue)
{
  if (itsInfoVersion >= 7.)
    SetWindGustV7(theValue);
  else
    SetWindGustV6(theValue);
}

// ======================================================================
