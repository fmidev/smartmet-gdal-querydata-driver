// ======================================================================
/*!
 * \file NFmiRegressionItem.cpp
 * \brief Implementation of class NFmiRegressionItem
 */
// ======================================================================
/*!
 * \class NFmiRegressionItem
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiRegressionItem.h"
#include "NFmiSuperSmartInfo.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiRegressionItem::~NFmiRegressionItem() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theCoefficient Undocumented
 * \param theDataIdent Undocumented
 * \param theLevel Undocumented
 * \param theData Undocumented
 */
// ----------------------------------------------------------------------

NFmiRegressionItem::NFmiRegressionItem(double theCoefficient,
                                       NFmiDataIdent *theDataIdent,
                                       NFmiLevel *theLevel,
                                       NFmiQueryInfo *theData)
    : NFmiInfoModifier(theDataIdent, theLevel, theData),
      itsReturnValue(),
      itsCoefficient(theCoefficient),
      itsConstant()
{
}

// ----------------------------------------------------------------------
/*!
 * \param theData Undocumented
 */
// ----------------------------------------------------------------------

void NFmiRegressionItem::Calculate(NFmiQueryInfo *theData)
{
  try
  {
    if (itsParam)
    {
      if (theData)
      {
        NFmiDataIdent tmpIdent(theData->Param());  // hidasta koodia, pitäisi olla PeekValue!!!!!!!!
        theData->Param(*itsParam);
        itsReturnValue = itsConstant + itsCoefficient * theData->FloatValue();
        theData->Param(tmpIdent);
      }
      itsReturnValue = kFloatMissing;
    }
    itsReturnValue = itsConstant;
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

float NFmiRegressionItem::CalculationResult()
{
  return static_cast<float>(itsReturnValue);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiRegressionItem::FloatValue()
{
  try
  {
    if (itsParam)
    {
      if (itsData)
      {
        NFmiParam tmpParam(
            *itsData->Param().GetParam());  // hidasta koodia, pitäisi olla PeekValue!!!!!!!!
        itsData->Param(*itsParam->GetParam());
        itsReturnValue = itsCoefficient * itsData->FloatValue();
        itsData->Param(tmpParam);

        return itsReturnValue;
      }
      return kFloatMissing;
    }
    return itsCoefficient;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
