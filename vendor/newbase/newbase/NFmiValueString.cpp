// ======================================================================
/*!
 * \file NFmiValueString.cpp
 * \brief Implementation of class NFmiValueString
 */
// ----------------------------------------------------------------------

#include "NFmiValueString.h"
#include <macgyver/Exception.h>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiValueString::~NFmiValueString() = default;

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 */
// ----------------------------------------------------------------------

NFmiValueString& NFmiValueString::operator = (const NFmiValueString&) = default;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiValueString::NFmiValueString() : NFmiString(), itsValueType(eNull) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::NFmiValueString(const char *theValue) : NFmiString(theValue), itsValueType(eChar)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::NFmiValueString(const unsigned char *theValue)
    : NFmiString(theValue), itsValueType(eChar)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::NFmiValueString(const NFmiString &theValue)
    : NFmiString(theValue), itsValueType(eChar)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theValue The other object being copied
 */
// ----------------------------------------------------------------------

NFmiValueString::NFmiValueString(const NFmiValueString &theValue)
    : NFmiString(theValue), itsValueType(eChar)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theValue Undocumented
 * \param theFormat Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::NFmiValueString(const short theValue, const char *theFormat)
    : NFmiString(), itsValueType(eShort)
{
  try
  {
    SetValue(theValue, theFormat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theValue Undocumented
 * \param theFormat Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::NFmiValueString(const int theValue, const char *theFormat)
    : NFmiString(), itsValueType(eInt)
{
  try
  {
    SetValue(theValue, theFormat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theFormat Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::NFmiValueString(const long theValue, const char *theFormat)
    : NFmiString(), itsValueType(eLong)
{
  try
  {
    SetValue(theValue, theFormat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theFormat Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::NFmiValueString(const float theValue, const char *theFormat)
    : NFmiString(), itsValueType(eFloat)
{
  try
  {
    SetValue(theValue, theFormat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theValue Undocumented
 * \param theFormat Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::NFmiValueString(const double theValue, const char *theFormat)
    : NFmiString(), itsValueType(eDouble)
{
  try
  {
    SetValue(theValue, theFormat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theFormat Undocumented
 */
// ----------------------------------------------------------------------

void NFmiValueString::SetValue(const short theValue, const char *theFormat)
{
  try
  {
    unsigned char valueString[16];
    unsigned int valueLength;

#ifdef _MSC_VER
    valueLength = ::_snprintf(
        reinterpret_cast<char *>(valueString), sizeof(valueString) - 1, theFormat, theValue);
#else
    valueLength = ::snprintf(
        reinterpret_cast<char *>(valueString), sizeof(valueString) - 1, theFormat, theValue);
#endif
    valueString[sizeof(valueString) - 1] = 0;  // pitää varmistaa että päättyy 0-merkkiin!!!!

    Set(valueString, static_cast<short>(valueLength));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theFormat Undocumented
 */
// ----------------------------------------------------------------------

void NFmiValueString::SetValue(const int theValue, const char *theFormat)
{
  try
  {
    unsigned char valueString[16];
    unsigned int valueLength;

#ifdef _MSC_VER
    valueLength = ::_snprintf(
        reinterpret_cast<char *>(valueString), sizeof(valueString) - 1, theFormat, theValue);
#else
    valueLength = ::snprintf(
        reinterpret_cast<char *>(valueString), sizeof(valueString) - 1, theFormat, theValue);
#endif
    valueString[sizeof(valueString) - 1] = 0;  // pitää varmistaa että päättyy 0-merkkiin!!!!

    Set(valueString, static_cast<short>(valueLength));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theFormat Undocumented
 */
// ----------------------------------------------------------------------

void NFmiValueString::SetValue(const float theValue, const char *theFormat)
{
  try
  {
    unsigned char valueString[64];
    unsigned int valueLength;

#ifdef _MSC_VER
    valueLength = ::_snprintf(
        reinterpret_cast<char *>(valueString), sizeof(valueString) - 1, theFormat, theValue);
#else
    valueLength = ::snprintf(
        reinterpret_cast<char *>(valueString), sizeof(valueString) - 1, theFormat, theValue);
#endif
    valueString[sizeof(valueString) - 1] = 0;  // pitää varmistaa että päättyy 0-merkkiin!!!!

    Set(valueString, static_cast<short>(valueLength));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theFormat Undocumented
 */
// ----------------------------------------------------------------------

void NFmiValueString::SetValue(const long theValue, const char *theFormat)
{
  try
  {
    unsigned char valueString[16];
    unsigned int valueLength;

#ifdef _MSC_VER
    valueLength = ::_snprintf(
        reinterpret_cast<char *>(valueString), sizeof(valueString) - 1, theFormat, theValue);
#else
    valueLength = ::snprintf(
        reinterpret_cast<char *>(valueString), sizeof(valueString) - 1, theFormat, theValue);
#endif
    valueString[sizeof(valueString) - 1] = 0;  // pitää varmistaa että päättyy 0-merkkiin!!!!

    Set(valueString, static_cast<short>(valueLength));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theFormat Undocumented
 */
// ----------------------------------------------------------------------

void NFmiValueString::SetValue(const double theValue, const char *theFormat)
{
  try
  {
    unsigned char valueString[310];
    unsigned int valueLength;

#ifdef _MSC_VER
    valueLength = ::_snprintf(
        reinterpret_cast<char *>(valueString), sizeof(valueString) - 1, theFormat, theValue);
#else
    valueLength = ::snprintf(
        reinterpret_cast<char *>(valueString), sizeof(valueString) - 1, theFormat, theValue);
#endif
    valueString[sizeof(valueString) - 1] = 0;  // pitää varmistaa että päättyy 0-merkkiin!!!!

    Set(valueString, static_cast<short>(valueLength));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiValueString::IsShort() const
{
  try
  {
    short returnValue;

    if (itsValueType == eShort)
      return true;

    if (!IsNumeric() || Search(reinterpret_cast<const unsigned char *>(".")))
      return false;

    if (itsValueType == eChar)
      return sscanf(CharPtr(), "%hd", &returnValue) == 1 ? true : false;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned int NFmiValueString::SignificantDecimals() const
{
  try
  {
    unsigned int pointIndex = Search(reinterpret_cast<const unsigned char *>("."));

    if (pointIndex > 0)
    {
      unsigned int idx = GetLen() - 1;
      while (idx >= pointIndex)
      {
        if (fChar[idx] != '0')
        {
          return idx - pointIndex + 1;
        }
        idx--;
      }
    }
    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theStart Undocumented
 * \param theSize Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiValueString::IsNumeric(int theStart, int theSize) const
{
  try
  {
    bool digitFound = false;
    for (unsigned int idx = theStart; idx < (theSize ? theSize : GetLen()); idx++)
      if (!isdigit(fChar[idx]))
      {
        if ((fChar[idx] != ' ') && (fChar[idx] != '.') && (fChar[idx] != '+') &&
            (fChar[idx] != '-'))
          return false;
      }
      else
        digitFound = true;

    return GetLen() && digitFound ? true : false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Hakee merkkijonon ensimmäisen numeerisen merkin. Haku aloitetaan
 * annetusta indeksistä start, jonka oletusarvo on 1.
 * \param start, indeksi, josta haku aloitetaan, oletusarvo 1
 * \return ensimmäiseksi löydetyn numeerisen merkin indeksi
 */
// ----------------------------------------------------------------------
unsigned long NFmiValueString::SearchNumeric(unsigned long start) const
{
  try
  {
    if (start == 0)
      start = 1;

    unsigned long index = start - 1;
    int len = GetLen();
    while (static_cast<int>(index) < len)
    {
      if (isdigit(fChar[index]))
        return index + 1;
      index++;
    }
    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theStart Undocumented
 * \param theSize Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiValueString::IsInt(int theStart, int theSize) const
{
  try
  {
    int returnValue;

    if (!IsNumeric() || Search(reinterpret_cast<const unsigned char *>(".")))
      return false;

    return ConvertToInt(returnValue, theStart, theSize);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theStart Undocumented
 * \param theSize Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiValueString::IsFloat(int theStart, int theSize) const
{
  try
  {
    float returnValue;

    if (!IsNumeric())
      return false;

    return ConvertToFloat(returnValue, theStart, theSize);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theStart Undocumented
 * \param theSize Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiValueString::IsLong(int theStart, int theSize) const
{
  try
  {
    long returnValue;

    if (!IsNumeric())
      return false;

    return ConvertToLong(returnValue, theStart, theSize);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theStart Undocumented
 * \param theSize Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiValueString::IsDouble(int theStart, int theSize) const
{
  try
  {
    double returnValue;

    if (itsValueType == eDouble)
      return true;

    if (!IsNumeric())
      return false;

    return (sscanf(theSize ? static_cast<char *>(GetChars(theStart, theSize)) : CharPtr(),
                   "%lf",
                   &returnValue) == 1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theStart Undocumented
 * \param theSize Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiValueString::ConvertToInt(int &theValue, const int theStart, const int theSize) const
{
  try
  {
    if (theSize)
      return sscanf(GetChars(theStart, theSize), "%d", &theValue) ? true : false;

    return sscanf(CharPtr(), "%d", &theValue) ? true : false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theStart Undocumented
 * \param theSize Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiValueString::ConvertToLong(long &theValue, const int theStart, const int theSize) const
{
  try
  {
    NFmiString theConvertStr;

    if (theSize)
      theConvertStr = GetChars(theStart, theSize);
    else
      theConvertStr = CharPtr();

    if (sscanf(theConvertStr, "%ld", &theValue))
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theStart Undocumented
 * \param theSize Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiValueString::ConvertToFloat(float &theValue, const int theStart, const int theSize) const
{
  try
  {
    NFmiString theConvertStr;

    if (theSize)
      theConvertStr = GetChars(theStart, theSize);
    else
      theConvertStr = CharPtr();

    if (sscanf(theConvertStr, "%f", &theValue) > 0)
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theStart Undocumented
 * \param theSize Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiValueString::ConvertToDouble(double &theValue, const int theStart, const int theSize) const
{
  try
  {
    if (*this == NFmiString(""))
      return false;

    if (theSize > 0)
      return sscanf(GetChars(theStart, theSize), "%lf", &theValue) != 0;

    return sscanf(CharPtr(), "%lf", &theValue) != 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::operator short() const
{
  try
  {
    short returnValue;

    return short(sscanf(CharPtr(), "%hd", &returnValue) ? returnValue : 0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::operator int() const
{
  try
  {
    int returnValue = 0;

    sscanf(CharPtr(), "%d", &returnValue);

    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::operator float() const
{
  try
  {
    float returnValue = 0.0;
    sscanf(CharPtr(), "%f", &returnValue);
    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::operator long() const
{
  try
  {
    long returnValue = 0;
    sscanf(CharPtr(), "%ld", &returnValue);

    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiValueString::operator double() const
{
  try
  {
    double returnValue;

    sscanf(CharPtr(), "%lf", &returnValue);

    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 * \param maxDecimals Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiValueString::GetStringWithMaxDecimalsSmartWay(double value, int maxDecimals)
{
  try
  {
    /* AKa 11-Mar-2008: Removing 'static' to get SVG generation happen multithreaded
     *                  (Marko pinpointed the problem to be here)
      static char buffer[128]="";
      static char buffer2[128]="";
    */
    char buffer[128] = "";
    char buffer2[128] = "";
    int precisionValue = maxDecimals;
    NFmiString format;
    if (precisionValue)
    {
      for (double tmp = fabs(value); tmp > 1; tmp /= 10.)
        precisionValue++;
#ifdef _MSC_VER
      ::_snprintf(buffer2, sizeof(buffer2) - 1, "%%0.%dg", precisionValue);
#else
      ::snprintf(buffer2, sizeof(buffer2) - 1, "%%0.%dg", precisionValue);
#endif
      buffer2[sizeof(buffer2) - 1] = 0;  // pitää varmistaa että päättyy 0-merkkiin!!!!
      format = buffer2;
    }
    else  // jos maxDecimals oli 0 ja luku vaikka 10, tuli MSVC:lla tulos 1e+001 (tämä XP:llä,
          // mielestäni toimi NT4:lla?!?!?)
      format += "%.0f";
#ifdef _MSC_VER
    ::_snprintf(buffer, sizeof(buffer) - 1, format, value);
#else
    ::snprintf(buffer, sizeof(buffer) - 1, format, value);
#endif
    buffer[sizeof(buffer) - 1] = 0;  // pitää varmistaa että päättyy 0-merkkiin!!!!
    return NFmiString(buffer);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
