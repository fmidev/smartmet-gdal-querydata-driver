// ======================================================================
/*!
 * \file NFmiDataModifier.h
 * \brief Interface of class NFmiDataModifier
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierBase.h"
#include "NFmiDef.h"

class NFmiCombinedParam;
class NFmiQueryInfo;

//! Undocumented
class NFmiDataModifier : public NFmiDataModifierBase
{
 public:
  virtual ~NFmiDataModifier();
  NFmiDataModifier(const NFmiDataModifier& theModier);
  virtual NFmiDataModifier* Clone() const;
  NFmiDataModifier(FmiJoinOperator theJoinOperator = kFmiAdd,
                   bool missingValuesAllowed = true,
                   NFmiCombinedParam* thePotentialCombinedParam = 0);

  operator double();

  virtual unsigned long ClassId() const;
  virtual const char* ClassName() const;
  bool IsCombinedParam();

  virtual bool BoolOperation(float theValue);
  virtual float FloatOperation(float theValue);
  virtual float CalculationResult();
  virtual float CalculationResult(double theSomeValueThatAffectsResults);
  virtual NFmiCombinedParam* CombinedCalculationResult();
  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);
  virtual void Clear();
  virtual double FloatValue();
  bool IsInside(float theValue);
  bool SetLimits(float theLowerLimit, float theUpperLimit);
  void SetMissingAllowed(bool OnOff);

  // nämä fastInfojen indeksien asettaja metodit on sitä varten että jos muokattava data ja
  // modifierin käyttämät apuInfo ovat
  // peräisin samasta datasta, voidaan datoja juoksuttaa nopeasti ja helposti samaan tahtiin. Näitä
  // käytetään nyt vain
  // multi-threaddausta käyttävän NFmiQueryInfo::ModifyTimesLocationData_FullMT -metodin
  // alaisuudessa.
  virtual void SetLocationIndex(unsigned long theIndex);
  virtual void SetTimeIndex(unsigned long theIndex);
  virtual void InitLatlonCache();  // jos datamodifierissa on fastInfoja tai maskeja joissa on
                                   // fastInfoja, niiden latlon-cache pitää alustaa ennen
                                   // multi-thread-ajoa

 protected:
  virtual bool CheckMissingValues(float theValue);
  virtual bool IsAllowedValue(float theValue);

  NFmiCombinedParam* itsCombinedParam;
  bool fCalculationResultOk;  // käytetään mm. keskiarvon laskun tuloksen palautuksen tarkastelussa
  // eli jos ei ole ollut mitään laskettavaa, ei palauteta arvoa 0 vaan
  // kFloatMissing
  bool fMissingValuesAllowed;

  int itsNumberOfMissingValues;

 private:
  NFmiDataModifier& operator=(const NFmiDataModifier& theModifier);

  bool fIsCombinedParam;
  bool fIsLimitCheck;
  float itsLowerLimit;
  float itsUpperLimit;

};  // class NFmiDataModifier

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiDataModifier::operator double()
{
  return FloatValue();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiDataModifier::ClassId() const
{
  return 0;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiDataModifier::ClassName() const
{
  return "NFmiDataModifier";
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataModifier::BoolOperation(float /* theValue */)
{
  return false;
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifier::FloatOperation(float theValue)
{
  return theValue;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifier::CalculationResult()
{
  return kFloatMissing;
}
// ----------------------------------------------------------------------
/*!
 * \param theSomeValueThatAffectsResults
 * \return Undocumented Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifier::CalculationResult(double /* theSomeValueThatAffectsResults */)
{
  return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifier::Calculate(float /* theValue */) {}
// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifier::Calculate(NFmiQueryInfo* /* theQI */) {}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifier::Clear()
{
  itsNumberOfMissingValues = 0;
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumenetd
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataModifier::IsAllowedValue(float theValue)
{
  return CheckMissingValues(theValue) && IsInside(theValue);
}
// ----------------------------------------------------------------------
/*!
 * \param OnOff Undocumenetd
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifier::SetMissingAllowed(bool OnOff)
{
  fMissingValuesAllowed = OnOff;
}

// ======================================================================
