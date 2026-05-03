// ======================================================================
/*!
 * \file NFmiRegressionModifier.cpp
 * \brief Implementation of class NFmiRegressionModifier
 */
// ======================================================================
/*!
 * \class NFmiRegressionModifier
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiRegressionModifier.h"
#include "NFmiDataIdent.h"
#include "NFmiDataModifierConstant.h"
#include "NFmiDataModifierList.h"
#include "NFmiDataModifierLogical.h"
#include "NFmiRegressionItem.h"
#include <macgyver/Exception.h>
#include <algorithm>
#include <cmath>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiRegressionModifier::~NFmiRegressionModifier()
{
  try
  {
    delete itsRegressionItems;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \param theData Undocumented
 */
// ----------------------------------------------------------------------

NFmiRegressionModifier::NFmiRegressionModifier(NFmiDataIdent *theParam,
                                               NFmiLevel *theLevel,
                                               NFmiQueryInfo *theData)
    : NFmiInfoModifier(theParam, theLevel, theData), itsRegressionItems(new NFmiDataModifierList)
{
  try
  {
    if (*itsParam == NFmiDataIdent(NFmiParam(kFmiTemperature)))
    {
      auto *firstAlternative = new NFmiDataModifierList;
      firstAlternative->Add(new NFmiDataModifierConstant(3.0, kFmiAdd));
      firstAlternative->Add(new NFmiRegressionItem(
          -0.02, new NFmiDataIdent(NFmiParam(kFmiTotalCloudCover)), itsLevel, itsData));
      firstAlternative->Add(new NFmiRegressionItem(
          -0.8, new NFmiDataIdent(NFmiParam(kFmiWindSpeedMS)), itsLevel, itsData));

      auto *secondAlternative = new NFmiDataModifierList;
      secondAlternative->Add(new NFmiRegressionItem(5.0));
      secondAlternative->Add(new NFmiRegressionItem(
          -0.03, new NFmiDataIdent(NFmiParam(kFmiTotalCloudCover)), itsLevel, itsData));
      secondAlternative->Add(new NFmiRegressionItem(
          -0.5, new NFmiDataIdent(NFmiParam(kFmiWindSpeedMS)), itsLevel, itsData));

      NFmiInfoModifier *conditionData =
          new NFmiInfoModifier(new NFmiDataIdent(NFmiParam(kFmiTemperature)), itsLevel, itsData);
      auto *condition = new NFmiDataModifierBoolean(
          kFmiModifierValueGreaterThan, conditionData, new NFmiRegressionItem(14.0));
      auto *alternatives =
          new NFmiDataModifierLogical(condition, firstAlternative, secondAlternative);
      itsRegressionItems->Add(alternatives);
    }
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

double NFmiRegressionModifier::FloatValue()
{
  try
  {
    if (!itsData)
      return kFloatMissing;

    return std::max(static_cast<double>(*itsRegressionItems), 0.0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiRegressionModifier::WriteOperand(std::ostream &file) const
{
  try
  {
    file << "<operand>";
    itsRegressionItems->Write(file);
    file << "</operator>";
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
