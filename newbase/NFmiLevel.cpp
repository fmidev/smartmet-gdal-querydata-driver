// ======================================================================
/*!
 * \file NFmiLevel.cpp
 * \brief Implementation of class NFmiLevel
 */
// ======================================================================
/*!
 * \class NFmiLevel
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiLevel.h"
#include "NFmiLevelType.h"
#include <macgyver/Exception.h>
#include <iostream>
#include <string>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLevel::~NFmiLevel() = default;
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiLevel::LevelTypeId() const
{
  try
  {
    return GetIdent();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiLevel::LevelValue() const
{
  try
  {
    return itsLevelValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

FmiLevelType NFmiLevel::LevelType() const
{
  try
  {
    return FmiLevelType(GetIdent());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theLevel The object being compared with
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiLevel::operator==(const NFmiLevel& theLevel) const
{
  try
  {
    return (itsLevelValue == theLevel.itsLevelValue && GetIdent() == theLevel.GetIdent());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiLevel::operator<(const NFmiLevel& theLevel) const
{
  try
  {
    if (itsLevelValue != theLevel.itsLevelValue)
      return itsLevelValue < theLevel.itsLevelValue;

    if (GetIdent() != theLevel.GetIdent())
      return GetIdent() < theLevel.GetIdent();

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const char* NFmiLevel::ClassName() const
{
  return "NFmiLevel";
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLevel::NFmiLevel() : NFmiIndividual(), itsLevelValue(kFloatMissing) {}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theIdent Undocumented
 * \param theName Undocumented
 * \param theLevelValue Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevel::NFmiLevel(unsigned long theIdent, const NFmiString& theName, float theLevelValue)
    : NFmiIndividual(theIdent, theName), itsLevelValue(theLevelValue)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theLevelType Undocumented
 * \param theLevelValue Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevel::NFmiLevel(FmiLevelType theLevelType, float theLevelValue)
    : NFmiIndividual(theLevelType), itsLevelValue(theLevelValue)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theLevel Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevel::NFmiLevel(const NFmiLevel& theLevel)
    : NFmiIndividual(theLevel.GetIdent(), theLevel.GetName()), itsLevelValue(theLevel.itsLevelValue)
{
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theLevel The other object being assigned
 * \return The assigned object
 * \todo NFmiIndividual::operator= is unknown, hence self-assignment
 *       should be protected.
 * \todo Use static_cast instead of C-style cast
 */
// ----------------------------------------------------------------------

NFmiLevel& NFmiLevel::operator=(const NFmiLevel& theLevel) = default;

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiLevel::Write(std::ostream& file) const
{
  try
  {
    NFmiIndividual::Write(file);
    file << itsLevelValue << std::endl;
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new NFmiLevel contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream& NFmiLevel::Read(std::istream& file)
{
  try
  {
    NFmiIndividual::Read(file);
    file >> itsLevelValue;
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return true if the level is in uninitialized state
 *
 * The values are what where used in the old static NFmiLevel::gMissingLevel
 * object.
 */
// ----------------------------------------------------------------------

bool NFmiLevel::IsMissing() const
{
  try
  {
    return (GetIdent() == 0 && !GetName().IsValue() && itsLevelValue == 0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
