// ======================================================================
/*!
 * \file NFmiParamDataModifier.h
 * \brief Interface of class NFmiParamDataModifier
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataIdent;
class NFmiLevel;
class NFmiDataModifierList;

//! Tämä modifier on tehty parametri ja level kohtaiseksi. Match-metodilla kysytään onko haluttu
//! modifier.

class NFmiParamDataModifier : public NFmiDataModifier
{
 public:
  virtual ~NFmiParamDataModifier();
  // NFmiDataModifier(const NFmiDataModifier & theModier); // compiler generated
  NFmiParamDataModifier(NFmiDataIdent* theParam,
                        NFmiLevel* theLevel = 0,
                        FmiJoinOperator theJoinOperator = kFmiAdd);

  virtual std::ostream& WriteOperand(std::ostream& file) const;
  virtual bool BoolOperation(float);
  virtual float FloatOperation(float theValue);

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;

  virtual float CalculationResult();
  virtual void Calculate(float);

  void Clear();

  bool Match(const NFmiDataIdent& theParam, const NFmiLevel* theLevel);
  bool AddSubModifier(NFmiDataModifier* theModifier);
  NFmiDataModifierList* SubModifiers();
  NFmiDataIdent* Param();
  NFmiLevel* Level();

 protected:
  NFmiDataIdent* itsParam;           // Omistaa/tuhoaa
  NFmiLevel* itsLevel;               // Omistaa/tuhoaa
  NFmiDataModifierList* itsSubList;  // tässä voi olla mitä tahansa modifiereita esim. max tai min
                                     // jne. // Omistaa/tuhoaa

 private:
  NFmiParamDataModifier(const NFmiParamDataModifier& theMod);
  NFmiParamDataModifier& operator=(const NFmiParamDataModifier& theMod);

};  // class NFmiParamDataModifier

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiDataIdent* NFmiParamDataModifier::Param()
{
  return itsParam;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel* NFmiParamDataModifier::Level()
{
  return itsLevel;
}

// ======================================================================
