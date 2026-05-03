// ======================================================================
/*!
 * \file NFmiStatusPositionString.cpp
 * \brief Implementation of class NFmiStatusPositionString
 */
// ======================================================================
/*!
 * \class NFmiStatusPositionString
 *
 * String with status attribute and start and end positions (given outside).
 * This is utility class used to hold info about smaller parts of bigger
 * string (e.g. NFmiString "EFHK 100009 12012KT CAVOK=") can be devided to
 * smaller parts NFmiStatusPositionString("12012KT",kWindError,13,19) ).
 *
 */
// ======================================================================

#include "NFmiStatusPositionString.h"
#include <iostream>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiStatusPositionString::~NFmiStatusPositionString() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiStatusPositionString::NFmiStatusPositionString()
    : NFmiStatusString(), itsStartPosition(0), itsEndPosition(0)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theString Undocumented
 * \param theStatus Undocumented
 * \param theStartPos Undocumented
 * \param theEndPos Undocumented
 */
// ----------------------------------------------------------------------

NFmiStatusPositionString::NFmiStatusPositionString(const NFmiString& theString,
                                                   long theStatus,
                                                   long theStartPos,
                                                   long theEndPos)
    : NFmiStatusString(theString, theStatus),
      itsStartPosition(theStartPos),
      itsEndPosition(theEndPos)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theString Undocumented
 * \param theStartPos Undocumented
 * \param theEndPos Undocumented
 */
// ----------------------------------------------------------------------

NFmiStatusPositionString::NFmiStatusPositionString(const NFmiStatusString& theString,
                                                   long theStartPos,
                                                   long theEndPos)
    : NFmiStatusString(theString), itsStartPosition(theStartPos), itsEndPosition(theEndPos)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theStatusPositionString The other object being copied
 */
// ----------------------------------------------------------------------

NFmiStatusPositionString::NFmiStatusPositionString(
    const NFmiStatusPositionString& theStatusPositionString)

    = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiStatusPositionString::StartPosition()
{
  return itsStartPosition;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiStatusPositionString::EndPosition()
{
  return itsEndPosition;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiString* NFmiStatusPositionString::Clone() const
{
  return new NFmiStatusPositionString(*this);
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theStatusPositionString The other object being copied
 * \return The object assigned to
 *
 * \bug Since NFmiStatusString::operator= is unknown, one must
 *      protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiStatusPositionString& NFmiStatusPositionString::operator=(
    const NFmiStatusPositionString& theStatusPositionString) = default;

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiStatusPositionString::Write(std::ostream& file) const
{
  file << itsStartPosition << " " << itsEndPosition << " ";
  NFmiStatusString::Write(file);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream& NFmiStatusPositionString::Read(std::istream& file)
{
  file >> itsStartPosition >> itsEndPosition;
  NFmiStatusString::Read(file);
  return file;
}

// ======================================================================
