// ======================================================================
/*!
 * \file NFmiDataModifierBase.cpp
 * \brief Implementation of class NFmiDataModifierBase
 */
// ======================================================================
/*!
 * \class NFmiDataModifierBase
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierBase.h"
#include <iostream>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierBase::~NFmiDataModifierBase() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theJoinOperator Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierBase::NFmiDataModifierBase(FmiJoinOperator theJoinOperator)
    : itsJoinOperator(theJoinOperator), itsExpressionType("value")
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param other The other object being copied
 */
// ----------------------------------------------------------------------

NFmiDataModifierBase::NFmiDataModifierBase(const NFmiDataModifierBase &other)

    = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

FmiJoinOperator NFmiDataModifierBase::JoinOperator()
{
  return itsJoinOperator;
}
// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiDataModifierBase::Write(std::ostream &file)
{
  WriteExpressionStart(file);
  WriteExpressionBody(file);
  WriteExpressionEnd(file);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiDataModifierBase::WriteOperator(std::ostream &file) const
{
  file << "<operator>";
  switch (itsJoinOperator)
  {
    case kFmiVal:
      file << " = ";
      break;
    case kFmiAdd:
      file << " + ";
      break;
    case kFmiSub:
      file << " - ";
      break;
    case kFmiMul:
      file << " * ";
      break;
    case kFmiDiv:
      file << " / ";
      break;
    case kFmiPow:
      file << " ** ";
      break;
    default:
      file << " ? ";
  }
  file << "</operator>";
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 * \bug This really does nothing sensible since the value is not output
 */
// ----------------------------------------------------------------------

std::ostream &NFmiDataModifierBase::WriteOperand(std::ostream &file) const
{
  file << "<operand>";
  // file << FloatValue();
  file << "</operand>";
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiDataModifierBase::WriteExpressionStart(std::ostream &file) const
{
  file << std::endl << "<expression type='" << static_cast<char *>(itsExpressionType) << "'>";
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiDataModifierBase::WriteExpressionEnd(std::ostream &file) const
{
  file << "</expression>";
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 * \bug Since WriteOperand does nothing sensible, this does not make sense either
 */
// ----------------------------------------------------------------------

std::ostream &NFmiDataModifierBase::WriteExpressionBody(std::ostream &file)
{
  WriteOperand(file);
  return file;
}

// ======================================================================
