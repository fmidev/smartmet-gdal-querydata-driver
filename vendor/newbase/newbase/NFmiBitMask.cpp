// ======================================================================
/*!
 * \file NFmiBitMask.cpp
 * \brief Implementation of class NFmiBitMask
 */
// ======================================================================
/*!
 * \class NFmiBitMask
 *
 * This class contains bitfield for X x Y grid data. This is used with
 * NFmiMaskedDataPool to mask used data. You can set on and off
 * bits at wanted index and then ask with given index if mask is on or off.
 *
 * \note
 * Even if this is bit field for 2 dimensional data, it can be accessad
 * only with one index that means index = (Y * XNumber) + X.
 * At the end of file there is couple of class-functions that can be used to
 * initialize NFmiBitMask with given NFmiGridBase entity. E.g. you can initialize
 * bitmask so that with given grid all the values over e.g. 0 are set to 1 and
 * others are set to 0.
 *
 * \see NFmiMaskedDataPool and NFmiGridBase
 *
 * \todo Several methods take as parameters const references to bool or long
 *       types, use regular params instead.
 */
// ======================================================================

#include "NFmiBitMask.h"
#include "NFmiDataModifier.h"
#include "NFmiQueryInfo.h"
#include <macgyver/Exception.h>

using namespace std;

long NFmiBitMask::itsNumberOfBitsInLong = sizeof(long) * 8;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theSize Undocumented
 */
// ----------------------------------------------------------------------

NFmiBitMask::NFmiBitMask(long theSize)
    : itsSize(), itsNumberOfBytesInMask(), itsNumberOfLongsInMask(), itsMaskData()
{
  try
  {
    itsMaskData = nullptr;
    Init(theSize);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theBitMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiBitMask::NFmiBitMask(const NFmiBitMask& theBitMask)
    : itsSize(theBitMask.itsSize),
      itsNumberOfBytesInMask(theBitMask.itsNumberOfBytesInMask),
      itsNumberOfLongsInMask(theBitMask.itsNumberOfLongsInMask),
      itsMaskData(nullptr)
{
  try
  {
    if (theBitMask.itsMaskData)
    {
      itsMaskData = new long[itsNumberOfLongsInMask];
      memcpy(itsMaskData, theBitMask.itsMaskData, itsNumberOfBytesInMask);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//---------------------------------------------------------------------------
//	Bit manipulation methods for the long typed mask arrays
//---------------------------------------------------------------------------

// ----------------------------------------------------------------------
/*!
 * \param theSize Undocumented
 * \todo Input is a const reference to a long???
 */
// ----------------------------------------------------------------------

void NFmiBitMask::CalculateMaskProperties(const unsigned long& theSize)
{
  try
  {
    itsSize = theSize;  // The number of BITS to be stored in mask array

    itsNumberOfBitsInLong = sizeof(long) * 8;

    if (theSize % itsNumberOfBitsInLong == 0)
      itsNumberOfLongsInMask = theSize / itsNumberOfBitsInLong;
    else
      itsNumberOfLongsInMask = theSize / itsNumberOfBitsInLong + 1;

    itsNumberOfBytesInMask = itsNumberOfLongsInMask * sizeof(long);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSize Undocumented
 */
// ----------------------------------------------------------------------

void NFmiBitMask::Init(long theSize)
{
  try
  {
    // Initializes the mask array with 0.

    CalculateMaskProperties(theSize);  // theSize: the number of BITS to be stored in mask array

    if (itsMaskData)
      delete[] itsMaskData;

    itsMaskData = new long[itsNumberOfLongsInMask];

    Mask(false);  // Unset all of the bits of the current mask
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBitMask Undocumented
 */
// ----------------------------------------------------------------------

void NFmiBitMask::Init(NFmiBitMask* theBitMask)
{
  try
  {
    // Initializes the mask array with the input bit mask

    CalculateMaskProperties(theBitMask->Size());

    if (itsMaskData)
      delete[] itsMaskData;

    itsMaskData = new long[itsNumberOfLongsInMask];

    // Initialize the mask with input bit mask
    memcpy(itsMaskData, theBitMask->itsMaskData, itsNumberOfBytesInMask);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param theBit Undocumented
 * \return Undocumented
 * \todo Input parameters are const references to elementary objects????
 */
// ----------------------------------------------------------------------

bool NFmiBitMask::Mask(const long& theIndex, const bool& theBit)
{
  try
  {
    // Sets/unsets the bit value in the long located in mask array at index 'theIndex'.
    // This proceeds in two steps:
    //
    //	 1) In the mask array, find the long item position by index 'theIndex'
    //	 2) Set/unset the bit value in the current long by index 'longIndex'

    if (theIndex >= itsSize)
      return false;

    long arrayIndex = theIndex / itsNumberOfBitsInLong;   // The long position in mask array
    long longIndex = theIndex % itsNumberOfBitsInLong;    // The bit position in long
    LongBit(longIndex, theBit, itsMaskData[arrayIndex]);  // Set/unset the bit value in current long

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBit Undocumented
 * \return Undocumented
 * \todo Input is a const reference to a bool????
 */
// ----------------------------------------------------------------------

bool NFmiBitMask::Mask(const bool& theBit)
{
  try
  {
    // Sets/unsets ALL of the bits of the mask array value to 'theBit'.
    // This is done by filling the whole mask array with masked longs.

    for (long i = 0; i < itsNumberOfLongsInMask; i++)
      LongBit(theBit, itsMaskData[i]);

    return true;
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
 * \todo Const reference to a long as input????
 */
// ----------------------------------------------------------------------

bool NFmiBitMask::IsMasked(const long& theIndex) const
{
  try
  {
    // Tells whether the data value associated with index 'theIndex' is masked or not
    // by returning the bit value from the long located in mask array at index 'theIndex'.
    // This proceeds in two steps:
    //
    //	 1) In the mask array, find the long item position by index 'theIndex'
    //	 2) Extract the bit value from the current long by index 'longIndex'

    if (theIndex >= itsSize)
      return false;

    long arrayIndex = theIndex / itsNumberOfBitsInLong;  // The long position in mask array
    long longIndex = theIndex % itsNumberOfBitsInLong;   // The bit position in long
    return IsLongBit(longIndex,
                     itsMaskData[arrayIndex]);  // Return the bit value in current long as Boolean
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//---------------------------------------------------------------------------
//	Bit manipulation methods for the long type
//---------------------------------------------------------------------------

// ----------------------------------------------------------------------
/*!
 * \param theBitIndex Undocumented
 * \param theBit Undocumented
 * \param theMaskedValue Undocumented
 * \todo Remove use of trivial const references
 */
// ----------------------------------------------------------------------

void NFmiBitMask::LongBit(const long& theBitIndex, const bool& theBit, long& theMaskedValue)
{
  try
  {
    // Set/unsets the bit 'theBitIndex' in long 'theMaskedValue' into Boolean 'theBit'

    long maskedLong = 1L << theBitIndex;

    if (theBit)
      theMaskedValue = theMaskedValue | maskedLong;  // Set bit
    else
      theMaskedValue = theMaskedValue & ~maskedLong;  // Unset bit
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBit Undocumented
 * \param theMaskedValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiBitMask::LongBit(const bool& theBit, long& theMaskedValue)
{
  try
  {
    // Set ALL bits of 'theMaskedValue' into Boolean 'theBit'
    if (theBit)
      theMaskedValue = ~0L;  // Turn all bits in long into 1
    else
      theMaskedValue = 0L;  // Turn all bits in long into 0
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBitIndex Undocumented
 * \param theMaskedValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiBitMask::IsLongBit(const long& theBitIndex, const long& theMaskedValue) const
{
  try
  {
    // Tells whether the bit 'theBitIndex' in long 'theMaskedValue' is set or unset.
    long maskedLong = 1L << theBitIndex;

    return (theMaskedValue & maskedLong) == maskedLong;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//   3. Operators

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theBitMask The other object being copied
 * \return The object assigned to
 * \bug Must protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiBitMask& NFmiBitMask::operator=(const NFmiBitMask& theBitMask)
{
  try
  {
    itsSize = theBitMask.Size();
    itsNumberOfBitsInLong = theBitMask.itsNumberOfBitsInLong;
    itsNumberOfBytesInMask = theBitMask.itsNumberOfBytesInMask;
    itsNumberOfLongsInMask = theBitMask.itsNumberOfLongsInMask;

    if (itsMaskData)
      delete[] itsMaskData;

    itsMaskData = new long[itsNumberOfLongsInMask];

    memcpy(itsMaskData, theBitMask.itsMaskData, itsNumberOfBytesInMask);

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theBitMask The other object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiBitMask::operator==(const NFmiBitMask& theBitMask) const
{
  try
  {
    if (itsSize == theBitMask.Size())
    {
      if (memcmp(itsMaskData, theBitMask.itsMaskData, itsNumberOfBytesInMask) == 0)
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
 * Inequality comparison
 *
 * \param theBitMask The other object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiBitMask::operator!=(const NFmiBitMask& theBitMask) const
{
  try
  {
    return !(*this == theBitMask);
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

long NFmiBitMask::MaskedCount() const
{
  try
  {
    long returnValue = 0;
    for (long i = 0; i < itsSize; i++)
    {
      if (IsMasked(i))
        returnValue++;
    }
    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theData Undocumented
 * \param theMaskMethod Undocumented
 */
// ----------------------------------------------------------------------

void NFmiBitMask::Init(NFmiQueryInfo* theData, NFmiDataModifier* theMaskMethod)
{
  try
  {
    Init(theData->HPlaceDescriptor().Size());
    long i = 0;
    theData->ResetLocation();
    while (theData->NextLocation())
    {
      Mask(i++, theMaskMethod->BoolOperation(theData->FloatValue()));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Kaikki maskin bitit käännetään vastakkaisiksi.
void NFmiBitMask::InverseMask()
{
  try
  {
    for (int i = 0; i < itsNumberOfLongsInMask; i++)
      itsMaskData[i] = ~itsMaskData[i];  // bitwise not kääntää bitit vastakkain
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
