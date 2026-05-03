// ======================================================================
/*!
 * \file NFmiRadarStation.cpp
 * \brief Implementation of class NFmiRadarStation
 */
// ----------------------------------------------------------------------
/*!
 * \class NFmiRadarStation
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiRadarStation.h"
#include <macgyver/Exception.h>
#include <fstream>

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiRadarStation::~NFmiRadarStation() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiRadarStation::NFmiRadarStation()
    : NFmiStation(),
      itsResolution(static_cast<unsigned long>(kFloatMissing)),
      itsXNumber(static_cast<unsigned long>(kFloatMissing)),
      itsYNumber(static_cast<unsigned long>(kFloatMissing))
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theStation The object being copied
 */
// ----------------------------------------------------------------------

NFmiRadarStation::NFmiRadarStation(const NFmiRadarStation &theStation)

    = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param station Undocumented
 * \param resolution Undocumented
 * \param theXNumber Undocumented
 * \param theYNumber Undocumented
 */
// ----------------------------------------------------------------------

NFmiRadarStation::NFmiRadarStation(NFmiStation &station,
                                   unsigned long resolution,
                                   unsigned long theXNumber,
                                   unsigned long theYNumber)
    : NFmiStation(station),
      itsResolution(resolution),
      itsXNumber(theXNumber),
      itsYNumber(theYNumber)
{
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theStation The object being copied
 * \return The object assigned to
 * \todo Remove the ugly C-style cast
 * \todo Since NFmiStation::operator= is unknown, we should protect
 *       from self assignment
 */
// ----------------------------------------------------------------------

NFmiRadarStation &NFmiRadarStation::operator=(const NFmiRadarStation &theStation)
{
  try
  {
    NFmiStation::operator=(*static_cast<const NFmiStation *>(&theStation));
    itsResolution = theStation.itsResolution;
    itsXNumber = theStation.itsXNumber;
    itsYNumber = theStation.itsYNumber;

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 * \todo Remove the ugly C-style casts
 */
// ----------------------------------------------------------------------

bool NFmiRadarStation::IsEqual(const NFmiSortable &theLocation) const
{
  try
  {
    return (NFmiStation::IsEqual(theLocation) &&
            (Resolution() ==
             (static_cast<const NFmiRadarStation *>(static_cast<const NFmiLocation *>(&theLocation))
                  ->Resolution())) &&
            (XNumber() ==
             (static_cast<const NFmiRadarStation *>(static_cast<const NFmiLocation *>(&theLocation))
                  ->XNumber())) &&
            (YNumber() ==
             (static_cast<const NFmiRadarStation *>(static_cast<const NFmiLocation *>(&theLocation))
                  ->YNumber())));
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
 * \param file The output stream to writeto
 * \return The output stream written to
 * \todo Remove the C-style casts
 */
// ----------------------------------------------------------------------

std::ostream &NFmiRadarStation::Write(std::ostream &file) const
{
  try
  {
    NFmiStation::Write(file);

    // We trust everything to be version 6 or 7 by now
    if (DefaultFmiInfoVersion >= 4)
    {
      file << itsResolution << " ";
      file << static_cast<unsigned int>(itsXNumber) << " " << static_cast<unsigned int>(itsYNumber)
           << " ";
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
 * \todo Remove the ugly C-style casts
 */
// ----------------------------------------------------------------------

std::istream &NFmiRadarStation::Read(std::istream &file)
{
  try
  {
    NFmiStation::Read(file);

    itsResolution = static_cast<unsigned long>(kFloatMissing);
    itsXNumber = static_cast<unsigned long>(kFloatMissing);
    itsYNumber = static_cast<unsigned long>(kFloatMissing);

    // We trust everything to be version 6 or 7 by now
    if (DefaultFmiInfoVersion >= 4)
    {
      file >> itsResolution;
      file >> itsXNumber;
      file >> itsYNumber;
    }

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
