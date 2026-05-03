// ======================================================================
/*!
 * \file NFmiAreaMaskImpl.h
 * \brief Interface for class NFmiAreaMaskImpl
 */
// ======================================================================

#pragma once

#include "NFmiAreaMask.h"
#include "NFmiAreaMaskHelperStructures.h"
#include "NFmiCalculationCondition.h"
#include "NFmiInfoData.h"

class NFmAreaMaskList;

//! A basic area mask implementation
class NFmiAreaMaskImpl : public NFmiAreaMask
{
 public:
  virtual ~NFmiAreaMaskImpl();
  NFmiAreaMaskImpl();
  NFmiAreaMaskImpl(const NFmiCalculationCondition &theOperation,
                   Type theMaskType,
                   NFmiInfoData::Type theDataType,
                   BinaryOperator thePostBinaryOperator);
  NFmiAreaMaskImpl(const NFmiAreaMaskImpl &theOther);
  void Initialize() override;

  bool IsMasked(int theIndex) const override;
  bool IsMasked(const NFmiPoint &theLatLon) const override;
  double MaskValue(const NFmiPoint &theLatLon) const override;
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  // oletuksenä HeightValue palauttaa saman kuin Value-metodi, homma overridataan vain
  // NFmiInfoAreaMask-luokassa
  double HeightValue(double /* theHeight */,
                     const NFmiCalculationParams &theCalculationParams) override
  {
    // Poikkileikkaus/aikasarja laskuissa pitää aina käyttää 'aikainterpolaatiota', muuten ei (koska
    // laskussa käytetyt ajat on jo asetettu muualla)
    bool useTimeInterpolationAlways = theCalculationParams.fSpecialCalculationCase;
    return Value(theCalculationParams, useTimeInterpolationAlways);
  }
  // oletuksenä PressureValue palauttaa saman kuin Value-metodi, homma overridataan vain
  // NFmiInfoAreaMask-luokassa
  double PressureValue(double /* thePressure */,
                       const NFmiCalculationParams &theCalculationParams) override
  {
    // Poikkileikkaus/aikasarja laskuissa pitää aina käyttää 'aikainterpolaatiota', muuten ei (koska
    // laskussa käytetyt ajat on jo asetettu muualla)
    bool useTimeInterpolationAlways = theCalculationParams.fSpecialCalculationCase;
    return Value(theCalculationParams, useTimeInterpolationAlways);
  }

  bool IsEnabled() const override;
  void Enable(bool theNewState) override;

  bool Time(const NFmiMetTime &theTime) override;
  void Condition(const NFmiCalculationCondition &theCondition) override;
  const NFmiCalculationCondition &Condition() const override;

  bool IsRampMask() const override;
  bool IsWantedParam(const NFmiDataIdent &theParam, const NFmiLevel *theLevel = 0) const override;
  const NFmiString MaskString() const override;
  std::shared_ptr<NFmiFastQueryInfo> Info() override;
  void UpdateInfo(std::shared_ptr<NFmiFastQueryInfo> &theInfo) override;
  const NFmiDataIdent *DataIdent() const override;
  const NFmiParam *Param() const override;
  const NFmiLevel *Level() const override;
  void Level(const NFmiLevel &theLevel) override;
  NFmiInfoData::Type GetDataType() const override;
  void SetDataType(NFmiInfoData::Type theType) override;
  bool UseLevelInfo() const override;
  bool UsePressureLevelInterpolation() const override;
  void UsePressureLevelInterpolation(bool newValue) override;
  double UsedPressureLevelValue() const override;
  void UsedPressureLevelValue(double newValue) override;

  void LowerLimit(double theLowerLimit) override;
  void UpperLimit(double theUpperLimit) override;
  double LowerLimit() const override;
  double UpperLimit() const override;
  void MaskOperation(FmiMaskOperation theMaskOperation) override;
  FmiMaskOperation MaskOperation() const override;

  bool AddMask(NFmiAreaMask *theMask) override;
  NFmiAreaMask *AreaMask(int theIndex) const override;
  bool RemoveSubMask(int theIndex) override;
  void MaskType(Type theType) override;
  Type MaskType() const override;
  NFmiAreaMask *Clone() const override;
  void PostBinaryOperator(BinaryOperator newOperator) override;
  BinaryOperator PostBinaryOperator() const override;
  CalculationOperationType GetCalculationOperationType() const override;
  void SetCalculationOperationType(CalculationOperationType newValue) override;
  CalculationOperator GetCalculationOperator() const override;
  void SetCalculationOperator(CalculationOperator newValue) override;
  MathFunctionType GetMathFunctionType() const override;
  void SetMathFunctionType(MathFunctionType newValue) override;
  FunctionType GetFunctionType() const override { return itsFunctionType; }
  void SetFunctionType(FunctionType newType) override { itsFunctionType = newType; }
  FunctionType GetSecondaryFunctionType() const override { return itsSecondaryFunctionType; }
  void SetSecondaryFunctionType(FunctionType newType) override
  {
    itsSecondaryFunctionType = newType;
  }
  MetFunctionDirection GetMetFunctionDirection() const override { return itsMetFunctionDirection; }
  void GetMetFunctionDirection(MetFunctionDirection newValue) override
  {
    itsMetFunctionDirection = newValue;
  }
  int IntegrationFunctionType() const override { return itsIntegrationFunctionType; }
  void IntegrationFunctionType(int newValue) override { itsIntegrationFunctionType = newValue; }
  void SetArguments(std::vector<float> & /* theArgumentVector */) override{};
  int FunctionArgumentCount() const override { return itsFunctionArgumentCount; }
  void FunctionArgumentCount(int newValue) override { itsFunctionArgumentCount = newValue; }
  // HUOM! seuraavat toimivat oikeasti vain NFmiBinaryMask:in kanssa. Tässä vain tyhjät oletus
  // toteutukset.
  void SetAll(bool /* theNewState */) override{};
  void Mask(int /* theIndex */, bool /* newStatus */) override{};
  const std::shared_ptr<NFmiSimpleCondition> &SimpleCondition() const override
  {
    return itsSimpleCondition;
  }
  void SimpleCondition(std::shared_ptr<NFmiSimpleCondition> &theSimpleCondition) override
  {
    itsSimpleCondition = theSimpleCondition;
  }
  float FunctionDataTimeOffsetInHours() const override { return itsFunctionDataTimeOffsetInHours; }
  void FunctionDataTimeOffsetInHours(float newValue) override
  {
    itsFunctionDataTimeOffsetInHours = newValue;
  }
  bool CheckPossibleObservationDistance(const NFmiCalculationParams &) override { return true; }

 protected:
  virtual double CalcValueFromLocation(const NFmiPoint &theLatLon) const;
  virtual const NFmiString MakeSubMaskString() const;
  // Seuraavat virtuaali funktiot liittyvät integraatio funktioihin ja niiden mahdollisiin
  // Simplecondition tarkasteluihin
  virtual void DoIntegrationCalculations(float value);
  virtual void InitializeIntegrationValues();
  virtual bool SimpleConditionCheck(const NFmiCalculationParams &theCalculationParams);
  virtual float CalculationPointValue(int theOffsetX,
                                      int theOffsetY,
                                      const NFmiMetTime &theInterpolationTime,
                                      bool useInterpolatedTime);

 protected:
  NFmiCalculationCondition itsMaskCondition;
  Type itsMaskType;
  NFmiInfoData::Type itsDataType;
  CalculationOperationType itsCalculationOperationType;
  // Jos maskeja lista, tämän operaation mukaan lasketaan maskit yhteen esim. NOT, AND ja OR
  CalculationOperator
      itsCalculationOperator;  // myös smarttool systeemi, pitäisi suunnitella uusiksi!
  BinaryOperator itsPostBinaryOperator;
  MathFunctionType itsMathFunctionType;
  FunctionType itsFunctionType;  // onko mahd. funktio esim. min, max jne. (ei matemaattisia
                                 // funktioita kuten sin, cos, pow, jne.)
  FunctionType itsSecondaryFunctionType;  // tässä on ainakin vertikaali funktioiden lasku tapa
                                          // VertP, VertZ, VertFL ja VertHyb eli missä vertikaali
                                          // asteikossa operoidaan
  MetFunctionDirection itsMetFunctionDirection;  // grad, adv, div rot ja lap -funktioille (ja
  // näiden 2-versioille) määrätään myös suunta, joka
  // voi olla X, Y tai molemmat
  int itsIntegrationFunctionType;  // 1=SumT tyylinen ja 2=SumZ tyylinen ja 3=MinH tyylinen funktio
  int itsFunctionArgumentCount;
  bool fHasSubMasks;
  bool fEnabled;
  std::shared_ptr<NFmiSimpleCondition> itsSimpleCondition;
  // Jossain tilanteissa smarttool funktion datalle voidaan haluta tehdä aikasiirto,
  // jolloin käytetään tämän arvoa tekemään siirto, esim. peekxy3(T_ec[-3h] 10 0)
  // kutsussa tehdään funktiole annettavalle parametrille T_ec 3 tunnin siirto taaksepäin.
  float itsFunctionDataTimeOffsetInHours = 0;
};  // class NFmiAreaMaskImpl

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiAreaMaskImpl::IsMasked(int /* theIndex */) const
{
  return false;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiAreaMaskImpl::IsEnabled() const
{
  return fEnabled;
}
// ----------------------------------------------------------------------
/*!
 * \param theNewState Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::Enable(bool theNewState)
{
  fEnabled = theNewState;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiCalculationCondition &NFmiAreaMaskImpl::Condition() const
{
  return itsMaskCondition;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline std::shared_ptr<NFmiFastQueryInfo> NFmiAreaMaskImpl::Info()
{
  return std::shared_ptr<NFmiFastQueryInfo>();
}

inline void NFmiAreaMaskImpl::UpdateInfo(std::shared_ptr<NFmiFastQueryInfo> & /* theInfo */) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiInfoData::Type NFmiAreaMaskImpl::GetDataType() const
{
  return itsDataType;
}
// ----------------------------------------------------------------------
/*!
 * \param theType Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::SetDataType(NFmiInfoData::Type theType)
{
  itsDataType = theType;
}
// ----------------------------------------------------------------------
/*!
 * \param theType Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::MaskType(Type theType)
{
  itsMaskType = theType;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMaskImpl::Type NFmiAreaMaskImpl::MaskType() const
{
  return itsMaskType;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMask *NFmiAreaMaskImpl::Clone() const
{
  return new NFmiAreaMaskImpl(*this);
}
// ----------------------------------------------------------------------
/*!
 * \param newOperator Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::PostBinaryOperator(BinaryOperator newOperator)
{
  itsPostBinaryOperator = newOperator;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMaskImpl::BinaryOperator NFmiAreaMaskImpl::PostBinaryOperator() const
{
  return itsPostBinaryOperator;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMaskImpl::CalculationOperationType NFmiAreaMaskImpl::GetCalculationOperationType()
    const
{
  return itsCalculationOperationType;
}

// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::SetCalculationOperationType(CalculationOperationType newValue)
{
  itsCalculationOperationType = newValue;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMaskImpl::CalculationOperator NFmiAreaMaskImpl::GetCalculationOperator() const
{
  return itsCalculationOperator;
}

// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::SetCalculationOperator(CalculationOperator newValue)
{
  itsCalculationOperator = newValue;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMaskImpl::MathFunctionType NFmiAreaMaskImpl::GetMathFunctionType() const
{
  return itsMathFunctionType;
}

// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::SetMathFunctionType(MathFunctionType newValue)
{
  itsMathFunctionType = newValue;
}

// ----------------------------------------------------------------------
/*!
 * \param theLowerLimit Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::LowerLimit(double theLowerLimit)
{
  itsMaskCondition.LowerLimit(theLowerLimit);
}

// ----------------------------------------------------------------------
/*!
 * \param theUpperLimit Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::UpperLimit(double theUpperLimit)
{
  itsMaskCondition.UpperLimit(theUpperLimit);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAreaMaskImpl::LowerLimit() const
{
  return itsMaskCondition.LowerLimit();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAreaMaskImpl::UpperLimit() const
{
  return itsMaskCondition.UpperLimit();
}
// ----------------------------------------------------------------------
/*!
 * \param theMaskOperation Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::MaskOperation(FmiMaskOperation theMaskOperation)
{
  itsMaskCondition.Condition(theMaskOperation);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiMaskOperation NFmiAreaMaskImpl::MaskOperation() const
{
  return itsMaskCondition.Condition();
}

// ======================================================================
