// ======================================================================
/*!
 * \file NFmiAreaMask.cpp
 * \brief Implementation of class NFmiAreaMask
 */
// ======================================================================
/*!
 * \class NFmiAreaMask
 *
 * A basic area mask class. Also contains a list for possible submasks,
 * which can be indexed.
 *
 */
// ======================================================================

#include "NFmiAreaMask.h"
#include "NFmiFastQueryInfo.h"

#include <macgyver/Exception.h>
#include <cmath>

//! Destructor
NFmiAreaMask::~NFmiAreaMask() = default;

std::shared_ptr<NFmiFastQueryInfo> NFmiAreaMask::DoShallowCopy(
    const std::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  try
  {
    if (theInfo)
      return std::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*theInfo));

    return theInfo;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<std::shared_ptr<NFmiFastQueryInfo>> NFmiAreaMask::DoShallowCopy(
    const std::vector<std::shared_ptr<NFmiFastQueryInfo>> &infoVector)
{
  // tehd‰‰n matala kopio info-vektorista
  std::vector<std::shared_ptr<NFmiFastQueryInfo>> shallowCopyVector;
  for (const auto &info : infoVector)
  {
    shallowCopyVector.push_back(std::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*info)));
  }
  return shallowCopyVector;
}

std::shared_ptr<NFmiAreaMask> NFmiAreaMask::DoShallowCopy(
    const std::shared_ptr<NFmiAreaMask> &theMask)
{
  try
  {
    if (theMask)
      return std::shared_ptr<NFmiAreaMask>(theMask->Clone());

    return theMask;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<std::shared_ptr<NFmiAreaMask>> NFmiAreaMask::DoShallowCopy(
    const std::vector<std::shared_ptr<NFmiAreaMask>> &theMaskVector)
{
  try
  {
    std::vector<std::shared_ptr<NFmiAreaMask>> returnVector(theMaskVector.size());
    for (size_t i = 0; i < theMaskVector.size(); i++)
      returnVector[i] = NFmiAreaMask::DoShallowCopy(theMaskVector[i]);
    return returnVector;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
