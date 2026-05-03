// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiSvgTools
 */
// ======================================================================
/*!
 * \namespace NFmiSvgTools
 *
 * \brief Sisältää NFmiSvgPath luokkaan liittyviä työkaluja
 *
 * Tämä namespace sisältää erilaisia työkaluja, joiden toteuttamiseen
 * riittää NFmiSvgPath luokan iteraattori interface, ja siten niitä
 * ei tarvitse toteuttaa NFmiSvgPath luokan metodeina.
 */
// ======================================================================

#include "NFmiSvgTools.h"
#include "NFmiArea.h"
#include "NFmiEquidistArea.h"
#include "NFmiGeoTools.h"
#include "NFmiPoint.h"
#include "NFmiSvgPath.h"
#include <macgyver/Exception.h>

namespace NFmiSvgTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Testaa onko annettu piste polun sisällä
 *
 * \param thePath Polku
 * \param thePoint Testattava piste
 * \return True, jos piste on polun sisällä
 */
// ----------------------------------------------------------------------

bool IsInside(const NFmiSvgPath& thePath, const NFmiPoint& thePoint)
{
  try
  {
    return thePath.IsInside(thePoint);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Laske pisteen etäisyys polusta
 *
 * \param thePath Polku
 * \param thePoint Piste
 * \return Etäisyys polusta
 */
// ----------------------------------------------------------------------

double Distance(const NFmiSvgPath& thePath, const NFmiPoint& thePoint)
{
  try
  {
    if (thePath.empty())
      return -1;

    double minDist = -1;

    auto firstPoint = thePath.begin();

    double lastX = 0;
    double lastY = 0;

    for (auto it = thePath.begin(); it != thePath.end(); ++it)
    {
      switch (it->itsType)
      {
        case NFmiSvgPath::kElementMoveto:
          firstPoint = it;
          lastX = it->itsX;
          lastY = it->itsY;
          break;
        case NFmiSvgPath::kElementClosePath:
        {
          double dist = NFmiGeoTools::DistanceFromLineSegment(
              thePoint.X(), thePoint.Y(), lastX, lastY, firstPoint->itsX, firstPoint->itsY);

          if (minDist < 0 || dist < minDist)
            minDist = dist;
          lastX = firstPoint->itsX;
          lastY = firstPoint->itsY;
          break;
        }
        case NFmiSvgPath::kElementLineto:
        {
          double dist = NFmiGeoTools::DistanceFromLineSegment(
              thePoint.X(), thePoint.Y(), lastX, lastY, it->itsX, it->itsY);

          if (minDist < 0 || dist < minDist)
            minDist = dist;
          lastX = it->itsX;
          lastY = it->itsY;
          break;
        }
        case NFmiSvgPath::kElementNotValid:
          return -1;
      }
    }
    return minDist;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Laske latlon pisteen etäisyys latlon polusta
 *
 * Laskennassa käytetään hyväksi Equidistant projektiota
 * niin, että tutkittava piste on projektion keskipiste.
 * Tällöin etäisyydet laskettuna suoraan projektion yksikössä
 * ovat valideja.
 *
 * Palauttaa -1, jos polku on tyhjä.
 *
 * \param thePath Polku (latlon)
 * \param thePoint Piste (latlon)
 * \return Etäisyys polusta kilometreinä
 */
// ----------------------------------------------------------------------

double GeoDistance(const NFmiSvgPath& thePath, const NFmiPoint& thePoint)
{
  try
  {
    if (thePath.empty())
      return -1;

    // Nurkilla ei ole väliä
    NFmiEquidistArea area(NFmiPoint(20, 50),
                          NFmiPoint(40, 70),
                          thePoint.X(),
                          NFmiPoint(0, 0),
                          NFmiPoint(1, 1),
                          thePoint.Y());

    double minDist = -1;

    NFmiPoint firstPoint =
        area.LatLonToWorldXY(NFmiPoint(thePath.front().itsX, thePath.front().itsY));
    NFmiPoint lastPoint = firstPoint;
    NFmiPoint center = area.LatLonToWorldXY(thePoint);

    for (const auto& it : thePath)
    {
      switch (it.itsType)
      {
        case NFmiSvgPath::kElementMoveto:
          lastPoint = area.LatLonToWorldXY(NFmiPoint(it.itsX, it.itsY));
          firstPoint = lastPoint;
          break;
        case NFmiSvgPath::kElementClosePath:
        {
          double dist = NFmiGeoTools::DistanceFromLineSegment(
              center.X(), center.Y(), lastPoint.X(), lastPoint.Y(), firstPoint.X(), firstPoint.Y());

          if (minDist < 0 || dist < minDist)
            minDist = dist;
          lastPoint = firstPoint;
          break;
        }
        case NFmiSvgPath::kElementLineto:
        {
          NFmiPoint p = area.LatLonToWorldXY(NFmiPoint(it.itsX, it.itsY));
          double dist = NFmiGeoTools::DistanceFromLineSegment(
              center.X(), center.Y(), lastPoint.X(), lastPoint.Y(), p.X(), p.Y());

          if (minDist < 0 || dist < minDist)
            minDist = dist;
          lastPoint = p;
          break;
        }
        case NFmiSvgPath::kElementNotValid:
          return -1;
      }
    }
    return minDist;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate path bounding box
 *
 * The bounding box is stored into the 4 variables
 * passed as references to doubles. If the path is empty,
 * all variables will be set to zero.
 *
 * \param thePath The path from which to extract the bounds
 * \param theXmin Reference to the x-minimum variable
 * \param theYmin Reference to the y-minimum variable
 * \param theXmax Reference to the x-maximum variable
 * \param theYmax Reference to the y-maximum variable
 */
// ----------------------------------------------------------------------

void BoundingBox(
    const NFmiSvgPath& thePath, double& theXmin, double& theYmin, double& theXmax, double& theYmax)
{
  try
  {
    if (thePath.empty())
    {
      theXmin = theYmin = theXmax = theYmax = 0;
      return;
    }

    bool firstmove = true;

    for (const auto& it : thePath)
    {
      switch (it.itsType)
      {
        case NFmiSvgPath::kElementMoveto:
        case NFmiSvgPath::kElementLineto:

          if (firstmove || it.itsX < theXmin)
            theXmin = it.itsX;
          if (firstmove || it.itsX > theXmax)
            theXmax = it.itsX;
          if (firstmove || it.itsY < theYmin)
            theYmin = it.itsY;
          if (firstmove || it.itsY > theYmax)
            theYmax = it.itsY;
          firstmove = false;
          break;
        case NFmiSvgPath::kElementNotValid:
        case NFmiSvgPath::kElementClosePath:
          break;
      }
    }
    return;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Project path from latlon to given world coordinates
 *
 * \param thePath The path to project
 * \param theArea The area to projecto to
 */
// ----------------------------------------------------------------------

void LatLonToWorldXY(NFmiSvgPath& thePath, const NFmiArea& theArea)
{
  try
  {
    NFmiPoint tmp;
    for (auto& it : thePath)
    {
      switch (it.itsType)
      {
        case NFmiSvgPath::kElementMoveto:
        case NFmiSvgPath::kElementLineto:
        {
          tmp = theArea.LatLonToWorldXY(NFmiPoint(it.itsX, it.itsY));
          it.itsX = tmp.X();
          it.itsY = tmp.Y();
          break;
        }
        case NFmiSvgPath::kElementNotValid:
        case NFmiSvgPath::kElementClosePath:
          break;
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PointToSvgPath(NFmiSvgPath& thePath, double x, double y)
{
  try
  {
    NFmiSvgPath::Element element1(NFmiSvgPath::kElementMoveto, x, y);
    NFmiSvgPath::Element element2(NFmiSvgPath::kElementClosePath, 0, 0);
    thePath.push_back(element1);
    thePath.push_back(element2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void BBoxToSvgPath(NFmiSvgPath& thePath, double x1, double y1, double x2, double y2)
{
  try
  {
    NFmiSvgPath::Element element1(NFmiSvgPath::kElementMoveto, x1, y1);
    NFmiSvgPath::Element element2(NFmiSvgPath::kElementLineto, x1, y2);
    NFmiSvgPath::Element element3(NFmiSvgPath::kElementLineto, x2, y2);
    NFmiSvgPath::Element element4(NFmiSvgPath::kElementLineto, x2, y1);
    NFmiSvgPath::Element element5(NFmiSvgPath::kElementClosePath, x1, y1);
    thePath.push_back(element1);
    thePath.push_back(element2);
    thePath.push_back(element3);
    thePath.push_back(element4);
    thePath.push_back(element5);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace NFmiSvgTools

// ======================================================================
