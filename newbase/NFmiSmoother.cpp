// ======================================================================
/*!
 * \file NFmiSmoother.cpp
 * \brief Implementation of class NFmiSmoother
 */
// ======================================================================

#include "NFmiSmoother.h"
#include <macgyver/Exception.h>
#include <cassert>
#include <cstdlib>
// abs(int)

using namespace std;

//! A macro to generate the sequence  0,-1,1,-2,2,-3,3...

#define NEXTDIR(a) (((a) >= 0) ? (-(a)-1) : (-(a)))

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theSmoother The smoothing method to use
 * \param theFactor The smoothing factor to use
 * \param theRadius The smoothing radius
 */
// ----------------------------------------------------------------------

NFmiSmoother::NFmiSmoother(NFmiSmootherMethod theSmoother, int theFactor, float theRadius)
    : itsSmoother(theSmoother), itsRadius(theRadius), itsFactor(theFactor)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theSmootherName The name of the smoothing method to use
 * \param theFactor The smoothing factor to use
 * \param theRadius The smoothing radius
 */
// ----------------------------------------------------------------------

NFmiSmoother::NFmiSmoother(const std::string& theSmootherName, int theFactor, float theRadius)
    : itsSmoother(SmootherValue(theSmootherName)), itsRadius(theRadius), itsFactor(theFactor)
{
}

// ----------------------------------------------------------------------
/*!
 * Smoothen the given data matrix with the given coordinates with the
 * currently active method. The coordinates are assumed to form a regular
 * 2D matrix. That is, there should be a conformal mapping from a uniform
 * 2D grid to the coordinates.
 *
 * \param thePts The coordinates of the points
 * \param theValues The values to smoothen
 */
// ----------------------------------------------------------------------

const NFmiDataMatrix<float> NFmiSmoother::Smoothen(const Fmi::CoordinateMatrix& thePts,
                                                   const NFmiDataMatrix<float>& theValues) const
{
  try
  {
    assert(thePts.width() == theValues.NX() && thePts.height() == theValues.NY());

    switch (Smoother())
    {
      case kFmiSmootherNeighbourhood:
      case kFmiSmootherPseudoGaussian:
        return SmoothenKernel(thePts, theValues);
      default:
        break;
    }
    return theValues;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Smoothen the given data vector with the given coordinates with the
 * currently active method. No ordering is assumed on the coordinates.
 *
 * \param theX The coordinates of the points
 * \param theY The values to smoothen
 */
// ----------------------------------------------------------------------

const std::vector<float> NFmiSmoother::Smoothen(const std::vector<float>& theX,
                                                const std::vector<float>& theY) const
{
  try
  {
    assert(theX.size() == theY.size());

    switch (Smoother())
    {
      case kFmiSmootherNeighbourhood:
      case kFmiSmootherPseudoGaussian:
        return SmoothenKernel(theX, theY);
      default:
        break;
    }
    return theY;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Smoothen the given data with near neighbourhood weighted interpolation.
 * Contour the given data values with neighbourhood interpolation
 * The given radius is the search radius distance for interpolating
 * the actual Z-value to be used in the grid. The idea is based
 * on GMT (Generic Mapping Tools) 'nearneighbour' command, and is intended
 * for contouring poor resolution data, such as wind speed (integers).
 * We have extended the method to use a parametrized weight function.
 *
 * \param thePts The coordinates of the values
 * \param theValues The values to be smoothened
 */
// ----------------------------------------------------------------------

const NFmiDataMatrix<float> NFmiSmoother::SmoothenKernel(
    const Fmi::CoordinateMatrix& thePts, const NFmiDataMatrix<float>& theValues) const
{
  try
  {
    // Temporary holder for the interpolated values

    NFmiDataMatrix<float> values(theValues.NX(), theValues.NY());

    // From how may sectors do we require values?
    // Should not exceed the number of bits in an integer

    const int nsectors = 4;
    const float pi = 3.14159265358979323f;
    const auto factor = static_cast<float>(nsectors / (2.0 * pi));

    // Bitmask indicating full set of sectors:

    const int allsectors = (1 << nsectors) - 1;

    for (unsigned int j = 0; j < thePts.height(); j++)
      for (unsigned int i = 0; i < thePts.width(); i++)
      {
        // The coordinates to which we're interpolating

        auto x = static_cast<float>(thePts.x(i, j));
        auto y = static_cast<float>(thePts.y(i, j));

        float zsum = 0.0;  // weighted sum of function values
        float wsum = 0.0;  // sum of the weights

        // Search the grid for nearby values

        int sectors = 0;
        int ymisses = 0;
        for (int dj = 0; static_cast<unsigned int>(abs(dj)) < theValues.NY(); dj = NEXTDIR(dj))
        {
          if (ymisses >= 2)
            break;
          int xmisses = 0;
          int xgoods = 0;
          for (int di = 0; static_cast<unsigned int>(abs(di)) < theValues.NX(); di = NEXTDIR(di))
          {
            if (xmisses >= 2)
              break;

            // Test against grid bounds

            if (static_cast<int>(i) + di < 0 || static_cast<int>(j) + dj < 0 ||
                i + di >= theValues.NX() || j + dj >= theValues.NY())
            {
              xmisses++;
              continue;
            }

            // Test the coordinate and the value are valid

            if (thePts.x(i + di, j + dj) == kFloatMissing ||
                thePts.y(i + di, j + dj) == kFloatMissing ||
                theValues[i + di][j + dj] == kFloatMissing)
              continue;

            // See if within search radius

            auto dx = static_cast<float>(thePts.x(i + di, j + dj) - x);
            auto dy = static_cast<float>(thePts.y(i + di, j + dj) - y);

            float dist = sqrt(dx * dx + dy * dy);

            if (dist > Radius())
              xmisses++;
            else
            {
              xmisses = 0;
              xgoods++;

              float weight = Weight(dist);

              wsum += weight;
              zsum += weight * theValues[i + di][j + dj];

              // The sector in question (0..nsectors-1)

              int sector = static_cast<int>((atan2(dx, dy) + pi) * factor) % nsectors;
              // Set the bit on for this sector

              sectors |= (1 << sector);
            }
          }
          if (xgoods == 0)
            ymisses++;
          else
            ymisses = 0;
        }
        // If got all desired sectors, then we can handle the sum

        if ((sectors == allsectors) && (wsum > 0.0))
          values[i][j] = zsum / wsum;
        else
          values[i][j] = theValues[i][j];
      }

    // And return the interpolated values

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Smoothen the given 1D curve with the currently active method.
 * No ordering of the X-coordinates is assumed. Currently a quadratic
 * algorithm is used, if the method needs to be called for large
 * data sets, the algorithm should be optimized via an auxiliary
 * vector containing the X-ordering.
 *
 * \param theX The X-coordinates
 * \param theY The Y-coordinates to smoothen
 */
// ----------------------------------------------------------------------

const std::vector<float> NFmiSmoother::SmoothenKernel(const std::vector<float>& theX,
                                                      const std::vector<float>& theY) const
{
  try
  {
    // We need a dummy
    std::vector<float> result(theY.size(), kFloatMissing);

    for (unsigned int i = 0; i < result.size(); i++)
    {
      float sum = 0;
      float wsum = 0;
      for (unsigned int j = 0; j < theX.size(); j++)
      {
        if (theY[j] != kFloatMissing)
        {
          float dist = abs(theX[i] - theX[j]);
          if (dist <= itsRadius)
          {
            float w = Weight(dist);
            sum += w * theY[j];
            wsum += w;
          }
        }
      }
      if (wsum > 0)
        result[i] = sum / wsum;
    }

    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Convert a contour smoothening method name to a method enum
 * The name should be the trailing part of the enum-name.
 * Returns kFmiSmootherMissing for unrecognized name
 *
 * \param theName The name of the method
 * \return The enumerated method
 */
// ----------------------------------------------------------------------

NFmiSmoother::NFmiSmootherMethod NFmiSmoother::SmootherValue(const string& theName)
{
  try
  {
    if (theName == "None")
      return kFmiSmootherNone;

    if (theName == "Neighbourhood")
      return kFmiSmootherNeighbourhood;

    if (theName == "PseudoGaussian")
      return kFmiSmootherPseudoGaussian;

    return kFmiSmootherMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Convert a contour smoothening method enum to a method name.
 * The name is guaranteed to be the trailing part of the enum-name.
 * Returns a "Missing" if the name is unknown
 *
 * \param theSmoother The enumerated smoother method
 * \return The name of the method
 */
// ----------------------------------------------------------------------

const string NFmiSmoother::SmootherName(NFmiSmoother::NFmiSmootherMethod theSmoother)
{
  try
  {
    switch (theSmoother)
    {
      case kFmiSmootherNone:
        return string("None");
      case kFmiSmootherNeighbourhood:
        return string("Neighbourhood");
      case kFmiSmootherPseudoGaussian:
        return string("PseudoGaussian");
      default:
        return string("Missing");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
