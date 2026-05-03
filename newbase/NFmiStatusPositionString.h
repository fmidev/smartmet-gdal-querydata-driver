// ======================================================================
/*!
 * \file NFmiStatusPositionString.h
 * \brief Interface of class NFmiStatusPositionString
 */
// ======================================================================

#pragma once

#include "NFmiStatusString.h"

//! Undocumented
class NFmiStatusPositionString : public NFmiStatusString
{
 public:
  ~NFmiStatusPositionString();
  NFmiStatusPositionString();
  NFmiStatusPositionString(const NFmiStatusPositionString& theStatusPositionString);
  NFmiStatusPositionString(const NFmiString& theString,
                           long theStatus,
                           long theStartPos,
                           long theEndPos);
  NFmiStatusPositionString(const NFmiStatusString& theString, long theStartPos, long theEndPos);

  NFmiStatusPositionString& operator=(const NFmiStatusPositionString& theStatusPositionString);

  long StartPosition();
  long EndPosition();
  virtual unsigned long ClassId() const;
  virtual NFmiString* Clone() const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 private:
  long itsStartPosition;
  long itsEndPosition;

};  // class NFmiStatusPositionString

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiStatusPositionString::ClassId() const
{
  return kNFmiStatusPositionString;
}

// ======================================================================
