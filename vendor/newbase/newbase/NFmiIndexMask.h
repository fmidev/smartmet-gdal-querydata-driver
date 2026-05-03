// ======================================================================
/*!
 * \file
 * \brief Interface of class NFmiIndexMask
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"  // for std::auto_ptr
#include <vector>     // for std::vector

class NFmiIndexMaskIterator;

// ----------------------------------------------------------------------
//						MAIN CLASS
// ----------------------------------------------------------------------

class NFmiIndexMask
{
 public:
  typedef unsigned long value_type;
  typedef unsigned long size_type;
  typedef long difference_type;
  typedef std::vector<value_type> storage_type;
  typedef storage_type::const_iterator const_iterator;

  ~NFmiIndexMask();
  NFmiIndexMask();
  NFmiIndexMask(size_type theXSize, size_type theYSize);
  NFmiIndexMask(const NFmiIndexMask& theMask);
  NFmiIndexMask& operator=(const NFmiIndexMask& theMask);

  void clear();
  bool empty() const;
  size_type size() const;

  void insert(value_type theIndex);
  const_iterator find(value_type theIndex) const;

  const_iterator begin() const;
  const_iterator end() const;

  bool operator==(const NFmiIndexMask& theMask) const;
  bool operator!=(const NFmiIndexMask& theMask) const;

  NFmiIndexMask& operator&=(const NFmiIndexMask& theMask);
  NFmiIndexMask& operator|=(const NFmiIndexMask& theMask);
  NFmiIndexMask& operator-=(const NFmiIndexMask& theMask);
  NFmiIndexMask& operator^=(const NFmiIndexMask& theMask);

  size_type XSize() const { return itsXSize; }
  size_type YSize() const { return itsYSize; }
  void XSize(size_type newSize) { itsXSize = newSize; }
  void YSize(size_type newSize) { itsYSize = newSize; }
  void Move(int theXMove, int theYMove);  //! raaka hila määräinen siirto maskille
 private:
  mutable storage_type itsData;
  mutable bool itsSorted;

  void require_sorted() const;

  // HUOM! En ole ottanut koko määreitä huomioon missään vertailu tai yhdistely operaatioissa.
  size_type itsXSize;  //! gridin koko x-suunnassa (jos maski on tehty hiladatasta)
  size_type itsYSize;  //! gridin koko y-suunnassa (jos maski on tehty hiladatasta)

};  // class NFmiIndexMask

// ----------------------------------------------------------------------
//						FREE FUNCTIONS
// ----------------------------------------------------------------------

NFmiIndexMask operator&(const NFmiIndexMask& theLhs, const NFmiIndexMask& theRhs);

NFmiIndexMask operator|(const NFmiIndexMask& theLhs, const NFmiIndexMask& theRhs);

NFmiIndexMask operator-(const NFmiIndexMask& theLhs, const NFmiIndexMask& theRhs);

NFmiIndexMask operator^(const NFmiIndexMask& theLhs, const NFmiIndexMask& theRhs);

// ======================================================================
