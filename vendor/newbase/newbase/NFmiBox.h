// ======================================================================
/*!
 * \file NFmiBox.h
 * \brief Interface of class NFmiBox
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiParameterName.h"
#include <iostream>

//! Undocumented

class NFmiBox
{
 public:
  virtual ~NFmiBox();
  NFmiBox(NFmiBox &theBox);
  NFmiBox(long theSize = 0);

  bool Initialize(unsigned long newSize = 0);
  long GetSize() const;
  void Fill(float theFillValue);
  float Value(unsigned long theIndex);

  float &operator[](long theIndex);

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

 protected:
  virtual long CalcSize();
  bool IsInside(unsigned long theIndex) const;

  float *itsValues;
  unsigned long itsSize;

 private:
  NFmiBox &operator=(const NFmiBox &theBox);

  friend class NFmiBoxIterator;

};  // class NFmiBox

// ----------------------------------------------------------------------
/*!
 * Output operator for NFmiBox class
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiBox &ob)
{
  return ob.Write(file);
}
// ----------------------------------------------------------------------
/*!
 * Input operator for NFmiBox class
 *
 * \param file The input strem to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiBox &ob)
{
  return ob.Read(file);
}

// ======================================================================
