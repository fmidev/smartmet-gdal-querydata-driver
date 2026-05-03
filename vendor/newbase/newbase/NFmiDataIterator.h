// ======================================================================
/*!
 * \file NFmiDataIterator.h
 * \brief Interface of class NFmiDataIterator
 */
// ======================================================================

#pragma once

#include "NFmiPoint.h"

class NFmiDataModifier;
class NFmiAreaMask;
class NFmiQueryInfo;

//! Undocumented
class NFmiDataIterator
{
 public:
  virtual ~NFmiDataIterator();
  NFmiDataIterator(NFmiQueryInfo* theData = 0);

  virtual void DoForEach(NFmiDataModifier* theDataModifier = 0);
  virtual void SetDimensions(long dx, long dy, long dt);

  NFmiAreaMask* Mask();
  void Mask(NFmiAreaMask* newMask);
  bool MaskInUse() const;

 protected:
  void CheckIfMaskIsUsed();  // tarkistaa käytetäänkö maskia ja asetetaan fMaskInUse arvo
  bool IsMasked(const NFmiPoint& theLatLonPoint);
  NFmiQueryInfo* itsData;  // ei omista, ei tuhoa!
  NFmiAreaMask* itsMask;  // ei omista, ei tuhoa! // jos iterointia halutaan rajata maskilla, tähän
                          // talletetaan maski
  bool fMaskInUse;  // ennen maskin käyttöä optimoinnin vuoksi on tarkastettu, onko maski käytössä
                    // vai ei (onko maski olemassa ja onko se enbloitu)

 private:
  NFmiDataIterator(const NFmiDataIterator& theIterator);
  NFmiDataIterator& operator=(const NFmiDataIterator& theIterator);

};  // class NFmiDataIterator

// ----------------------------------------------------------------------
/*!
 * Marko: Laitoin tämän tänne, että voi säätää relatiivisen iteraattorin asetuksia
 */
// ----------------------------------------------------------------------

inline void NFmiDataIterator::SetDimensions(long /* dx */, long /* dy */, long /* dt */) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should probably be const
 */
// ----------------------------------------------------------------------

inline NFmiAreaMask* NFmiDataIterator::Mask()
{
  return itsMask;
}
// ----------------------------------------------------------------------
/*!
 * Anna 0-pointteri, jos haluat lopettaa maskin käytön
 *
 * \param newMask Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataIterator::Mask(NFmiAreaMask* newMask)
{
  itsMask = newMask;
}
// ----------------------------------------------------------------------
/*!
 * Tätä ei aseteta, vaan se tarkistetaan
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIterator::MaskInUse() const
{
  return fMaskInUse;
}

// ======================================================================
