// ======================================================================
/*!
 * \file NFmiSmoother.h
 * \brief Interface of class NFmiSmoother
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"  // for NFmiDataMatrix
#include "NFmiGlobals.h"     // for kFloatMissing
#include "NFmiPoint.h"       // for NFmiPoint
#include <gis/CoordinateMatrix.h>
#include <string>

//! Undocumented
class NFmiSmoother
{
 public:
  //! Different smoothening methods

  enum NFmiSmootherMethod
  {
    kFmiSmootherMissing,
    kFmiSmootherNone,
    kFmiSmootherNeighbourhood,
    kFmiSmootherPseudoGaussian
  };

  // Constructors

  NFmiSmoother(NFmiSmootherMethod theSmoother, int theFactor = 1, float theRadius = 0.0);
  NFmiSmoother(const std::string& theSmootherName, int theFactor = 1, float theRadius = 0.0);

  static NFmiSmootherMethod SmootherValue(const std::string& theName);
  static const std::string SmootherName(NFmiSmootherMethod theSmoother);

  NFmiSmootherMethod Smoother() const;
  int Factor() const;
  float Radius() const;

  void Smoother(NFmiSmootherMethod smoother);
  void Factor(int factor);
  void Radius(float radius);

  // Smoothen the given data

  const NFmiDataMatrix<float> Smoothen(const Fmi::CoordinateMatrix& thePts,
                                       const NFmiDataMatrix<float>& theValues) const;

  const std::vector<float> Smoothen(const std::vector<float>& theX,
                                    const std::vector<float>& theY) const;

  float Weight(float distance) const;

 private:
  // Disable void constructor
  NFmiSmoother();

  const NFmiDataMatrix<float> SmoothenKernel(const Fmi::CoordinateMatrix& thePts,
                                             const NFmiDataMatrix<float>& theValues) const;

  const std::vector<float> SmoothenKernel(const std::vector<float>& theX,
                                          const std::vector<float>& theY) const;

  // Data members

  NFmiSmootherMethod itsSmoother;
  float itsRadius;
  int itsFactor;

};  // class NFmiSmoother

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiSmoother::NFmiSmootherMethod NFmiSmoother::Smoother() const
{
  return itsSmoother;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiSmoother::Factor() const
{
  return itsFactor;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiSmoother::Radius() const
{
  return itsRadius;
}
// ----------------------------------------------------------------------
/*!
 * \param smoother Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSmoother::Smoother(NFmiSmootherMethod smoother)
{
  itsSmoother = smoother;
}
// ----------------------------------------------------------------------
/*!
 * \param factor Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSmoother::Factor(int factor)
{
  itsFactor = factor;
}
// ----------------------------------------------------------------------
/*!
 * \param radius Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSmoother::Radius(float radius)
{
  itsRadius = radius;
}
// ----------------------------------------------------------------------
/*!
 * The weighting function. This is public so that external
 * programs may plot the weight functions.
 *
 * \param theDistance The distance for the weight to be calculated
 * \return The calculated weight function
 */
// ----------------------------------------------------------------------

inline float NFmiSmoother::Weight(float theDistance) const
{
  float weight;
  switch (Smoother())
  {
    case kFmiSmootherNeighbourhood:
    {
      float delta = static_cast<float>(3.0 / Radius() * theDistance * Factor());
      weight = static_cast<float>((1.0 / (1.0 + delta * delta)));
      break;
    }
    case kFmiSmootherPseudoGaussian:
    {
      float a = Radius();
      float x = theDistance * Factor();
      weight = (a * a / (a * a + x * x));
      break;
    }
    default:
    {
      weight = (theDistance == 0.0f ? 1.0f : 0.0f);
    }
  }
  return weight;
}

// ======================================================================
