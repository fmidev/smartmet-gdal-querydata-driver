// ======================================================================
/*!
 * \file NFmiSize.h
 * \brief Interface of class NFmiSize
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include <iostream>

//! Undocumenetd
class NFmiSize
{
 public:
  virtual ~NFmiSize() {}
  NFmiSize();
  NFmiSize(const NFmiSize& theSize);
  NFmiSize(unsigned long theSize);
  NFmiSize(unsigned long theIndex, unsigned long theSize);
  NFmiSize& operator = (const NFmiSize& theSize);

  virtual void Reset(FmiDirection directionToIter = kForward);
  virtual bool First();
  virtual bool Next();
  virtual bool Previous();
  virtual long CurrentIndex() const;

  virtual unsigned long GetSize() const;
  virtual void SetSize(unsigned long newSize);
  bool SetCurrentIndex(unsigned long theIndex);

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  virtual const char* ClassName() const;

 protected:
  unsigned long itsSize;
  long itsIndex;

};  // class NFmiSize

// ======================================================================
