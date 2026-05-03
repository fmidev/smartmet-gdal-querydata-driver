// ======================================================================
/*!
 * \file NFmiRelativeDataIterator.cpp
 * \brief Implementation of class NFmiRelativeDataIterator
 */
// ======================================================================
/*!
 * \class NFmiRelativeDataIterator
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiRelativeDataIterator.h"
#include "NFmiDataModifier.h"
#include "NFmiQueryInfo.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiRelativeDataIterator::~NFmiRelativeDataIterator() = default;
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

NFmiRelativeDataIterator::NFmiRelativeDataIterator(NFmiQueryInfo* theData,
                                                   long dx,
                                                   long dy,
                                                   long dt)
    : NFmiDataIterator(theData),
      itsDtStart(-dt),
      itsDyStart(-dy),
      itsDxStart(-dx),
      itsDtEnd(dt),
      itsDyEnd(dy),
      itsDxEnd(dx)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theData Undocumented
 * \param dxStart Undocumented
 * \param dyStart Undocumented
 * \param dtStart Undocumented
 * \param dxEnd Undocumented
 * \param dyEnd Undocumented
 * \param dtEnd Undocumented
 */
// ----------------------------------------------------------------------

NFmiRelativeDataIterator::NFmiRelativeDataIterator(NFmiQueryInfo* theData,
                                                   long dxStart,
                                                   long dyStart,
                                                   long dtStart,
                                                   long dxEnd,
                                                   long dyEnd,
                                                   long dtEnd)
    : NFmiDataIterator(theData),
      itsDtStart(dtStart),
      itsDyStart(dyStart),
      itsDxStart(dxStart),
      itsDtEnd(dtEnd),
      itsDyEnd(dyEnd),
      itsDxEnd(dxEnd)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theDataModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiRelativeDataIterator::DoForEach(NFmiDataModifier* theDataModifier)
{
  try
  {
    if (!theDataModifier)
      return;

    CheckIfMaskIsUsed();  // käy asettamassa fMaskInUse-flagin päälle (optimointia, jos ei ole
                          // maskia käytössä)
    theDataModifier->Clear();

    for (long t = itsDtStart; t <= itsDtEnd; t++)
    {
      for (long y = itsDyStart; y <= itsDyEnd; y++)
      {
        for (long x = itsDxStart; x <= itsDxEnd; x++)
        {
          if ((!fMaskInUse) || IsMasked(itsData->PeekLocationLatLon(x, y)))  // tehdään laskut, jos
            // maski ei ole käytössä
            // tai sitten jos maski
            // on käytössä ja
            // 'päällä'
            theDataModifier->Calculate(itsData->PeekValue(t, x, y));
        }
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
 * \param dx Undocumented
 * \param dy Undocumented
 * \param dt Undocumented
 */
// ----------------------------------------------------------------------

void NFmiRelativeDataIterator::SetDimensions(long dx, long dy, long dt)
{
  try
  {
    itsDxStart = -dx;
    itsDyStart = -dy;
    itsDtStart = -dt;
    itsDxEnd = dx;
    itsDyEnd = dy;
    itsDtEnd = dt;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
