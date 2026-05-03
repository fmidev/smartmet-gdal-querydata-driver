// ======================================================================
/*!
 * \file NFmiValueString.h
 * \brief Interface of class NFmiValueString
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include "NFmiString.h"

enum FmiValues
{
  eNull,
  eChar,
  eShort,
  eInt,
  eLong,
  eFloat,
  eDouble
};

//! Undocumented
class NFmiValueString : public NFmiString
{
 public:
  ~NFmiValueString();
  NFmiValueString();

  NFmiValueString(const short theValue, const char *theFormat = "%hd");
  NFmiValueString(const int theValue, const char *format = "%d");
  NFmiValueString(const float theValue, const char *format = "%f");
  NFmiValueString(const long theValue, const char *format = "%ld");
  NFmiValueString(const double theValue, const char *format = "%lf");

  NFmiValueString(const char *);
  NFmiValueString(const unsigned char *);
  NFmiValueString(const NFmiString &);
  NFmiValueString(const NFmiValueString &);

  NFmiValueString& operator = (const NFmiValueString&);

  void SetValue(const short value, const char *format = "%hd");
  void SetValue(const int value, const char *format = "%d");
  void SetValue(const float value, const char *format = "%f");
  void SetValue(const long value, const char *format = "%ld");
  void SetValue(const double value, const char *format = "%lf");

  bool ConvertToInt(int &convertValue, const int start = 0, const int size = 0) const;
  bool ConvertToFloat(float &convertValue, const int start = 0, const int size = 0) const;
  bool ConvertToLong(long &convertValue, const int start = 0, const int size = 0) const;
  bool ConvertToDouble(double &convertValue, const int start = 0, const int size = 0) const;

  unsigned int SignificantDecimals() const;

  bool IsNumeric(int start = 0, int size = 0) const;
  unsigned long SearchNumeric(unsigned long start = 1) const;

  bool IsShort() const;
  bool IsInt(int start = 0, int size = 0) const;
  bool IsFloat(int start = 0, int size = 0) const;
  bool IsLong(int start = 0, int size = 0) const;
  bool IsDouble(int start = 0, int size = 0) const;

  operator float() const;
  operator int() const;
  operator short() const;
  operator long() const;
  operator double() const;

  // palauttaa stringin, jossa on maksimissaan haluttu määrä desimaaleja. Mutta jos perään tulisi
  // jostain alkaen vain nollia, ne jätetään pois stringistä.
  // HUOM! static funktio eli käyttö koodissa: NFmiString str =
  // NFmiValueString::GetStringWithMaxDecimalsSmartWay(12.54, 4);
  // Palauttaa "12.54" eikä "12.5400"

  static const NFmiString GetStringWithMaxDecimalsSmartWay(double value, int maxDecimals);

 private:
  FmiValues itsValueType;

};  // NFmiValueString

// ======================================================================
