// ======================================================================
/*!
 * \file NFmiParamDataModifier.cpp
 * \brief Implementation of class NFmiParamDataModifier
 */
// ======================================================================
/*!
 * \class NFmiParamDataModifier
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiParamDataModifier.h"
#include "NFmiDataModifierList.h"
#include "NFmiQueryInfo.h"
#include <macgyver/Exception.h>
#include <stdexcept>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiParamDataModifier::~NFmiParamDataModifier()
{
  try
  {
    delete itsParam;
    delete itsLevel;
    delete itsSubList;
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
 * \param theJoinOperator Undocumented
 */
// ----------------------------------------------------------------------

NFmiParamDataModifier::NFmiParamDataModifier(NFmiDataIdent* theParam,
                                             NFmiLevel* theLevel,
                                             FmiJoinOperator theJoinOperator)
    : NFmiDataModifier(theJoinOperator),
      itsParam(theParam ? new NFmiDataIdent(*theParam) : nullptr),
      itsLevel(theLevel ? new NFmiLevel(*theLevel) : nullptr),
      itsSubList(nullptr)
{
  try
  {
    itsSubList = new NFmiDataModifierList;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifier::Match(const NFmiDataIdent& theParam, const NFmiLevel* theLevel)
{
  try
  {
    // HUOM!!! Tarkistus tehdään ainakin aluksi parId tasolla ja levelID tasolla!!!!!!
    if (*itsParam->GetParam() == *theParam.GetParam())
    {
      if (itsLevel && theLevel && (*itsLevel == *theLevel))
        return true;
      else if (!(itsLevel && theLevel))  // jos molemmat ovat 0-pointtereita myös true
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
 * \param theModifier Undocumented
 * \return Undocumented
 *
 * \todo Not implemented yet!
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifier::AddSubModifier(NFmiDataModifier* /* theModifier */)
{
  try
  {
    throw Fmi::Exception(BCP, "NFmiParamDataModifier::AddSubModifier not implemented yet!");
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

NFmiDataModifierList* NFmiParamDataModifier::SubModifiers()
{
  try
  {
    return itsSubList;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented, unused
 * \return Undocumented, always false
 */
// ----------------------------------------------------------------------

bool NFmiParamDataModifier::BoolOperation(float /* theValue */)
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented, unused
 * \return Undocumented, always theValue itself
 */
// ----------------------------------------------------------------------

float NFmiParamDataModifier::FloatOperation(float theValue)
{
  return theValue;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always kFloatMissing
 */
// ----------------------------------------------------------------------

float NFmiParamDataModifier::CalculationResult()
{
  return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented, unused
 */
// ----------------------------------------------------------------------

void NFmiParamDataModifier::Calculate(float /* theValue */) {}

// ----------------------------------------------------------------------
/*!
 * Undocumented
 */
// ----------------------------------------------------------------------

void NFmiParamDataModifier::Clear() {}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiParamDataModifier::WriteOperand(std::ostream& file) const
{
  try
  {
    file << "<operand type='dataparam'>";
    file << "<id>" << itsParam->GetParam()->GetIdent() << "</id>";
    file << "</operand>";
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
