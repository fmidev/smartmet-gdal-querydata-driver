// ======================================================================
/*!
 * \file NFmiDataModifierConstant.cpp
 * \brief Implementation of class NFmiDataModifierConstant
 */
// ======================================================================
/*!
 * \class NFmiDataModifierConstant
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierConstant.h"
#include <iostream>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierConstant::~NFmiDataModifierConstant() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theConstant Undocumented
 * \param theJoinOperator Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierConstant::NFmiDataModifierConstant(double theConstant,
                                                   FmiJoinOperator theJoinOperator)
    : NFmiDataModifier(theJoinOperator), itsConstant(theConstant)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiDataModifierConstant::FloatValue()
{
  return itsConstant;
}
// ----------------------------------------------------------------------
/*!
 * Write the object to the given stream.
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiDataModifierConstant::WriteOperand(std::ostream& file) const
{
  file << "<operand>" << itsConstant << "</operand>";
  return file;
}

// ======================================================================
