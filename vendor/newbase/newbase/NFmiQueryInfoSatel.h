// ======================================================================
/*!
 * \file NFmiQueryInfoSatel.h
 * \brief Interface of class NFmiQueryInfoSatel
 */
// ======================================================================

#pragma once

#include "NFmiQueryInfo.h"

class NFmiQueryData;

//! Undocumented

class NFmiQueryInfoSatel : public NFmiQueryInfo
{
 public:
  ~NFmiQueryInfoSatel();
  NFmiQueryInfoSatel();
  NFmiQueryInfoSatel(const NFmiQueryInfoSatel& theInfo);

  NFmiQueryInfoSatel(const NFmiParamDescriptor& theParamDescriptor,
                     const NFmiTimeDescriptor& theTimeDescriptor,
                     const NFmiHPlaceDescriptor& theHPlaceDescriptor,
                     const NFmiVPlaceDescriptor& theVPlaceDescriptor = NFmiVPlaceDescriptor());

  NFmiQueryInfoSatel(NFmiQueryData* data,
                     NFmiParamDescriptor* theParamDescriptor = 0,
                     NFmiTimeDescriptor* theTimeDescriptor = 0,
                     NFmiHPlaceDescriptor* theHPlaceDescriptor = 0,
                     NFmiVPlaceDescriptor* theVPlaceDescriptor = 0);

  virtual NFmiQueryInfo& operator=(const NFmiQueryInfo& theInfo);

  NFmiQueryInfo* Clone() const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  virtual unsigned long ClassId() const;
  virtual const char* ClassName() const;

 private:
  NFmiQueryInfoSatel& operator=(const NFmiQueryInfoSatel& theInfo);

  void Destroy();
  NFmiString* itsSatelName;

};  // class NFMiQueryInfoSatel

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfoSatel::ClassId() const
{
  return kNFmiSatelQueryInfo;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiQueryInfoSatel::ClassName() const
{
  return "NFmiQueryInfoSatel";
}

// ======================================================================
