// ======================================================================
/*!
 * \file NFmiBox.cpp
 * \brief Implementation of class NFmiBox
 */
// ======================================================================
/*!
 * \class NFmiBox
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiBox.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiBox::~NFmiBox()
{
  try
  {
    delete[] itsValues;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newSize Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiBox::Initialize(unsigned long newSize)
{
  try
  {
    itsSize = newSize;
    if (itsValues)
    {
      delete[] itsValues;
      itsValues = nullptr;
    }

    if (itsSize > 0)
      itsValues = new float[itsSize];

    return itsValues ? true : false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiBox::Write(std::ostream& file) const
{
  try
  {
    for (unsigned long i = 0; i < itsSize; i++)
      file << itsValues[i] << " ";

    file << std::endl;

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream& NFmiBox::Read(std::istream& file)
{
  try
  {
    for (unsigned long i = 0; i < itsSize; i++)
      file >> itsValues[i];

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theBox The object being copied
 */
// ----------------------------------------------------------------------

NFmiBox::NFmiBox(NFmiBox& theBox) : itsValues(new float[theBox.itsSize]), itsSize(theBox.itsSize)
{
  try
  {
    for (unsigned long i = 0; i < itsSize; i++)
    {
      itsValues[i] = theBox.itsValues[i];
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theSize Undocumented
 */
// ----------------------------------------------------------------------

NFmiBox::NFmiBox(long theSize) : itsValues(nullptr), itsSize(theSize)
{
  try
  {
    if (itsSize)
      itsValues = new float[itsSize];
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

long NFmiBox::GetSize() const
{
  try
  {
    return itsSize;
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

float& NFmiBox::operator[](long theIndex)
{
  try
  {
    return itsValues[theIndex];
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

long NFmiBox::CalcSize()
{
  return 0;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiBox::IsInside(unsigned long theIndex) const
{
  try
  {
    return (theIndex < itsSize);
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

float NFmiBox::Value(unsigned long theIndex)
{
  try
  {
    return IsInside(theIndex) ? itsValues[theIndex] : kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFillValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

void NFmiBox::Fill(float theFillValue)
{
  try
  {
    for (unsigned long theIndex = 0; theIndex < itsSize; theIndex++)
      itsValues[theIndex] = theFillValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
