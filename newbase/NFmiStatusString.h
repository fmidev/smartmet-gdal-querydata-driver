// ======================================================================
/*!
 * \file NFmiStatusString.h
 * \brief Interface of class NFmiStatusString
 */
// ======================================================================

#pragma once

#include "NFmiString.h"

//! Undocumented
class NFmiStatusString : public NFmiString
{
 public:
  ~NFmiStatusString();
  NFmiStatusString();
  NFmiStatusString(const NFmiStatusString& theStatusString);
  NFmiStatusString(const NFmiString& theString, long theStatus);

  NFmiStatusString& operator=(const NFmiStatusString& theStausString);

  long Status();
  const NFmiString& String();
  virtual unsigned long ClassId() const { return kNFmiStatusString; };
  virtual NFmiString* Clone() const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 private:
  long itsStatus;

};  // class NFmiStatusString

// ======================================================================
