// ======================================================================
/*!
 * \file NFmiTimePeriod.cpp
 * \brief Implementation of class NFmiTimePeriod
 */
// ======================================================================
/*!
 * \class NFmiTimePeriod
 *
 * Undocumented
 */
// ======================================================================

#include "NFmiTimePeriod.h"

#include "NFmiVersion.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiTimePerioid::NFmiTimePerioid()
    : itsYears(0),
      itsMonths(0),
      itsDays(0),
      itsHours(0),
      itsMinutes(0),
      itsSeconds(0),
      itsMicroSeconds(0)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param years Undocumented
 * \param months Undocumented
 * \param days Undocumented
 * \param hours Undocumented
 * \param minutes Undocumented
 * \param seconds Undocumented
 * \param microSeconds Undocumented
 */
// ----------------------------------------------------------------------

NFmiTimePerioid::NFmiTimePerioid(
    long years, long months, long days, long hours, long minutes, long seconds, long microSeconds)
    : itsYears(years),
      itsMonths(months),
      itsDays(days),
      itsHours(hours),
      itsMinutes(minutes),
      itsSeconds(seconds),
      itsMicroSeconds(microSeconds)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param minutes Undocumented
 */
// ----------------------------------------------------------------------

NFmiTimePerioid::NFmiTimePerioid(long minutes)
    : itsYears(0),
      itsMonths(0),
      itsDays(0),
      itsHours(0),
      itsMinutes(minutes),
      itsSeconds(0),
      itsMicroSeconds(0)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theTimePerioid
 */
// ----------------------------------------------------------------------

NFmiTimePerioid::NFmiTimePerioid(const NFmiTimePerioid& theTimePerioid)
    : itsYears(theTimePerioid.Year()),
      itsMonths(theTimePerioid.Month()),
      itsDays(theTimePerioid.Day()),
      itsHours(theTimePerioid.Hour()),
      itsMinutes(theTimePerioid.Minute()),
      itsSeconds(theTimePerioid.Second()),
      itsMicroSeconds(theTimePerioid.MicroSecond())
{
}

// ----------------------------------------------------------------------
/*!
 * \todo Unfinished ???
 */
// ----------------------------------------------------------------------

void NFmiTimePerioid::CalculatePerioid()
{
  try
  {
    itsSeconds += itsMicroSeconds / 100;
    itsMicroSeconds = itsMicroSeconds % 100;

    itsMinutes += itsSeconds / 60;
    itsSeconds = itsSeconds % 60;

    itsHours += itsMinutes / 60;
    itsMinutes = itsMinutes % 60;

    itsDays += itsHours / 24;
    itsHours = itsHours % 24;

    // Kesken
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theTimePerioid The other period being copied
 */
// ----------------------------------------------------------------------

NFmiTimePerioid& NFmiTimePerioid::operator=(const NFmiTimePerioid& theTimePerioid)
{
  try
  {
    itsYears = theTimePerioid.itsYears;
    itsMonths = theTimePerioid.itsMonths;
    itsDays = theTimePerioid.itsDays;
    itsHours = theTimePerioid.itsHours;
    itsMinutes = theTimePerioid.itsMinutes;
    itsSeconds = theTimePerioid.itsSeconds;
    itsMicroSeconds = theTimePerioid.itsMicroSeconds;

    CalculatePerioid();

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Addition operator
 *
 * \param theTimePerioid The other period being added
 * \return The period added to
 */
// ----------------------------------------------------------------------

NFmiTimePerioid& NFmiTimePerioid::operator+=(const NFmiTimePerioid& theTimePerioid)
{
  try
  {
    itsYears += theTimePerioid.itsYears;
    itsMonths += theTimePerioid.itsMonths;
    itsDays += theTimePerioid.itsDays;
    itsHours += theTimePerioid.itsHours;
    itsMinutes += theTimePerioid.itsMinutes;
    itsSeconds += theTimePerioid.itsSeconds;
    itsMicroSeconds += theTimePerioid.itsMicroSeconds;

    CalculatePerioid();

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator from minutes
 *
 * \param theMinutes Undocumented
 * \return The period assigned to
 */
// ----------------------------------------------------------------------

NFmiTimePerioid& NFmiTimePerioid::operator=(const long theMinutes)
{
  try
  {
    itsYears = 0;
    itsMonths = 0;
    itsDays = 0;
    itsHours = 0;
    itsMinutes = theMinutes;
    itsSeconds = 0;
    itsMicroSeconds = 0;

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

std::ostream& NFmiTimePerioid::Write(std::ostream& file) const
{
  try
  {
    // We trust all data to be version 6-7 by now
    if (DefaultFmiInfoVersion <= 2)
    {
      file << itsMinutes << std::endl;

      return file;
    }

    file << itsYears << " ";
    file << itsMonths << " ";
    file << itsDays << " ";
    file << itsHours << " ";
    file << itsMinutes << " ";
    file << itsSeconds << " ";
    file << itsMicroSeconds << std::endl;

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

std::istream& NFmiTimePerioid::Read(std::istream& file)
{
  try
  {
    // We trust all data to be version 6-7 by now
    if (DefaultFmiInfoVersion <= 2)
    {
      itsYears = 0;
      itsMonths = 0;
      itsDays = 0;
      itsHours = 0;
      file >> itsMinutes;
      itsSeconds = 0;
      itsMicroSeconds = 0;
      return file;
    }

    file >> itsYears;
    file >> itsMonths;
    file >> itsDays;
    file >> itsHours;
    file >> itsMinutes;
    file >> itsSeconds;
    file >> itsMicroSeconds;

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
