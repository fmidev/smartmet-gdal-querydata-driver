// ======================================================================
/*!
 * \class LatLonTree
 *
 * \brief Nearest neighbour search tree adapted to spherical lat lon coordinates
 *
 * We assume a spherical FMI earth so that the triangle inequality holds.
 *
 * NOT THREAD SAFE! Insert all points first and flush, after that accessing
 * is thread safe.
 *
 * Note: Using NearTreeLatLon objects in a normal NearTree is potentially
 *       much faster if there are a lot of searches, since the number of
 *       trigonometric functions is much smalled than using GeoDistance.
 *
 */
// ======================================================================

#pragma once

#include "Geometry.h"
#include "NearTree.h"

namespace Fmi
{
template <typename P>
struct LatLonDistance
{
  double operator()(const P& lhs, const P& rhs) const
  {
    return Fmi::Geometry::GeoDistance(lhs.longitude, lhs.latitude, rhs.longitude, rhs.latitude) /
           1000.0;
  }
};

/*
 * \brief LatLon search tree for FMI spherical coordinates
 */

template <typename T, typename F = LatLonDistance<T> >
class LatLonTree : public NearTree<T, F>
{
};

}  // namespace Fmi
