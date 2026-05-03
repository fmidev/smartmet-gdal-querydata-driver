// ======================================================================
/*!
 * \file NFmiEnumConverter.cpp
 * \brief Implementation of class NFmiEnumConverter
 */
// ======================================================================
/*!
 * \class NFmiEnumConverter
 *
 * A parameter descriptor type conversion class.
 *
 * Note that the single data member is made static and is hidden
 * in the .cpp file.
 *
 * Correct usage:
 *
 * \code
 *  NFmiParamConverter myconverter;
 *  NFmiParameterName temp = myconverter.ToName("Temperature");
 *  string str = myconverter.ToString(kNFmiTemperature);
 * \endcode
 *
 * Available converters are created with
 *
 * \code
 *  NFmiParamConverter myconverter(enumname);
 * \endcode
 *
 * where enumname is one of
 *
 * \code
 *     kParamNames (default)
 *     kRoadRegions
 *     kPressRegions
 * \endcode
 *
 * Works, but is slow if the converter is used multiple times:
 *
 * \code
 *	FmiParameterName temp = NFmiParamConverter().ToName("Temperature");
 *	string str = NFmiParamConverter().ToString(kNFmiTemperature);
 * \endcode
 *
 * The former returns kNFmiBadParameter if the string is unidentified,
 * the latter an empty string
 *
 * \todo Make the access methods const by using mutable if necessary
 */
// ======================================================================

#include "NFmiEnumConverter.h"
#include "NFmiDef.h"
#include "NFmiPressMasks.h"
#include "NFmiTiesaaAlueet.h"
#include <macgyver/Exception.h>
#include <map>
#include <vector>

using namespace std;

// Case insensitive < operator
bool NFmiEnumConverter::Comparator::operator()(const char *a, const char *b) const
{
  try
  {
#ifdef _MSC_VER
    // MSVC++ 2008 (or before) doesn't support strcasecmp-function so using _stricmp instead.
    return (::_stricmp(a, b) < 0);
#else
    return (::strcasecmp(a, b) < 0);
#endif
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiEnumConverter::Impl::Impl(FmiEnumSpace theEnumspace) : itsEnumspace(theEnumspace)
{
  try
  {
    switch (itsEnumspace)
    {
      case kParamNames:
      {
        itsBadEnum = kFmiBadParameter;
        initParamNames();
        break;
      }
      case kRoadRegions:
      {
        itsBadEnum = kTieAlueNone;
        initRoadRegions();
        break;
      }
      case kPressRegions:
      {
        itsBadEnum = kPressMaskNone;
        initPressRegions();
        break;
      }
      default:
      {
        itsBadEnum = 0;
        break;
      }
    }
    initEnumMap();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiEnumConverter::Impl::initEnumMap()
{
  try
  {
    // Establish maximum parameter number
    int maxnum = -99999;
    for (const auto &name_id : itsParamMap)
      maxnum = std::max(maxnum, name_id.second);

    // Create a vector mapping enum to name (char *)

    itsEnumMap.resize(maxnum + 1, nullptr);

    for (const auto &name_id : itsParamMap)
      itsEnumMap[name_id.second] = name_id.first;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Destructor needs to be defined when using unique_ptr for Impl
 */
// ----------------------------------------------------------------------

NFmiEnumConverter::~NFmiEnumConverter() = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theEnumspace Undocumented
 *
 * \todo Move this to the cpp file
 */
// ----------------------------------------------------------------------

NFmiEnumConverter::NFmiEnumConverter(FmiEnumSpace theEnumspace)
    : impl(new NFmiEnumConverter::Impl(theEnumspace))
{
}

// ----------------------------------------------------------------------
/*!
 * Convert parameter string to parameter name.
 *
 * Return value is kFmiBadParameter if string is unknown.
 *
 * This method is fast, once it has been used atleast once so that
 * the parameter table has been initialized.
 *
 * \param theString The name of the paramter
 * \return The enumeration value
 *
 * \todo Use const_iterator internally
 */
// ----------------------------------------------------------------------

int NFmiEnumConverter::ToEnum(const char *s)
{
  try
  {
    auto pos = impl->itsParamMap.find(s);

    if (pos == impl->itsParamMap.end())
      return impl->itsBadEnum;

    return pos->second;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Convert parameter name to parameter string
 */
// ----------------------------------------------------------------------

const std::string NFmiEnumConverter::ToString(int theValue)
{
  try
  {
    const char *ptr = ToCharPtr(theValue);
    if (ptr == nullptr)
      return "";

    return ptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Convert parameter name to parameter string.
 *
 * Returns an empty string if the value is invalid.
 *
 * This method is relatively slow, as it uses a linear search
 * in a large collection of parameter value-name pairs.
 *
 * \param theName The enumeration value to search for
 * \returns The named enumeration as a string
 *
 * \todo Use const_iterator internally
 */
// ----------------------------------------------------------------------

const char *NFmiEnumConverter::ToCharPtr(int theName)
{
  try
  {
    if (theName < 0 || static_cast<std::size_t>(theName) >= impl->itsEnumMap.size())
      return nullptr;

    return impl->itsEnumMap.at(theName);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Return the registered names in a list.
 *
 * \return List of parameter names
 */
// ----------------------------------------------------------------------

list<string> NFmiEnumConverter::Names()
{
  try
  {
    list<string> out;

    for (const auto &name_value : impl->itsParamMap)
      out.push_back(name_value.first);

    return out;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
