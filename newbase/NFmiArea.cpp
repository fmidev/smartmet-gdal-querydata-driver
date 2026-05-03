// ======================================================================
/*!
 * \file NFmiArea.cpp
 * \brief Implementation of class NFmiArea
 */
// ======================================================================
/*!
 * \class NFmiArea
 *
 * Undocumented
 */
// ======================================================================

#include "NFmiArea.h"
#include "NFmiAreaFactory.h"
#include "NFmiAreaTools.h"

// Needed until HashValue API is changed

#include "NFmiAzimuthalArea.h"
#include "NFmiEquidistArea.h"
#include "NFmiGdalArea.h"
#include "NFmiGnomonicArea.h"
#include "NFmiKKJArea.h"
#include "NFmiLambertEqualArea.h"
#include "NFmiLatLonArea.h"
#include "NFmiMercatorArea.h"
#include "NFmiOrthographicArea.h"
#include "NFmiPKJArea.h"
#include "NFmiRect.h"
#include "NFmiRotatedLatLonArea.h"
#include "NFmiStereographicArea.h"
#include "NFmiYKJArea.h"
#include <gis/CoordinateMatrix.h>
#include <gis/CoordinateTransformation.h>
#include <gis/ProjInfo.h>
#include <macgyver/Exception.h>
#include <macgyver/Hash.h>
#include <iostream>

// ----------------------------------------------------------------------
/*!
 * \brief Detect legacy projection from PROJ.x information
 */
// ----------------------------------------------------------------------

int DetectClassId(const Fmi::ProjInfo &proj)
{
  auto name = proj.getString("proj");
  if (!name)
    throw Fmi::Exception(BCP, "Projection name not set, should be impossible");

  // Accept WGS84 and sphere for legacy projections despite possible errors
  bool ok = false;
  auto opt_datum = proj.getString("datum");
  if (opt_datum)
    ok = (opt_datum == std::string("WGS84"));
  else
  {
    auto opt_ellps = proj.getString("ellps");
    if (opt_ellps)
      ok = (opt_ellps == std::string("sphere"));
    else
    {
      auto opt_R = proj.getDouble("R");  // any sphere will do
      auto opt_a = proj.getDouble("a");  // a must equal b or b must be missing
      auto opt_b = proj.getDouble("b");
      ok = (opt_R > 0.0 || (opt_a > 0.0 && !opt_b) || (opt_a > 0.0 && opt_a == opt_b));
    }
  }

  if (ok)
  {
    if (*name == "eqc")
      return kNFmiLatLonArea;
    if (*name == "merc")
    {
      auto lat_ts = proj.getDouble("lat_ts");
      auto lon_0 = proj.getDouble("lon_0");
      if (lat_ts && lat_ts != 0.0)
        return kNFmiGdalArea;
      if (lon_0 && lon_0 != 0.0)
        return kNFmiGdalArea;
      return kNFmiMercatorArea;
    }
    if (*name == "stere")
      return kNFmiStereographicArea;
    if (*name == "aeqd")
      return kNFmiEquiDistArea;
    if (*name == "laea")
      return kNFmiLambertEqualArea;
    if (*name == "lcc")
      return kNFmiLambertConformalConicArea;
    if (*name == "gnom")
      return kNFmiGnomonicArea;
    if (*name == "ob_tran" && proj.getString("o_proj") == std::string("eqc") &&
        proj.getDouble("o_lon_p") == 0.0)
      return kNFmiRotatedLatLonArea;
  }
  // We allow for two slightly different towgs84 strings, since legacy code uses more decimals than
  // PROJ.x prints
  else if (*name == "tmerc" && proj.getString("ellps") == std::string("intl") &&
           proj.getDouble("x_0") == 3500000.0 && proj.getDouble("lat_0") == 0.0 &&
           proj.getDouble("lon_0") == 27.0 &&
           (proj.getString("towgs84") ==
                std::string("-96.0617,-82.4278,-121.7535,4.80107,0.34543,-1.37646,1.4964") ||
            proj.getString("towgs84") ==
                std::string("-96.062,-82.428,-121.753,4.801,0.345,-1.376,1.496")))
    return kNFmiYKJArea;

  // Not a legacy projection, use PROJ.x
  return kNFmiGdalArea;
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a legacy projection from a spatial reference with dummy corners
 */
//----------------------------------------------------------------------

NFmiArea *Create(const Fmi::SpatialReference &theSR)
{
  const auto &proj = theSR.projInfo();
  auto id = DetectClassId(proj);

  NFmiPoint bl(-90, -180);  // these must be big enough since creating a new area with the
  NFmiPoint tr(90, 180);    // final WorldXY rectangle requires the bbox to be inside these limits
  switch (id)
  {
    case kNFmiLatLonArea:
      return NFmiAreaTools::CreateLegacyLatLonArea(bl, tr);
    case kNFmiMercatorArea:
      return NFmiAreaTools::CreateLegacyMercatorArea(bl, tr);
    case kNFmiStereographicArea:
    {
      auto clon = proj.getDouble("lon_0");
      auto clat = proj.getDouble("lat_0");
      auto tlat = proj.getDouble("lat_ts");
      return NFmiAreaTools::CreateLegacyStereographicArea(
          bl, tr, clon ? *clon : 0, clat ? *clat : 90, tlat ? *tlat : 60);
    }
    case kNFmiEquiDistArea:
    {
      auto clon = proj.getDouble("lon_0");
      auto clat = proj.getDouble("lat_0");
      return NFmiAreaTools::CreateLegacyEquiDistArea(bl, tr, clon ? *clon : 0, clat ? *clat : 90);
    }
    case kNFmiLambertEqualArea:
    {
      auto clon = proj.getDouble("lon_0");
      auto clat = proj.getDouble("lat_0");
      return NFmiAreaTools::CreateLegacyLambertEqualArea(
          bl, tr, clon ? *clon : 0, clat ? *clat : 9);
    }
    case kNFmiLambertConformalConicArea:
    {
      auto clon = proj.getDouble("lon_0");
      auto clat = proj.getDouble("lat_0");
      auto lat1 = proj.getDouble("lat_1");
      auto lat2 = proj.getDouble("lat_2");
      if (!lat2)
        lat2 = lat1;
      if (!lat1)
        lat1 = lat2;
      return NFmiAreaTools::CreateLegacyLambertConformalConicArea(
          bl, tr, clon ? *clon : 0, clat ? *clat : 0, lat1 ? *lat1 : 0, lat2 ? *lat2 : 0);
    }
    case kNFmiGnomonicArea:
    {
      auto clon = proj.getDouble("lon_0");
      auto clat = proj.getDouble("lat_0");
      return NFmiAreaTools::CreateLegacyGnomonicArea(bl, tr, clon ? *clon : 0, clat ? *clat : 90);
    }
    case kNFmiRotatedLatLonArea:
    {
      auto clon = proj.getDouble("lon_0");
      auto o_lon_p = proj.getDouble("o_lon_p");
      auto o_lat_p = proj.getDouble("o_lat_p");
      if (!o_lon_p)
        o_lon_p = 0;
      if (!o_lat_p)
        o_lat_p = 90;
      if (!clon)
        clon = 0;

      if (o_lon_p != 0.0)
        return new NFmiGdalArea("FMI", theSR, bl, tr);

      NFmiPoint spole(*clon, -*o_lat_p);
      return NFmiAreaTools::CreateLegacyRotatedLatLonArea(bl, tr, spole);
    }
    case kNFmiYKJArea:
      return NFmiAreaTools::CreateLegacyYKJArea(bl, tr);
    default:
      return new NFmiGdalArea("FMI", theSR, bl, tr);
  }
}

// ----------------------------------------------------------------------
/*!
 * \bug Turha argumentti konstruktorille
 */
// ----------------------------------------------------------------------

void NFmiArea::Init(bool /* fKeepWorldRect */)
{
  try
  {
    CheckForPacificView();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newArea Undocumented
 */
// ----------------------------------------------------------------------

void NFmiArea::SetXYArea(const NFmiRect &newArea)
{
  try
  {
    Place(newArea.TopLeft());
    Size(newArea.Size());
    Init();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static void FixPacificLongitude(NFmiPoint &lonLat)
{
  try
  {
    if (lonLat.X() < 0)
    {
      NFmiLongitude lon(lonLat.X(), true);
      lonLat.X(lon.Value());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

#if 0
static void FixAtlanticLongitude(NFmiPoint &lonLat)
{
    if(lonLat.X() > 180)
    {
        NFmiLongitude lon(lonLat.X(), false);
        lonLat.X(lon.Value());
    }
}
#endif

// ----------------------------------------------------------------------
/*!
 * \param theArea Undocumented
 */
// ----------------------------------------------------------------------

const NFmiRect NFmiArea::XYArea(const NFmiArea *theArea) const
{
  try
  {
    if (PacificView() && theArea->PacificView() == false)
    {
      NFmiPoint topLeftLatlon = theArea->ToLatLon(theArea->TopLeft());
      ::FixPacificLongitude(topLeftLatlon);
      NFmiPoint bottomRightLatlon = theArea->ToLatLon(theArea->BottomRight());
      ::FixPacificLongitude(bottomRightLatlon);

      NFmiPoint topLeft(ToXY(topLeftLatlon));
      NFmiPoint bottomRight(ToXY(bottomRightLatlon));
      NFmiRect rect(topLeft, bottomRight);
      return rect;
    }
    else if (PacificView() == false && theArea->PacificView())
    {
      std::unique_ptr<NFmiArea> pacificAreaFromThis(DoForcePacificFix());

      NFmiPoint topLeft(pacificAreaFromThis->ToXY(theArea->ToLatLon(theArea->TopLeft())));
      NFmiPoint bottomRight(pacificAreaFromThis->ToXY(theArea->ToLatLon(theArea->BottomRight())));
      NFmiRect rect(topLeft, bottomRight);
      return rect;
    }
    else
    {
      NFmiPoint topLeft(ToXY(theArea->ToLatLon(theArea->TopLeft())));
      NFmiPoint bottomRight(ToXY(theArea->ToLatLon(theArea->BottomRight())));
      NFmiRect rect(topLeft, bottomRight);
      return rect;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::ostream &NFmiArea::Write(std::ostream &file) const
{
  try
  {
    file << itsXYRectArea;
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::istream &NFmiArea::Read(std::istream &file)
{
  try
  {
    file >> itsXYRectArea;
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::WorldXYSize() const
{
  try
  {
    return WorldRect().Size();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::WorldXYPlace() const
{
  try
  {
    return WorldRect().Place();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::WorldXYWidth() const
{
  try
  {
    return WorldRect().Width();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::WorldXYHeight() const
{
  try
  {
    return WorldRect().Height();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::WorldXYAspectRatio() const
{
  try
  {
    return WorldXYWidth() / WorldXYHeight();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * Creates a new area from the current one by altering the corner points only.
 * Previously this method required the new area to be inside the original
 * one, but I failed to see the point in restricing the functionality - Mika
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 *
 * \todo Should return an std::shared_ptr
 * \todo Remove the unnecessary cast in the last return statement
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiArea::CreateNewArea(const NFmiPoint &theBottomLeftLatLon,
                                  const NFmiPoint &theTopRightLatLon) const
{
  try
  {
    return NewArea(theBottomLeftLatLon, theTopRightLatLon);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 * Creates a new sub-area inside the current "mother" area.
 * The new area is defined by the input local rectangle 'theRect'
 *
 * \note
 *   - the input rectangle defines a local XY coordinate area only, NOT the metric
 *     XY world rectangle
 *   - the sub-area MUST fit completely inside the current local area
 *   - the sub-area gets all the projection-specific properties but its dimensions
 *     from its "mother" area. For example, the orientation stays the same.
 *
 * \param theRect Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiArea::CreateNewArea(const NFmiRect &theRect) const
{
  try
  {
    NFmiPoint newBottomLeftXY = theRect.BottomLeft();
    NFmiPoint newTopRightXY = theRect.TopRight();

    NFmiPoint newBottomLeftLatLon = ToLatLon(newBottomLeftXY);
    NFmiPoint newTopRightLatLon = ToLatLon(newTopRightXY);

    NFmiArea *newArea = NewArea(newBottomLeftLatLon, newTopRightLatLon);
    return newArea;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Creates a new area with its aspect ratio defined by the input parameters
 *
 * \note
 *    - the new area MAY OR MAY NOT completely reside inside the current local area
 *    - the new area gets all the projection-specific properties but its dimensions
 *      from its "mother" area. For example, the orientation stays the same.
 *
 * \param theNewAspectRatioXperY Undocumented
 * \param theFixedPoint Undocumented
 * \param fShrinkArea Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiArea::CreateNewArea(double theNewAspectRatioXperY,
                                  FmiDirection theFixedPoint,
                                  bool fShrinkArea)
{
  try
  {
    double originalAspectRatio = WorldXYAspectRatio();

    bool keepWidth;

    if (fShrinkArea)
    {
      // The new area will be "shrunk" to completely fit inside the current area
      if ((theNewAspectRatioXperY < originalAspectRatio))
        keepWidth = false;  // Maintain height, compute width
      else
        keepWidth = true;  // Maintain width, compute height
    }
    else
    {
      // The new area will in part grow out of the current area
      if ((theNewAspectRatioXperY < originalAspectRatio))
        keepWidth = true;  // Maintain width, compute height
      else
        keepWidth = false;  // Maintain height, compute width
    }

    // Create copy of the original "world rectangle" to be freely modified
    NFmiRect newWorldRect = WorldRect();

    // REDIMENSIONING OF THE WORLD RECTANGLE
    //----------------------------------------

    if (!newWorldRect.AdjustAspectRatio(theNewAspectRatioXperY, keepWidth, theFixedPoint))
      return nullptr;

    // Create a new area with the new aspect ratio
    NFmiArea *newArea = NewArea(WorldXYToLatLon(newWorldRect.TopLeft()),
                                WorldXYToLatLon(newWorldRect.BottomRight()));

    // Return the re-dimensioned copy of the original area
    return newArea;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Creates a new sub-area inside the current "mother" area.
 * The new area is defined by the input metric rectangle 'theWorldRect'
 *
 * \note
 *   - the input rectangle defines a metric XY world rectangle
 *   - the sub-area MUST fit completely inside the current local area
 *   - the sub-area gets all the projection-specific properties but its dimensions
 *   - from its "mother" area. For example, the orientation stays the same.
 *
 * \param theWorldRect
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiArea::CreateNewAreaByWorldRect(const NFmiRect &theWorldRect)
{
  try
  {
    return CreateNewAreaByWorldRect(theWorldRect, true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiArea *NFmiArea::CreateNewAreaByWorldRect(const NFmiRect &theWorldRect, bool fMustBeInside)
{
  try
  {
    NFmiPoint newBottomLeftXY = theWorldRect.BottomLeft();
    NFmiPoint newTopRightXY = theWorldRect.TopRight();

    NFmiPoint newBottomLeftLatLon = WorldXYToLatLon(newBottomLeftXY);
    NFmiPoint newTopRightLatLon = WorldXYToLatLon(newTopRightXY);

    if (fMustBeInside)
      if (!IsInside(newBottomLeftLatLon) || !IsInside(newTopRightLatLon))
        return nullptr;

    auto *newArea = static_cast<NFmiArea *>(NewArea(newBottomLeftLatLon, newTopRightLatLon));

    if (fMustBeInside)
      if (!IsInside(*newArea))
        return nullptr;

    return newArea;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Fmi::CoordinateMatrix NFmiArea::CoordinateMatrix(std::size_t nx, std::size_t ny, bool wrap) const
{
  try
  {
#ifdef NEW_NFMIAREA
    auto x1 = impl->itsWorldRect.Left();
    auto x2 = impl->itsWorldRect.Right();
    const auto y1 = impl->itsWorldRect.Top();
    const auto y2 = impl->itsWorldRect.Bottom();

    if (impl->itsFlopped)
      std::swap(x1, x2);

#else
    auto x1 = WorldRect().Left();
    auto x2 = WorldRect().Right();
    const auto y1 = WorldRect().Top();
    const auto y2 = WorldRect().Bottom();
#endif

    if (!wrap)
      return Fmi::CoordinateMatrix(nx, ny, x1, y1, x2, y2);

    // Add one more column to the right since wrapping is requested. We assume an earlier phase
    // has already checked the data is geographic and global apart from one column.

    auto dx = (x2 - x1) / (nx - 1);

    // If the new coordinate is not an integer, say 180 or 360, reduce the extrapolated value
    // a little bit to avoid wrapping back to the left edge when reprojecting the coordinates.
    // Otherwise for example for the global GFS data in equidistant cylindrical coordinates
    // the X coordinate 20037508.34278925 is reprojected to 1.0177774980683254e-13 instead of 360
    // with crs = +init=epsg:4326 +lon_wrap=180

    if (x2 + dx != std::floor(x2 + dx))
      dx = 0.99999 * dx;

    return Fmi::CoordinateMatrix(nx + 1, ny, x1, y1, x2 + dx, y2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Measured from the input point, returns the azimuth angle between
 * the true geodetic north direction and the "up" Y-axis of the map
 * area rectangle. Azimuth angle runs 0..360 degrees clockwise, with
 * north zero degrees.
 *
 * \param theLatLonPoint Undocumented
 * \param theLatitudeEpsilon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiAngle NFmiArea::TrueNorthAzimuth(const NFmiPoint &theLatLonPoint,
                                           double theLatitudeEpsilon) const
{
  try
  {
    using namespace std;

    NFmiPoint xyWorldPoint = LatLonToWorldXY(theLatLonPoint);
    NFmiPoint latLonIncr =
        NFmiPoint(0., theLatitudeEpsilon);  // Arbitrary small latitude increment in degrees

    // Move up toward geo-north along the meridian of the input point
    NFmiPoint xyDistanceAlongMeridian = LatLonToWorldXY(theLatLonPoint + latLonIncr) - xyWorldPoint;

    // Get the angle between 'xyDistanceAlongMeridian.X()' and map "up" direction Y-axis
    if (xyDistanceAlongMeridian.Y() == 0.)
      return xyDistanceAlongMeridian.X() > 0.
                 ? NFmiAngle(90.)
                 : NFmiAngle(270.);  // Azimuth is exactly east 90 degrees or west 270 degrees,
                                     // respectively

    return NFmiAngle(FmiDeg(atan2(xyDistanceAlongMeridian.X(), xyDistanceAlongMeridian.Y())));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return center latlon coordinate
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::CenterLatLon() const
{
  try
  {
    NFmiPoint bl = BottomLeft();
    NFmiPoint tr = TopRight();

    NFmiPoint center(0.5 * (bl.X() + tr.X()), 0.5 * (bl.Y() + tr.Y()));

    return ToLatLon(center);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiArea::IsPacificLongitude(double theLongitude)
{
  try
  {
    if (theLongitude > 180 && theLongitude <= 360)
      return true;
    else
      return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiArea::CheckForPacificView()
{
  try
  {
    fPacificView = NFmiArea::IsPacificView(BottomLeftLatLon(), TopRightLatLon());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 1. Tarkistaa kattaako annetut pisteet Atlantic vai Pacific alueen
// 2. Jos Pacific, korjaa longitudet pacific:eiksi
// 3. Palauta originaali/korjatut pisteet pair:issa
PacificPointFixerData NFmiArea::PacificPointFixer(const NFmiPoint &theBottomLeftLatlon,
                                                  const NFmiPoint &theTopRightLatlon)
{
  try
  {
    bool usePacificView = NFmiArea::IsPacificView(theBottomLeftLatlon, theTopRightLatlon);
    if (usePacificView)
    {
      NFmiPoint bottomLeftLatLon = theBottomLeftLatlon;
      NFmiPoint topRightLatLon = theTopRightLatlon;
      NFmiAreaFactory::DoPossiblePacificFix(bottomLeftLatLon, topRightLatLon, usePacificView);
      return PacificPointFixerData(bottomLeftLatLon, topRightLatLon, usePacificView);
    }
    else
      return PacificPointFixerData(theBottomLeftLatlon, theTopRightLatlon, usePacificView);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiArea::IsPacificView(const NFmiPoint &bottomleftLatlon, const NFmiPoint &toprightLatlon)
{
  try
  {
    // Obvious case
    if (bottomleftLatlon.X() >= 0 && toprightLatlon.X() < 0)
      return true;
    // 0...360 coordinate system is used
    if (IsPacificLongitude(bottomleftLatlon.X()) || IsPacificLongitude(toprightLatlon.X()))
      return true;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiArea::FixLongitude(double theLon) const
{
  try
  {
    if (!fPacificView)
    {
      if (theLon > 180.00000001)  // Tämä ei voi olla tasan 180, koska SmartMet maailman rajaviivat
                                  // shapessa (maps\\shapes\\ne_10m_admin_0_countries) ja niiden
                                  // piirto imagine-kirjastolla menee jossain kohdissa sekaisin
        // reunoilla, koska siellä on käytetty vähän yli 180-pituuspiirin
        // meneviä arvoja Tyynenmeren 180 asteen pituuspiirin reunoilla
        return theLon - 360;
      else
        return theLon;
    }
    else if (theLon < 0)
      return theLon + 360;
    else
      return theLon;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiArea *NFmiArea::DoPossiblePacificFix() const
{
  try
  {
    // On olemassa pari erikoistapausta, mitkä halutaan eri areoissa korjata, että alueet toimisivat
    // paremmin newbase:ssa.
    if (fPacificView)
    {
      bool usedPacificViewState = fPacificView;
      NFmiPoint bottomleft = BottomLeftLatLon();
      NFmiPoint topright = TopRightLatLon();
      bool createNewArea =
          NFmiAreaFactory::DoPossiblePacificFix(bottomleft, topright, usedPacificViewState);

      if (createNewArea)
      {
        NFmiArea *newArea = NewArea(bottomleft, topright);
        if (newArea)
        {
          newArea->PacificView(usedPacificViewState);
          return newArea;
        }
      }
    }
    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiArea *NFmiArea::DoForcePacificFix() const
{
  try
  {
    // Joskus on pakko muuttaa atlantic-area pacific tyyppiseksi vaikka väkisin
    if (!fPacificView)
    {
      NFmiPoint bottomleftLatlon = BottomLeftLatLon();
      ::FixPacificLongitude(bottomleftLatlon);
      NFmiPoint toprightLatlon = TopRightLatLon();
      ::FixPacificLongitude(toprightLatlon);

      NFmiArea *newArea = NewArea(bottomleftLatlon, toprightLatlon, false);
      if (newArea)
      {
        newArea->PacificView(true);
        return newArea;
      }
    }
    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for the base NFmiArea components
 */
// ----------------------------------------------------------------------

std::size_t NFmiArea::HashValue() const
{
  try
  {
    std::size_t hash = itsXYRectArea.HashValue();
    Fmi::hash_merge(hash, fPacificView);
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Temporary fix until HashValue is made virtual (breaks ABI, delayed for now)
 */
// ----------------------------------------------------------------------

std::size_t NFmiArea::HashValueKludge() const
{
  try
  {
#ifdef UNIX
#ifndef DISABLED_GDAL
    if (const auto *a = dynamic_cast<const NFmiGdalArea *>(this))
      return a->HashValue();
#endif
#endif
    if (const auto *a = dynamic_cast<const NFmiGnomonicArea *>(this))
      return a->HashValue();

    if (const auto *a = dynamic_cast<const NFmiLambertEqualArea *>(this))
      return a->HashValue();

    if (const auto *a = dynamic_cast<const NFmiMercatorArea *>(this))
      return a->HashValue();

    // azimuthal is the base class
    if (const auto *a = dynamic_cast<const NFmiEquidistArea *>(this))
      return a->HashValue();
    if (const auto *a = dynamic_cast<const NFmiOrthographicArea *>(this))
      return a->HashValue();
    if (const auto *a = dynamic_cast<const NFmiStereographicArea *>(this))
      return a->HashValue();
    if (const auto *a = dynamic_cast<const NFmiAzimuthalArea *>(this))
      return a->HashValue();

    // kkj is the base class
    if (const auto *a = dynamic_cast<const NFmiYKJArea *>(this))
      return a->HashValue();
    if (const auto *a = dynamic_cast<const NFmiPKJArea *>(this))
      return a->HashValue();
    if (const auto *a = dynamic_cast<const NFmiKKJArea *>(this))
      return a->HashValue();

    // latlon is the base class
    if (const auto *a = dynamic_cast<const NFmiRotatedLatLonArea *>(this))
      return a->HashValue();
    if (const auto *a = dynamic_cast<const NFmiLatLonArea *>(this))
      return a->HashValue();

    return HashValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const Fmi::SpatialReference &NFmiArea::SpatialReference() const
{
  try
  {
    if (itsSpatialReference)
      return *itsSpatialReference;

    throw Fmi::Exception(BCP,
                         std::string("Spatial reference for ") + ClassName() +
                             " not available, PROJ.4 = '" + itsProjStr + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string NFmiArea::ProjStr() const
{
  try
  {
    return itsProjStr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiArea *NFmiArea::CreateFromBBox(const Fmi::SpatialReference &theSR,
                                   const NFmiPoint &theBottomLeftWorldXY,
                                   const NFmiPoint &theTopRightWorldXY)
{
  std::unique_ptr<NFmiArea> area(Create(theSR));
  NFmiRect rect(theBottomLeftWorldXY, theTopRightWorldXY);
  return area->CreateNewAreaByWorldRect(rect, false);
}

NFmiArea *NFmiArea::CreateFromCorners(const Fmi::SpatialReference &theSR,
                                      const Fmi::SpatialReference &theBBoxSR,
                                      const NFmiPoint &theBottomLeftLatLon,
                                      const NFmiPoint &theTopRightLatLon)
{
  Fmi::CoordinateTransformation trans(theBBoxSR, theSR);
  double x1 = theBottomLeftLatLon.X();
  double y1 = theBottomLeftLatLon.Y();
  double x2 = theTopRightLatLon.X();
  double y2 = theTopRightLatLon.Y();
  trans.transform(x1, y1);
  trans.transform(x2, y2);
  return CreateFromBBox(theSR, NFmiPoint(x1, y1), NFmiPoint(x2, y2));
}

NFmiArea *NFmiArea::CreateFromReverseCorners(const Fmi::SpatialReference &theSR,
                                             const Fmi::SpatialReference &theBBoxSR,
                                             const NFmiPoint &theTopLeftLatLon,
                                             const NFmiPoint &theBottomRightLatLon)
{
  Fmi::CoordinateTransformation trans(theBBoxSR, theSR);
  double x1 = theTopLeftLatLon.X();
  double y1 = theTopLeftLatLon.Y();
  double x2 = theBottomRightLatLon.X();
  double y2 = theBottomRightLatLon.Y();
  trans.transform(x1, y1);
  trans.transform(x2, y2);
  return CreateFromBBox(theSR, NFmiPoint(x1, y2), NFmiPoint(x2, y1));
}

NFmiArea *NFmiArea::CreateFromCornerAndSize(const Fmi::SpatialReference &theSR,
                                            const Fmi::SpatialReference &theCornerSR,
                                            const NFmiPoint &theBottomLeftLatLon,
                                            double theWidth,
                                            double theHeight)
{
  Fmi::CoordinateTransformation trans(theCornerSR, theSR);
  double x1 = theBottomLeftLatLon.X();
  double y1 = theBottomLeftLatLon.Y();
  trans.transform(x1, y1);
  double x2 = x1 + theWidth;
  double y2 = y1 + theHeight;
  return CreateFromBBox(theSR, NFmiPoint(x1, y1), NFmiPoint(x2, y2));
}

NFmiArea *NFmiArea::CreateFromCenter(const Fmi::SpatialReference &theSR,
                                     const Fmi::SpatialReference &theCenterSR,
                                     const NFmiPoint &theCenterLatLon,
                                     double theWidth,
                                     double theHeight)
{
  Fmi::CoordinateTransformation trans(theCenterSR, theSR);
  double x = theCenterLatLon.X();
  double y = theCenterLatLon.Y();
  trans.transform(x, y);
  return CreateFromBBox(theSR,
                        NFmiPoint(x - theWidth / 2, y - theHeight / 2),
                        NFmiPoint(x + theWidth / 2, y + theHeight / 2));
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiArea::~NFmiArea() {}
// ----------------------------------------------------------------------
/*!
 * Default constructor
 */
// ----------------------------------------------------------------------

NFmiArea::NFmiArea() : itsXYRectArea(), fPacificView(false) {}

// ----------------------------------------------------------------------
/*!
 * Constructor based on corner points
 *
 * \param theTopLeftCorner The top left corner coordinates
 * \param theBottomRightCorner The bottom right corner coordinates
 */
// ----------------------------------------------------------------------

NFmiArea::NFmiArea(const NFmiPoint &theTopLeftCorner,
                   const NFmiPoint &theBottomRightCorner,
                   bool usePacificView)
    : itsXYRectArea(theTopLeftCorner, theBottomRightCorner), fPacificView(usePacificView)
{
}

// ----------------------------------------------------------------------
/*!
 * Constuctor based on edge coordinates
 *
 * \param theLeft The left edge X-coordinate
 * \param theTop The top edge Y-coordinate
 * \param theRight The right edge X-coordinate
 * \param theBottom The bottom edge Y-coordinate
 */
// ----------------------------------------------------------------------

NFmiArea::NFmiArea(
    double theLeft, double theTop, double theRight, double theBottom, bool usePacificView)
    : itsXYRectArea(theLeft, theTop, theRight, theBottom), fPacificView(usePacificView)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::TopLeft() const
{
  return itsXYRectArea.TopLeft();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::BottomRight() const
{
  return itsXYRectArea.BottomRight();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::TopRight() const
{
  return itsXYRectArea.TopRight();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::BottomLeft() const
{
  return itsXYRectArea.BottomLeft();
}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theArea The object to copy
 */
// ----------------------------------------------------------------------

NFmiArea::NFmiArea(const NFmiArea &theArea)
    : itsSpatialReference(theArea.itsSpatialReference),
      itsProjStr(theArea.itsProjStr),
      itsXYRectArea(theArea.TopLeft(), theArea.BottomRight()),
      fPacificView(theArea.fPacificView)
{
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theArea The object to copy
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiArea &NFmiArea::operator=(const NFmiArea &theArea)
{
  if (this != &theArea)
  {
    itsSpatialReference = theArea.itsSpatialReference;
    itsProjStr = theArea.itsProjStr;
    itsXYRectArea = theArea.itsXYRectArea;
    fPacificView = theArea.fPacificView;
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiArea::IsInside(const NFmiPoint &theLatLonPoint) const
{
  NFmiPoint xyPoint = ToXY(theLatLonPoint);
  if (xyPoint == NFmiPoint::gMissingLatlon)
  {
    return false;
  }
  return itsXYRectArea.IsInside(xyPoint);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::TopLeftLatLon() const
{
  return ToLatLon(TopLeft());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::TopRightLatLon() const
{
  return ToLatLon(TopRight());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::BottomLeftLatLon() const
{
  return ToLatLon(BottomLeft());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::BottomRightLatLon() const
{
  return ToLatLon(BottomRight());
}
// ----------------------------------------------------------------------
/*!
 * \param theArea Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiArea::IsInside(const NFmiArea &theArea) const
{
  return IsInside(theArea.TopLeftLatLon()) && IsInside(theArea.TopRightLatLon()) &&
         IsInside(theArea.BottomLeftLatLon()) && IsInside(theArea.BottomRightLatLon());
}

// ----------------------------------------------------------------------
/*!
 * \param newPlace Undocumented
 */
// ----------------------------------------------------------------------

void NFmiArea::Place(const NFmiPoint &newPlace)
{
  itsXYRectArea.Place(newPlace);
}
// ----------------------------------------------------------------------
/*!
 * \param newSize Undocumented
 */
// ----------------------------------------------------------------------

void NFmiArea::Size(const NFmiPoint &newSize)
{
  itsXYRectArea.Size(newSize);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Top() const
{
  return itsXYRectArea.Top();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Bottom() const
{
  return itsXYRectArea.Bottom();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Left() const
{
  return itsXYRectArea.Left();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Right() const
{
  return itsXYRectArea.Right();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Height() const
{
  return itsXYRectArea.Height();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Width() const
{
  return itsXYRectArea.Width();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiRect &NFmiArea::XYArea() const
{
  return itsXYRectArea;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiArea::Clone() const
{
  return 0;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiArea::ClassId() const
{
  return kNFmiArea;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const char *NFmiArea::ClassName() const
{
  return "NFmiArea";
}
// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The object to compare against
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiArea::operator==(const NFmiArea &theArea) const
{
  return itsXYRectArea == theArea.itsXYRectArea;
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The object to compare against
 * \return True if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiArea::operator!=(const NFmiArea &theArea) const
{
  return itsXYRectArea != theArea.itsXYRectArea;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

int NFmiArea::Sign(double theValue) const
{
  return theValue < 0 ? -1 : 1;
}

// ----------------------------------------------------------------------
/*!
 * For compatibility with WGS84 branch where this returns WKT1_SIMPLE format
 */
// ----------------------------------------------------------------------

std::string NFmiArea::SimpleWKT() const
{
  return WKT();
}

// ----------------------------------------------------------------------
/*!
 * \brief Dummy for ABI compability with the WGS84 branch
 */
// ----------------------------------------------------------------------

void NFmiArea::SetGridSize(std::size_t theWidth, std::size_t theHeight) {}
