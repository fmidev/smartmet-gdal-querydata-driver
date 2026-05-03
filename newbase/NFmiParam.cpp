// ======================================================================
/*!
 * \file NFmiParam.cpp
 * \brief Implementation for class NFmiParam
 */
// ======================================================================
/*!
 * \class NFmiParam
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiParam.h"
#include <macgyver/Exception.h>
#include <fstream>

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * \param theIdent Undocumented
 * \param theName Undocumented
 * \param theMinValue Undocumented
 * \param theMaxValue Undocumented
 * \param theScale Undocumented
 * \param theBase Undocumented
 * \param thePrecision Undocumented
 * \param theInterpolationMethod Undocumented
 */
// ----------------------------------------------------------------------

NFmiParam::NFmiParam(unsigned long theIdent,
                     const NFmiString &theName,
                     double theMinValue,
                     double theMaxValue,
                     float theScale,
                     float theBase,
                     const NFmiString thePrecision,
                     FmiInterpolationMethod theInterpolationMethod)
    : NFmiIndividual(theIdent, theName),
      itsMinValue(theMinValue),
      itsMaxValue(theMaxValue),
      itsScale(theScale),
      itsBase(theBase),
      itsInterpolationMethod(theInterpolationMethod),
      itsPrecision(thePrecision)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 */
// ----------------------------------------------------------------------

NFmiParam::NFmiParam(const NFmiParam &theParam)
    : NFmiIndividual(theParam.GetIdent(), theParam.GetName()),
      itsMinValue(theParam.itsMinValue),
      itsMaxValue(theParam.itsMaxValue),
      itsScale(theParam.itsScale),
      itsBase(theParam.itsBase),
      itsInterpolationMethod(theParam.itsInterpolationMethod),
      itsPrecision(theParam.itsPrecision)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiParam &NFmiParam::operator=(const NFmiParam &theParam)
{
  try
  {
    NFmiIndividual::operator=(*(static_cast<const NFmiIndividual *>(&theParam)));
    itsMinValue = theParam.itsMinValue;
    itsMaxValue = theParam.itsMaxValue;
    itsScale = theParam.itsScale;
    itsBase = theParam.itsBase;
    itsPrecision = theParam.itsPrecision;
    itsInterpolationMethod = theParam.itsInterpolationMethod;
    return *this;
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

float NFmiParam::Scale() const
{
  try
  {
    if (itsScale == kFloatMissing)
      return 1;

    return itsScale;
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

float NFmiParam::Base() const
{
  try
  {
    if (itsBase == kFloatMissing)
      return 0;

    return itsBase;
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

std::ostream &NFmiParam::Write(std::ostream &file) const
{
  try
  {
    NFmiIndividual::Write(file);

    file << itsMinValue << " " << itsMaxValue << " " << static_cast<int>(itsInterpolationMethod)
         << " ";

    // We trust everything to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 4)
    {
      file << itsScale << " " << itsBase << " " << itsPrecision;
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
 */
// ----------------------------------------------------------------------

std::istream &NFmiParam::Read(std::istream &file)
{
  try
  {
    NFmiIndividual::Read(file);
    unsigned long theInterpolationMethod;

    file >> itsMinValue >> itsMaxValue >> theInterpolationMethod;
    itsInterpolationMethod = FmiInterpolationMethod(theInterpolationMethod);

    // We trust everything to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 4)
    {
      file >> itsScale >> itsBase;
      file >> itsPrecision;
    }

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
