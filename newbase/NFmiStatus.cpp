// ======================================================================
/*!
 * \file NFmiStatus.cpp
 * \brief Implementation of class NFmiStatus
 */
// ======================================================================

#include "NFmiStatus.h"

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiStatus::NFmiStatus() : itsErrorLog(), itsWarningLog(), itsMessageLog() {}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theStatus The other object being copied
 */
// ----------------------------------------------------------------------

NFmiStatus::NFmiStatus(const NFmiStatus& theStatus)

    = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStatus::IsWarning() const
{
  return itsWarningLog.IsValue();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStatus::IsError() const
{
  return itsErrorLog.IsValue();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStatus::IsMessage() const
{
  return itsMessageLog.IsValue();
}
// ----------------------------------------------------------------------
/*!
 * \param theErrorLog Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStatus::ErrorLog(NFmiString& theErrorLog)
{
  itsErrorLog = theErrorLog;
}
// ----------------------------------------------------------------------
/*!
 * \param theWarningLog Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStatus::WarningLog(NFmiString& theWarningLog)
{
  itsWarningLog = theWarningLog;
}
// ----------------------------------------------------------------------
/*!
 * \param theMessageLog Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStatus::MessageLog(NFmiString& theMessageLog)
{
  itsMessageLog = theMessageLog;
}
// ----------------------------------------------------------------------
/*!
 * \param theErrorLog Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStatus::ErrorLog(const char* theErrorLog)
{
  itsErrorLog = theErrorLog;
}
// ----------------------------------------------------------------------
/*!
 * \param theWarningLog Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStatus::WarningLog(const char* theWarningLog)
{
  itsWarningLog = theWarningLog;
}
// ----------------------------------------------------------------------
/*!
 * \param theMessageLog Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStatus::MessageLog(const char* theMessageLog)
{
  itsMessageLog = theMessageLog;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiStatus::ErrorLog() const
{
  return itsErrorLog;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiStatus::WarningLog() const
{
  return itsWarningLog;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiStatus::MessageLog() const
{
  return itsMessageLog;
}
// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theStatus The other object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiStatus& NFmiStatus::operator=(const NFmiStatus& theStatus) = default;

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiStatus::Write(std::ostream& file) const
{
  file << itsErrorLog;
  file << itsWarningLog;
  file << itsMessageLog;
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

std::istream& NFmiStatus::Read(std::istream& file)
{
  file >> itsErrorLog;
  file >> itsWarningLog;
  file >> itsMessageLog;
  return file;
}

// ======================================================================
