// ======================================================================
/*!
 * \file NFmiDataModifierAllValidClipSum.cpp
 * \brief Implementation of class NFmiDataModifierAllValidClipSum
 */
// ======================================================================
/*!
 * \class NFmiDataModifierAllValidClipSum
 *
 * Undocumented
 *
 *
 * 19.9.2000/Lasse; puolittaa alku- ja loppuhetken arvot
 * soveltuu esim sadesumman llskemiseen kun lähdetään hetkellisistä intensiteeteistä
 * AllValid= vaatii että kaikki arvot ovat käypiä
 */
// ======================================================================

#include "NFmiDataModifierAllValidClipSum.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierAllValidClipSum::~NFmiDataModifierAllValidClipSum() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierAllValidClipSum::NFmiDataModifierAllValidClipSum()
    : NFmiDataModifierAllValidSum(), itsHalfValue(0)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAllValidClipSum::Calculate(NFmiQueryInfo* theQI)
{
  // PITÄIS TARKISTAA VOIKO TOIMIA NÄIN ja tässähän ei ole arvovälin tsekkausta
  float value = theQI->FloatValue();
  float halfValue = 0;
  if (CheckMissingValues(value))
  {
    halfValue = static_cast<float>(value * .5);
    if (itsCounter == 0)
      fCalculationResultOk = true;
    itsSum += halfValue + itsHalfValue;
  }
  else
  {
    fCalculationResultOk = false;
  }
  itsHalfValue = halfValue;
  itsCounter++;
}

// ======================================================================
