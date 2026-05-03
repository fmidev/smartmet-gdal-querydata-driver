// ======================================================================
/*!
 * \file NFmiLevelBag.h
 * \brief Interface of class NFmiLevelBag
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiLevel.h"
#include "NFmiSize.h"
#include "NFmiString.h"

class NFmiLevel;

//! Undocumented
class NFmiLevelBag : public NFmiSize
{
 public:
  virtual ~NFmiLevelBag();
  NFmiLevelBag();
  NFmiLevelBag(const NFmiLevelBag &theLevelBag);
  NFmiLevelBag(NFmiLevel *theLevelArray, unsigned long numOfLevels);

  NFmiLevelBag(FmiLevelType theLevelType, float theMinValue, float theMaxValue, float theStep);

  NFmiLevelBag &operator=(const NFmiLevelBag &theLevelBag);
  bool operator==(const NFmiLevelBag &theLevelBag) const;

  bool IsStep();
  NFmiLevel *LevelMinValue();
  NFmiLevel *LevelMaxValue();

  NFmiLevel *Level() const;
  NFmiLevel *Level(unsigned long theIndex) const;
  bool Level(const NFmiLevel &theLevel);
  bool AddLevel(const NFmiLevel &theLevel);

  virtual const NFmiLevelBag Combine(const NFmiLevelBag &theBag) const;
  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual const char *ClassName() const;

 private:
  NFmiLevel *itsLevels;
  float itsStep;

};  // class NFmiLevelBag

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiLevelBag
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiLevelBag &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiLevelBag
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiLevelBag &ob)
{
  return ob.Read(file);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiLevelBag::IsStep()
{
  return true;
  // return (itsStep >=0 ? true : false);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel *NFmiLevelBag::LevelMinValue()
{
  return &itsLevels[0];
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel *NFmiLevelBag::LevelMaxValue()
{
  return &itsLevels[itsSize - 1];
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel *NFmiLevelBag::Level() const
{
  //  return &(itsLevels[itsIndex]);
  if (itsIndex >= 0 && itsIndex < static_cast<long>(GetSize()))
    return &(itsLevels[itsIndex]);
  else
    return 0;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel *NFmiLevelBag::Level(unsigned long theIndex) const
{
  if (theIndex < GetSize())
    return &(itsLevels[theIndex]);
  else
    return 0;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiLevelBag::ClassName() const
{
  return "NFmiLevelBag";
}

// ======================================================================
