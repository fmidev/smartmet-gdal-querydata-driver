// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiSvgTools
 */
// ======================================================================

#pragma once

class NFmiArea;
class NFmiPoint;
class NFmiSvgPath;

namespace NFmiSvgTools
{
bool IsInside(const NFmiSvgPath& thePath, const NFmiPoint& thePoint);
double Distance(const NFmiSvgPath& thePath, const NFmiPoint& thePoint);
double GeoDistance(const NFmiSvgPath& thePath, const NFmiPoint& thePoint);
void BoundingBox(
    const NFmiSvgPath& thePath, double& theXmin, double& theYmin, double& theXmax, double& theYmax);
void LatLonToWorldXY(NFmiSvgPath& thePath, const NFmiArea& theArea);
void PointToSvgPath(NFmiSvgPath& thePath, double x, double y);
void BBoxToSvgPath(NFmiSvgPath& thePath, double x1, double y1, double x2, double y2);

}  // namespace NFmiSvgTools

// ======================================================================
