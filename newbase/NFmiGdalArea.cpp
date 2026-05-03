// ======================================================================
/*!
 * \file NFmiGdalArea.cpp
 * \brief Implementation of class NFmiGdalArea
 */
// ======================================================================

#ifndef DISABLED_GDAL

#include "NFmiGdalArea.h"
#include "NFmiString.h"
#include "NFmiStringTools.h"
#include <macgyver/Hash.h>
#include <boost/math/constants/constants.hpp>
#include <gis/CoordinateTransformation.h>
#include <gis/OGR.h>
#include <gis/SpatialReference.h>
#include <macgyver/Exception.h>
#include <cmath>
#include <iomanip>
#include <ogr_spatialref.h>

using namespace std;

// See also NFmiLatLonArea::WKT()
std::string fmiwkt =
    R"(GEOGCS["FMI_Sphere",DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]])";

// ----------------------------------------------------------------------
/*!
 * \brief Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiGdalArea::~NFmiGdalArea() = default;
// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea() : NFmiArea(), itsDatum("WGS84") {}

// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const NFmiGdalArea &theArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 */
// ----------------------------------------------------------------------

NFmiGdalArea &NFmiGdalArea::operator=(const NFmiGdalArea &theArea) = default;

// ----------------------------------------------------------------------
/*!
 * \brief Construct from user input
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const std::string &theDatum,
                           const std::string &theDescription,
                           const NFmiPoint &theBottomLeftLatLon,
                           const NFmiPoint &theTopRightLatLon,
                           const NFmiPoint &theTopLeftXY,
                           const NFmiPoint &theBottomRightXY,
                           bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsDatum(theDatum),
      itsDescription(theDescription),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsWorldRect()
{
  try
  {
    init();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from user input
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const std::string &theDatum,
                           const OGRSpatialReference &theCRS,
                           const NFmiPoint &theBottomLeftLatLon,
                           const NFmiPoint &theTopRightLatLon,
                           const NFmiPoint &theTopLeftXY,
                           const NFmiPoint &theBottomRightXY,
                           bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsDatum(theDatum),
      itsDescription(),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsWorldRect()
{
  try
  {
    itsProjStr = Fmi::OGR::exportToProj(theCRS);
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);

    // Guess a good value for itsDescription

    const char *auth = theCRS.GetAuthorityName(nullptr);
    if (auth != nullptr)
    {
      itsDescription = std::string(auth);
      const char *code = theCRS.GetAuthorityCode(nullptr);
      if (code != nullptr)
        itsDescription += ":" + std::string(code);
    }
    else
    {
      itsDescription = Fmi::OGR::exportToWkt(theCRS);
    }

    init();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from bounding box
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const std::string &theDatum,
                           const OGRSpatialReference &theCRS,
                           double theXmin,
                           double theYmin,
                           double theXmax,
                           double theYmax,
                           const NFmiPoint &theTopLeftXY,
                           const NFmiPoint &theBottomRightXY,
                           bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsDatum(theDatum),
      itsDescription(),
      itsBottomLeftLatLon(),
      itsTopRightLatLon(),
      itsWorldRect(NFmiPoint(theXmin, theYmin), NFmiPoint(theXmax, theYmax))
{
  try
  {
    itsProjStr = Fmi::OGR::exportToProj(theCRS);
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);

    // Guess a good value for itsDescription

    const char *auth = theCRS.GetAuthorityName(nullptr);
    if (auth != nullptr)
    {
      itsDescription = std::string(auth);
      const char *code = theCRS.GetAuthorityCode(nullptr);
      if (code != nullptr)
        itsDescription += ":" + std::string(code);
    }
    else
    {
      itsDescription = Fmi::OGR::exportToWkt(theCRS);
    }

    // The needed spatial references

    OGRErr err;
    std::shared_ptr<OGRSpatialReference> datum(new OGRSpatialReference);
    if (itsDatum == "FMI")
      err = datum->SetFromUserInput(fmiwkt.c_str());
    else
      err = datum->SetFromUserInput(itsDatum.c_str());

    if (err != OGRERR_NONE)
      throw Fmi::Exception(BCP, "Failed to set datum: '" + itsDatum + "'");

    // The needed coordinate transformations

    itsWorldXYToLatLonTransformation =
        std::make_shared<Fmi::CoordinateTransformation>(*itsSpatialReference, *datum);
    itsLatLonToWorldXYTransformation =
        std::make_shared<Fmi::CoordinateTransformation>(*datum, *itsSpatialReference);

    // Bottom left and top right coordinates - needed only for geographic projections (Width()
    // calculations)
    // The same data could be extracted from the WorldXYRect too though - and these variables
    // would not be needed.

    double x1 = theXmin;
    double y1 = theYmin;
    double x2 = theXmax;
    double y2 = theYmax;
    itsWorldXYToLatLonTransformation->transform(x1, y1);
    itsWorldXYToLatLonTransformation->transform(x2, y2);
    itsBottomLeftLatLon = NFmiPoint(x1, y1);
    itsTopRightLatLon = NFmiPoint(x2, y2);

    // Initialize itsWKT
    itsWKT = Fmi::OGR::exportToWkt(*itsSpatialReference);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return class ID
 */
// ----------------------------------------------------------------------

unsigned long NFmiGdalArea::ClassId() const
{
  try
  {
    return kNFmiGdalArea;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return class name
 */
// ----------------------------------------------------------------------

const char *NFmiGdalArea::ClassName() const
{
  try
  {
    return "kNFmiGdalArea";
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiGdalArea::Clone() const
{
  try
  {
    return new NFmiGdalArea(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Area descriptor
 */
// ----------------------------------------------------------------------

const std::string NFmiGdalArea::AreaStr() const
{
  try
  {
    std::ostringstream out;
    out << itsDatum << ':' << itsDescription << "|" << BottomLeftLatLon().X() << ","
        << BottomLeftLatLon().Y() << "," << TopRightLatLon().X() << "," << TopRightLatLon().Y();
    return out.str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Datum
 */
// ----------------------------------------------------------------------

const std::string &NFmiGdalArea::Datum() const
{
  try
  {
    return itsDatum;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Return the WKT description
 */
// ----------------------------------------------------------------------

const std::string NFmiGdalArea::WKT() const
{
  try
  {
    return itsWKT;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator==(const NFmiGdalArea &theArea) const
{
  try
  {
    return (itsBottomLeftLatLon == theArea.itsBottomLeftLatLon &&
            itsTopRightLatLon == theArea.itsTopRightLatLon &&
            itsWorldRect == theArea.itsWorldRect && itsDescription == theArea.itsDescription &&
            itsDatum == theArea.itsDatum);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator!=(const NFmiGdalArea &theArea) const
{
  try
  {
    return !(*this == theArea);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator==(const NFmiArea &theArea) const
{
  try
  {
    return *this == static_cast<const NFmiGdalArea &>(theArea);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator!=(const NFmiArea &theArea) const
{
  try
  {
    return !(*this == theArea);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Write the projection definition to a file
 */
// ----------------------------------------------------------------------

std::ostream &NFmiGdalArea::Write(std::ostream &file) const
{
  try
  {
    NFmiString tmp1 = itsDatum;
    NFmiString tmp2 = itsDescription;

    NFmiArea::Write(file);
    file << itsBottomLeftLatLon << itsTopRightLatLon << tmp1 << tmp2;

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Read new projection definition from the input stream
 */
// ----------------------------------------------------------------------

std::istream &NFmiGdalArea::Read(std::istream &file)
{
  try
  {
    NFmiString tmp1;
    NFmiString tmp2;

    NFmiArea::Read(file);
    file >> itsBottomLeftLatLon >> itsTopRightLatLon >> tmp1 >> tmp2;

    itsDatum = tmp1.CharPtr();
    itsDescription = tmp2.CharPtr();

    init();

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief XY coordinate to LatLon
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  try
  {
    double xscale = Width() / itsWorldRect.Width();
    double yscale = Height() / itsWorldRect.Height();

    double worldx = itsWorldRect.Left() + (theXYPoint.X() - Left()) / xscale;
    double worldy = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / yscale;

    return WorldXYToLatLon(NFmiPoint(worldx, worldy));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief LatLon to XY-coordinate
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    NFmiPoint latlon(FixLongitude(theLatLonPoint.X()), theLatLonPoint.Y());
    NFmiPoint worldxy = LatLonToWorldXY(latlon);

    double xscale = Width() / itsWorldRect.Width();
    double yscale = Height() / itsWorldRect.Height();

    double x = Left() + xscale * (worldxy.X() - itsWorldRect.Left());
    double y = Top() + yscale * (itsWorldRect.Bottom() - worldxy.Y());

    return NFmiPoint(x, y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief XY-coordinate to World coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
{
  try
  {
    double xscale = Width() / itsWorldRect.Width();
    double yscale = Height() / itsWorldRect.Height();

    double worldx = itsWorldRect.Left() + (theXYPoint.X() - Left()) / xscale;
    double worldy = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / yscale;

    return NFmiPoint(worldx, worldy);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theWorldXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::WorldXYToXY(const NFmiPoint &theWorldXYPoint) const
{
  try
  {
    double xscale = Width() / itsWorldRect.Width();
    double yscale = Height() / itsWorldRect.Height();

    double x = xscale * (theWorldXYPoint.X() - itsWorldRect.Left()) + Left();
    double y = Top() - yscale * (theWorldXYPoint.Y() - itsWorldRect.Bottom());
    return NFmiPoint(x, y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief World coordinates to LatLon
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::WorldXYToLatLon(const NFmiPoint &theXYPoint) const
{
  try
  {
    if (!itsWorldXYToLatLonTransformation)
      throw Fmi::Exception(BCP, "Trying to use an uninitialized GDAL area");

    double x = theXYPoint.X();
    double y = theXYPoint.Y();
    if (!itsWorldXYToLatLonTransformation->transform(x, y))
      return NFmiPoint(kFloatMissing, kFloatMissing);

    return NFmiPoint(x, y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief LatLon to world coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    if (!itsLatLonToWorldXYTransformation)
      throw Fmi::Exception(BCP, "Trying to use an uninitialized GDAL area");

    double x = theLatLonPoint.X();
    double y = theLatLonPoint.Y();
    if (!itsLatLonToWorldXYTransformation->transform(x, y))
      return NFmiPoint(kFloatMissing, kFloatMissing);

    return NFmiPoint(x, y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the world rectangle
 */
// ----------------------------------------------------------------------

const NFmiRect NFmiGdalArea::WorldRect() const
{
  try
  {
    return itsWorldRect;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiGdalArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                const NFmiPoint &theTopRightLatLon,
                                bool allowPacificFix) const
{
  try
  {
    if (allowPacificFix)
    {
      PacificPointFixerData fix =
          NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
      return new NFmiGdalArea(itsDatum,
                              itsDescription,
                              fix.itsBottomLeftLatlon,
                              fix.itsTopRightLatlon,
                              TopLeft(),
                              BottomRight(),
                              fix.fIsPacific);
    }

    return new NFmiGdalArea(itsDatum,
                            itsDescription,
                            theBottomLeftLatLon,
                            theTopRightLatLon,
                            TopLeft(),
                            BottomRight(),
                            PacificView());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize the projection transformation objects
 */
// ----------------------------------------------------------------------

void NFmiGdalArea::init()
{
  try
  {
    // The needed spatial references

    if (!itsSpatialReference)
    {
      itsProjStr = itsDescription;
      itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);
    }

    OGRErr err;
    std::shared_ptr<OGRSpatialReference> datum(new OGRSpatialReference);
    if (itsDatum == "FMI")
      err = datum->SetFromUserInput(fmiwkt.c_str());
    else
      err = datum->SetFromUserInput(itsDatum.c_str());

    if (err != OGRERR_NONE)
      throw Fmi::Exception(BCP, "Failed to set datum: '" + itsDatum + "'");

    // The needed coordinate transformations

    itsWorldXYToLatLonTransformation =
        std::make_shared<Fmi::CoordinateTransformation>(*itsSpatialReference, *datum);
    itsLatLonToWorldXYTransformation =
        std::make_shared<Fmi::CoordinateTransformation>(*datum, *itsSpatialReference);

    // The needed world XY rectangle

    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));

    // Initialize itsWKT
    itsWKT = Fmi::OGR::exportToWkt(*itsSpatialReference);
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

double NFmiGdalArea::WorldXYWidth() const
{
  try
  {
    if (!itsSpatialReference->isGeographic())
      return WorldRect().Width();

    double pi = boost::math::constants::pi<double>();
    double circumference = 2 * pi * 6371220;
    double dlon = itsTopRightLatLon.X() - itsBottomLeftLatLon.X();
    if (dlon < 0)
      dlon += 360;

    double clat = 0.5 * (itsBottomLeftLatLon.Y() + itsTopRightLatLon.Y());
    return dlon / 360 * circumference * cos(clat * pi / 180);
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

double NFmiGdalArea::WorldXYHeight() const
{
  try
  {
    if (!itsSpatialReference->isGeographic())
      return WorldRect().Height();

    double pi = boost::math::constants::pi<double>();
    double circumference = 2 * pi * 6371220;
    double dlat = itsTopRightLatLon.Y() - itsBottomLeftLatLon.Y();
    return dlat / 360.0 * circumference;  // angle -> meters
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Hash value
 */
// ----------------------------------------------------------------------

std::size_t NFmiGdalArea::HashValue() const
{
  try
  {
    std::size_t hash = NFmiArea::HashValue();

    // some of these may be redundant:
    Fmi::hash_combine(hash, Fmi::hash_value(itsDatum));
    Fmi::hash_combine(hash, Fmi::hash_value(itsDescription));
    Fmi::hash_combine(hash, Fmi::hash_value(itsWKT));

    Fmi::hash_combine(hash, itsBottomLeftLatLon.HashValue());
    Fmi::hash_combine(hash, itsTopRightLatLon.HashValue());
    Fmi::hash_combine(hash, itsWorldRect.HashValue());

    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

#endif  // DISABLED_GDAL

// ======================================================================
