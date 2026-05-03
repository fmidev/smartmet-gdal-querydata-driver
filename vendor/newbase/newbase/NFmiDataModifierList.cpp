// ======================================================================
/*!
 * \file NFmiDataModifierList.cpp
 * \brief Implementation of class NFmiDataModifierList
 */
// ======================================================================
/*!
 * \class NFmiDataModifierList
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierList.h"
#include <cmath>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierList::~NFmiDataModifierList() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param thePotentialCombinedParam Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierList::NFmiDataModifierList(NFmiCombinedParam* /* thePotentialCombinedParam */)
    : NFmiDataModifier(), itsReturnValue(), itsList(), itsIter()
{
  itsExpressionType = "list";
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always false
 */
// ----------------------------------------------------------------------
bool NFmiDataModifierList::IsCombinedParam()
{
  return false;
}
// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierList::Add(NFmiDataModifier* theModifier)
{
  return itsList.AddEnd(theModifier) == true;
}

// ----------------------------------------------------------------------
/*!
 * \return Always true
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierList::Reset()
{
  itsIter = itsList.Start();
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierList::Next()
{
  return itsIter.Next() == true;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifier* NFmiDataModifierList::Current()
{
  return itsIter.CurrentPtr();
}
// ----------------------------------------------------------------------
/*
 * \param fDeleteData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierList::Remove(bool fDeleteData)
{
  if (itsIter.Remove(fDeleteData))
    return true;
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierList::Clear(bool fDeleteData)
{
  itsList.Clear(fDeleteData);
}
// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented, unused
 */
// ----------------------------------------------------------------------

void NFmiDataModifierList::Calculate(float /* theParam */)
{
  Reset();
  while (Next())
  {
    itsReturnValue += *Current();
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiDataModifierList::FloatValue()
{
  itsReturnValue = 0;
  Reset();
  while (Next())
  {
    double itemValue = (*Current()).FloatValue();
    if (itemValue == kFloatMissing)
      return kFloatMissing;

    switch (Current()->JoinOperator())
    {
      case kFmiAdd:
        itsReturnValue += itemValue;
        break;
      case kFmiSub:
        itsReturnValue -= itemValue;
        break;
      case kFmiMul:
        itsReturnValue *= itemValue;
        break;
      case kFmiDiv:
        itsReturnValue /= itemValue;
        break;
      case kFmiPow:
        itsReturnValue = pow(itsReturnValue, itemValue);
        break;
      default:
        itsReturnValue += kFloatMissing;
    }
  }
  return itsReturnValue;
}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 * \todo Pitää muuttaa const metodiksi, jos mielii saamaan toimimaan virtuaalisesti.
 */
// ----------------------------------------------------------------------

std::ostream& NFmiDataModifierList::WriteExpressionBody(std::ostream& file)
{
  Reset();
  int i = 0;
  while (Next())
  {
    if (i++ > 0)
      Current()->WriteOperator(file);
    Current()->WriteOperand(file);
  }
  return file;
}

// ======================================================================
