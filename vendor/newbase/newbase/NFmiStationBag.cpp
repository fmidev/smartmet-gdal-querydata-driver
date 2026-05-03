// ======================================================================
/*!
 * \file NFmiStationBag.cpp
 * \brief Implementation of class NFmiStationBag
 */
// ======================================================================
/*!
 * \class NFmiStationBag
 *
 * Do not use this for anything, use NFmiLocation instead!
 *
 */
// ======================================================================

#include "NFmiStationBag.h"
#include "NFmiValueString.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiStationBag::NFmiStationBag() : NFmiLocationBag() {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theStationArray Undocumented
 * \param numberOfStations Undocumented
 */
// ----------------------------------------------------------------------

NFmiStationBag::NFmiStationBag(unsigned long *theStationArray, unsigned long numberOfStations)
    : NFmiLocationBag()
{
  try
  {
    itsSize = numberOfStations;
    itsLocations.reserve(numberOfStations);
    for (unsigned long i = 0; i < numberOfStations; i++)
      itsLocations.push_back(NFmiStation(theStationArray[i]).Clone());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theStationArray Undocumented
 * \param numberOfStations Undocumented
 */
// ----------------------------------------------------------------------

NFmiStationBag::NFmiStationBag(NFmiStation *theStationArray, unsigned long numberOfStations)
    : NFmiLocationBag(theStationArray, numberOfStations)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theBag The other object being copied
 */
// ----------------------------------------------------------------------

NFmiStationBag::NFmiStationBag(const NFmiStationBag &theBag) = default;
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiStationBag::Destroy()
{
  try
  {
    for (unsigned long i = 0; i < itsLocations.size(); i++)
      delete itsLocations[i];

    itsLocations.clear();
    itsSize = 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBag Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiLocationBag NFmiStationBag::Combine(const NFmiLocationBag &theBag)
{
  try
  {
    NFmiLocationBag outbag(theBag);

    StorageType::const_iterator begin = itsLocations.begin();
    StorageType::const_iterator end = itsLocations.end();
    StorageType::const_iterator iter;
    for (iter = begin; iter != end; ++iter)
      outbag.AddLocation(*(*iter), true);

    return outbag;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theStation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStationBag::SetCurrent(long theStation)
{
  try
  {
    Reset();
    while (Next())
    {
      if (theStation == CurrentStation())
        return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \param theChecking Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStationBag::AddLocation(const NFmiLocation &theLocation, bool theChecking)
{
  try
  {
    return (NFmiLocationBag::AddLocation(theLocation, theChecking));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theStation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStationBag::AddStation(const NFmiStation &theStation)
{
  try
  {
    return (NFmiLocationBag::AddLocation(theStation));
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

NFmiLocationBag *NFmiStationBag::Clone() const
{
  try
  {
    return new NFmiStationBag(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiStationBag::Write(std::ostream &file) const
{
  try
  {
    NFmiLocationBag::Write(file);
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiStationBag::Read(std::istream &file)
{
  try
  {
    NFmiLocationBag::Read(file);
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theStationBag The other object being copied
 */
// ----------------------------------------------------------------------

NFmiStationBag &NFmiStationBag::operator=(const NFmiStationBag &theStationBag)
{
  try
  {
    NFmiLocationBag::operator=(*static_cast<const NFmiLocationBag *>(&theStationBag));
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
