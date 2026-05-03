// ======================================================================
/*!
 * \file NFmiLevel.h
 * \brief Interface of class NFmiLevel
 */
// ======================================================================

#pragma once

#include "NFmiIndividual.h"
#include "NFmiLevelType.h"

//! Undocumented

class NFmiLevel : public NFmiIndividual
{
 public:
  ~NFmiLevel();
  NFmiLevel();
  NFmiLevel(const NFmiLevel& theLevel);
  NFmiLevel(unsigned long theIdent, const NFmiString& theName, float theLevelValue);
  NFmiLevel(FmiLevelType theLevelType, float theLevelValue);

  bool operator==(const NFmiLevel& theLevel) const;
  bool operator<(const NFmiLevel& theLevel) const;
  NFmiLevel& operator=(const NFmiLevel& theLevel);

  unsigned long LevelTypeId() const;
  float LevelValue() const;
  void LevelValue(float theLevelValue) { itsLevelValue = theLevelValue; }
  FmiLevelType LevelType() const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  virtual const char* ClassName() const;

  bool IsMissing() const;

 private:
  float itsLevelValue;

};  // class NFmiLevel

// ======================================================================
