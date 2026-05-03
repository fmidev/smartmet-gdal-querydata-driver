// ======================================================================
/*!
 * \file NFmiMap.cpp
 * \brief Implementation for class NFmiMap
 */
// ======================================================================
/*!
 * \class NFmiMap
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiMap.h"

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiMap::NFmiMap() : itsOffset(0.F, 0.F), itsScale(1.F, 1.F) {}
// ----------------------------------------------------------------------
/*!
 * \param theOffset Undocumented
 * \param theScale Undocumented
 */
// ----------------------------------------------------------------------

NFmiMap::NFmiMap(const NFmiPoint &theOffset, const NFmiPoint &theScale)
    : itsOffset(theOffset), itsScale(theScale)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMap The map to be copied
 *
 */
// ----------------------------------------------------------------------

NFmiMap::NFmiMap(const NFmiMap &theMap) = default;
// ======================================================================
