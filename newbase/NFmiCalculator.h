// ======================================================================
/*!
 * \file NFmiCalculator.h
 * \brief Interface of class NFmiCalculator
 */
// ======================================================================

#pragma once

#include "NFmiDataIterator.h"
#include "NFmiDataModifier.h"

//! Undocumented

class NFmiCalculator
{
 public:
  virtual ~NFmiCalculator();
  NFmiCalculator(NFmiQueryInfo* theData, NFmiDataModifier* theDataModifier);
  NFmiCalculator(NFmiDataIterator* theDataIterator, NFmiDataModifier* theDataModifier);

  virtual double FloatValue();
  virtual NFmiCombinedParam* CombinedValue();
  float CalculatedValue();

  void SetData(NFmiQueryInfo* theData);

  void DataIterator(NFmiDataIterator* theIterator);
  NFmiDataIterator* DataIterator();

  void SetDataModifier(NFmiDataModifier* theModifier);
  NFmiDataModifier* DataModifier();
  NFmiDataModifier* CalculatedModifier();

 private:
  NFmiCalculator(const NFmiCalculator& theCalculator);
  NFmiCalculator& operator=(const NFmiCalculator& theCalculator);

  NFmiDataModifier* itsDataModifier;  // ei omista
  NFmiDataIterator* itsDataIterator;  // ei omista
  NFmiQueryInfo* itsData;             // ei omista

};  // class NFmiCalculator

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiCalculator::SetDataModifier(NFmiDataModifier* theModifier)
{
  itsDataModifier = theModifier;
}

// ----------------------------------------------------------------------
/*!
 * \param theIterator Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiCalculator::DataIterator(NFmiDataIterator* theIterator)
{
  itsDataIterator = theIterator;
}

// ----------------------------------------------------------------------
/*!
 * \param theData Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiCalculator::SetData(NFmiQueryInfo* theData)
{
  itsData = theData;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should probably be a const method
 */
// ----------------------------------------------------------------------

inline NFmiDataIterator* NFmiCalculator::DataIterator()
{
  return itsDataIterator;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should probably be a const method
 */
// ----------------------------------------------------------------------

inline NFmiDataModifier* NFmiCalculator::DataModifier()
{
  return itsDataModifier;
}

// ======================================================================
