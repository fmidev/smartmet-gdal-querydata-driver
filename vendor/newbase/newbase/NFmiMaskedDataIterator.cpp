// ======================================================================
/*!
 * \file NFmiMaskedDataIterator.cpp
 * \brief Implementation of class NFmiMaskedDataIterator
 */
// ======================================================================
/*!
 * \class NFmiMaskedDataIterator
 *
 * Undocumented
 *
 * \todo Hide member variables
 */
// ======================================================================

#include "NFmiMaskedDataIterator.h"
#include "NFmiCombinedParam.h"
#include "NFmiDataModifier.h"
#include "NFmiIntegrationSelector.h"
#include "NFmiSuperSmartInfo.h"
#include "NFmiWeatherAndCloudiness.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiMaskedDataIterator::~NFmiMaskedDataIterator() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theData Undocumented
 * \param isCombinedParam Undocumented
 * \param theIntegrationSelector Undocumented
 */
// ----------------------------------------------------------------------

NFmiMaskedDataIterator::NFmiMaskedDataIterator(NFmiSuperSmartInfo *theData,
                                               bool isCombinedParam,
                                               NFmiIntegrationSelector *theIntegrationSelector)
    : NFmiDataIterator(theData),
      fIsCombinedParam(isCombinedParam),
      itsIntegrationSelector(theIntegrationSelector)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theDataModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMaskedDataIterator::DoForEach(NFmiDataModifier *theDataModifier)
{
  try
  {
    if (!theDataModifier)
      return;

    auto *superDataInfo = static_cast<NFmiSuperSmartInfo *>(itsData);
    unsigned long oldLocationIndex = superDataInfo->LocationIndex();
    theDataModifier->Clear();
    superDataInfo->ResetLocation();
    while (superDataInfo->NextLocation())
    {
      if (fIsCombinedParam)
      {
        auto *combParam = static_cast<NFmiWeatherAndCloudiness *>(superDataInfo->CombinedValue());
        if (combParam)
          theDataModifier->Calculate(
              NFmiWeatherAndCloudiness(combParam->IntegratedLongValue(itsIntegrationSelector),
                                       kFmiPackedWeather,
                                       kFloatMissing,
                                       combParam->InfoVersion())
                  .TransformedFloatValue());
      }
      else
        theDataModifier->Calculate(superDataInfo->FloatValue());
    }
    superDataInfo->LocationIndex(oldLocationIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
