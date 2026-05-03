// ======================================================================
/*!
 * \file NFmiSize.cpp
 * \brief Implementation of class NFmiSize
 */
// ======================================================================
/*!
 * \class NFmiSize
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiSize.h"
#include <macgyver/Exception.h>
#include <fstream>

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiSize::NFmiSize() : itsSize(0), itsIndex(-1) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theSize Undocumented
 */
// ----------------------------------------------------------------------

NFmiSize::NFmiSize(unsigned long theSize) : itsSize(theSize), itsIndex(-1) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theIndex Undocumented
 * \param theSize Undocumented
 */
// ----------------------------------------------------------------------

NFmiSize::NFmiSize(unsigned long theIndex, unsigned long theSize)
    : itsSize(theSize), itsIndex(theIndex)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theSize The object being copied
 * \todo Use initialization lists
 */
// ----------------------------------------------------------------------

NFmiSize::NFmiSize(const NFmiSize& theSize) = default;

NFmiSize& NFmiSize::operator = (const NFmiSize& theSize) = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSize::First()
{
  try
  {
    Reset();
    return Next();
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

unsigned long NFmiSize::GetSize() const
{
  return itsSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiSize::CurrentIndex() const
{
  return itsIndex;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSize::Previous()
{
  return --itsIndex >= 0;
}
// ----------------------------------------------------------------------
/*!
 * \param newSize Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSize::SetSize(unsigned long newSize)
{
  itsSize = newSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const char* NFmiSize::ClassName() const
{
  return "NFmiSize";
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSize::Next()
{
  try
  {
    if (itsIndex + 1 < static_cast<long>(itsSize))
    {
      itsIndex++;
      return true;
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param directionToIter Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSize::Reset(FmiDirection directionToIter)
{
  try
  {
    if (directionToIter == kForward)
      itsIndex = -1;
    else
      itsIndex = GetSize();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSize::SetCurrentIndex(unsigned long theIndex)
{
  try
  {
    if (theIndex < itsSize)
    {
      itsIndex = theIndex;
      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the NFmiSize object to the given putput stream.
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiSize::Write(std::ostream& file) const
{
  try
  {
    file << itsSize << "\n";
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Replace the active NFmiSize with the contents from the given
 * input stream.
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
//----------------------------------------------------------------------------

std::istream& NFmiSize::Read(std::istream& file)
{
  try
  {
    file >> itsSize;
    Reset();
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
