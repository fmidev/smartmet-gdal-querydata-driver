// ======================================================================
/*!
 * \file NFmiGlobals.h
 * \brief Global constants
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include "NFmiPragma.h"

PRAGMA_WARNING_PUSH()
PRAGMA_WARNING_DISABLE(4996)
#include <boost/math/special_functions/fpclassify.hpp>
PRAGMA_WARNING_POP()

namespace Fmi
{
bool LegacyMode();
}

//! keep Chinese as last because no names defined
enum FmiLanguage
{
  kFinnish = 1,
  kSwedish,
  kEnglish,
  kSpanish,
  kGermany,
  kFrench,
  kSaame,
  kLatvian,
  kChinese  // keep this the last
};

//! Undocumented
enum FmiInterpolationMethod
{
  kNoneInterpolation,
  kLinearly,
  kNearestPoint,
  kByCombinedParam,
  kLinearlyFast,
  kLagrange,
  kNearestNonMissing = 6,  // Tämä on Weather yhdistelmäparametrin purkuun liittyvä uusi
                           // interpolaatio kFmiPrecipitationType ja kFmiPrecipitationForm
                           // parametreille.
  // Jos interpolaatiossa ei lähimmästä ajasta/pisteestä löydy arvoa, katsotaan löytyykö
  // ei-puuttuvaa kauemmasta ajasta/hilapisteistä.
  // Ilman tätä tulee purettujen kFmiPrecipitation1h (lineaarinen) ja kFmiPrecipitationType +
  // kFmiPrecipitationForm (nearest) arvoille toisinaan
  // konflikti arvoja, missä sateelle löytyy intensiteettiä, mutta ei tyyppiä tai olomuotoa.
  kMaxInterpolation  // Tämän pitää olla aina listan viimeinen, että tietyt tarkastelut onnistuvat
};

//! Undocumented
enum FmiTimeLocalzation
{
  kUTC,
  kLongitude,
  kState
};

//! Undocumented
enum FmiStationType
{
  kNone,
  kWMO,
  kLPNN,
  kNotWMO,
  kLatLon,
  kGrid,
  kAll
};

//! Undocumented, contains undocumented magic numbers
enum FmiDirection
{
  kNoDirection = 0,
  kLeft = 1,
  kRight = 2,
  kUp = 4,
  kUpLeft = 5,
  kUpRight = 6,
  kDown = 8,
  kLeftDown = 9,
  kRightDown = 10,
  kTop = 16,
  kTopLeft = 17,
  kTopRight = 18,
  kBottom = 32,
  kBottomLeft = 33,
  kBottomRight = 34,
  kBase = 64,
  kBaseLeft = 65,
  kBaseRight = 66,
  kBaseCenter,
  kForward,
  kBackward,
  kCenter,
  kTopCenter,
  kBottomCenter,
  kMiddleLeft,
  kMiddleCenter,
  kMiddleRight
};

//! Undocumented
enum FmiMaskOperation
{
  kFmiNoMaskOperation = 0,
  kFmiMaskEqual,                         // ==
  kFmiMaskGreaterThan,                   // >
  kFmiMaskLessThan,                      // <
  kFmiMaskGreaterOrEqualThan,            // >=
  kFmiMaskLessOrEqualThan,               // <=
  kFmiMaskGreaterThanAndLessThan,        // > && <
  kFmiMaskGreaterOrEqualAndLessOrEqual,  // >= && <=
  kFmiMaskGreaterOrEqualAndLessThan,     // >= && <
  kFmiMaskGreaterThanAndLessOrEqual,     // > && <=
  kFmiMaskLessThanOrGreaterThan,        // < || >
  kFmiMaskLessOrEqualOrGreaterOrEqual,  // <= || >=
  kFmiMaskLessOrEqualOrGreaterThan,     // <= || >
  kFmiMaskLessThanOrGreaterOrEqual,     // < || >=
  kFmiMaskEqualOrEqual,                 // = || =

  // näiden 'ramppimaskien' avulla voidaan laskea tarvittaessa liukuvia maskeja

  kFmiMaskRisingRamp,  // kerroin 0, kun arvo alle alarajan, 1, kun yli ylärajan, 0-1
                            // (lasketaan lineaarisesti), kun siinä välillä
  kFmiMaskLoweringRamp,  // kerroin 1, kun arvo alle alarajan, 0, kun yli ylärajan, 1-0, kun siinä välillä
  kFmiMaskDoubleRamp,  // kerroin -1, kun arvo alle alarajan, 1, kun yli ylärajan, -1 - 1, kun siinä välillä

  kFmiMaskAll,
  kFmiMaskNotEqual,  // != tai <> miten sitä milläkin kielellä esitetään
  kFmiMaskContinuousEqual  // ->    (simple condition where two consecutive values have wanted
                           // value in between them)

};

//! Undocumented
enum FmiEnumSpace
{
  kNoneEnumSpace,
  kParamNames,
  kRoadRegions,
  kPressRegions
};

// Mika: Mikäs tämä on?? Include suojaus keskellä tiedostoa?

#ifndef __FMITYPES_H__
#ifdef __cplusplus
const unsigned char kByteMissing = 255;
const short int kShortMissing = 32700;
const long int kLongMissing = 0xffffffffL;
const unsigned long kUnsignedLongMissing = 0xffffffffL;
const float kFloatMissing = 32700.0F;
const double kPii = 3.14159265358979323846264338327950288419716939937510;
const double kRearth = 6371220.;
const double kRedMissing = 1.0;
const double kGreenMissing = 0.0;
const double kBlueMissing = 0.0;
const double kAlphaMissing = 0.0;
const double kMaxDouble = 1.7E+307;
const double kMinDouble = -1.7E+307;
const float kMaxFloat = 3.4E+37f;
const float kMinFloat = -3.4E+37f;
const float kRadarPrecipitationMissing = 65535;
// shape datoissa menee jotkin longitudet hieman yli 180 asteen, mutta niitä pitää käsitellä silti
// atlantic-maailmassa (cntry06 data has longitudes such as 180.00000033527612686)
const double kEpsPacific = 0.00001;
#else
#define kByteMissing 255
#define kShortMissing 32700
#define kLongMissing 0xffffffffL
#define kFloatMissing 32700.0F
#define kPii 3.14159265358979323846264338327950288419716939937510;
#define kRearth 6371220.;
#define kRedMissing 1.0;
#define kGreenMissing 0.0;
#define kBlueMissing 0.0;
#define kAlphaMissing 0.0;
#define kRadarPrecipitationMissing 65535

#define kMaxDouble 1.7E+307
#define kMinDouble -1.7E+307
#define kMaxFloat 3.4E+37f
#define kMinFloat -3.4E+37f
#endif  // __cplusplus

//! Undocumented
typedef unsigned short FmiCounter;

// Tämä on kaikille u-long tyyppisille indekseille puuttuva tai alustamaton arvo.
// Sitä voi käyttää newbase indeksien tutkimiseen, ilman ikäviä static_cast:eja.
static const unsigned long gMissingIndex = static_cast<unsigned long>(-1);

//! Undocumented, should be removed
enum Logical
{
  isFalse,
  isTrue
};

#endif  // __FMITYPES_H__

// ÄLÄ KÄYTÄ ENÄÄ FmiMin ja FmiMax makroja, käytä std::min ja std::max funktioita.
// FmiMin ja FmiMax poistuvat lähiaikoina 8.10.2002
//#define	FmiMax(x, y)		((x) > (y) ? (x) : (y))
//#define	FmiMin(x, y)		((x) < (y) ? (x) : (y))
#define FmiRad(theAngle) ((theAngle)*kPii / 180.)
#define FmiDeg(theRadian) ((theRadian)*180. / kPii)
#define FmiKnot(theMeter) (theMeter / 0.5144)
#define FmiMeter(theKnot) (theKnot * 0.5144)

// use C99 math.h instead of these:
//#if 0  //poisto päälle taas kun Lassella uusi newbase
#define FmiRound(x) ((x) < 0.0 ? static_cast<long>((x)-0.5) : static_cast<long>((x) + 0.5))
#define FmiTrunc(x) (static_cast<long>(x))
//#endif

// MS Visual C++ doesn't provide C99 conformance so round and trunc functions must be
// defined and implemented here.
// Linux gcc compiler seems to provide C99 conformance and these functions.
// Visual C++ 2013 finally supports C99 (_MSC_VER = 1800)
#if defined _MSC_VER && _MSC_VER < 1800
inline int round(double x)
{
  return (x < 0.0) ? static_cast<long>(x - 0.5) : static_cast<long>(x + 0.5);
}
inline int trunc(double x)
{
  return static_cast<int>(x);
}
#endif

template <typename T>
inline bool FmiIsValidNumber(T theValue)
{
  if (boost::math::isfinite(theValue))
    return true;
  return false;
}

template <typename T>
inline T FmiMakeValidNumber(T theValue)
{
  if (::FmiIsValidNumber(theValue))
    return theValue;
  return kFloatMissing;
}

// Jouduin ottamaan FmiMin ja FmiMax funktiot käyttöön (std::min ja max versiot) newbasessa
// ja muissa omissa kirjastoissa. Tein niistä nyt makrojen sijasta kunnon template funktioita.
// Tämä sen takia, koska MSVC-kääntäjällä on idioottimaiset makrot min ja max, joita on vaikea
// ohittaa
// ja jotka aiheuttavat käännös ongelmia, kun otin käyttöön std::vector:iin (NFmiDataMatrix.h)
// call stack reportoinnin virhetilanteissa (toimii vain MSVC kääntäjällä). Tämä feature
// vaatii stdafx.h headerin includointia ja sieltä tulee min ja max ja muita kivoja
// MSVC makroja sotkemaan hommia.
template <class _Tp>
inline const _Tp& FmiMin(const _Tp& __a, const _Tp& __b)
{
  return __b < __a ? __b : __a;
}

template <class _Tp>
inline const _Tp& FmiMax(const _Tp& __a, const _Tp& __b)
{
  return __a < __b ? __b : __a;
}

// Double/int -tulkinnat aiheuttavat jatkuvasti ongelmaa FmiMax:n kanssa
// (viimeksi VC++ 2008-käännöksessä). Käyttämällä tätä fiksataan tyyppi
// doubleksi.   --AKa 7-Oct-2008
//
inline double FmiMin(double a, double b)
{
  return b < a ? b : a;
}
inline double FmiMax(double a, double b)
{
  return a < b ? b : a;
}
/* C++ equivalents
   inline double	FmiMax(double x, double y) const {x > y ? x : y;};
   inline double	FmiMin(double x, double y) const {x < y ? x : y;};
   inline double	FmiRad(double theAngle) const {theAngle*kPii/180.;};	// 20.4.98/EL
   inline double	FmiDeg(double theRadian) const {theRadian*180./kPii;};	// 20.4.98/EL
   inline long		FmiRound(double x) const {x < 0.0 ? (long) (x - 0.5) : (long) (x + 0.5);};
   // 8.6.98/EL
   inline long		FmiTrunc(double x) const { return (long) x;}; // 8.6.98/EL
*/

// ======================================================================
