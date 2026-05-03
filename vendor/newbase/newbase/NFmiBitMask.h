// ======================================================================
/*!
 * \file NFmiBitMask.h
 * \brief Interface of class NFmiBitMask
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiGridBase.h"
#include <memory>

class NFmiQueryInfo;
class NFmiDataModifier;

//! Undocumented
class NFmiBitMask
{
 public:
  virtual ~NFmiBitMask();
  explicit NFmiBitMask(long theSize = 32);
  NFmiBitMask(const NFmiBitMask& theMaskData);

  NFmiBitMask& operator=(const NFmiBitMask& theBitMask);
  bool operator==(const NFmiBitMask& theBitMask) const;
  bool operator!=(const NFmiBitMask& theBitMask) const;

  // Bit-related 'long' typed mask array modification methods
  long Size() const;
  void Init(long theSize = 32);
  void Init(NFmiBitMask* theBitMask);
  void Init(NFmiQueryInfo* theData, NFmiDataModifier* theMaskMethod);

  bool Mask(const bool& theBit);
  bool Mask(const long& theIndex, const bool& theBit);
  bool IsMasked(const long& theIndex) const;
  long MaskedCount() const;
  void InverseMask();

 private:
  // Bit-related 'long' modification methods
  void LongBit(const long& theBitIndex, const bool& theBit, long& theMaskedValue);
  void LongBit(const bool& theBit, long& theMaskedValue);
  bool IsLongBit(const long& theBitIndex, const long& theMaskedValue) const;
  void CalculateMaskProperties(const unsigned long& theSize);

  static long itsNumberOfBitsInLong;

  long itsSize;
  long itsNumberOfBytesInMask;
  long itsNumberOfLongsInMask;
  long* itsMaskData;

 public:  // template functions
  // if both x and y are 0, use grids x and y numbers
  template <class T>
  void Init(NFmiGridBase& theGrid,
            T theMaskOperation,
            unsigned long theXNumber = 0,
            unsigned long theYNumber = 0)
  {
    unsigned long gridXNumber = theGrid.XNumber();
    unsigned long gridYNumber = theGrid.YNumber();
    if (theXNumber == gridXNumber && theYNumber == gridYNumber)
      InitEqualSize(theGrid, theMaskOperation);
    else
      InitFixedSize(theGrid, theMaskOperation, theXNumber, theYNumber);
  }

  template <class T>
  void Operation(NFmiBitMask& theMask, T theMaskOperation)
  {
  }

 private:  // template functions
  template <class T>
  void InitEqualSize(NFmiGridBase& theGrid, T theMaskOperation)
  {
    Init(theGrid.Size());
    for (theGrid.Reset(); theGrid.Next();)
      Mask(theGrid.Index(), theMaskOperation(theGrid.FloatValue()));
  }

  template <class T>
  void InitFixedSize(NFmiGridBase& theGrid,
                     T theMaskOperation,
                     unsigned long theXNumber,
                     unsigned long theYNumber)
  {
    unsigned long gridXNumber = theGrid.XNumber();
    unsigned long gridYNumber = theGrid.YNumber();
    double dx = 0.;
    double dy = 0.;
    double deltaX = 1. / (theXNumber - 1);
    double deltaY = 1. / (theYNumber - 1);
    double value;

    Init(theXNumber * theYNumber);
    for (unsigned long i = 0; i < theYNumber; i++)
    {
      dx = 0.;
      for (unsigned long j = 0; j < theXNumber; j++)
      {
        if (theGrid.InterpolateToGridPoint(dx * (gridXNumber - 1), dy * (gridYNumber - 1), value))
          Mask(i * theXNumber + j, theMaskOperation(value));
        dx += deltaX;
      }
      dy += deltaY;
    }
  }

};  // class NFmiBitMask

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiBitMask::~NFmiBitMask()
{
  if (itsMaskData)
    delete[] itsMaskData;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiBitMask::Size() const
{
  return itsSize;
}

// ======================================================================
