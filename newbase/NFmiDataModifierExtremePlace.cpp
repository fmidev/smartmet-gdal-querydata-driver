// ======================================================================
/*!
 * \file NFmiDataModifierExtremePlace.cpp
 * \brief Implementation of class NFmiDataModifierExtreme
 */
// ======================================================================
/*!
 * \class NFmiDataModifierExtremePlace
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierExtremePlace.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierExtremePlace::~NFmiDataModifierExtremePlace() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierExtremePlace::NFmiDataModifierExtremePlace()
    : itsExtremeValue(), itsExtremeLocation()
{
}

// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierExtremePlace::SetLocation(NFmiQueryInfo* theQI)
{  // virtual const NFmiLocation * Location() const;

  if (theQI)
    itsExtremeLocation = *(theQI->Location());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiLocation NFmiDataModifierExtremePlace::GetLocation()
{
  return itsExtremeLocation;
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierExtremePlace::Calculate(float theValue, NFmiQueryInfo* theQI)
{
  if (IsAllowedValue(theValue))
  {
    //	 if(value != kFloatMissing && value != kRadarPrecipitationMissing)  //puuttuvat
    // tutkasateet==65535??
    if (IsNewExtreme(theValue))
    {
      itsExtremeValue = theValue;
      // TÄSSÄ SAA
      SetLocation(theQI);
    }
  }
}

// ======================================================================
