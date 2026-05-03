// ======================================================================
/*!
 * \file NFmiSettings.cpp
 * \brief Definition of NFmiSettings methods
 */
// ======================================================================

#include "NFmiSettings.h"
#include <macgyver/Exception.h>

namespace NFmiSettings
{
void Clear()
{
  try
  {
    NFmiSettingsImpl::Instance().Clear();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool Init()
{
  try
  {
    return NFmiSettingsImpl::Instance().Init();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool Read(const std::string& theFile)
{
  try
  {
    return NFmiSettingsImpl::Instance().Read(theFile);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Save()
{
  try
  {
    NFmiSettingsImpl::Instance().Save();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Set(const std::string& theName, const std::string& theValue, bool modifyOnlyExisting)
{
  try
  {
    NFmiSettingsImpl::Instance().Set(theName, theValue, modifyOnlyExisting);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool IsSet(const std::string& theName)
{
  try
  {
    return NFmiSettingsImpl::Instance().IsSet(theName);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string ToString()
{
  try
  {
    return NFmiSettingsImpl::Instance().ToString();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace NFmiSettings

// ======================================================================
