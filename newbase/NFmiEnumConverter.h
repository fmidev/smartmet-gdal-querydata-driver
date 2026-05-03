// ======================================================================
/*!
 * \file NFmiEnumConverter.h
 * \brief Interface of class NFmiEnumConverter
 */
// ----------------------------------------------------------------------

#pragma once

#include "NFmiDef.h"
#include "NFmiGlobals.h"
#include "NFmiParameterName.h"
#include <list>
#include <map>
#include <memory>
#include <string>

class NFmiEnumConverter
{
 public:
  ~NFmiEnumConverter();
  NFmiEnumConverter(FmiEnumSpace theEnumspace = kParamNames);

  const char *ToCharPtr(int theValue);
  const std::string ToString(int theValue);

  int ToEnum(const char *s);
  int ToEnum(const std::string &theString) { return ToEnum(theString.c_str()); }
  std::list<std::string> Names();

 private:
  struct Comparator
  {
    bool operator()(const char *a, const char *b) const;
  };

  class Impl
  {
   public:
    Impl(FmiEnumSpace theEnumspace);

    using ParameterMap = std::map<const char *, int, Comparator>;

    FmiEnumSpace itsEnumspace;
    ParameterMap itsParamMap;
    std::vector<const char *> itsEnumMap;
    int itsBadEnum;

   private:
    void initParamNames();
    void initRoadRegions();
    void initPressRegions();
    void initEnumMap();
  };

  std::unique_ptr<Impl> impl;

};  // class NFmiEnumConverter

// ======================================================================
