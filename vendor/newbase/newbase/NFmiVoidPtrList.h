// ======================================================================
/*!
 * \file NFmiVoidPtrList.h
 * \brief Interface to class NFmiVoidPtrList
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"

const int kDelete = true;
const int kNoDelete = false;
const long kNotInList = -1;

// ----------------------------------------------------------------------
/*!
 * \class NFmiVoidPtrData
 *
 * Ignore it, do not use it.
 */
// ----------------------------------------------------------------------

class NFmiVoidPtrData
{
  friend class NFmiVoidPtrList;
  friend class NFmiVoidPtrItem;
  friend class NFmiVoidPtrIterator;

 public:
  NFmiVoidPtrData(void* value) : itsDataValue(value), itsReferenceCount(0) {}
  void* GetVoidPtr() { return itsDataValue; }
  bool Remove() { return --itsReferenceCount ? false : true; }

 private:
  NFmiVoidPtrData(const NFmiVoidPtrData& theData);
  NFmiVoidPtrData& operator=(const NFmiVoidPtrData& theData);

  void* itsDataValue;
  int itsReferenceCount;
};

// ----------------------------------------------------------------------
/*!
 * \class NFmiVoidPtrItem
 *
 * Ignore it, do not use it
 *
 */
// ----------------------------------------------------------------------

class NFmiVoidPtrItem
{
  friend class NFmiVoidPtrList;
  friend class NFmiVoidPtrIterator;

 public:
  ~NFmiVoidPtrItem()
  {
    if (itsValue->Remove())
      delete itsValue;
  }

  NFmiVoidPtrItem(NFmiVoidPtrData* value, NFmiVoidPtrItem* item = 0) : itsNextItem(item), itsValue()
  {
    if (value->itsReferenceCount)
    {
      itsValue = value;
    }
    else
      itsValue = value;
    itsValue->itsReferenceCount++;
  }

 private:
  NFmiVoidPtrItem(const NFmiVoidPtrItem& theItem);
  NFmiVoidPtrItem& operator=(const NFmiVoidPtrItem& theItem);

  NFmiVoidPtrItem* itsNextItem;
  NFmiVoidPtrData* itsValue;
};

// ----------------------------------------------------------------------
/*!
 * \class NFmiVoidPtrList
 *
 * Your risk your life for using this for anything new! Use
 * std::list instead.
 */
// ----------------------------------------------------------------------

class NFmiVoidPtrList
{
  friend class NFmiVoidPtrIterator;

 public:
  virtual ~NFmiVoidPtrList() { NFmiVoidPtrList::Clear(0); }
  NFmiVoidPtrList();
  NFmiVoidPtrList(const NFmiVoidPtrList& listItem);

  virtual void Add(void* value) { Add(new NFmiVoidPtrData(value)); }
  virtual void AddStart(void* value) { AddStart(new NFmiVoidPtrData(value)); }
  virtual void AddEnd(void* value) { AddEnd(new NFmiVoidPtrData(value)); }
  virtual void AddBefore(void* value) { AddBefore(new NFmiVoidPtrData(value)); }
  virtual void Add(NFmiVoidPtrData* value);
  virtual void AddStart(NFmiVoidPtrData* value);

  virtual void AddEnd(NFmiVoidPtrData* value);
  virtual void AddBefore(NFmiVoidPtrData* value);
  virtual void Remove(NFmiVoidPtrData* removeValue);
  virtual void Clear(bool doDelete = kNoDelete);

  int NumberOfItems() { return itsNumberOffItems; }
  NFmiVoidPtrItem* FirstItem() { return itsFirstItem; }
  void operator+=(const NFmiVoidPtrList& listItem);

  virtual void CopyList(const NFmiVoidPtrList& listItem);
  virtual void DeleteItem();

 protected:
  NFmiVoidPtrItem* itsFirstItem;
  NFmiVoidPtrItem* itsCurrentItem;
  NFmiVoidPtrItem* itsPreviousItem;
  int itsNumberOffItems;

 private:
  NFmiVoidPtrList& operator=(const NFmiVoidPtrList& theList);
};

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
//                            Iterator                          II
// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

// ----------------------------------------------------------------------
/*!
 * \class NFmiVoidPtrIterator
 *
 * Do not use this
 *
 */
// ----------------------------------------------------------------------

class NFmiVoidPtrIterator
{
 public:
  virtual ~NFmiVoidPtrIterator() {}
  NFmiVoidPtrIterator(NFmiVoidPtrList* listItem);
  NFmiVoidPtrIterator(NFmiVoidPtrList& listItem);

  void Reset();
  virtual void* Next();
  virtual NFmiVoidPtrData* NextPtr();
  virtual NFmiVoidPtrData* CurrentPtr();
  virtual bool Next(void*& theItem);
  virtual void NextPreviousPtr();  // tämä on todella tyhmää
  virtual bool NextPtr(NFmiVoidPtrData*& theItem);
  long Index() const;
  bool Index(long theNewValue);

  void* operator++() { return Next(); }
  virtual void* Current();

 private:
  //  NFmiVoidPtrIterator(const NFmiVoidPtrIterator & theIterator);
  //  NFmiVoidPtrIterator & operator=(const NFmiVoidPtrIterator & theIterator);

  bool CheckIndex(long theValue) const;
  long itsIndex;
  NFmiVoidPtrItem* itsCurrentItem;
  NFmiVoidPtrItem* itsPreviousItem;
  NFmiVoidPtrList* itsListItem;
};

// ======================================================================
