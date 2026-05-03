// ======================================================================
/*!
 * \file NFmiDataModifierWindChill.h
 * \brief Interface of class NFmiDataModifierWindChill
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiPoint.h"

class NFmiQueryInfo;

class NFmiDataModifierWindChill : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierWindChill();

  // anna se queryinfo mistä halutaan laskea windchilliä konstruktorissa!
  NFmiDataModifierWindChill(NFmiQueryInfo* theQueryInfo,
                            NFmiDataModifier* theDataModifier,
                            const NFmiPoint& theLatLonPoint,
                            int theTeperatureParamId = 4,
                            int theWindSpeedParamId = 21);

  void Clear();

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult();

  const NFmiPoint& LatLon() const;
  void LatLon(const NFmiPoint& newLatLon);

 private:
  NFmiDataModifierWindChill(const NFmiDataModifierWindChill& theMod);
  NFmiDataModifierWindChill& operator=(const NFmiDataModifierWindChill& theMod);

  int itsTemperatureParamId;
  int itsWindSpeedParamId;
  NFmiPoint itsLatLonPoint;           // luokka tekee kaiken interpoloituna tähän pisteeseen!!!!
  NFmiQueryInfo* itsQueryInfo;        // ei omista!!!, käytä 'juoksutettua' infoa
  NFmiDataModifier* itsDataModifier;  // ei omista!!!

};  // class NFmiDataModifierWindChill

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint& NFmiDataModifierWindChill::LatLon() const
{
  return itsLatLonPoint;
}
// ----------------------------------------------------------------------
/*!
 * \param newLatLon Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifierWindChill::LatLon(const NFmiPoint& newLatLon)
{
  itsLatLonPoint = newLatLon;
}

// ======================================================================
