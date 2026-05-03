// ======================================================================
/*!
 * \file NFmiDataIterator.cpp
 * \brief Implementation of class NFmiDataIterator
 */
// ======================================================================
/*
 * \class NFmiDataIterator
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataIterator.h"
#include "NFmiAreaMask.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataIterator::~NFmiDataIterator() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theData Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIterator::NFmiDataIterator(NFmiQueryInfo* theData)
    : itsData(theData), itsMask(nullptr), fMaskInUse(false)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theDataModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataIterator::DoForEach(NFmiDataModifier* /* theDataModifier */) {}
// ----------------------------------------------------------------------
/*!
 * Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataIterator::CheckIfMaskIsUsed()
{
  try
  {
    if (itsMask && itsMask->IsEnabled())
      fMaskInUse = true;
    else
      fMaskInUse = false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataIterator::IsMasked(const NFmiPoint& theLatLonPoint)
{
  try
  {
    if (fMaskInUse && itsMask)
      return itsMask->IsMasked(theLatLonPoint);

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
