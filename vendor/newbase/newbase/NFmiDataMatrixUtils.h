/*!
 * \file NFmiDataMatrixUtils.h
 * \brief Helper functions for class NFmiDataMatrix
 */

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiGlobals.h"  // kFloatMissing
#include "NFmiParameterName.h"
#include "NFmiRect.h"

namespace DataMatrixUtils
{
static const NFmiRect g_defaultCoordinates(0, 0, 1, 1);

// Tämä funktio laskee interpoloidun arvon matriisin datasta.
// Annettu piste on halutussa suhteellisessa koordinaatistossa (esim. 0,0  -  1,1 maailmassa)
// ja kyseinen koordinaatisto (rect-olio) pitää antaa parametrina.
// HUOM! rect-olio ja matriisi ovat y-akselin suhteen käänteisiä!
float InterpolatedValue(const NFmiDataMatrix<float>& m,
                        const NFmiPoint& thePoint,
                        const NFmiRect& theRelativeCoords,
                        FmiParameterName theParamId,
                        bool fDontInvertY = false,
                        FmiInterpolationMethod interp = kLinearly);

// Tämä funktio laskee interpoloidun arvon matriisin datasta.
// Oletus annettu piste on aina 0,0  -  1,1 maailmassa ja lasketaan siihen halutut indeksit.
inline float InterpolatedValue(const NFmiDataMatrix<float>& m,
                               const NFmiPoint& thePoint,
                               FmiParameterName theParamId,
                               bool fDontInvertY = false,
                               FmiInterpolationMethod interp = kLinearly)
{
  return InterpolatedValue(m, thePoint, g_defaultCoordinates, theParamId, fDontInvertY, interp);
}

void PrettyPrint(std::ostream& s,
                 const NFmiDataMatrix<float>& m,
                 bool printYInverted,
                 bool printIndexAxies);
}  // namespace DataMatrixUtils
