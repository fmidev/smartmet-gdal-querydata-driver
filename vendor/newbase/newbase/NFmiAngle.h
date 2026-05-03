// ======================================================================
/*!
 * \file NFmiAngle.h
 * \brief Deklaraatiot luokille NFmiAngle, NFmiWindDirection, NFmiLatitude
 *        ja NFmiLongitude.
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include <cmath>

//! Tyyppimääritelmä kulmayksiköille.
typedef enum
{
  kDeg,
  kRad
} FmiAngleUnit;

//! Yleinen kulman määrittely perusluokka

class NFmiAngle
{
 public:
  //! Destruktori
  virtual ~NFmiAngle() {}
  //! Konstruktori
  NFmiAngle(double theAngle = 0.0, FmiAngleUnit theUnit = kDeg);

  //! Kulman asetus optionaalisessa yksikössä
  void SetValue(double theValue, FmiAngleUnit theUnit = kDeg);

  //! Kulman palautus
  double Value() const { return itsValue; }
  //! Suurempi kuin vertailuoperaattori
  bool operator>(const NFmiAngle &theAngle) const;

  //! Lisäysoperaattori
  NFmiAngle &operator+=(const NFmiAngle &theAngle);

  //! Vähennysoperaattori
  NFmiAngle &operator-=(const NFmiAngle &theAngle);

  //! Muunnos radiaaneiksi
  double ToRad() const { return itsValue / 180.0 * kPii; }
  //! Muunnos asteiksi
  double ToDeg() const { return itsValue; }
  // 14.5.2002/Marko+Mika MSVC++:ssa ei sin ja muut vastaavat ole std:ssa, siitä using viritys.

  //! Kulman sini
  double Sin() const
  {
    using namespace std;
    return sin(ToRad());
  }

  //! Kulman kosini
  double Cos() const
  {
    using namespace std;
    return cos(ToRad());
  }

  //! Kulman tangentti
  double Tan() const
  {
    using namespace std;
    return tan(ToRad());
  }

  //! Kulman arcustangentti
  double Atan() const
  {
    using namespace std;
    return atan(ToRad());
  }

  std::size_t HashValue() const;

 protected:
  //! Itse kulma asteissa.
  double itsValue;
};

// ======================================================================
/*!
 * \class NFmiWindDirection
 *
 * Määrittely yleiselle kulmasuureelle. Luokan keskeinen ominaisuus on, että
 * luokan metodit huolehtivat automaattisesti, että kulma pysyy sallituissa
 * rajoissa jokaisen operaation jälkeen, eli operaatiot toimivat modulo 360.
 *
 * Suunnan vaihtelurajat ovat 0 <= arvo <= 360, jossa arvo=0 tarkoittaa
 * vaihtelevaa tuulta.
 */
// ======================================================================

//! Tuulen suunnan esitysluokka

class NFmiWindDirection : public NFmiAngle
{
 public:
  //! Konstruktori suunnan perusteella
  NFmiWindDirection(double theAngle, FmiAngleUnit theUnit = kDeg);

  //! Konstruktori suunnan U- ja V-komponenttien perusteella
  NFmiWindDirection(double theU, double theV);

  //! Kulman vähennysoperaattori
  NFmiWindDirection &operator-=(const NFmiWindDirection &theAngle);
};

// ======================================================================
/*!
 * \class NFmiLatitude
 *
 * Määrittely yleiselle latitudisuureelle. Luokan keskeinen ominaisuus on, että
 * luokan metodit huolehtivat automaattisesti, että kulma pysyy sallituissa
 * rajoissa jokaisen operaation jälkeen, eli operaatiot toimivat symmetrisenä
 * modulona 180.
 *
 * Latitudin vaihtelurajat ovat -90 <= latitudi <= 90
 */
// ======================================================================

//! Latitudin esitysluokka

class NFmiLatitude : public NFmiAngle
{
 public:
  //! Konstruktori
  NFmiLatitude(double theAngle, FmiAngleUnit theUnit = kDeg);

  NFmiLatitude() {}
  //! Latitudin muutosmetodi
  void SetValue(double theAngle, FmiAngleUnit theUnit = kDeg);

  //! Latitudin vähennysoperaattori
  NFmiLatitude &operator-=(const NFmiLatitude &theAngle);

  //! Latitudin kasvatusoperaattori
  NFmiLatitude &operator+=(const NFmiLatitude &theAngle);

 private:
  //! Latitudin vähennysoperaattori annetun kulman perusteella
  NFmiLatitude &operator-=(const double &theAngle);

  //! Latitudin kasvatusoperaattori annetun kulman perusteella
  NFmiLatitude &operator+=(const double &theAngle);
};

// ======================================================================
/*!
 * \class NFmiLongitude
 *
 * Määrittely yleiselle longitudisuureelle. Luokan keskeinen ominaisuus on, että
 * luokan metodit huolehtivat automaattisesti, että kulma pysyy sallituissa
 * rajoissa jokaisen operaation jälkeen, eli operaatiot toimivat symmetrisenä
 * modulona 360.
 *
 * Longitudin vaihtelurajat ovat -180 <= latitudi <= 180
 */
// ======================================================================

//! Longitudin esitysluokka

class NFmiLongitude : public NFmiAngle
{
 public:
  //! Konstruktori
  NFmiLongitude(double theAngle, bool usePacificView, FmiAngleUnit theUnit = kDeg);

  NFmiLongitude() : fPacificView(false) {}
  //! Longitudin muutosmetodi
  void SetValue(double theValue, FmiAngleUnit theUnit = kDeg);

  //! Longitudin vähennysoperaattori
  NFmiLongitude &operator-=(const NFmiLongitude &theAngle);

  //! Longitudin lisäysoperaattori
  NFmiLongitude &operator+=(const NFmiLongitude &theAngle);

 private:
  //! Longitudin vähennysoperaattori annetulla kulmalla
  NFmiLongitude &operator-=(const double &theAngle);

  //! Longitudin lisäysoperaattori annetulla kulmalla
  NFmiLongitude &operator+=(const double &theAngle);

  bool fPacificView;
};

// ======================================================================
