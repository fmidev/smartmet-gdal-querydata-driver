// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiIndexMaskTools
 */
// ======================================================================

#pragma once

#include <string>
#include <vector>
class NFmiIndexMask;
class NFmiPoint;
class NFmiGrid;
class NFmiSvgPath;
class NFmiFastQueryInfo;
class NFmiCalculationCondition;

namespace NFmiIndexMaskTools
{
// Area inside the path
const NFmiIndexMask MaskInside(const NFmiGrid &theGrid, const NFmiSvgPath &thePath);

// Area outside the path
const NFmiIndexMask MaskOutside(const NFmiGrid &theGrid, const NFmiSvgPath &thePath);

// Expand area with given length
const NFmiIndexMask MaskExpand(const NFmiGrid &theGrid,
                               const NFmiSvgPath &thePath,
                               double theDistance);

// Shrink area with given length
const NFmiIndexMask MaskShrink(const NFmiGrid &theGrid,
                               const NFmiSvgPath &thePath,
                               double theDistance);

// Area with given distance from path edges
const NFmiIndexMask MaskDistance(const NFmiGrid &theGrid,
                                 const NFmiSvgPath &thePath,
                                 double theDistance);

// Area with given distance from point
const NFmiIndexMask MaskDistance(const NFmiGrid &theGrid,
                                 const NFmiPoint &thePoint,
                                 double theDistance);

// Multiple area masks quickly
const std::vector<NFmiIndexMask> MaskExpand(const NFmiGrid &theGrid,
                                            const NFmiSvgPath &thePath,
                                            std::vector<double> theDistances);

// Makes mask to wanted grid from info-grid and calculated by given condition in info's current
// time, param and level.
const NFmiIndexMask MaskCondition(const NFmiGrid &theGrid,
                                  NFmiFastQueryInfo &theInfo,
                                  const NFmiCalculationCondition &theCondition);

// The mask as a string
const std::string MaskString(const NFmiIndexMask &theMask,
                             unsigned long theWidth,
                             unsigned long theHeight);

}  // namespace NFmiIndexMaskTools

// ======================================================================
