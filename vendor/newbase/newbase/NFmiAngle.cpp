// ======================================================================
/*!
 * \file NFmiAngle.cpp
 * \brief Määrittelyt luokkien NFmiAngle, NFmiWindDirection, NFmiLatitude
 *        ja NFmiLongitude metodeille.
 */
// ======================================================================
/*!
 * \class NFmiAngle
 *
 * Määrittely yleiselle kulmasuureelle. Luokan keskeinen ominaisuus on, että
 * luokan metodit huolehtivat automaattisesti, että kulma pysyy sallituissa
 * rajoissa jokaisen operaation jälkeen.
 */
// ======================================================================

#include "NFmiAngle.h"
#include <macgyver/Hash.h>
#include <macgyver/Exception.h>
#include <iostream>

using namespace std;

// ======================================================================
/*!
 * Konstruktori luokalle NFmiAngle asettaa sisäiset arvot käyttäen SetValue
 * metodia, jotta mahdollinen kulmamuunnos tulee suoritetuksi, ja kulman
 * rajat tarkistetuiksi.
 *
 * \param theValue Undocumented
 * \param theUnit Undocumented
 */
// ======================================================================

NFmiAngle::NFmiAngle(double theValue, FmiAngleUnit theUnit)
    : itsValue(0)  // dummy to prevent advanced warnings
{
  try
  {
    SetValue(theValue, theUnit);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Muutosmetodi yleiselle kulmalle. Asetettu arvo muutetaan asteiksi,
 * jos annettu yksikkö on radiaani. Asteille suoritetaan ainoastaan
 * negatiivisuus tarkistus, minkä voi olettaa bugiksi. Jostain syystä
 * korrekti koodi on vaan kommentoitu pois. Ei pitäisi luottaa
 * käyttäjään näin paljon.
 *
 * \bug Ei varmista kunnolla, että kulma on sallituissa rajoissa.
 *
 * \param theValue Undocumented
 * \param theUnit Undocumented
 */
// ======================================================================

void NFmiAngle::SetValue(double theValue, FmiAngleUnit theUnit)
{
  try
  {
    if (theUnit == kRad)
    {
      itsValue = 180 / kPii * theValue;
      if (itsValue != 0 && fmod(itsValue, 360) == 0)
        itsValue = 360;
      else
        itsValue = fmod(itsValue, 360);
    }
    else
      itsValue = theValue;  // fmod(theValue, 360);

    if (itsValue < 0)
      itsValue = itsValue + 360;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Suurempi-kuin vertailuoperaattori kulmille. Tämä on periaatteessa
 * väärin, standardi komposiitit vaativat nimenomaan < ja == operaattorien
 * määrittelyn. Eli, nyt kulmia ei saa esim. laitettua listaan.
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

bool NFmiAngle::operator>(const NFmiAngle& theAngle) const
{
  try
  {
    return (itsValue > theAngle.itsValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Lisäysoperaattori.
 *
 * \bug Toteutus vaikuttaa tyhmältä. Miksei käytetä SetValue metodia,
 * vaan luodaan turha temppi?
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiAngle& NFmiAngle::operator+=(const NFmiAngle& theAngle)
{
  try
  {
    itsValue = NFmiAngle(itsValue + theAngle.itsValue).itsValue;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Vähennysoperaattori.
 *
 * \bug Toteutus vaikuttaa tyhmältä. Miksei käytetä SetValue metodia,
 * vaan luodaan turha temppi?
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiAngle& NFmiAngle::operator-=(const NFmiAngle& theAngle)
{
  try
  {
    itsValue = NFmiAngle(itsValue - theAngle.itsValue).itsValue;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Konstruktori luokalle NFmiWindDirection asettaa sisäiset arvot käyttäen
 * NFmiAngle luokan SetValue metodia, jotta mahdollinen kulmamuunnos tulee
 * suoritetuksi, ja kulman rajat tarkistetuiksi.
 *
 * \param theValue Undocumented
 * \param theUnit Undocumented
 */
// ======================================================================

NFmiWindDirection::NFmiWindDirection(double theValue, FmiAngleUnit theUnit)
    : NFmiAngle(theValue, theUnit)
{
}

// ======================================================================
/*!
 * Spesiaalikonstruktori luokalle NFmiWindDirection asettaa sisäiset arvot
 * perustuen tuulen U- ja V-komponentteihin. Jos kumpi tahansa komponentti
 * on kFloatMissing, asetetaan myös kulma puuttuvaksi.
 *
 * \param theU Undocumented
 * \param theV Undocumented
 */
// ======================================================================

NFmiWindDirection::NFmiWindDirection(double theU, double theV) : NFmiAngle()
{
  try
  {
    if (theU == kFloatMissing || theV == kFloatMissing)
    {
      itsValue = kFloatMissing;
      return;
    }

    if (theU == 0 && theV == 0)
    {
      SetValue(0, kDeg);
    }
    else
    {
      SetValue(atan2(theU, theV) + kPii, kRad);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Vähennysoperaattori.
 *
 * \bug Siis, jos suunta on operaation jälkeen > 180, esim. 200, niin
 * 360-200 = 160, uusi suunta. Tämähän on peilausta eikä moduloa.
 * Mikä hiton logiikka tässä on? Onkos näissä luokissa kaikki vialla?
 * Itse toteutuskin on omituinen temppiluokkineen. Koko perintä
 * on hieman omituista, kaikki virtuaalimetodit puuttuvat.
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiWindDirection& NFmiWindDirection::operator-=(const NFmiWindDirection& theAngle)
{
  try
  {
    itsValue = NFmiAngle(NFmiAngle::operator-=(theAngle)).Value();
    if (itsValue > 180)
      itsValue = 360 - itsValue;

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Konstruktori luokalle NFmiLatitude asettaa sisäiset arvot käyttäen
 * NFmiLatitude luokan SetValue metodia, jotta mahdollinen kulmamuunnos tulee
 * suoritetuksi, ja kulman rajat tarkistetuiksi.
 *
 * \param theAngle Undocumented
 * \param theUnit Undocumented
 */
// ======================================================================

NFmiLatitude::NFmiLatitude(double theAngle, FmiAngleUnit theUnit)
{
  try
  {
    SetValue(theAngle, theUnit);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Asetusmetodi latitudille. Metodi varmistaa, että annettu arvo
 * rajoitetaan välille -90 <= latitudi <= 90.
 *
 * \bug Annettu kulmayksikkö jätetään täysin huomioimatta, selvästikin
 * oletetaan, että yksikkö on asteina. Lisäksi oletettujen
 * pyöristysvirheiden korjailu on aina riskaabelia. Lisäksi SetValue
 * kutsuu joskus itseään uudelleen, mikä on vähän ihmeellistä, kun
 * arvo kerran tunnetaan.
 *
 * \param theAngle Undocumented
 * \param theUnit Undocumented
 */
// ======================================================================

void NFmiLatitude::SetValue(double theAngle, FmiAngleUnit theUnit)
{
  try
  {
    if (fabs(theAngle - 90) < 0.0000001)
      itsValue = 90;
    else if (fabs(theAngle + 90) < 0.0000001)
      itsValue = -90;
    else if (theAngle > 90)
      SetValue(90, theUnit);
    else if (theAngle < -90)
      SetValue(-90, theUnit);
    else
      itsValue = theAngle;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Vähennysoperaattori.
 *
 * \bug Ja taas luodaan turha temppi SetValue metodin kutsumisen sijaan.
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiLatitude& NFmiLatitude::operator-=(const NFmiLatitude& theAngle)
{
  try
  {
    itsValue = NFmiLatitude(itsValue - theAngle.itsValue).itsValue;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Lisäysoperaattori
 *
 * \bug Ja taas luodaan turha temppi SetValue metodin kutsumisen sijaan.
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiLatitude& NFmiLatitude::operator+=(const NFmiLatitude& theAngle)
{
  try
  {
    itsValue = NFmiLatitude(itsValue + theAngle.itsValue).itsValue;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Vähennysoperaattori annetulla kulmalla.
 *
 * \bug Ja taas luodaan turha temppi SetValue metodin kutsumisen sijaan.
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiLatitude& NFmiLatitude::operator-=(const double& theAngle)
{
  try
  {
    itsValue = NFmiLatitude(itsValue - theAngle).itsValue;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Lisäysoperaattori
 *
 * \bug Ja taas luodaan turha temppi SetValue metodin kutsumisen sijaan.
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiLatitude& NFmiLatitude::operator+=(const double& theAngle)
{
  try
  {
    itsValue = NFmiLatitude(itsValue + theAngle).itsValue;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Konstruktori luokalle NFmiLongitude asettaa sisäiset arvot käyttäen
 * NFmiLongitude luokan SetValue metodia, jotta mahdollinen kulmamuunnos tulee
 * suoritetuksi, ja kulman rajat tarkistetuiksi.
 *
 * \param theAngle Undocumented
 * \param theUnit Undocumented
 */
// ======================================================================

NFmiLongitude::NFmiLongitude(double theAngle, bool usePacificView, FmiAngleUnit theUnit)
    : fPacificView(usePacificView)
{
  try
  {
    SetValue(theAngle, theUnit);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Asetusmetodi longitudille. Metodi varmistaa, että annettu arvo
 * rajoitetaan välille -180 <= latitudi <= 180.
 *
 * \bug Annettu kulmayksikkö jätetään täysin huomioimatta, selvästikin
 * oletetaan, että yksikkö on asteina. Lisäksi käytetään rekursiota
 * rajaehtojen toteuttamiseksi, kun pitäisi käyttää oikeata mod
 * käskyä. Muuten käsky voi kestää tarpeettoman kauan. Mitä jos
 * joku antaa arvon 100000? Monta iteraatiota menee...?
 *
 * \param theAngle Undocumented
 * \param theUnit Undocumented
 */
// ======================================================================

void NFmiLongitude::SetValue(double theAngle, FmiAngleUnit theUnit)
{
  try
  {
    if (fPacificView)
    {
      if (::fabs(theAngle - 360.) < 0.0000001)
        itsValue = 360.;
      else if (::fabs(theAngle + 360.) < 0.0000001)
        itsValue = 0.;
      else if (theAngle > 360)
        SetValue(theAngle - 360, theUnit);
      else if (theAngle < 0)
        SetValue(theAngle + 360, theUnit);
      else
        itsValue = theAngle;
    }
    else
    {
      if (::fabs(theAngle - 180.) < 0.0000001)
        itsValue = 180.;
      else if (::fabs(theAngle + 180.) < 0.0000001)
        itsValue = -180.;
      else if (theAngle > 180)
        SetValue(theAngle - 360, theUnit);
      else if (theAngle < -180)
        SetValue(theAngle + 360, theUnit);
      else
        itsValue = theAngle;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Vähennysoperaattori.
 *
 * \bug Ja taas luodaan turha temppi SetValue metodin kutsumisen sijaan.
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiLongitude& NFmiLongitude::operator-=(const NFmiLongitude& theAngle)
{
  try
  {
    itsValue = NFmiLongitude(itsValue - theAngle.itsValue, fPacificView).itsValue;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Lisäysoperaattori
 *
 * \bug Ja taas luodaan turha temppi SetValue metodin kutsumisen sijaan.
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiLongitude& NFmiLongitude::operator+=(const NFmiLongitude& theAngle)
{
  try
  {
    itsValue = NFmiLongitude(itsValue + theAngle.itsValue, fPacificView).itsValue;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Vähennysoperaattori annetulla kulmalla.
 *
 * \bug Ja taas luodaan turha temppi SetValue metodin kutsumisen sijaan.
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiLongitude& NFmiLongitude::operator-=(const double& theAngle)
{
  try
  {
    itsValue = NFmiLongitude(itsValue - theAngle, fPacificView).itsValue;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Lisäysoperaattori annetulla kulmalla.
 *
 * \bug Ja taas luodaan turha temppi SetValue metodin kutsumisen sijaan.
 *
 * \param theAngle Undocumented
 * \return Undocumented
 */
// ======================================================================

NFmiLongitude& NFmiLongitude::operator+=(const double& theAngle)
{
  try
  {
    itsValue = NFmiLongitude(itsValue + theAngle, fPacificView).itsValue;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * \brief Hash value
 */
// ======================================================================

std::size_t NFmiAngle::HashValue() const
{
  try
  {
    return Fmi::hash_value(itsValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
