// ======================================================================
/*!
 * \file NFmiIntegrationSelector.cpp
 * \brief Implementation of class NFmiIntergationSelector
 */
// ======================================================================
/*!
 * \class NFmiIntegrationSelector
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiIntegrationSelector.h"

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theSelector The object being copied
 */
// ----------------------------------------------------------------------

NFmiIntegrationSelector::NFmiIntegrationSelector(const NFmiIntegrationSelector& theSelector)

    = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theSelectorType Undocumented
 * \param theProbabilityCondition Undocumented
 * \param theProbabilityLowerLimit Undocumented
 * \param theProbabilityUpperLimit Undocumented
 * \param theProbabilityScale Undocumented
 */
// ----------------------------------------------------------------------

NFmiIntegrationSelector::NFmiIntegrationSelector(FmiIntegrationSelector theSelectorType,
                                                 FmiProbabilityCondition theProbabilityCondition,
                                                 float theProbabilityLowerLimit,
                                                 float theProbabilityUpperLimit,
                                                 float theProbabilityScale)
    : itsType(theSelectorType),
      itsProbabilityCondition(theProbabilityCondition),
      itsProbabilityLowerLimit(theProbabilityLowerLimit),
      itsProbabilityUpperLimit(theProbabilityUpperLimit),
      itsProbabilityScale(theProbabilityScale)
{
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiIntegrationSelector::~NFmiIntegrationSelector() = default;
// ======================================================================
