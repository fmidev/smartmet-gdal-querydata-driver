// ======================================================================
/*!
 * \file NFmiDataModifierBoolean.cpp
 * \brief Implementation of class NFmiDataModifierBoolean
 */
// ======================================================================
/*!
 * \class NFmiDataModifierBoolean
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierBoolean.h"
#include <iostream>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierBoolean::~NFmiDataModifierBoolean() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theCondition Undocumented
 * \param theFirstValue Undocumented
 * \param theSecondValue Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierBoolean::NFmiDataModifierBoolean(FmiModifierBoolOperations theCondition,
                                                 NFmiDataModifier* theFirstValue,
                                                 NFmiDataModifier* theSecondValue)
    : itsSecondValue(theSecondValue),
      itsFirstValue(theFirstValue),
      itsCondition(theCondition),
      itsBooleanValue()
{
  itsExpressionType = "boolean";
}

// ----------------------------------------------------------------------
/*!
 * Automatic conversion to a boolean value.
 *
 * \bug The operator has no defined return type
 */
// ----------------------------------------------------------------------

NFmiDataModifierBoolean::operator bool()
{
  return BooleanValue();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Remove the C-style casts
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierBoolean::BooleanValue()
{
  if (itsFirstValue && itsSecondValue)
  {
    switch (itsCondition)
    {
      case kFmiModifierValueLessThan:
        return static_cast<double>(*itsFirstValue) < static_cast<double>(*itsSecondValue);
      case kFmiModifierValueGreaterThan:
        return static_cast<double>(*itsFirstValue) > static_cast<double>(*itsSecondValue);
      case kFmiModifierValueLessOrEqualThan:
        return static_cast<double>(*itsFirstValue) <= static_cast<double>(*itsSecondValue);
      case kFmiModifierValueGreaterOrEqualThan:
        return static_cast<double>(*itsFirstValue) >= static_cast<double>(*itsSecondValue);
      case kFmiModifierValueIsEqual:
        return static_cast<double>(*itsFirstValue) == static_cast<double>(*itsSecondValue);
    }
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiDataModifierBoolean::WriteOperator(std::ostream& file) const
{
  file << std::endl << "<operator type='boolean'>";
  switch (itsCondition)
  {
    case kFmiModifierValueLessThan:
      file << "LessThan";
      break;
    case kFmiModifierValueGreaterThan:
      file << "GreaterThan";
      break;
    case kFmiModifierValueLessOrEqualThan:
      file << "LessOrEqualThan";
      break;
    case kFmiModifierValueGreaterOrEqualThan:
      file << "GreaterOrEqualThan";
      break;
    case kFmiModifierValueIsEqual:
      file << "IsEqual";
      break;
    default:
      file << " Boolean ? ";
  }
  file << "</operator>";
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiDataModifierBoolean::WriteExpressionBody(std::ostream& file)
{
  itsFirstValue->Write(file);
  WriteOperator(file);
  itsSecondValue->Write(file);
  return file;
}

// ======================================================================
