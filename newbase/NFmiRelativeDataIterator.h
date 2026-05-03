// ======================================================================
/*!
 * \file NFmiRelativeDataIterator.h
 * \brief Interface of class NFmiRelativeDataIterator
 */
// ======================================================================

#pragma once

#include "NFmiDataIterator.h"

//! Undocumented

class NFmiRelativeDataIterator : public NFmiDataIterator
{
 public:
  virtual ~NFmiRelativeDataIterator();
  NFmiRelativeDataIterator(NFmiQueryInfo* theData, long dx = 0, long dy = 0, long dt = 0);

  NFmiRelativeDataIterator(NFmiQueryInfo* theData,
                           long dxStart,
                           long dyStart,
                           long dtStart,
                           long dxEnd,
                           long dyEnd,
                           long dtEnd);

  void SetDimensions(long dx, long dy, long dt);
  virtual void DoForEach(NFmiDataModifier* theDataModifier = 0);

 protected:
  long itsDtStart;
  long itsDyStart;
  long itsDxStart;
  long itsDtEnd;
  long itsDyEnd;
  long itsDxEnd;

};  // class NFmiRelativeDataIterator

// ======================================================================
