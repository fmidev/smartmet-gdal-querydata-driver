// ======================================================================
/*!
 * \file NFmiStatusString.cpp
 * \brief Implementation of class NFmiStatusString
 */
// ======================================================================

#include "NFmiStatusString.h"
#include <iostream>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiStatusString::~NFmiStatusString() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiStatusString::NFmiStatusString() : NFmiString(), itsStatus(0) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theString Undocumented
 * \param theStatus Undocumented
 */
// ----------------------------------------------------------------------

NFmiStatusString::NFmiStatusString(const NFmiString &theString, long theStatus)
    : NFmiString(theString), itsStatus(theStatus)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theStatusString The object being copied
 */
// ----------------------------------------------------------------------

NFmiStatusString::NFmiStatusString(const NFmiStatusString &theStatusString)

    = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiStatusString::Status()
{
  return itsStatus;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Remove the extremely ugly C-style cast
 */
// ----------------------------------------------------------------------

const NFmiString &NFmiStatusString::String()
{
  return *static_cast<NFmiString *>(this);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Clone is supposed to return a copy of the original type (?)
 * \todo Remove the C-style cast
 */
// ----------------------------------------------------------------------

NFmiString *NFmiStatusString::Clone() const
{
  return new NFmiStatusString(*this);
}
// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theStatusString The object being copied
 * \return The object assigned to
 * \todo Parent operator= is called, hence self assignment should be
 *       protected
 */
// ----------------------------------------------------------------------

NFmiStatusString &NFmiStatusString::operator=(const NFmiStatusString &theStatusString) = default;

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiStatusString::Write(std::ostream &file) const
{
  file << itsStatus << " ";
  NFmiString::Write(file);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiStatusString::Read(std::istream &file)
{
  file >> itsStatus;
  NFmiString::Read(file);
  return file;
}

// ======================================================================
