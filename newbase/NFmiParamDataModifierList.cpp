// ======================================================================
/*!
 * \file NFmiParamDataModifierList.cpp
 * \brief Implementation of class NFmiParamDataModifierList
 */
// ======================================================================
/*!
 * \class NFmiParamDataModifierList
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiParamDataModifierList.h"
#include "NFmiDataIdent.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiParamDataModifierList::~NFmiParamDataModifierList()
{
  try
  {
    Clear(true);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiParamDataModifierList::NFmiParamDataModifierList() : itsList(), itsIter() {}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Add(NFmiParamDataModifier* theModifier)
{
  try
  {
    return itsList.AddEnd(theModifier) == true;
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

bool NFmiParamDataModifierList::Reset()
{
  try
  {
    itsIter = itsList.Start();
    return true;
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

bool NFmiParamDataModifierList::Next()
{
  try
  {
    return itsIter.Next() == true;
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

NFmiParamDataModifier* NFmiParamDataModifierList::Current()
{
  try
  {
    return itsIter.CurrentPtr();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Remove(bool fDeleteData)
{
  try
  {
    if (itsIter.Remove(fDeleteData))
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented
 */
// ----------------------------------------------------------------------

void NFmiParamDataModifierList::Clear(bool fDeleteData)
{
  try
  {
    itsList.Clear(fDeleteData);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Index(unsigned long theIndex)
{
  try
  {
    itsIter = itsList.Index(theIndex);
    if (Current())
      return true;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Find(const NFmiDataIdent& theParam)
{
  try
  {
    NFmiDataIdent* paramPtr = nullptr;
    for (Reset(); Next();)
    {
      paramPtr = Current()->Param();
      if (paramPtr && (*paramPtr == theParam))
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
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifierList::Find(const NFmiDataIdent& theParam, const NFmiLevel* theLevel)
{
  try
  {
    for (Reset(); Next();)
    {
      if (Current()->Match(theParam, theLevel))
        return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
