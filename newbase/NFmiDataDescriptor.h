// ======================================================================
/*!
 * \file NFmiDataDescriptor.h
 * \brief Interface of class NFmiDataDescriptor
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include <iostream>

//! Undocumented
class NFmiDataDescriptor
{
 public:
  virtual ~NFmiDataDescriptor() {}
  NFmiDataDescriptor() {}
  virtual unsigned long Index() const = 0;
  virtual unsigned long Size() const = 0;

  virtual bool IsActive() const = 0;
  virtual bool SetActivity(bool theActivityState) = 0;
  virtual bool NextActive() = 0;

  virtual std::ostream& Write(std::ostream& file) const = 0;
  virtual std::istream& Read(std::istream& file) = 0;

};  // class NFmiDataDescriptor

// ======================================================================
