// ======================================================================
/*!
 * \file NFmiModifiedDataIterator.cpp
 * \brief Implementation of class NFmiModifiedDataIterator
 */
// ======================================================================
/*!
 * \class NFmiModifierDataIterator
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiModifiedDataIterator.h"
#include "NFmiInfoModifier.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiModifiedDataIterator::~NFmiModifiedDataIterator() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theData Undocumented
 * \param dx Undocumented
 * \param dy Undocumented
 * \param dt Undocumented
 */
// ----------------------------------------------------------------------

NFmiModifiedDataIterator::NFmiModifiedDataIterator(NFmiInfoModifier* theData,
                                                   long dx,
                                                   long dy,
                                                   long dt)
    : NFmiRelativeDataIterator(theData->Info(), dx, dy, dt), itsPrimaryModifier(theData)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theDataModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiModifiedDataIterator::DoForEach(NFmiDataModifier* theDataModifier)
{
  try
  {
    if (!theDataModifier)
      return;

    theDataModifier->Clear();

    for (long t = itsDtStart; t <= itsDtEnd; t++)
    {
      for (long y = itsDyStart; y <= itsDyEnd; y++)
      {
        for (long x = itsDxStart; x <= itsDxEnd; x++)
        {
          theDataModifier->Calculate(static_cast<float>(itsPrimaryModifier->PeekValue(t, x, y)));
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
