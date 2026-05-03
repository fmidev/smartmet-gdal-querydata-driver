// ======================================================================
/*!
 * \file NFmiSpaceDescriptor.h
 * \brief Interface of class NFmiSpaceDescriptor
 */
// ======================================================================
/*!
 * \class NFmiSpaceDescriptor
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"
#include "NFmiHPlaceDescriptor.h"
#include "NFmiVPlaceDescriptor.h"

// ÄLÄ KÄYTÄ TÄTÄ, TESTI LUOKKA/18.2.2000/Marko

//! Undocumented
class NFmiSpaceDescriptor : public NFmiDataDescriptor
{
 public:
  //! Void constructor
  NFmiSpaceDescriptor() : itsLevels(0), itsPlaces(0) {}
  bool NextLevel() const { return itsLevels->Next(); }
  bool NextPlace() const { return itsPlaces->Next(); }

 private:
  NFmiVPlaceDescriptor* itsLevels;
  NFmiHPlaceDescriptor* itsPlaces;

};  // class NFmiSpaceDescriptor

// ======================================================================
