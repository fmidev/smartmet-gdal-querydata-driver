// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiStringTools
 *
 * Functions to handle the transition from NFmiString to string.
 * These functions should be removed as soon as possible.
 *
 * HUOM!!! Kaikki NFmiStringTools-funktiot eivät ole käytössä MSVC60:lla.
 *
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include "NFmiTypeNameTraits.h"
#include <macgyver/Exception.h>
#include <cstring>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace NFmiStringTools
{
char toupperfi(char theChar);
char tolowerfi(char theChar);

std::string &FirstCharToUpper(std::string &theString);
std::string &LowerCase(std::string &theString);
std::string &UpperCase(std::string &theString);
std::string &ReplaceChars(std::string &theString, char fromChar, char toChar);
std::string &ReplaceAll(std::string &theString,
                        const std::string &fromStr,
                        const std::string &toStr);
bool IsValue(const std::string &theString);
std::string &TrimL(std::string &theString, char theChar = ' ');
std::string &TrimR(std::string &theString, char theChar = ' ');
std::string &Trim(std::string &theString, char theChar = ' ');
std::string TrimAll(std::string &theString, bool replaceInsideNewlinesWithSpace = false);

const std::string ReadFile(std::istream &is);

const std::string UrlDecode(const std::string &theString);
const std::string UrlEncode(const std::string &theString);
const std::map<std::string, std::string> ParseQueryString();
const std::map<std::string, std::string> ParseQueryString(const std::string &theQueryString);

const std::string Basename(const std::string &theFile);
const std::string Suffix(const std::string &theFile);

template <typename T>
T Convert(const char *theThing);

template <typename T>
T Convert(const std::string &theThing);

template <typename T>
std::string Convert(const T &theThing);

template <typename T>
const T Split(const std::string &theString, const std::string &theSeparator = std::string(","));

const std::vector<std::string> Split(const std::string &theString,
                                     const std::string &theSeparator = std::string(","));

template <typename T>
const std::string Join(const T &theWords, const std::string &theSeparator = std::string(""));

}  // namespace NFmiStringTools

namespace NFmiStringTools
{
// Convert object to string
template <typename T>
inline std::string Convert(const T &theThing)
{
  std::ostringstream output;
  output << theThing;
  return output.str();
}

// Convert character string to bool

template <>
inline bool Convert<bool>(const char *theThing)
{
  if (strcmp(theThing, "true") == 0)
    return true;
  if (strcmp(theThing, "false") == 0)
    return false;
  if (strcmp(theThing, "1") == 0)
    return true;
  if (strcmp(theThing, "0") == 0)
    return false;
  throw Fmi::Exception(BCP, "Failed to convert " + std::string(theThing) + " to boolean");
}

// This is requred so that "a b c" remains "a b c"

template <>
inline std::string Convert<std::string>(const char *theThing)
{
  return theThing;
}

// Convert character string to object
template <typename T>
inline T Convert(const char *theThing)
{
  T result;
  std::istringstream parser(theThing);
  parser >> result;
  if (parser.fail())
    throw Fmi::Exception(
        BCP,
        "Failed to convert " + std::string(theThing) + " to type " + NFmiTypeNameTraits::Name<T>());

  long parserpos = static_cast<long>(parser.tellg());
  long strpos = static_cast<long>(strlen(theThing));
  if (parserpos != strpos && parserpos != -1)
    throw Fmi::Exception(BCP,
                         "Failed to fully convert " + std::string(theThing) + " to type " +
                             NFmiTypeNameTraits::Name<T>());
  return result;
}

// Convert string to object
template <typename T>
inline T Convert(const std::string &theThing)
{
  return Convert<T>(theThing.c_str());
}

// Split string into container objects
template <typename T>
inline const T Split(const std::string &theString, const std::string &theSeparator)
{
  if (theSeparator.empty())
    throw Fmi::Exception(BCP, "Cannot split based on empty separator string");

  T ret;

  if (theString.empty())
    return ret;

  std::string::size_type pos1 = 0;
  while (pos1 <= theString.size())
  {
    std::string::size_type pos2 = theString.find(theSeparator, pos1);
    if (pos2 == std::string::npos)
      pos2 = theString.size();
    std::inserter(ret, ret.end()) =
        Convert<typename T::value_type>(theString.substr(pos1, pos2 - pos1));
    pos1 = pos2 + theSeparator.size();
  }
  return ret;
}

// Join string container back to a single string

template <typename T>
inline const std::string Join(const T &theWords, const std::string &theSeparator)
{
  std::string ret;
  for (typename T::const_iterator it = theWords.begin(); it != theWords.end(); ++it)
  {
    if (!ret.empty())
      ret += theSeparator;
    ret += *it;
  }
  return ret;
}
}  // namespace NFmiStringTools

// ======================================================================
