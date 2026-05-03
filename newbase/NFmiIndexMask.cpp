// ======================================================================
/*!
 * \file
 * \brief Implementation of class NFmiIndexMask
 */
// ======================================================================
/*!
 * \class NFmiIndexMask
 *
 * \brief NFmiIndexMask provides iterated access to index values.
 *
 * NFmiIndexMask provides a const_iterator for accessing a set
 * of indices. Typical use involves iterating through an area
 * bounded by a polygon in some array of indexed data.
 *
 * Logical AND and OR operations are provided for combining
 * the masks.
 *
 * To add an individual index range one would use
 * \code
 * NFmiIndexMask mask;
 * mask.add(start_index,index_step,repeat_count);
 * \endcode
 * Logical operations can be performed either in place
 * \code
 * mask1 &= mask2;
 * mask1 |= mask2;
 * \endcode
 * or via a temporary
 * \code
 * NFmiIndexMask mask = mask1 & mask2;
 * NFmiIndexMask mask = mask1 | mask2;
 * \endcode
 * Regular iteration is performed as follows
 * \code
 * for(NFmiIndexMask::const_iterator it=mask.begin(); it!=mask.end(); ++it)
 * {
 *   NFmiIndexMask::value_type idx = *it;     // most likely unsigned long
 *   ...
 * }
 * \endcode
 *
 * The fastest way to use index masks is
 * \code
 * NFmiIndexMask::const_iterator end = mask.end();
 * for(NFmiIndexMask::const_iterator it=mask.begin(); it!=end; ++it)
 * {
 *    queryinfo.LocationIndex(*it);
 *    float value = queryinfo.FloatValue();
 *    ...
 * }
 * \endcode
 *
 * To test whether one mask is a subset of another, use
 * \code
 *  NFmiIndexMask mask1 = ...;
 *  NFmiIndexMask mask2 = ...;
 *  if(std::includes(mask1.begin(), mask1.end(), mask2.begin(), mask2.end())
 *  {
 *    // mask1 includes mask2
 *  }
 * \endcode
 */
// ======================================================================

#include "NFmiIndexMask.h"
#include <macgyver/Exception.h>
#include <algorithm>
#include <deque>
#include <iterator>
#include <set>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

NFmiIndexMask::~NFmiIndexMask() = default;
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

NFmiIndexMask::NFmiIndexMask() : itsData(), itsSorted(true), itsXSize(0), itsYSize(0) {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor with grid sizes.
 */
// ----------------------------------------------------------------------

NFmiIndexMask::NFmiIndexMask(size_type theXSize, size_type theYSize)
    : itsData(), itsSorted(true), itsXSize(theXSize), itsYSize(theYSize)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 *
 * \param theMask The mask to copy
 */
// ----------------------------------------------------------------------

NFmiIndexMask::NFmiIndexMask(const NFmiIndexMask& theMask)

    = default;

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 *
 * \param theMask The mask to copy
 * \return The mask assigned to
 */
// ----------------------------------------------------------------------

NFmiIndexMask& NFmiIndexMask::operator=(const NFmiIndexMask& theMask)
{
  try
  {
    if (this != &theMask)
    {
      itsData = theMask.itsData;
      itsSorted = theMask.itsSorted;
      itsXSize = theMask.itsXSize;
      itsYSize = theMask.itsYSize;
    }
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear the mask
 */
// ----------------------------------------------------------------------

void NFmiIndexMask::clear()
{
  try
  {
    itsData.clear();
    itsSorted = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the mask is empty
 *
 * \return True if the mask is empty
 */
// ----------------------------------------------------------------------

bool NFmiIndexMask::empty() const
{
  try
  {
    return itsData.empty();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Return the size of the mask
 *
 * \return The size
 */
// ----------------------------------------------------------------------

NFmiIndexMask::size_type NFmiIndexMask::size() const
{
  try
  {
    require_sorted();
    return itsData.size();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new index to the mask
 *
 * \param theIndex The index to add
 */
// ----------------------------------------------------------------------

void NFmiIndexMask::insert(value_type theIndex)
{
  try
  {
    itsData.push_back(theIndex);
    itsSorted = false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Find an index from the mask
 *
 * \param theIndex The index to search for
 * \return A const_iterator to the index (or end iterator)
 *
 * \todo Should use a binary_search for speed, but this method
 *       isn't an important one.
 */
// ----------------------------------------------------------------------

NFmiIndexMask::const_iterator NFmiIndexMask::find(value_type theIndex) const
{
  try
  {
    // not optimal, could use binary_search once sorted
    require_sorted();
    return std::find(itsData.begin(), itsData.end(), theIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the begin iterator
 *
 * \return The begin() iterator
 */
// ----------------------------------------------------------------------

NFmiIndexMask::const_iterator NFmiIndexMask::begin() const
{
  try
  {
    require_sorted();
    return itsData.begin();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the end iterator
 *
 * \return The end() iterator
 */
// ----------------------------------------------------------------------

NFmiIndexMask::const_iterator NFmiIndexMask::end() const
{
  try
  {
    require_sorted();
    return itsData.end();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison
 *
 * \param theMask The mask to compare with
 * \return True if the masks are equal
 */
// ----------------------------------------------------------------------

bool NFmiIndexMask::operator==(const NFmiIndexMask& theMask) const
{
  try
  {
    require_sorted();
    theMask.require_sorted();
    if (itsData.size() != theMask.itsData.size())
      return false;

    return (std::equal(itsData.begin(), itsData.end(), theMask.itsData.begin()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison
 *
 * \param theMask The mask to compare with
 * \return True if the masks are not equal
 */
// ----------------------------------------------------------------------

bool NFmiIndexMask::operator!=(const NFmiIndexMask& theMask) const
{
  try
  {
    require_sorted();
    theMask.require_sorted();
    return !operator==(theMask);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief AND operation with given mask
 *
 * \param theMask The mask to AND with
 * \return This
 */
// ----------------------------------------------------------------------

NFmiIndexMask& NFmiIndexMask::operator&=(const NFmiIndexMask& theMask)
{
  try
  {
    // Collect unique indices for both sets

    require_sorted();
    theMask.require_sorted();

    storage_type indexes;

    std::set_intersection(
        begin(), end(), theMask.begin(), theMask.end(), inserter(indexes, indexes.begin()));

    std::swap(indexes, itsData);

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief OR operation with given mask
 *
 * \param theMask The mask to OR with
 * \return This
 */
// ----------------------------------------------------------------------

NFmiIndexMask& NFmiIndexMask::operator|=(const NFmiIndexMask& theMask)
{
  try
  {
    require_sorted();
    theMask.require_sorted();

    storage_type indexes;

    std::set_union(
        begin(), end(), theMask.begin(), theMask.end(), inserter(indexes, indexes.begin()));

    std::swap(indexes, itsData);

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief DIFFERENCE operation with given mask
 *
 * \param theMask The mask to DIFFERENCE with
 * \return This
 */
// ----------------------------------------------------------------------

NFmiIndexMask& NFmiIndexMask::operator-=(const NFmiIndexMask& theMask)
{
  try
  {
    require_sorted();
    theMask.require_sorted();

    storage_type indexes;

    std::set_difference(
        begin(), end(), theMask.begin(), theMask.end(), inserter(indexes, indexes.begin()));

    std::swap(indexes, itsData);

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief SYMMETRIC DIFFERENCE operation with given mask
 *
 * \param theMask The mask to SYMMETRIC DIFFERENCE with
 * \return This
 */
// ----------------------------------------------------------------------

NFmiIndexMask& NFmiIndexMask::operator^=(const NFmiIndexMask& theMask)
{
  try
  {
    require_sorted();
    theMask.require_sorted();

    storage_type indexes;

    std::set_symmetric_difference(
        begin(), end(), theMask.begin(), theMask.end(), inserter(indexes, indexes.begin()));

    std::swap(indexes, itsData);

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief AND operation for masks
 *
 * \param theLhs The left hand side
 * \param theRhs The right hand side
 * \return The ANDed mask
 */
// ----------------------------------------------------------------------

NFmiIndexMask operator&(const NFmiIndexMask& theLhs, const NFmiIndexMask& theRhs)
{
  try
  {
    NFmiIndexMask tmp(theLhs);
    tmp &= theRhs;
    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief OR operation for masks
 *
 * \param theLhs The left hand side
 * \param theRhs The right hand side
 * \return The ORed mask
 */
// ----------------------------------------------------------------------

NFmiIndexMask operator|(const NFmiIndexMask& theLhs, const NFmiIndexMask& theRhs)
{
  try
  {
    NFmiIndexMask tmp(theLhs);
    tmp |= theRhs;
    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief DIFFERENCE operation for masks
 *
 * \param theLhs The left hand side
 * \param theRhs The right hand side
 * \return The Differenced mask
 */
// ----------------------------------------------------------------------

NFmiIndexMask operator-(const NFmiIndexMask& theLhs, const NFmiIndexMask& theRhs)
{
  try
  {
    NFmiIndexMask tmp(theLhs);
    tmp -= theRhs;
    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief SYMMETRIC DIFFERENCE operation for masks
 *
 * \param theLhs The left hand side
 * \param theRhs The right hand side
 * \return The Differenced mask
 */
// ----------------------------------------------------------------------

NFmiIndexMask operator^(const NFmiIndexMask& theLhs, const NFmiIndexMask& theRhs)
{
  try
  {
    NFmiIndexMask tmp(theLhs);
    tmp ^= theRhs;
    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Simple mask moving operation.
 *
 * Moves current mask to wanted direction but you can move it only by
 * grid-points (you have store original grid size in order to this to work).
 * If you move somethin outside the edge, that something will vanish. Nothing
 * will come out side the edges except 'emptyness'.
 *
 * \code
 *  |------| move by  |------| move it back  |------|
 *  |      |  2, -1   |      |   -2, 1       |      |
 *  |   ***|          |      |               |   ** |
 *  |  *** |  ==>     |     **   ==>         |  *** |
 *  |___**_|          |____***               |______|
 * \endcode
 *
 * Original purpose of this was to create e.g. 100 km 'radius' mask
 * and move it around the grid so that it wont have to be generated
 * (more costly than moving) for every grid-point separately.
 *
 * \param theXMove How many grid points you want to move the mask in x-direction (pos. value to
 * right).
 * \param theYMove How many grid points you want to move the mask in y-direction (pos. value to up).
 */
// ----------------------------------------------------------------------

void NFmiIndexMask::Move(int theXMove, int theYMove)
{
  try
  {
    // Have to have knowledge about the original grid in able to operate
    if (itsXSize && itsYSize)
    {
      // pitää täyttää uusi set-otus, koska ei voida editoida suoraan
      // set:issä olevaa dataa
      storage_type tmpData;
      value_type origIndex = 0;
      int xPos = 0;
      int yPos = 0;

      auto it = begin();
      auto endIt = end();
      for (; it != endIt; ++it)
      {
        origIndex = *it;
        xPos = theXMove + origIndex % itsXSize;
        yPos = theYMove + origIndex / itsXSize;
        if (xPos >= 0 && yPos >= 0 && static_cast<unsigned int>(xPos) < itsXSize &&
            static_cast<unsigned int>(yPos) < itsYSize)
        {
          tmpData.push_back(yPos * itsXSize + xPos);
        }
      }
      itsData.swap(tmpData);
      itsSorted = false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Make sure the data is sorted
 */
// ----------------------------------------------------------------------

void NFmiIndexMask::require_sorted() const
{
  try
  {
    if (itsSorted)
      return;

    sort(itsData.begin(), itsData.end());

    auto it = unique(itsData.begin(), itsData.end());
    itsData.erase(it, itsData.end());

    itsSorted = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
