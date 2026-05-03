// ======================================================================
/*!
 * \brief Implementation of class NFmiModMeanCalculator
 */
// ======================================================================
/*!
 * \class NFmiModMeanCalculator
 *
 * \brief A class to calculate weighted modular sums
 *
 * To calculate mean wind direction:
 * \code
 * NFmiModMeanCalculator calculator;
 * calculator(100);
 * calculator(110);
 * calculator(120);
 * cout << calculator() << endl;
 * \endcode
 *
 * To calculate weighted mean wind direction:
 * \code
 * NFmiModMeanCalculator calculator;
 * calculator(100,0.3);
 * calculator(110,0.5);
 * calculator(120,0.2);
 * \endcode
 * The weights do not need to sum to 1.0.
 *
 * The Mitsuta algorithms used here are available from
 * http://www.webmet.com/met_monitoring/621.html
 *
 */
// ======================================================================

#include "NFmiModMeanCalculator.h"
#include <macgyver/Exception.h>
#include <cmath>

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theModulo The modulo value
 */
// ----------------------------------------------------------------------

NFmiModMeanCalculator::NFmiModMeanCalculator(float theModulo)
    : itsModulo(theModulo),
      itsValid(true),
      itsCount(0),
      itsLastValue(0),
      itsWeightedSum(0),
      itsWeightSum(0)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the number of values inserted into the calculator
 */
// ----------------------------------------------------------------------

int NFmiModMeanCalculator::count() const
{
  return itsCount;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the calculated mean
 *
 * Returns kFloatMissing if the value is not valid, or if any of the
 * input values was not valid, or if the sum of the weights is zero.
 *
 */
// ----------------------------------------------------------------------

float NFmiModMeanCalculator::operator()() const
{
  try
  {
    if (!itsValid || itsCount == 0 || itsWeightSum == 0)
      return kFloatMissing;

    float mean = itsWeightedSum / itsWeightSum;
    mean -= itsModulo * floor(mean / itsModulo);
    return mean;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new value to the calculator
 *
 * \param theValue The value to be added to the mean
 * \param theWeight The weight of the value
 */
// ----------------------------------------------------------------------

void NFmiModMeanCalculator::operator()(float theValue, float theWeight)
{
  try
  {
    ++itsCount;

    if (!itsValid)
      return;

    if (theValue == kFloatMissing || theWeight == kFloatMissing)
    {
      itsValid = false;
      return;
    }

    if (itsCount == 1)
    {
      itsLastValue = theValue;
      itsWeightedSum = theValue * theWeight;
      itsWeightSum = theWeight;
    }
    else
    {
      const float delta = theValue - itsLastValue;
      itsLastValue = theValue;
      if (delta < -itsModulo / 2.0)
        itsLastValue += itsModulo;
      else if (delta > itsModulo / 2.0)
        itsLastValue -= itsModulo;

      itsWeightedSum += theWeight * itsLastValue;
      itsWeightSum += theWeight;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
