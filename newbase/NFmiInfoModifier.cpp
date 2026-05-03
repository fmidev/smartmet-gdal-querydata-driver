// ======================================================================
/*!
 * \file NFmiInfoModifier.cpp
 * \brief Implementation for class NFmiInfoModifier
 */
// ======================================================================
/*!
 * \class NFmiInfoModifier
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiInfoModifier.h"
#include "NFmiQueryInfo.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiInfoModifier::~NFmiInfoModifier() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \param theData Undocumented
 * \param theJoinOperator Undocumented
 */
// ----------------------------------------------------------------------

NFmiInfoModifier::NFmiInfoModifier(NFmiDataIdent* theParam,
                                   NFmiLevel* theLevel,
                                   NFmiQueryInfo* theData,
                                   FmiJoinOperator theJoinOperator)
    : NFmiParamDataModifier(theParam, theLevel, theJoinOperator), itsData(theData)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiInfoModifier::FloatValue()
{
  try
  {
    return itsData->FloatValue();
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiInfoModifier::PeekValue(long dx, long dy, long dt)
{
  try
  {
    return itsData->PeekValue(dt, dx, dy);
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

NFmiQueryInfo* NFmiInfoModifier::Info()
{
  try
  {
    return itsData;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
