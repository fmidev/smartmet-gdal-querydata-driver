// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiAreaTools
 */
// ======================================================================
/*!
 * \namespace NFmiAreaTools
 *
 * \brief Various utility functions to handle NFmiArea objects
 *
 */
// ======================================================================

#include "NFmiAreaTools.h"
#include "NFmiPoint.h"
#include <macgyver/Exception.h>
#include <algorithm>
#include <iostream>

#include "NFmiEquidistArea.h"
#include "NFmiGnomonicArea.h"
#include "NFmiLambertConformalConicArea.h"
#include "NFmiLambertEqualArea.h"
#include "NFmiLatLonArea.h"
#include "NFmiMercatorArea.h"
#include "NFmiRotatedLatLonArea.h"
#include "NFmiStereographicArea.h"
#include "NFmiYKJArea.h"

// Local utility functions

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Update bounding box based on given point
 *
 * \param thePoint The point to add to the bounding box
 * \param theMinLon The variable in which to store the minimum longitude
 * \param theMinLat The variable in which to store the minimum latitude
 * \param theMaxLon The variable in which to store the maximum longitude
 * \param theMaxLat The variable in which to store the maximum latitude
 */
// ----------------------------------------------------------------------

void update_bbox(const NFmiPoint& thePoint,
                 double& theMinLon,
                 double& theMinLat,
                 double& theMaxLon,
                 double& theMaxLat)
{
  try
  {
    theMinLon = std::min(theMinLon, thePoint.X());
    theMinLat = std::min(theMinLat, thePoint.Y());
    theMaxLon = std::max(theMaxLon, thePoint.X());
    theMaxLat = std::max(theMaxLat, thePoint.Y());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

namespace NFmiAreaTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Find geographic bounding box for given area
 *
 * The bounding box is found by traversing the edges of the area
 * and converting the coordinates to geographic ones for extrema
 * calculations.
 *
 * \param theArea The area
 * \param theMinLon The variable in which to store the minimum longitude
 * \param theMinLat The variable in which to store the minimum latitude
 * \param theMaxLon The variable in which to store the maximum longitude
 * \param theMaxLat The variable in which to store the maximum latitude
 */
// ----------------------------------------------------------------------

void LatLonBoundingBox(const NFmiArea& theArea,
                       double& theMinLon,
                       double& theMinLat,
                       double& theMaxLon,
                       double& theMaxLat)
{
  try
  {
    // Good initial values are obtained from the corners

    theMinLon = theArea.TopLeftLatLon().X();
    theMinLat = theArea.TopLeftLatLon().Y();
    theMaxLon = theMinLon;
    theMaxLat = theMinLat;

    const unsigned int divisions = 500;

    // Go through the top edge

    const double left = theArea.Left();
    const double right = theArea.Right();
    const double bottom = theArea.Bottom();
    const double top = theArea.Top();
    const double width = right - left;
    const double height = bottom - top;

    // Go through the top edge

    unsigned int i = 0;
    for (i = 0; i <= divisions; i++)
    {
      NFmiPoint xy(left + width * i / divisions, top);
      NFmiPoint latlon(theArea.ToLatLon(xy));
      update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
    }

    // Go through the bottom edge

    for (i = 0; i <= divisions; i++)
    {
      NFmiPoint xy(left + width * i / divisions, bottom);
      NFmiPoint latlon(theArea.ToLatLon(xy));
      update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
    }

    // Go through the left edge

    for (i = 0; i <= divisions; i++)
    {
      NFmiPoint xy(left, top + height * i / divisions);
      NFmiPoint latlon(theArea.ToLatLon(xy));
      update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
    }

    // Go through the right edge

    for (i = 0; i <= divisions; i++)
    {
      NFmiPoint xy(right, top + height * i / divisions);
      NFmiPoint latlon(theArea.ToLatLon(xy));
      update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiArea* CreateLegacyLatLonArea(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight)
{
  bool pacific = (theBottomLeft.X() < 180 && theTopRight.X() > 180);
  return new NFmiLatLonArea(theBottomLeft, theTopRight, NFmiPoint(0, 0), NFmiPoint(1, 1), pacific);
}

NFmiArea* CreateLegacyLatLonArea(const NFmiPoint& theBottomLeft,
                                 const NFmiPoint& theTopRight,
                                 bool fUsePacificView)
{
  return new NFmiLatLonArea(
      theBottomLeft, theTopRight, NFmiPoint(0, 0), NFmiPoint(1, 1), fUsePacificView);
}

NFmiArea* CreateLegacyRotatedLatLonArea(const NFmiPoint& theBottomLeft,
                                        const NFmiPoint& theTopRight,
                                        const NFmiPoint& theSouthPole)
{
  bool rotated_corners = true;
  if (theBottomLeft.Y() < theTopRight.Y())
  {
    return new NFmiRotatedLatLonArea(theBottomLeft,
                                     theTopRight,
                                     theSouthPole,
                                     NFmiPoint(0, 0),
                                     NFmiPoint(1, 1),
                                     rotated_corners);
  }

  NFmiPoint bl(theBottomLeft.X(), theTopRight.Y());
  NFmiPoint tr(theTopRight.X(), theBottomLeft.Y());
  return new NFmiRotatedLatLonArea(
      bl, tr, theSouthPole, NFmiPoint(0, 0), NFmiPoint(1, 1), rotated_corners);
}

NFmiArea* CreateLegacyStereographicArea(const NFmiPoint& theBottomLeft,
                                        const NFmiPoint& theTopRight,
                                        double theCentralLongitude,
                                        double theCentralLatitude,
                                        double theTrueLatitude)
{
  return new NFmiStereographicArea(theBottomLeft,
                                   theTopRight,
                                   theCentralLongitude,
                                   NFmiPoint(0, 0),
                                   NFmiPoint(1, 1),
                                   theCentralLatitude,
                                   theTrueLatitude);
}

NFmiArea* CreateLegacyEquiDistArea(const NFmiPoint& theBottomLeft,
                                   const NFmiPoint& theTopRight,
                                   double theCentralLongitude,
                                   double theCentralLatitude)
{
  return new NFmiEquidistArea(theBottomLeft,
                              theTopRight,
                              theCentralLongitude,
                              NFmiPoint(0, 0),
                              NFmiPoint(1, 1),
                              theCentralLatitude);
}

NFmiArea* CreateLegacyMercatorArea(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight)
{
  bool pacific = (theBottomLeft.X() < 180 && theTopRight.X() > 180);
  return new NFmiMercatorArea(
      theBottomLeft, theTopRight, NFmiPoint(0, 0), NFmiPoint(1, 1), pacific);
}

NFmiArea* CreateLegacyLambertEqualArea(const NFmiPoint& theBottomLeft,
                                       const NFmiPoint& theTopRight,
                                       double theCentralLongitude,
                                       double theCentralLatitude)
{
  return new NFmiLambertEqualArea(theBottomLeft,
                                  theTopRight,
                                  theCentralLongitude,
                                  NFmiPoint(0, 0),
                                  NFmiPoint(1, 1),
                                  theCentralLatitude);
}

NFmiArea* CreateLegacyLambertConformalConicArea(const NFmiPoint& theBottomLeft,
                                                const NFmiPoint& theTopRight,
                                                double theCentralLongitude,
                                                double theCentralLatitude,
                                                double theTrueLatitude1,
                                                double theTrueLatitude2)
{
  return new NFmiLambertConformalConicArea(theBottomLeft,
                                           theTopRight,
                                           theCentralLongitude,
                                           theCentralLatitude,
                                           theTrueLatitude1,
                                           theTrueLatitude2);
}

NFmiArea* CreateLegacyGnomonicArea(const NFmiPoint& theBottomLeft,
                                   const NFmiPoint& theTopRight,
                                   double theCentralLongitude,
                                   double theCentralLatitude)
{
  return new NFmiGnomonicArea(theBottomLeft,
                              theTopRight,
                              theCentralLongitude,
                              NFmiPoint(0, 0),
                              NFmiPoint(1, 1),
                              theCentralLatitude);
}

NFmiArea* CreateLegacyYKJArea(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight)
{
  return new NFmiYKJArea(theBottomLeft, theTopRight);
}

NFmiArea* CreateLegacyYKJArea(const NFmiPoint& theBottomLeft,
                              const NFmiPoint& theTopRight,
                              bool fMeters)
{
  return new NFmiYKJArea(theBottomLeft, theTopRight, fMeters);
}

}  // namespace NFmiAreaTools
