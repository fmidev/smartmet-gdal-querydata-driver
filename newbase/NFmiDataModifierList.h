// ======================================================================
/*!
 * \file NFmiDataModifierList.h
 * \brief Interface of class NFmiDataModifierList
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiPtrList.h"

//! Undocumented
class NFmiDataModifierList : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierList();
  // NFmiDataModifier(const NFmiDataModifier & theModier); // compiler generates
  NFmiDataModifierList(NFmiCombinedParam* thePotentialCombinedParam = 0);

  bool Add(NFmiDataModifier* theModifier);
  virtual void Clear();

  bool IsCombinedParam();

  bool BoolOperation(float);
  virtual float FloatOperation(float theValue);

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  using NFmiDataModifier::WriteExpressionBody;
  virtual float CalculationResult();
  virtual void Calculate(float NotInUse);
  virtual std::ostream& WriteExpressionBody(std::ostream& file);

 public:
  virtual double FloatValue();
  bool Reset();
  bool Next();
  NFmiDataModifier* Current();
  bool Remove(bool fDeleteData = false);
  void Clear(bool fDeleteData = false);
  unsigned long NumberOfItems();

 protected:
  double itsReturnValue;
  NFmiPtrList<NFmiDataModifier> itsList;
  NFmiPtrList<NFmiDataModifier>::Iterator itsIter;

};  // class NFmiDataModifierList

// ----------------------------------------------------------------------
/*!
 * return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataModifierList::BoolOperation(float)
{
  return false;
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifierList::FloatOperation(float theValue)
{
  return theValue;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifierList::CalculationResult()
{
  return static_cast<float>(itsReturnValue);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifierList::Clear() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiDataModifierList::NumberOfItems()
{
  return itsList.NumberOfItems();
}

// ======================================================================
