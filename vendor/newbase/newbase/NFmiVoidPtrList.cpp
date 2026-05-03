// ======================================================================
/*!
 * \file NFmiVoidPtrList.cpp
 * \brief Implementation of class NFmiVoidPtrList
 * \todo This file should be annihilated
 */
// ======================================================================

#include "NFmiVoidPtrList.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiVoidPtrList::NFmiVoidPtrList()
    : itsFirstItem(nullptr), itsCurrentItem(nullptr), itsPreviousItem(nullptr), itsNumberOffItems(0)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param listItem The other list being copied
 */
// ----------------------------------------------------------------------

NFmiVoidPtrList::NFmiVoidPtrList(const NFmiVoidPtrList &listItem)
    : itsFirstItem(nullptr), itsCurrentItem(nullptr), itsPreviousItem(nullptr), itsNumberOffItems(0)
{
  try
  {
    CopyList(listItem);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::Add(NFmiVoidPtrData *value)
{
  try
  {
    AddEnd(value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::AddStart(NFmiVoidPtrData *value)
{
  try
  {
    if (value)
    {
      if (itsFirstItem)
        itsFirstItem = new NFmiVoidPtrItem(value, itsFirstItem);
      else
        itsFirstItem = itsCurrentItem = new NFmiVoidPtrItem(value, itsFirstItem);

      ++itsNumberOffItems;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::AddEnd(NFmiVoidPtrData *value)
{
  try
  {
    if (value)
    {
      ++itsNumberOffItems;
      if (!itsFirstItem)
      {
        itsFirstItem = itsCurrentItem = new NFmiVoidPtrItem(value);
      }
      else
      {
        while (itsCurrentItem->itsNextItem)
          itsCurrentItem = itsCurrentItem->itsNextItem;
        itsCurrentItem->itsNextItem = new NFmiVoidPtrItem(value);
        itsPreviousItem = itsCurrentItem;
        itsCurrentItem = itsCurrentItem->itsNextItem;
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::AddBefore(NFmiVoidPtrData *value)
{
  try
  {
    if (value)
    {
      if (!itsCurrentItem)  // empty list
        AddEnd(value);
      else if (!itsPreviousItem)  // added before first item
      {
        itsFirstItem = itsPreviousItem = new NFmiVoidPtrItem(value);
        itsFirstItem->itsNextItem = itsCurrentItem;
        ++itsNumberOffItems;
      }
      else
      {
        itsPreviousItem = itsPreviousItem->itsNextItem = new NFmiVoidPtrItem(value);
        itsPreviousItem->itsNextItem = itsCurrentItem;
        ++itsNumberOffItems;
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param removeValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::Remove(NFmiVoidPtrData *removeValue)
{
  try
  {
    NFmiVoidPtrItem *temp = itsFirstItem;
    if (temp->itsValue == removeValue)
    {
      DeleteItem();
      itsNumberOffItems--;
      return;
    }
    NFmiVoidPtrItem *previousItem = nullptr;

    for (; temp; previousItem = temp, temp = temp->itsNextItem)
    {
      if (temp->itsValue == removeValue)
      {
        previousItem->itsNextItem = temp->itsNextItem;
        delete temp;
        itsNumberOffItems--;
        return;
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param doDelete Undocumented, unused
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::Clear(
    bool /* doDelete */)  // miksi ei tarkisteta tätä ja jätetä deletoimatta tarvittaessa?
{
  try
  {
    while (itsFirstItem)
    {
      DeleteItem();
    }
    itsNumberOffItems = 0;
    itsCurrentItem = nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::DeleteItem()
{
  try
  {
    NFmiVoidPtrItem *deleteItem;
    deleteItem = itsFirstItem;
    itsFirstItem = itsFirstItem->itsNextItem;
    if (deleteItem == itsCurrentItem)
      itsCurrentItem = itsFirstItem;

    delete deleteItem;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param listItem Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::CopyList(const NFmiVoidPtrList &listItem)
{
  try
  {
    NFmiVoidPtrItem *temp = listItem.itsFirstItem;
    for (; temp; temp = temp->itsNextItem)
    {
      AddEnd(temp->itsValue);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param listItem Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::operator+=(const NFmiVoidPtrList &listItem)
{
  try
  {
    CopyList(listItem);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

//                            Iterator                          II

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param listItem Undocumented
 */
// ----------------------------------------------------------------------

NFmiVoidPtrIterator::NFmiVoidPtrIterator(NFmiVoidPtrList *listItem)
    : itsIndex(0),
      itsCurrentItem(listItem->itsFirstItem),
      itsPreviousItem(nullptr),
      itsListItem(listItem)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param listItem Undocumented
 */
// ----------------------------------------------------------------------

NFmiVoidPtrIterator::NFmiVoidPtrIterator(NFmiVoidPtrList &listItem)
    : itsIndex(0),
      itsCurrentItem(listItem.itsFirstItem),
      itsPreviousItem(nullptr),
      itsListItem(&listItem)
{
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrIterator::Reset()
{
  try
  {
    itsListItem->itsPreviousItem = itsPreviousItem = nullptr;
    itsCurrentItem = itsListItem->itsFirstItem;
    itsIndex = 0;
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

void *NFmiVoidPtrIterator::Next()
{
  try
  {
    NFmiVoidPtrItem *temp = itsCurrentItem;
    if (temp)
    {
      itsListItem->itsPreviousItem = itsPreviousItem;
      itsListItem->itsCurrentItem = temp;
      itsPreviousItem = itsCurrentItem;
      itsCurrentItem = itsCurrentItem->itsNextItem;
      itsIndex++;
      return temp->itsValue->itsDataValue;
    }
    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrIterator::NextPreviousPtr()
{
  try
  {
    if (itsPreviousItem)
      itsPreviousItem = itsPreviousItem->itsNextItem;
    else
      itsPreviousItem = itsListItem->itsFirstItem;
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

NFmiVoidPtrData *NFmiVoidPtrIterator::NextPtr()
{
  try
  {
    NFmiVoidPtrItem *temp = itsCurrentItem;
    if (temp)
    {
      itsPreviousItem = itsCurrentItem;
      itsCurrentItem = itsCurrentItem->itsNextItem;
      itsIndex++;
      return temp->itsValue;
    }

    return nullptr;
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

NFmiVoidPtrData *NFmiVoidPtrIterator::CurrentPtr()
{
  try
  {
    return itsCurrentItem->itsValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVoidPtrIterator::Next(void *&theItem)
{
  try
  {
    theItem = Next();
    return *&theItem ? true : false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVoidPtrIterator::NextPtr(NFmiVoidPtrData *&theItem)
{
  try
  {
    theItem = NextPtr();
    return *&theItem ? true : false;
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
void *NFmiVoidPtrIterator::Current()
{
  try
  {
    return itsCurrentItem ? itsCurrentItem->itsValue->itsDataValue : nullptr;
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

long NFmiVoidPtrIterator::Index() const
{
  try
  {
    return CheckIndex(itsIndex) ? itsIndex : kNotInList;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theNewValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVoidPtrIterator::Index(long theNewValue)
{
  try
  {
    if (!CheckIndex(theNewValue))
      return false;

    Reset();
    do
    {
      if (theNewValue == itsIndex)
      {
        return true;
      }
    } while (Next());
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVoidPtrIterator::CheckIndex(long theValue) const
{
  try
  {
    return theValue >= 0 && theValue < itsListItem->itsNumberOffItems;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
