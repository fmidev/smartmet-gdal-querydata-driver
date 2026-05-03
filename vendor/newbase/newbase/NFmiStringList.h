// ======================================================================
/*!
 * \file NFmiStringList.h
 * \brief Interface of class NFmiStringList
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiString.h"
#include "NFmiVoidPtrList.h"

//! Undocumented
class NFmiStringList
{
 public:
  virtual ~NFmiStringList();
  NFmiStringList();
  NFmiStringList(const NFmiStringList &theList);

  NFmiStringList &operator=(NFmiStringList &theList);

  virtual void Add(NFmiString *theItem);  // ei tee kopiota!!!!!!
  virtual void Add(NFmiStringList *theList);
  void Add(NFmiString *theStr, unsigned short theLengthLimitForStrItem);
  bool Reset(FmiDirection theDirect = kForward);
  bool Next(NFmiString **theItem);
  bool Next();
  bool Previous();
  NFmiString *Current() const;

  bool FindWithStatus(long status);
  bool Find(const NFmiString &string);
  int NumberOfItems() const;
  void Clear(bool fDeleteData = false);
  bool Remove();

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  bool Index(int theIndex);
  int Index() const;

 private:
  typedef std::vector<NFmiString *> StorageType;
  StorageType itsList;

  //! Note: For backward compatibility this index runs from 0 to N-1
  unsigned int itsIndex;

  void Destroy();

};  // class NFmiStringList

//! Undocumented, should be removed
typedef NFmiStringList *PNFmiStringList;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiStringList::NFmiStringList() : itsList(), itsIndex(0) {}
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiStringList::Index(int theIndex)
{
  if (theIndex >= 0 && static_cast<unsigned int>(theIndex) < itsList.size())
  {
    itsIndex = theIndex;
    return true;
  }
  else
  {
    itsIndex = 0;
    return false;
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the index of current item.
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiStringList::Index() const
{
  return itsIndex;
}
// ----------------------------------------------------------------------
/*!
 * Returns the number of items in the list itsList.
 *
 * \return The size of the list
 */
// ----------------------------------------------------------------------

inline int NFmiStringList::NumberOfItems() const
{
  return static_cast<int>(itsList.size());
}
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiStringList
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 * \todo Interesting const_cast - kill it
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiStringList &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiStringList
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiStringList &ob)
{
  return ob.Read(file);
}

// ======================================================================
