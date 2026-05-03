// ======================================================================
/*!
 * \file NFmiCombinedParam.cpp
 * \brief Implementation of class NFmiCombinedParam
 */
// ======================================================================
/*!
 * \class NFmiCombinedParam
 *
 * Undocumented
 *
 * \todo Should define operator= explicitly since pointers are involved
 */
// ======================================================================

#include "NFmiCombinedParam.h"
#include "NFmiDataModifierCombi.h"
#include "NFmiParamBag.h"
#include "NFmiParamDataModifierList.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiCombinedParam::~NFmiCombinedParam()
{
  try
  {
    if (itsSubParams)
    {
      int size = itsSubParams->GetSize();
      for (int i = 0; i < size; i++)
        delete itsIntegrators[i];
    }
    delete[] itsIntegrators;
    delete itsSubParams;
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
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam::NFmiCombinedParam(double theInfoVersion)
    : itsSubParams(nullptr),
      itsIntegrators(nullptr),
      fIntegrationMode(false),
      fIntegrationReady(false),
      fIntegrationStarted(),
      fDataOk(true),
      fAutoUpdate(true),
      itsInfoVersion(theInfoVersion)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theParam The other object being copied
 */
// ----------------------------------------------------------------------

NFmiCombinedParam::NFmiCombinedParam(const NFmiCombinedParam &theParam)
    : itsSubParams(theParam.itsSubParams ? new NFmiParamBag(*theParam.itsSubParams) : nullptr),
      itsIntegrators(nullptr)  // Tässä pitäisi käyttää ParamModifierListin copy construktoria, vaan
                               // ei ole vielä
      ,
      fIntegrationMode(theParam.fIntegrationMode),
      fIntegrationReady(theParam.fIntegrationReady),
      fIntegrationStarted(theParam.fIntegrationStarted),
      fDataOk(theParam.fDataOk),
      fAutoUpdate(theParam.fAutoUpdate),
      itsInfoVersion(theParam.InfoVersion())
{
  try
  {
    int size = itsSubParams ? itsSubParams->GetSize() : 0;
    if (size)
    {
      itsIntegrators = new NFmiDataModifierCombi *[size];
      for (int i = 0; i < size; i++)
        itsIntegrators[i] = new NFmiDataModifierCombi(*theParam.itsIntegrators[i]);
    }
    //	InitIntegration();//Tämä pitäisi tehdä kopioimalla
    //	CreateIntegrators();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 *
 * \param theParam The other object being copied
 */
// ----------------------------------------------------------------------

NFmiCombinedParam &NFmiCombinedParam::operator=(const NFmiCombinedParam &theParam)
{
  try
  {
    if (this != &theParam)
    {
      itsSubParams = (theParam.itsSubParams ? new NFmiParamBag(*theParam.itsSubParams) : nullptr);
      itsIntegrators =
          nullptr;  // Tässä pitäisi käyttää ParamModifierListin operator=, vaan ei ole vielä
      fIntegrationMode = theParam.fIntegrationMode;
      fIntegrationReady = theParam.fIntegrationReady;
      fIntegrationStarted = theParam.fIntegrationStarted;
      fDataOk = theParam.fDataOk;
      fAutoUpdate = theParam.fAutoUpdate;
      itsInfoVersion = theParam.InfoVersion();

      int size = itsSubParams ? itsSubParams->GetSize() : 0;
      if (size)
      {
        itsIntegrators = new NFmiDataModifierCombi *[size];
        for (int i = 0; i < size; i++)
          itsIntegrators[i] = new NFmiDataModifierCombi(*theParam.itsIntegrators[i]);
      }
      //	InitIntegration();//Tämä pitäisi tehdä kopioimalla
      //	CreateIntegrators();
    }
    return *this;
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

bool NFmiCombinedParam::TransformFromFloatValue(float theValue)
{
  try
  {
    return LongValue(ConvertFloatToLong(theValue));
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

float NFmiCombinedParam::TransformedFloatValue()
{
  try
  {
    union converter
    {
      unsigned long ulongvalue;
      float floatvalue;
    };

    converter tmp;
    tmp.ulongvalue = LongValue();
    return tmp.floatvalue;
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

unsigned long NFmiCombinedParam::ConvertFloatToLong(float theValue)
{
  try
  {
    // MSVC60:ssa on bugi ja ylempi rivi (if(theValue == kFloatMissing)) ei toimi oikein
    // TotalWind-arvoille,
    // jotka tulkitaan jotenkin NaN:eiksi ja ylemmässä versiossa tulee true vertailussa. Alempi joka
    // on käytännössä
    // sama toimii oikein molemmissa kääntäjä versioissa (MSVC60 ja MSVC71).

    //  if(theValue == kFloatMissing)
    if (theValue == 32700.0F)
      return kTCombinedWeatherMissing;

    union converter
    {
      unsigned long ulongvalue;
      float floatvalue;
    };

    converter tmp;
    tmp.floatvalue = theValue;
    return tmp.ulongvalue;
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

void NFmiCombinedParam::InitIntegration()
{
  try
  {
    CreateSubParams();
    CreateIntegrators();
    fIntegrationMode = true;
    fIntegrationStarted = false;
    fIntegrationReady = false;
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

void NFmiCombinedParam::ClearIntegration()
{
  try
  {
    fIntegrationMode = true;
    fIntegrationStarted = false;
    fIntegrationReady = false;
    if (itsSubParams)
    {
      for (unsigned int idx = 0; idx < itsSubParams->GetSize(); idx++)
      {
        NFmiDataModifier *integrator = GetSubIntegrator(idx);
        if (integrator)
          integrator->Clear();
      }
    }
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

void NFmiCombinedParam::EndIntegration()
{
  try
  {
    if (!fIntegrationMode)
      return;

    if (fIntegrationStarted && itsSubParams)
    {
      itsSubParams->Reset();
      int idx = 0;
      while (itsSubParams->Next())
      {
        // Mika 08.11.2001: No huh-huh?!
        // NFmiDataModifierCombi *integrator =
        // GetSubIntegrator(index);itsIntegrators[index];//FindSubParamIntegrator(itsSubParams->CurrentParam());
        NFmiDataModifierCombi *integrator = GetSubIntegrator(idx);
        if (integrator)
        {
          //				SubValue(integrator->CalculationResult(),
          // itsSubParams->CurrentParam());
          /////Persa poisti koska yrittää toisella tavalla tuloksia
          ///				SubValue(integrator->CalcResult(kFmiMean),
          /// itsSubParams->CurrentParam());
        }
        idx++;
      }
    }
    //	DeleteIntegrators();
    //	fIntegrationMode = false;
    fIntegrationStarted = false;
    fIntegrationReady = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::Integrate(float theValue)
{
  try
  {
    if (!fIntegrationMode)
      return;

    fIntegrationStarted = true;

    if (itsSubParams)
    {
      NFmiCombinedParam *addNewValue = CreateNew(theValue);
      itsSubParams->Reset();
      int idx = 0;
      while (itsSubParams->Next())
      {
        NFmiDataModifier *integrator = GetSubIntegrator(
            idx);  // itsIntegrators[index];//FindSubParamIntegrator(itsSubParams->CurrentParam());
        if (integrator)
        {
          integrator->Calculate(
              static_cast<float>(addNewValue->RawSubValue(itsSubParams->CurrentParam())));
        }
        idx++;
      }
      delete addNewValue;
    }
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

void NFmiCombinedParam::CreateIntegrators() {}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::DeleteIntegrators() {}

// ----------------------------------------------------------------------
/*!
 * \param theName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombi *NFmiCombinedParam::FindSubParamIntegrator(FmiParameterName theName)
{
  try
  {
    int idx = FindSubParamIntegratorIndex(theName);
    if (idx >= 0)
      return itsIntegrators[idx];

    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

int NFmiCombinedParam::FindSubParamIntegratorIndex(FmiParameterName theName)
{
  try
  {
    if (itsIntegrators && itsSubParams)
    {
      int i = 0;
      for (itsSubParams->Reset(); itsSubParams->Next();)
      {
        if (itsSubParams->CurrentParam() == theName)
          return i;

        i++;
      }
    }
    return -1;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSubParamName Undocumented
 * \param theModifier Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCombinedParam::SetSubIntegrator(FmiParameterName theSubParamName,
                                         NFmiDataModifierCombi *theModifier)
{
  try
  {
    int idx = FindSubParamIntegratorIndex(theSubParamName);
    if (idx >= 0)
    {
      itsIntegrators[idx] = theModifier;
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
 * \param theInitValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam *NFmiCombinedParam::CreateNew(float /* theInitValue */)
{
  try
  {
    return nullptr;
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

void NFmiCombinedParam::CreateSubParams()
{
  try
  {
    itsSubParams = nullptr;
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
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombi *NFmiCombinedParam::GetSubIntegrator(unsigned long theIndex)
{
  try
  {
    if (theIndex < itsSubParams->GetSize())
      return itsIntegrators[theIndex];

    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSubParamName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombi *NFmiCombinedParam::SubIntegrator(FmiParameterName theSubParamName)
{
  try
  {
    int idx = FindSubParamIntegratorIndex(theSubParamName);
    if (idx >= 0)
      return GetSubIntegrator(idx);

    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::CrossCheck(FmiParameterName /* theParam */) {}
// ======================================================================
