// ======================================================================
/*!
 * \file NFmiCalculationCondition.cpp
 * \brief Implementation of class NFmiCalculationCondition
 */
// ======================================================================
/*!
 * \class NFmiCalculationCondition
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiCalculationCondition.h"
#include "NFmiValueString.h"
#include <macgyver/Exception.h>
#include <cmath>
#include <iostream>

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theCondition Undocumented
 * \param theLowerLimit Undocumented
 * \param theUpperLimit Undocumented
 */
// ----------------------------------------------------------------------

NFmiCalculationCondition::NFmiCalculationCondition(FmiMaskOperation theCondition,
                                                   double theLowerLimit,
                                                   double theUpperLimit)
    : itsCondition(theCondition), itsUpperLimit(theUpperLimit), itsLowerLimit(theLowerLimit)
{
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiCalculationCondition::NFmiCalculationCondition()
    : itsCondition(kFmiNoMaskOperation), itsUpperLimit(1), itsLowerLimit(0)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCalculationCondition::IsMasked(double theValue) const
{
  try
  {
    if (itsCondition == kFmiNoMaskOperation)
      return true;
    else if (theValue == kFloatMissing)
      return false;
    else
    {
      switch (itsCondition)
      {
        case kFmiNoMaskOperation:
          return true;
        case kFmiMaskEqual:
          return (round(theValue) == itsLowerLimit);  // kuinka suurella tarkkuudella???
        case kFmiMaskNotEqual:
          return (round(theValue) != itsLowerLimit);  // kuinka suurella tarkkuudella???
        case kFmiMaskGreaterThan:
          return (theValue > itsLowerLimit);
        case kFmiMaskLessThan:
          return (theValue < itsLowerLimit);
        case kFmiMaskGreaterOrEqualThan:
          return (theValue >= itsLowerLimit);
        case kFmiMaskLessOrEqualThan:
          return (theValue <= itsLowerLimit);
        case kFmiMaskGreaterThanAndLessThan:
          return (theValue > itsLowerLimit && theValue < itsUpperLimit);
        case kFmiMaskGreaterOrEqualAndLessOrEqual:
          return (theValue >= itsLowerLimit && theValue <= itsUpperLimit);
        case kFmiMaskGreaterOrEqualAndLessThan:
          return (theValue >= itsLowerLimit && theValue < itsUpperLimit);
        case kFmiMaskGreaterThanAndLessOrEqual:
          return (theValue > itsLowerLimit && theValue <= itsUpperLimit);
        case kFmiMaskLessThanOrGreaterThan:
          return (theValue < itsLowerLimit || theValue > itsUpperLimit);
        case kFmiMaskLessOrEqualOrGreaterOrEqual:
          return (theValue <= itsLowerLimit || theValue >= itsUpperLimit);
        case kFmiMaskLessOrEqualOrGreaterThan:
          return (theValue <= itsLowerLimit || theValue > itsUpperLimit);
        case kFmiMaskLessThanOrGreaterOrEqual:
          return (theValue < itsLowerLimit || theValue >= itsUpperLimit);
        case kFmiMaskEqualOrEqual:
          return (round(theValue) == itsLowerLimit || round(theValue) == itsUpperLimit);
        case kFmiMaskAll:
          return true;
        case kFmiMaskRisingRamp:
          return (theValue > itsLowerLimit);  // tähän ei ole oikeastaan kunnon tarkastelua,
                                              // todellinen käyttö kun lasketaan maskikertoimia!
        case kFmiMaskLoweringRamp:
          return (theValue < itsLowerLimit);  // tähän ei ole oikeastaan kunnon tarkastelua,
                                              // todellinen käyttö kun lasketaan maskikertoimia!
        case kFmiMaskDoubleRamp:
          return (theValue < itsLowerLimit);  // tähän ei ole oikeastaan kunnon tarkastelua,
                                              // todellinen käyttö kun lasketaan maskikertoimia!
      }
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
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// palauttaa kertoimen 0:n ja 1:n väliltä

double NFmiCalculationCondition::MaskValue(double theValue) const
{
  try
  {
    if (itsCondition == kFmiNoMaskOperation)
      return 1.;
    else if (theValue == kFloatMissing)
    {
      return 0.;
    }
    else
    {
      switch (itsCondition)
      {
        case kFmiNoMaskOperation:
          return 1.;
        case kFmiMaskEqual:
          return (round(theValue) == itsLowerLimit ? 1 : 0);  // kuinka suurella tarkkuudella???
        case kFmiMaskNotEqual:
          return (round(theValue) != itsLowerLimit ? 1 : 0);  // kuinka suurella tarkkuudella???
        case kFmiMaskGreaterThan:
          return (theValue > itsLowerLimit ? 1 : 0);
        case kFmiMaskLessThan:
          return (theValue < itsLowerLimit ? 1 : 0);
        case kFmiMaskGreaterOrEqualThan:
          return (theValue >= itsLowerLimit ? 1 : 0);
        case kFmiMaskLessOrEqualThan:
          return (theValue <= itsLowerLimit ? 1 : 0);
        case kFmiMaskGreaterThanAndLessThan:
          return (theValue > itsLowerLimit && theValue < itsUpperLimit ? 1 : 0);
        case kFmiMaskGreaterOrEqualAndLessOrEqual:
          return (theValue >= itsLowerLimit && theValue <= itsUpperLimit ? 1 : 0);
        case kFmiMaskGreaterOrEqualAndLessThan:
          return (theValue >= itsLowerLimit && theValue < itsUpperLimit ? 1 : 0);
        case kFmiMaskGreaterThanAndLessOrEqual:
          return (theValue > itsLowerLimit && theValue <= itsUpperLimit ? 1 : 0);
        case kFmiMaskLessThanOrGreaterThan:
          return (theValue < itsLowerLimit || theValue > itsUpperLimit ? 1 : 0);
        case kFmiMaskLessOrEqualOrGreaterOrEqual:
          return (theValue <= itsLowerLimit || theValue >= itsUpperLimit ? 1 : 0);
        case kFmiMaskLessOrEqualOrGreaterThan:
          return (theValue <= itsLowerLimit || theValue > itsUpperLimit ? 1 : 0);
        case kFmiMaskLessThanOrGreaterOrEqual:
          return (theValue < itsLowerLimit || theValue >= itsUpperLimit ? 1 : 0);
        case kFmiMaskRisingRamp:
          if (theValue <= itsLowerLimit)
            return 0.;
          if (theValue > itsLowerLimit && theValue <= itsUpperLimit)
            return (theValue - itsLowerLimit) / (itsUpperLimit - itsLowerLimit);
          return 1.;
        case kFmiMaskLoweringRamp:
          if (theValue <= itsLowerLimit)
            return 1.;
          if (theValue > itsLowerLimit && theValue <= itsUpperLimit)
            return 1. - (theValue - itsLowerLimit) / (itsUpperLimit - itsLowerLimit);
          return 0.;
        case kFmiMaskEqualOrEqual:
          return (round(theValue) == itsLowerLimit || round(theValue) == itsUpperLimit ? 1 : 0);
        case kFmiMaskDoubleRamp:
          if (theValue <= itsLowerLimit)
            return -1.;
          if (theValue > itsLowerLimit && theValue < itsUpperLimit)
            return 2 * ((theValue - itsLowerLimit) / (itsUpperLimit - itsLowerLimit)) - 1;
          return 1.;
        case kFmiMaskAll:
          return 1;
      }
    }
    return 0.;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParamName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// annetaan käytetyn parametrin nimi, valmistaa stringin, missä on ehdot näkyvissä

const NFmiString NFmiCalculationCondition::MaskString(const NFmiString& theParamName) const
{
  try
  {
    NFmiString returnString;
    NFmiValueString lowerValue;
    NFmiValueString upperValue;
    returnString += theParamName;
    returnString += " ";
    lowerValue = NFmiValueString(itsLowerLimit, "%.1f");
    upperValue = NFmiValueString(itsUpperLimit, "%.1f");

    switch (itsCondition)
    {
      case kFmiNoMaskOperation:
        returnString += "-";
        break;
      case kFmiMaskEqual:
        returnString += "=";
        returnString += lowerValue;
        break;
      case kFmiMaskNotEqual:
        returnString += "!=";
        returnString += lowerValue;
        break;
      case kFmiMaskGreaterThan:
        returnString += ">";
        returnString += lowerValue;
        break;
      case kFmiMaskLessThan:
        returnString += "<";
        returnString += lowerValue;
        break;
      case kFmiMaskGreaterOrEqualThan:
        returnString += ">=";
        returnString += lowerValue;
        break;
      case kFmiMaskLessOrEqualThan:
        returnString += "<=";
        returnString += lowerValue;
        break;
      case kFmiMaskGreaterThanAndLessThan:
        returnString += lowerValue;
        returnString += "< x <";
        returnString += upperValue;
        break;
      case kFmiMaskGreaterOrEqualAndLessOrEqual:
        returnString += lowerValue;
        returnString += "<= x <=";
        returnString += upperValue;
        break;
      case kFmiMaskGreaterOrEqualAndLessThan:
        returnString += lowerValue;
        returnString += "<= x <";
        returnString += upperValue;
        break;
      case kFmiMaskGreaterThanAndLessOrEqual:
        returnString += lowerValue;
        returnString += "< x <=";
        returnString += upperValue;
        break;
      case kFmiMaskLessThanOrGreaterThan:
        returnString += lowerValue;
        returnString += "> x tai x >";
        returnString += upperValue;
        break;
      case kFmiMaskLessOrEqualOrGreaterOrEqual:
        returnString += lowerValue;
        returnString += ">= x tai x >=";
        returnString += upperValue;
        break;
      case kFmiMaskLessOrEqualOrGreaterThan:
        returnString += lowerValue;
        returnString += ">= x tai x >";
        returnString += upperValue;
        break;
      case kFmiMaskLessThanOrGreaterOrEqual:
        returnString += lowerValue;
        returnString += "> x tai x >=";
        returnString += upperValue;
        break;
      case kFmiMaskAll:
        returnString += "*";
        break;
      case kFmiMaskEqualOrEqual:
        returnString += "x=";
        returnString += lowerValue;
        returnString += " tai x=";
        returnString += upperValue;
        break;
      case kFmiMaskRisingRamp:
        returnString += " nouseva R";
        break;
      case kFmiMaskLoweringRamp:
        returnString += " laskeva R";
        break;
      case kFmiMaskDoubleRamp:
        returnString += " tupla R";
        break;
    }
    return returnString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiCalculationCondition::Write(std::ostream& os) const
{
  try
  {
    os << static_cast<int>(itsCondition) << ' ' << itsLowerLimit << ' ' << itsUpperLimit
       << std::endl;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiCalculationCondition::Read(std::istream& is)
{
  try
  {
    int tmp = 0;
    is >> tmp >> itsLowerLimit >> itsUpperLimit;
    itsCondition = static_cast<FmiMaskOperation>(tmp);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
