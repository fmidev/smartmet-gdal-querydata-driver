// ======================================================================
/*!
 * \file NFmiStatus.h
 * \brief Interface of class NFmiStatus
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiString.h"

//! Undocumented

class NFmiStatus
{
 public:
  virtual ~NFmiStatus() {}
  NFmiStatus();
  NFmiStatus(const NFmiStatus& theStatus);

  NFmiStatus& operator=(const NFmiStatus& theStatus);
  operator bool() const;

  void ErrorLog(NFmiString& theErrorLog);
  void WarningLog(NFmiString& theWarningLog);
  void MessageLog(NFmiString& theMessageLog);

  void ErrorLog(const char* theErrorLog);
  void WarningLog(const char* theWarningLog);
  void MessageLog(const char* theMessageLog);

  bool IsError() const;
  bool IsWarning() const;
  bool IsMessage() const;

  const NFmiString ErrorLog() const;
  const NFmiString WarningLog() const;
  const NFmiString MessageLog() const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 private:
  NFmiString itsErrorLog;
  NFmiString itsWarningLog;
  NFmiString itsMessageLog;

};  // class NFmiStatus

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should probably be const
 */
// ----------------------------------------------------------------------

inline NFmiStatus::operator bool() const
{
  return !itsErrorLog.IsValue();
}

// ======================================================================
