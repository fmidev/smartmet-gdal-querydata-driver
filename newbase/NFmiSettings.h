// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiSettings
 */
// ----------------------------------------------------------------------

#pragma once

#include "NFmiSettingsImpl.h"
#include "NFmiStringTools.h"

#include <sstream>
#include <stdexcept>
#include <string>

namespace NFmiSettings
{
void Clear();
bool Init();
bool Read(const std::string& theFile);
void Save();

std::string ToString();

// modifyOnlyExisting -parametri: Jos false, lisätään/muokataan parametria. Jos true, vain
// olemassaolevan asetuksen muokkaus onnistuu.
void Set(const std::string& theName, const std::string& theValue, bool modifyOnlyExisting = false);
bool IsSet(const std::string& theName);

template <typename T>
T Require(const std::string& thename);

template <typename T>
T RequireRange(const std::string& theName, const T& theLowerLimit, const T& theUpperLimit);

template <typename T>
T Optional(const std::string& theName, const T& theDefault);

template <typename T>
T OptionalRange(const std::string& theName,
                const T& theDefault,
                const T& theLowerLimit,
                const T& theUpperLimit);

std::vector<std::string> ListChildren(const std::string& thePrefix);

}  // namespace NFmiSettings

// ======================================================================
// TEMPLATE FUNCTION IMPLEMENTATIONS
// ======================================================================

namespace NFmiSettings
{
// ----------------------------------------------------------------------
/*!
 * \brief Require specialization for strings
 */
// ----------------------------------------------------------------------

template <>
inline std::string Require<std::string>(const std::string& theName)
{
  return NFmiSettingsImpl::Instance().Require(theName);
}

inline std::vector<std::string> ListChildren(const std::string& thePrefix)
{
  return NFmiSettingsImpl::Instance().ListChildren(thePrefix);
}

// ----------------------------------------------------------------------
/*!
 * \brief Require specialization for boolean type
 */
// ----------------------------------------------------------------------
template <>
inline bool Require<bool>(const std::string& theName)
{
  std::string tmp = Require<std::string>(theName);

  if (tmp == "true" || tmp == "1")
    return true;
  if (tmp == "false" || tmp == "0")
    return false;

  std::string msg;
  msg += "Variable ";
  msg += theName;
  msg += " value '";
  msg += tmp;
  msg += "' must be true/false";
  throw Fmi::Exception(BCP, msg);
}

// ----------------------------------------------------------------------
/*!
 * \brief Generic Require specialization
 */
// ----------------------------------------------------------------------

template <typename T>
inline T Require(const std::string& theName)
{
  std::string tmp = Require<std::string>(theName);

  try
  {
    return NFmiStringTools::Convert<T>(tmp);
  }
  catch (...)
  {
    std::string msg;
    msg += "Variable ";
    msg += theName;
    msg += " value '";
    msg += tmp;
    msg += "' is not of correct type";
    throw Fmi::Exception(BCP, msg);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Request an optional value
 *
 * If the variable is not set, the given default value is returned back.
 * Throws std::runtime_error if the variable cannot be parsed to the
 * given type.
 *
 * \param theName The variable name
 * \param theDefault The default value
 * \return The optional value
 */
// ----------------------------------------------------------------------

template <typename T>
inline T Optional(const std::string& theName, const T& theDefault)
{
  if (!IsSet(theName))
    return theDefault;
  else
    return Require<T>(theName);
}

// ----------------------------------------------------------------------
/*!
 * \brief Require a value within the given range
 *
 * Throws std::runtime_error if the value cannot be parsed or is not
 * within the required range.
 *
 * \param theName The variable name
 * \param theLowerLimit The minimum allowed value
 * \param theUpperLimit The maximum allowed value
 * \return The value
 */
// ----------------------------------------------------------------------

template <typename T>
inline T RequireRange(const std::string& theName, const T& theLowerLimit, const T& theUpperLimit)
{
  T value(Require<T>(theName));
  if (value >= theLowerLimit && value <= theUpperLimit)
    return value;

  std::string msg;
  msg += "Variable ";
  msg += theName;
  msg += " value '";
  msg += Require<std::string>(theName);
  msg += "' is not in range ";
  msg += NFmiStringTools::Convert(theLowerLimit);
  msg += "...";
  msg += NFmiStringTools::Convert(theUpperLimit);
  throw Fmi::Exception(BCP, msg);
}

// ----------------------------------------------------------------------
/*!
 * \brief Request an optional value within the given range
 *
 * Throws std::runtime_error if the value cannot be parsed or is not
 * within the required range.
 *
 * \param theName The variable name
 * \param theDefault The default value if the variable is not set
 * \param theLowerLimit The minimum allowed value
 * \param theUpperLimit The maximum allowed value
 * \return The value
 */
// ----------------------------------------------------------------------

template <typename T>
inline T OptionalRange(const std::string& theName,
                       const T& theDefault,
                       const T& theLowerLimit,
                       const T& theUpperLimit)
{
  if (!IsSet(theName))
    return theDefault;
  else
    return RequireRange<T>(theName, theLowerLimit, theUpperLimit);
}
}  // namespace NFmiSettings

// ======================================================================
