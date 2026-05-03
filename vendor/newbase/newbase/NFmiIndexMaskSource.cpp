// ======================================================================
/*!
 * \file
 * \brief Implementation of class NFmiIndexMaskSource
 */
// ======================================================================
/*!
 * \class NFmiIndexMaskSource
 *
 * \brief Provides masks for any desired time.
 *
 * A NFmiIndexMaskSource object stores NFmiIndexMask objects
 * for the user specified times. When requested a mask for
 * a user specified time, the mask whose time is nearest
 * to the desired time will be returned. If there are two
 * equally close, the mask for the earlier time will be returned.
 *
 * Sample use:
 * \code
 * NFmiIndexMaskSource maskstore;
 *
 * NFmiIndexMask mask1 = ...;
 * NFmiMetTime time1 = ...;
 * maskstore.Insert(time1,mask1);
 *
 * NFmiMetTime time2 = ...;
 * const NFmiIndexMask & mask = maskstore.Find(time2);
 * \endcode
 *
 */
// ======================================================================

#include "NFmiIndexMaskSource.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

NFmiIndexMaskSource::~NFmiIndexMaskSource() = default;
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

NFmiIndexMaskSource::NFmiIndexMaskSource() : itsData() {}
// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 *
 * \param theSource The mask source to copy
 */
// ----------------------------------------------------------------------

NFmiIndexMaskSource::NFmiIndexMaskSource(const NFmiIndexMaskSource& theSource)

    = default;

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 *
 * \param theSource The mask source to copy
 */
// ----------------------------------------------------------------------

NFmiIndexMaskSource& NFmiIndexMaskSource::operator=(const NFmiIndexMaskSource& theSource)
{
  try
  {
    if (this != &theSource)
      itsData = theSource.itsData;

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Insert a new mask with associated time
 *
 * If the time has been defined already, nothing is done.
 *
 * \param theTime The time of the mask
 * \param theMask The mask itself
 */
// ----------------------------------------------------------------------

void NFmiIndexMaskSource::Insert(const NFmiMetTime& theTime, const NFmiIndexMask& theMask)
{
  try
  {
    itsData.insert(storage_type::value_type(theTime, theMask));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Find a mask for the given time
 *
 * If no masks have been specified, an empty mask is returned.
 * If there is no mask for the exact given time, the mask
 * with the time closest to the given time is returned.
 * If there are two equally close, the one associated with
 * the earlier time is returned.
 *
 * \param theTime The time for which to return the mask
 * \return The mask
 */
// ----------------------------------------------------------------------

const NFmiIndexMask& NFmiIndexMaskSource::Find(const NFmiMetTime& theTime) const
{
  try
  {
    // this is needed in case no masks have been inserted
    static NFmiIndexMask empty_mask;

    // empty mask if storage is empty
    if (itsData.empty())
      return empty_mask;

    // find first time >= theTime
    auto it = itsData.lower_bound(theTime);

    // Return last mask when the time is too late
    if (it == itsData.end())
      return (--it)->second;

    // return exact match if there is one
    if (it->first == theTime)
      return it->second;

    // return first match if match was first
    if (it == itsData.begin())
      return it->second;

    // now see if the previous time (before theTime) is closer

    const long diff2 = it->first.DifferenceInMinutes(theTime);
    --it;
    const long diff1 = theTime.DifferenceInMinutes(it->first);

    if (diff1 > diff2)  // note how earlier time is favored
      ++it;

    return it->second;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Emptyes all the masks from source.
 *
 * After this call is NFmiIndexMaskSource-object empty.
 *
 */
// ----------------------------------------------------------------------

void NFmiIndexMaskSource::Clear()
{
  try
  {
    itsData.clear();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ======================================================================
