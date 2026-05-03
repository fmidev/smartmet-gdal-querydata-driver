// ======================================================================
/*!
 * \file NFmiIndividual.cpp
 * \brief Implementation of class NFmiIndividual
 */
// ======================================================================
/*!
 * \class NFmiIndividual
 *
 * NFmiIndividual whose purpose is to provide a derived class with
 * a name and a number.
 *
 * The class is old fashioned and should not be used unless necessary.
 *
 */
// ======================================================================

#include "NFmiIndividual.h"
#include "NFmiVersion.h"
#include <macgyver/Hash.h>
#include <macgyver/Exception.h>
#include <fstream>

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theIndividual The other object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiIndividual& NFmiIndividual::operator=(const NFmiIndividual& theIndividual)
{
  try
  {
    itsName = theIndividual.itsName;
    itsIdent = theIndividual.itsIdent;
    return *this;
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

std::ostream& NFmiIndividual::Write(std::ostream& file) const
{
  try
  {
    file << itsIdent << std::endl;
    // We trust all data to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 2)
    {
      file << itsName;
    }
    else
    {
      file << itsIdent << " " << itsName.GetCharPtr() << " ";
    }
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 * \todo Kill the cat
 */
// ----------------------------------------------------------------------

std::istream& NFmiIndividual::Read(std::istream& file)
{
  try
  {
    file >> itsIdent;

    // We trust all data to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 6)
    {
      file >> itsName;
    }
    else
    {
      char tmp[255];
      file >> tmp;

      itsName = tmp;
    }

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiIndividual::SetContents(long ident, NFmiString name)
{
  try
  {
    itsIdent = ident;
    itsName = name;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for the individual
 */
// ----------------------------------------------------------------------

std::size_t NFmiIndividual::HashValue() const
{
  try
  {
    std::size_t hash = itsName.HashValue();
    Fmi::hash_combine(hash, Fmi::hash_value(itsIdent));
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
