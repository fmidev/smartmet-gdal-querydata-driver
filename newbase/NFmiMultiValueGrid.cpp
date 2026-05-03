// ======================================================================
/*!
 * \file NFmiMultiValueGrid.cpp
 * \brief Implementation of class NFmiMultiValueGrid
 */
// ======================================================================
/*!
 * \class NFmiMultiValueGrid
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiMultiValueGrid.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 *
 * \todo Don't bother setting
 */
// ----------------------------------------------------------------------

NFmiMultiValuedGrid::~NFmiMultiValuedGrid()
{
  try
  {
    if (itsMultiData)
      delete itsMultiData;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theGrid Undocumented
 * \param theParamDescriptor Undocumented
 * \param theTimeDescriptor Undocumented
 * \param theVPlaceDescriptor Undocumented
 */
// ----------------------------------------------------------------------

NFmiMultiValuedGrid::NFmiMultiValuedGrid(const NFmiGrid &theGrid,
                                         const NFmiParamDescriptor &theParamDescriptor,
                                         const NFmiTimeDescriptor &theTimeDescriptor,
                                         const NFmiVPlaceDescriptor &theVPlaceDescriptor)
    : NFmiGrid(theGrid), itsMultiData(nullptr), itsCurrentDataStatus(0)
{
  try
  {
    itsParamDescriptor.reset(new NFmiParamDescriptor(theParamDescriptor));
    itsTimeDescriptor.reset(new NFmiTimeDescriptor(theTimeDescriptor));
    itsHPlaceDescriptor.reset();
    itsVPlaceDescriptor.reset(new NFmiVPlaceDescriptor(theVPlaceDescriptor));
    itsRefRawData = nullptr;
    Init();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMultiValuedGrid::Init()
{
  try
  {
    itsMultiData = new NFmiDataPool();
    if (!itsMultiData->Init(NFmiGridBase::Size() * NFmiQueryInfo::Size()))
    {
      std::cout << "Out of memory. Execution failed.";
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Remove unnecessary last return statement
 */
// ----------------------------------------------------------------------

bool NFmiMultiValuedGrid::FirstValue()
{
  try
  {
    Save();
    NFmiQueryInfo::First();
    return Edit();
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

bool NFmiMultiValuedGrid::Save()
{
  try
  {
    float *sourceAddress = nullptr;

    if (!itsMultiData)
      return false;

    // Mika: unsigned int
    // if(NFmiQueryInfo::Index() < 0 || NFmiQueryInfo::Index() >= NFmiQueryInfo::Size())

    if (NFmiQueryInfo::Index() >= NFmiQueryInfo::Size())
      return false;
    if (!itsData->FloatValueAddress(0, &sourceAddress))
      return false;

    itsMultiData->Index(itsCurrentDataStatus);
    return itsMultiData->MemCopy(NFmiGridBase::Size(), sourceAddress);
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

bool NFmiMultiValuedGrid::Edit()
{
  try
  {
    float *sourceAddress = nullptr;

    if (!itsMultiData)
      return false;
    // Mika: Index() is unsigned int
    // if(NFmiQueryInfo::Index() < 0 || NFmiQueryInfo::Index() >= NFmiQueryInfo::Size())

    if (NFmiQueryInfo::Index() >= NFmiQueryInfo::Size())
      return false;
    if (!itsMultiData->FloatValueAddress(NFmiQueryInfo::Index() * NFmiGridBase::Size(),
                                         &sourceAddress))
      return false;

    itsData->First();
    return itsData->MemCopy(NFmiGridBase::Size(), sourceAddress);
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

void NFmiMultiValuedGrid::UpDate()
{
  try
  {
    Save();
    itsCurrentDataStatus = NFmiQueryInfo::Index() * NFmiGridBase::Size();
    Edit();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param dx Undocumented
 * \param dy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiMultiValuedGrid::FloatValue(long dx, long dy)
{
  try
  {
    UpDate();
    return static_cast<float>(NFmiGrid::FloatValue(dx, dy));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param data Undcoumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMultiValuedGrid::FloatValue(float data)
{
  try
  {
    UpDate();
    return NFmiGrid::FloatValue(data);
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

bool NFmiMultiValuedGrid::InterpolateToLatLonPoint(double newLon, double newLat, double &theValue)
{
  try
  {
    UpDate();
    return NFmiGrid::InterpolateToLatLonPoint(newLon, newLat, theValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
