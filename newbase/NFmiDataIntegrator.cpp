// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiDataIntegrator
 */
// ======================================================================
/*!
 * \namespace NFmiDataIntegrator
 *
 * \brief Functions for integrating query data
 *
 * The NFmiDataIntegrator namespace defines several methods
 * for performing data integration in space and time.
 *
 */
// ======================================================================

#include "NFmiDataIntegrator.h"

#include "NFmiDataModifier.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiIndexMask.h"
#include "NFmiIndexMaskSource.h"
#include "NFmiMetTime.h"
#include <macgyver/Exception.h>

namespace NFmiDataIntegrator
{
// ----------------------------------------------------------------------
/*!
 * \brief Integrate over time
 *
 * Integrate over time with current location, parameter and level.
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * \param theQI The query info
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theTimeModifier The modifier for accumulating the result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                NFmiDataModifier& theTimeModifier)
{
  try
  {
    theTimeModifier.Clear();

    if (!theQI.Time(theStartTime))
      return kFloatMissing;

    do
    {
      const float tmp = theQI.FloatValue();
      theTimeModifier.Calculate(tmp);
    } while (theQI.NextTime() && theQI.Time() <= theEndTime);

    return theTimeModifier.CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over time with subintervals
 *
 * Integrate over time with current location, parameter and level.
 * The time interval is divided into subintervals and are integrated
 * separately.
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * If the given interval is negative, a missing value is returned.
 * If the interval is zero, no subintervals are created and
 * the sub modifier is ignored.
 *
 * \param theQI The query info
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theInterval The sub interval in hours
 * \param theSubTimeModifier The modifier for accumulating the subresult
 * \param theMainTimeModifier The modifier for subresults
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                int theInterval,
                NFmiDataModifier& theSubTimeModifier,
                NFmiDataModifier& theMainTimeModifier)
{
  try
  {
    // Safety against bad loop
    if (theInterval < 0)
      return kFloatMissing;

    // Default case
    if (theInterval == 0)
      return Integrate(theQI, theStartTime, theEndTime, theMainTimeModifier);

    theMainTimeModifier.Clear();

    NFmiTime time1(theStartTime);

    if (!theQI.Time(time1))
      return kFloatMissing;

    do
    {
      NFmiTime time2(time1);
      time1.ChangeByHours(theInterval);

      theSubTimeModifier.Clear();

      do
      {
        const float tmp = theQI.FloatValue();
        theSubTimeModifier.Calculate(tmp);
      } while (theQI.NextTime() && theQI.Time() < time2);

      const float subresult = theSubTimeModifier.CalculationResult();
      theMainTimeModifier.Calculate(subresult);

    } while (theQI.IsValidTime() && theQI.Time() <= theEndTime);

    return theMainTimeModifier.CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over space
 *
 * Integrate over space with current time, parameter and level.
 *
 * It is assumed that the index mask was generated from a grid
 * equivalent to the one in the query data.
 *
 * \param theQI The query info
 * \param theIndexMask The mask containing the space indices
 * \param theSpaceModifier The modifier for accumulating the result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMask& theIndexMask,
                NFmiDataModifier& theSpaceModifier)
{
  try
  {
    if (theIndexMask.empty())
      return kFloatMissing;

    for (unsigned long it : theIndexMask)
    {
      theQI.LocationIndex(it);
      const float tmp = theQI.FloatValue();
      theSpaceModifier.Calculate(tmp);
    }

    return theSpaceModifier.CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over space and time
 *
 * Integrate over space and time with current parameter and level.
 * Note that the integration order is different from the Integrate
 * command where the order of time and space arguments are reversed.
 * Here space integration is performed first, then time integration.
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * \param theQI The query info
 * \param theIndexMask The  mask containing the space indices
 * \param theSpaceModifier  The modifier for accumulating the space result
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theTimeModifier The modifier for accumulating the time result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMask& theIndexMask,
                NFmiDataModifier& theSpaceModifier,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                NFmiDataModifier& theTimeModifier)
{
  try
  {
    theTimeModifier.Clear();

    if (!theQI.Time(theStartTime))
      return kFloatMissing;

    if (theIndexMask.empty())
      return kFloatMissing;

    do
    {
      const float tmp = Integrate(theQI, theIndexMask, theSpaceModifier);
      theTimeModifier.Calculate(tmp);
    } while (theQI.NextTime() && theQI.Time() <= theEndTime);

    return theTimeModifier.CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over time and space
 *
 * Integrate over time and space with current parameter and level.
 * Note that the integration order is different from the Integrate
 * command where the order of time and space arguments are reversed.
 * Here time integration is performed first, then space integration.
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * \param theQI The query info
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theTimeModifier The modifier for accumulating the time result
 * \param theIndexMask The mask containing the space indices
 * \param theSpaceModifier The modifier for accumulating the space result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                NFmiDataModifier& theTimeModifier,
                const NFmiIndexMask& theIndexMask,
                NFmiDataModifier& theSpaceModifier)
{
  try
  {
    theSpaceModifier.Clear();

    if (theIndexMask.empty())
      return kFloatMissing;

    for (unsigned long it : theIndexMask)
    {
      theTimeModifier.Clear();

      if (!theQI.Time(theStartTime))
        return kFloatMissing;

      do
      {
        theQI.LocationIndex(it);

        const float tmp = theQI.FloatValue();

        theTimeModifier.Calculate(tmp);
      } while (theQI.NextTime() && theQI.Time() <= theEndTime);

      const float timeresult = theTimeModifier.CalculationResult();
      theSpaceModifier.Calculate(timeresult);
    }

    return theSpaceModifier.CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over space and time with time dependent masks
 *
 * Integrate over space and time with current parameter and level.
 * The space mask is time dependent and is obtained from a
 * NFmiIndexMaskSource.
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * \param theQI The query info
 * \param theMaskSource The masks for any specific time
 * \param theSpaceModifier  The modifier for accumulating the space result
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theTimeModifier The modifier for accumulating the time result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMaskSource& theMaskSource,
                NFmiDataModifier& theSpaceModifier,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                NFmiDataModifier& theTimeModifier)
{
  try
  {
    theTimeModifier.Clear();

    if (!theQI.Time(theStartTime))
      return kFloatMissing;

    do
    {
      const NFmiIndexMask& mask = theMaskSource.Find(theQI.Time());
      const float tmp = Integrate(theQI, mask, theSpaceModifier);
      theTimeModifier.Calculate(tmp);
    } while (theQI.NextTime() && theQI.Time() <= theEndTime);

    return theTimeModifier.CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over time, time and space
 *
 * Integrate over time and space with current parameter and level.
 * Note that the integration order is different from the Integrate
 * command where the order of time and space arguments are reversed.
 * Here time integration is performed first, then space integration.
 * Also, the time integration is split into two parts with separate
 * modifiers. For example, one may divide a 5 day period into 24 hour
 * segments, and then calculate the mean of 24 hour maximums in time,
 * and then mean in space again.
 *
 * If the start time does not exist in the data, a missing value
 * is returned.
 *
 * If the given interval is negative, a missing value is returned.
 * If the interval is zero, no subintervals are created and
 * the sub modifier is ignored.
 *
 * \param theQI The query info
 * \param theStartTime The start time of the integration.
 * \param theEndTime The end time of the integration.
 * \param theInterval The number of hours in a single interval
 * \param theSubTimeModifier The modifier for accumulating sub periods
 * \param theMainTimeModifier The modifier for accumulating sub period results
 * \param theIndexMask The mask containing the space indices
 * \param theSpaceModifier The modifier for accumulating the space result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                int theInterval,
                NFmiDataModifier& theSubTimeModifier,
                NFmiDataModifier& theMainTimeModifier,
                const NFmiIndexMask& theIndexMask,
                NFmiDataModifier& theSpaceModifier)
{
  try
  {
    // Safety against bad loop
    if (theInterval < 0)
      return kFloatMissing;

    // Don't create subintervals unless necessary
    if (theInterval == 0)
      return Integrate(
          theQI, theStartTime, theEndTime, theMainTimeModifier, theIndexMask, theSpaceModifier);

    theSpaceModifier.Clear();

    if (theIndexMask.empty())
      return kFloatMissing;

    for (unsigned long it : theIndexMask)
    {
      theMainTimeModifier.Clear();

      NFmiTime time1(theStartTime);

      if (!theQI.Time(time1))
        return kFloatMissing;

      do
      {
        NFmiTime time2(time1);
        time2.ChangeByHours(theInterval);

        theSubTimeModifier.Clear();

        do
        {
          theQI.LocationIndex(it);
          const float tmp = theQI.FloatValue();

          theSubTimeModifier.Calculate(tmp);
        } while (theQI.NextTime() && theQI.Time() < time2);

        const float subtimeresult = theSubTimeModifier.CalculationResult();
        theMainTimeModifier.Calculate(subtimeresult);

        time1 = time2;

      }

      while (theQI.IsValidTime() && theQI.Time() < theEndTime);

      const float timeresult = theMainTimeModifier.CalculationResult();
      theSpaceModifier.Calculate(timeresult);
    }

    return theSpaceModifier.CalculationResult();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace NFmiDataIntegrator

// ======================================================================
