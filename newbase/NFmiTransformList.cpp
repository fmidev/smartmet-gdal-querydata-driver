// ======================================================================
/*!
 * \file NFmiTransformList.cpp
 * \brief Implementation of class NFmiTransformList
 */
// ======================================================================
/*!
 * \class NFmiTransformList
 *
 * Performs data transformation based on data mapping list.
 * List transforms input data values into output data values in a piecewice manner
 * specified in transformation list. At its creation, list will be composed of usually
 * several linear "sub-ranges" telling how to transform values belonging to these
 * sub-ranges.
 *
 * NOTES: Currently list data structure is implemented in arrays instead of
 *			 real list structures
 *
 * INPUT PARAMETERS/ARGUMENT LISTS
 *
 * theMaxPairNumber = maximum number of data pairs to be mapped
 *_____________________________________________________________________________
 * 2.1	virtual AddDataMapping(const NFmiPoint &firstPair, const NFmiPoint &lastPair)
 *
 * Adds a new data mapping as two value pair:
 *
 * firstPair.X(): first value on INPUT range
 * firstPair.Y(): first value on OUTPUT range
 *
 * lastPair.X(): last value on INPUT range
 * lastPair.Y(): last value on OUTPUT range
 *
 *	NOTE:
 *		-input range is considered as an ascending range [firstPair.X(), lastPair.X()],
 *		where firstPair.X() <= lastPair.X()
 *
 *		-input values smaller than firstPair.X() are clamped to firstPair.Y().
 *
 *		-input values bigger than lastPair.X() are clamped to lastPair.Y().
 *
 *_____________________________________________________________________________
 * 2.2	virtual double Transform(double value)
 *
 * Selects the correct data mapping range and performs data transformation.
 * Linear interpolation is used to get the intermediate data value transformed.
 *
 *_____________________________________________________________________________
 * 2.3	virtual bool Interpolate(const NFmiPoint &firstPair, const NFmiPoint &lastPair)
 *
 *	Returns true if linear interpolation between points firstPair and lastPair succeeded.
 * Otherwise returns false. Value to be interpolated is retrieved from a private variable.
 * Respectively, resulting interpolated value is stored in an other private variable.
 *
 *_____________________________________________________________________________
 * 2.4	void First()
 *
 * Sets the current list item (=list sub range) index to zero
 *
 *_____________________________________________________________________________
 * 2.5	virtual void Init(int theMaxPairNumber)
 *
 * Initializes private variables
 *
 *_____________________________________________________________________________
 * 2.7	virtual bool Allocate(int theMaxPairNumber);
 *
 * Alocates memory for list arrays
 *
 *
 *
 * ------------
 * 3. OPERATORS
 * ------------
 *
 * // operator=
 *------------------------------------------------------------NFmiTransformList
 * NFmiTransformList& NFmiTransformList::operator= (const NFmiTransformList &theList);
 *
 *
 * -----------
 * 4. EXAMPLES
 * -----------
 *
 *
 * How to add a few transformation pairs into the NFmiTransformList to emphasize
 * low data values (<40) and push down high values (>40) on input range [0, 255]?
 * Output range will be [0, 60].
 *
 *		NFmiTransformList List;
 *
 *		-Stretch up lower input data values on range 0. .. 40.
 *
 *		NFmiPoint firstPair;
 *		NFmiPoint lastPair;
 *
 *		firstPair = NFmiPoint(0., 0.);
 *		lastPair = NFmiPoint(10., 30.);
 *		List.AddDataMapping(firstPair, lastPair);
 *
 *
 *		firstPair = NFmiPoint(10., 30.);
 *		lastPair = NFmiPoint(40., 40.);
 *		List.AddDataMapping(firstPair, lastPair);
 *
 *
 *		-Push down higher input data values on range 40. .. 60
 *
 *		firstPair = NFmiPoint(40., 40.);
 *		lastPair = NFmiPoint(255., 60.);
 *		List.AddDataMapping(firstPair, lastPair);
 *
 *
 *
 *
 *		-Transformation list is now completed and can be used to piecewise transform
 *arbitrary
 *		data values. For instance, what's the transformed value for input data value 80.5?
 *
 *		double outputValue = List.Transform(80.5);
 *
 *		At first Transform() will find out the correct input data range.
 *		Linear interpolation is used to find out the piecewise tranformed output value on
 *the
 *		output range. Finally the output value is returned and assigned to variable
 *outputValue.
 *
 *    The value for outputValue should be about 43.76
 *
 */
// ======================================================================

#include "NFmiTransformList.h"
#include <macgyver/Exception.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
// memcpy()

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiTransformList::~NFmiTransformList()
{
  try
  {
    if (itsFirstPair)
    {
      delete[] itsFirstPair;
      itsFirstPair = nullptr;
    }

    if (itsLastPair)
    {
      delete[] itsLastPair;
      itsLastPair = nullptr;
    }

    if (itsOutputInputRatio)
    {
      delete[] itsOutputInputRatio;
      itsOutputInputRatio = nullptr;
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theMaxPairNumber Undocumented
 */
// ----------------------------------------------------------------------

NFmiTransformList::NFmiTransformList(int theMaxPairNumber)
    : itsMaxPairNumber(),
      itsIncrementSize(),
      itsCurrentIndex(),
      itsCurrentMaxIndex(),
      itsPreviousInputRangeIndex(),
      itsPreviousInputValue(),
      itsPreviousOutputValue(),
      itsMinInputValue(),
      itsMaxInputValue(),
      itsInputValue(),
      itsOutputValue(),
      itsFirstPair(nullptr),
      itsLastPair(nullptr),
      itsVeryFirstPair(),
      itsVeryLastPair(),
      itsEps(NFmiPoint(0.0000001, 0.0000001)),
      itsOutputInputRatio(nullptr)
{
  try
  {
    Allocate(theMaxPairNumber);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theList The object being copied
 * \return The object assigned to
 * \bug Must protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiTransformList& NFmiTransformList::operator=(const NFmiTransformList& theList)
{
  try
  {
    // Copy FirstPair array
    if (itsFirstPair)
    {
      if (itsMaxPairNumber != theList.itsMaxPairNumber)
      {
        delete[] itsFirstPair;
        itsFirstPair = new NFmiPoint[theList.itsMaxPairNumber];
      }
    }
    else
      itsFirstPair = new NFmiPoint[theList.itsMaxPairNumber];

    memcpy(itsFirstPair, theList.itsFirstPair, theList.itsMaxPairNumber * sizeof(NFmiPoint));

    // Copy LastPair array
    if (itsLastPair)
    {
      if (itsMaxPairNumber != theList.itsMaxPairNumber)
      {
        delete[] itsLastPair;
        itsLastPair = new NFmiPoint[theList.itsMaxPairNumber];
      }
    }
    else
      itsLastPair = new NFmiPoint[theList.itsMaxPairNumber];

    memcpy(itsLastPair, theList.itsLastPair, theList.itsMaxPairNumber * sizeof(NFmiPoint));

    // Copy OutputInputRatio array
    if (itsOutputInputRatio)
    {
      if (itsMaxPairNumber != theList.itsMaxPairNumber)
      {
        delete[] itsOutputInputRatio;
        itsOutputInputRatio = new double[theList.itsMaxPairNumber];
      }
    }
    else
      itsOutputInputRatio = new double[theList.itsMaxPairNumber];

    memcpy(itsOutputInputRatio,
           theList.itsOutputInputRatio,
           theList.itsMaxPairNumber * sizeof(double));

    // Copy the very first and last pairs from the input list
    itsVeryFirstPair = theList.itsVeryFirstPair;
    itsVeryLastPair = theList.itsVeryLastPair;

    // 5.10.98/EL <--

    itsMaxPairNumber = theList.itsMaxPairNumber;
    itsIncrementSize = theList.itsIncrementSize;
    itsCurrentIndex = theList.itsCurrentIndex;
    itsCurrentMaxIndex = theList.itsCurrentMaxIndex;
    itsInputValue = theList.itsInputValue;
    itsOutputValue = theList.itsOutputValue;

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
 * \param theList The object being compared to
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiTransformList::operator==(const NFmiTransformList& theList)
{
  try
  {
    // NOTE: Currently doesn't check for 'itsOutputInputRatio' equality !

    if (!((itsMaxPairNumber == theList.itsMaxPairNumber) &&
          (itsIncrementSize == theList.itsIncrementSize) &&
          (itsCurrentIndex == theList.itsCurrentIndex) &&
          (itsCurrentMaxIndex == theList.itsCurrentMaxIndex)))
      return false;

    // No pairs inserted in lists ?
    // If so, lists can be taken as identical lists
    if ((!itsFirstPair) && (!theList.itsFirstPair) && (!itsLastPair) && (!theList.itsLastPair))
      return true;

    // Pairwise list item comparison
    int ind = 0;

    if (itsFirstPair && theList.itsFirstPair && itsLastPair && theList.itsLastPair)
    {
      do
      {
        if (!Equal(itsFirstPair[ind], theList.itsFirstPair[ind]))
          return false;
        if (!Equal(itsLastPair[ind], theList.itsLastPair[ind]))
          return false;
      } while (++ind <= itsCurrentMaxIndex);
    }
    else
      return false;

    // koskelam: This point is never reached
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param p1 Undocumented
 * \param p2 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTransformList::Equal(NFmiPoint& p1, NFmiPoint& p2)
{
  try
  {
    double diffX, diffY;

    diffX = fabs(p1.X() - p2.X());
    diffY = fabs(p1.Y() - p2.Y());

    if ((diffX < itsEps.X()) && (diffY < itsEps.Y()))
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
 *
 */
// ----------------------------------------------------------------------

void NFmiTransformList::First()
{
  itsCurrentIndex = 0;
}

// ----------------------------------------------------------------------
/*!
 * \param theMaxPairNumber Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTransformList::Init(int theMaxPairNumber)
{
  try
  {
    First();
    itsMaxPairNumber = theMaxPairNumber;
    itsIncrementSize = itsMaxPairNumber;
    itsCurrentMaxIndex = itsCurrentIndex;
    itsInputValue = itsOutputValue = kFloatMissing;
    itsFirstPair = nullptr;
    itsLastPair = nullptr;
    itsOutputInputRatio = nullptr;
    itsPreviousInputRangeIndex = 0;
    itsPreviousInputValue = itsPreviousOutputValue = kMaxDouble;
    itsVeryFirstPair = NFmiPoint(kMaxDouble, kMaxDouble);
    itsVeryLastPair = NFmiPoint(kMinDouble, kMaxDouble);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theMaxPairNumber Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTransformList::Allocate(int theMaxPairNumber)
{
  try
  {
    Init(theMaxPairNumber);

    if (itsCurrentIndex >= itsMaxPairNumber)
    {
      /* TÄÄ PITÄS TEHDÄ REMALLOCILLA TAI JOLLAIN!!!!

      // Allocate space for more pair data
      **/

      // Prevent crash in AddDataMapping since itsFirstPair is nullptr
      throw Fmi::Exception(BCP, "Unable to allocate more memory for transform list");
    }
    else
    {
      if (itsFirstPair)
        delete[] itsFirstPair;

      itsFirstPair = new NFmiPoint[itsMaxPairNumber];

      if (itsLastPair)
        delete[] itsLastPair;

      itsLastPair = new NFmiPoint[itsMaxPairNumber];

      if (itsOutputInputRatio)
        delete[] itsOutputInputRatio;

      itsOutputInputRatio = new double[itsMaxPairNumber];

      return true;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param firstPair Undocumented
 * \param lastPair Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTransformList::AddDataMapping(const NFmiPoint& firstPair, const NFmiPoint& lastPair)
{
  try
  {
    if (itsCurrentIndex >= itsMaxPairNumber)
      Allocate(itsMaxPairNumber);

    // Create current range [firstPair .. lastPair]

    itsFirstPair[itsCurrentIndex] = firstPair;
    itsLastPair[itsCurrentIndex] = lastPair;

    double firstInputValue = firstPair.X();
    double lastInputValue = lastPair.X();

    double firstOutputValue = firstPair.Y();
    double lastOutputValue = lastPair.Y();

    // Update the very first and last data pairs in the list

    if (firstInputValue < itsVeryFirstPair.X())
      itsVeryFirstPair = NFmiPoint(firstInputValue, firstOutputValue);

    if (lastInputValue > itsVeryLastPair.X())
      itsVeryLastPair = NFmiPoint(lastInputValue, lastOutputValue);

    // Pre-calculate the data mapping constant for the current range  [firstPair .. lastPair].
    // This constant can be used later on when transforming data values.

    if (lastInputValue - firstInputValue == 0.)
      itsOutputInputRatio[itsCurrentIndex] = kMaxDouble;
    else
      itsOutputInputRatio[itsCurrentIndex] =
          (lastOutputValue - firstOutputValue) / (lastInputValue - firstInputValue);

    itsCurrentMaxIndex = itsCurrentIndex;
    itsCurrentIndex++;

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 * \param theRangeIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiTransformList::Transform(const double value, int theRangeIndex)
{
  try
  {
    // Transforms 'value' into 'itsOutputValue'.

    if (value == itsPreviousInputValue)
      return itsPreviousOutputValue;

    itsPreviousInputValue = value;  // A new value encountered

    if (theRangeIndex >= 0)
      itsPreviousInputRangeIndex =
          theRangeIndex;  // Try to transform the 'value' on the input range 'theRangeIndex'
    else
      itsPreviousInputRangeIndex = RangeIndex(value);  // Brute force, then ...

    if (Interpolate(itsPreviousInputRangeIndex, value))
    {
      itsPreviousOutputValue = itsOutputValue;
      return itsOutputValue;  // Input range found in list and interpolated value will be returned
    }

    return itsOutputValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param firstPair Undocumented
 * \param lastPair Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTransformList::Interpolate(const NFmiPoint& firstPair, const NFmiPoint& lastPair)
{
  try
  {
    double firstInputValue = firstPair.X();
    double lastInputValue = lastPair.X();

    double firstOutputValue = firstPair.Y();
    double lastOutputValue = lastPair.Y();

    double inputValue = itsInputValue;

    if (inputValue <= firstInputValue)
    {
      // Clamp value
      itsOutputValue = firstOutputValue;

      if (inputValue == firstInputValue)
        return true;  // This is the starting point for input range

      return false;  // Outside of input range
    }

    if (inputValue >= lastInputValue)
    {
      // Clamp value
      itsOutputValue = lastOutputValue;

      if (inputValue == lastInputValue)
        return true;  // This is the ending point for input range

      return false;  // Outside of input range
    }

    if ((firstInputValue < inputValue) && (inputValue < lastInputValue))
    {
      // Interpolate
      itsOutputValue = firstOutputValue +
                       ((lastOutputValue - firstOutputValue) * (inputValue - firstInputValue)) /
                           (lastInputValue - firstInputValue);
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
 * \param theRangeIndex Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTransformList::Interpolate(const int theRangeIndex, double theValue)
{
  try
  {
    // Find the sub-range of current NFmiTransformList 'value' belongs to.
    // This done, linearly interpolate the output value on this sub-range

    if (theRangeIndex < 0)
      return false;

    itsInputValue = theValue;

    double firstInputValue = itsFirstPair[theRangeIndex].X();
    double lastInputValue = itsLastPair[theRangeIndex].X();

    double firstOutputValue = itsFirstPair[theRangeIndex].Y();
    double lastOutputValue = itsLastPair[theRangeIndex].Y();

    double inputValue = itsInputValue;

    if (IsConstantRange(theRangeIndex))
    {
      itsOutputValue = firstOutputValue;  // Constant valued output range
      return true;                        // - no interpolation required
    }

    if (inputValue < firstInputValue)
    {
      itsOutputValue = firstOutputValue;  // Out of input value range -  clamp input value
      return true;                        // - no interpolation required
    }

    if (inputValue > lastInputValue)
    {
      itsOutputValue = lastOutputValue;  // Out of input value range -  clamp input value
      return true;                       // - no interpolation required
    }

    // Interpolate
    itsOutputValue =
        firstOutputValue + itsOutputInputRatio[theRangeIndex] * (inputValue - firstInputValue);

    return true;
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

int NFmiTransformList::RangeIndex(double theValue)
{
  try
  {
    // Finds out the input range number where the input data value 'theValue' belongs to

    itsInputValue = theValue;

    // At first, try to transform the 'value' on the previous time range
    // 'itsPreviousInputRangeIndex'
    itsPreviousInputRangeIndex =
        std::min(itsCurrentMaxIndex, std::max(0, itsPreviousInputRangeIndex));

    double firstInputValue = itsFirstPair[itsPreviousInputRangeIndex].X();
    double lastInputValue = itsLastPair[itsPreviousInputRangeIndex].X();

    if ((firstInputValue <= itsInputValue) && (itsInputValue <= lastInputValue))
      return itsPreviousInputRangeIndex;

    if (itsInputValue < FirstInputValue())  // Range not found, use lowest range index
      return 0;

    if (itsInputValue > LastInputValue())  // Range not found, use highest range index
      return itsCurrentMaxIndex;

    // Okay, brute force, then ...
    for (int rangeIndex = 0; rangeIndex <= itsCurrentMaxIndex; rangeIndex++)
    {
      firstInputValue = itsFirstPair[rangeIndex].X();
      lastInputValue = itsLastPair[rangeIndex].X();

      if ((firstInputValue <= itsInputValue) && (itsInputValue <= lastInputValue))
        return rangeIndex;
    }

    return -1;  // Range not found!
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiTransformList::Write(std::ostream& file) const
{
  try
  {
    file << itsMaxPairNumber << std::endl;
    file << itsIncrementSize << std::endl;
    file << itsCurrentIndex << std::endl;
    file << itsCurrentMaxIndex << std::endl;
    file << itsInputValue << std::endl;
    file << itsOutputValue << std::endl << std::endl;
    for (int i = 0; i < itsMaxPairNumber; i++)
    {
      file << itsFirstPair[i];
      file << itsLastPair[i] << std::endl;
    }

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream& NFmiTransformList::Read(std::istream& file)
{
  try
  {
    file >> itsMaxPairNumber;
    file >> itsIncrementSize;
    file >> itsCurrentIndex;
    file >> itsCurrentMaxIndex;
    file >> itsInputValue;
    file >> itsOutputValue;

    if (itsFirstPair)
    {
      delete[] itsFirstPair;
      itsFirstPair = nullptr;
    }

    if (itsLastPair)
    {
      delete[] itsLastPair;
      itsLastPair = nullptr;
    }

    itsFirstPair = new NFmiPoint[itsMaxPairNumber];
    itsLastPair = new NFmiPoint[itsMaxPairNumber];

    for (int i = 0; i < itsMaxPairNumber; i++)
    {
      file >> itsFirstPair[i];
      file >> itsLastPair[i];
    }

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
