#pragma once

#include "NFmiDef.h"
#include <cmath>
#include <cstdlib>

/*
 * \brief Proxy to cast latlon coordinates to a unit sphere
 *
 * Using the Haversine formula to search for nearest points is very slow.
 * Casting all data to cartesian coordinates enables using much faster
 * distance formulas provided the great circle distance limit is
 * converted to an equivalent a unit sphere chrord length limit.
 * Note that we still need to use sqrt so that the triangle inequality
 * will be valid.
 */

#include <iostream>

class NFmiLocation;

class NFmiNearTreeLocation
{
 public:
  NFmiNearTreeLocation(const NFmiLocation& theLocation, std::size_t theIndex);
  NFmiNearTreeLocation();

  double GetX() const { return itsX; }
  double GetY() const { return itsY; }
  double GetZ() const { return itsZ; }
  std::size_t GetIndex() const { return itsIndex; }

 private:
  double itsX;
  double itsY;
  double itsZ;
  std::size_t itsIndex;
};

class NFmiNearTreeLocationDistance
{
 public:
  double operator()(const NFmiNearTreeLocation& theLhs, const NFmiNearTreeLocation& theRhs) const
  {
    double dx = theLhs.GetX() - theRhs.GetX();
    double dy = theLhs.GetY() - theRhs.GetY();
    double dz = theLhs.GetZ() - theRhs.GetZ();
    return std::sqrt(dx * dx + dy * dy + dz * dz);
  }
};
