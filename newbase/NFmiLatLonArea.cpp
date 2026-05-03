// ======================================================================
/*!
 * \file NFmiLatLonArea.cpp
 * \brief Implementation of class NFmiLatLonArea
 */
// ======================================================================
/*!
 * \class NFmiLatLonArea
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiLatLonArea.h"
#include "NFmiAreaFactory.h"
#include <macgyver/Hash.h>
#include <fmt/format.h>
#include <macgyver/Exception.h>

#include <iostream>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLatLonArea::~NFmiLatLonArea() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLatLonArea::NFmiLatLonArea()
    : NFmiArea(),
      itsBottomLeftLatLon(),
      itsTopRightLatLon(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiLatLonArea::NFmiLatLonArea(const NFmiPoint &theBottomLeftLatLon,
                               const NFmiPoint &theTopRightLatLon,
                               const NFmiPoint &theTopLeftXY,
                               const NFmiPoint &theBottomRightXY,
                               bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
{
  try
  {
    Init();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theLatLonArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiLatLonArea::NFmiLatLonArea(const NFmiLatLonArea &theLatLonArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiLatLonArea::Clone() const
{
  try
  {
    return new NFmiLatLonArea(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLatLonArea::Init(bool fKeepWorldRect)
{
  try
  {
    if (!fKeepWorldRect)
      itsWorldRect =
          NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon),
                   LatLonToWorldXY(itsTopRightLatLon));  // 28.8.2001/Marko&Esa Lisätty laskuihin.

    itsXScaleFactor = (Right() - Left()) / (itsTopRightLatLon.X() - itsBottomLeftLatLon.X());
    itsYScaleFactor = (Top() - Bottom()) / (itsTopRightLatLon.Y() - itsBottomLeftLatLon.Y());

    NFmiArea::Init(fKeepWorldRect);

    const char *fmt = "+proj=eqc +R={} +wktext +no_defs +type=crs";
    itsProjStr = fmt::format(fmt::runtime(fmt), kRearth);
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiLatLonArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  try
  {
    double dlon = (theXYPoint.X() - Left()) / itsXScaleFactor;
    NFmiLongitude lon(itsBottomLeftLatLon.X() + dlon, PacificView());
    NFmiLatitude lat(itsBottomLeftLatLon.Y() + (theXYPoint.Y() - Bottom()) / itsYScaleFactor);
    return NFmiPoint(lon.Value(), lat.Value());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiLatLonArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  try
  {
    double usedLongitude = FixLongitude(theLatLonPoint.X());
    double X = Left() + (usedLongitude - itsBottomLeftLatLon.X()) * itsXScaleFactor;
    double Y = Top() + (theLatLonPoint.Y() - itsTopRightLatLon.Y()) *
                           itsYScaleFactor;  // Tässä on edelleen virhe
    return NFmiPoint(X, Y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiLatLonArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
{
  try
  {
    return LatLonToWorldXY(ToLatLon(theXYPoint));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiLatLonArea::WorldXYToXY(const NFmiPoint &theWorldXYPoint) const
{
  try
  {
    return ToXY(WorldXYToLatLon(theWorldXYPoint));
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

double NFmiLatLonArea::XScale() const
{
  try
  {
    return 1. / itsXScaleFactor;
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

double NFmiLatLonArea::YScale() const
{
  try
  {
    return 1. / itsYScaleFactor;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiLatLonArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                  const NFmiPoint &theTopRightLatLon,
                                  bool allowPacificFix) const
{
  try
  {
    if (allowPacificFix)
    {
      PacificPointFixerData fixedPointData =
          NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
      return new NFmiLatLonArea(fixedPointData.itsBottomLeftLatlon,
                                fixedPointData.itsTopRightLatlon,
                                TopLeft(),
                                BottomRight(),
                                fixedPointData.fIsPacific);
    }
    else
      return new NFmiLatLonArea(
          theBottomLeftLatLon, theTopRightLatLon, TopLeft(), BottomRight(), PacificView());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiLatLonArea::Write(std::ostream &file) const
{
  try
  {
    NFmiArea::Write(file);
    file << itsBottomLeftLatLon;
    file << itsTopRightLatLon;

    // Dummies to replace old removed variables
    file << "0 0\n0 0\n";

    file << itsXScaleFactor << " ";
    file << itsYScaleFactor << std::endl;
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiLatLonArea::Read(std::istream &file)
{
  try
  {
    double dummy;

    NFmiArea::Read(file);
    file >> itsBottomLeftLatLon;
    file >> itsTopRightLatLon;
    PacificView(NFmiArea::IsPacificView(itsBottomLeftLatLon, itsTopRightLatLon));
    file >> dummy >> dummy >> dummy >> dummy;  // old removed variables

    file >> itsXScaleFactor;
    file >> itsYScaleFactor;

    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));

    Init();

    // 28.8.2001/Marko&Esa Tätä kutsuttaessa kaikki dataosat päivittyvät,
    // jolloin voidaan esim. editoida tiedostoissa olevia areoiden projektio
    // parametreja siten (tätä ei voitu tehdä ennen koska skaalauskertoimet eivät päivittyneet)

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const std::string NFmiLatLonArea::AreaStr() const
{
  try
  {
    std::ostringstream out;
    out << "latlon:" << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ','
        << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
    return out.str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return Well Known Text representation of the GCS
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  UNIT["Metre",1.0]
 */
// ----------------------------------------------------------------------

const std::string NFmiLatLonArea::WKT() const
{
  try
  {
    const char *fmt =
        R"(PROJCS["unknown",GEOGCS["FMI_Sphere",DATUM["FMI_2007",SPHEROID["FMI_Sphere",{},0]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]],PROJECTION["Equirectangular"],PARAMETER["standard_parallel_1",0],PARAMETER["central_meridian",0],PARAMETER["false_easting",0],PARAMETER["false_northing",0],UNIT["metre",1, AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH]])";

    return fmt::format(fmt::runtime(fmt), kRearth);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison with an area derived from NFmiLatLonArea.
 *
 * \param theArea The other area being compared to
 * \return True, if the NFmiLatLonArea parts are equivalent
 * \todo Use static_cast instead of C-style cast
 */
// ----------------------------------------------------------------------

bool NFmiLatLonArea::operator==(const NFmiArea &theArea) const
{
  try
  {
    return *this == static_cast<const NFmiLatLonArea &>(theArea);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other area being compared to
 * \return True, if the areas are equivalent
 * \todo Investigate whether NFmiArea::operator== should also be called.
 */
// ----------------------------------------------------------------------

bool NFmiLatLonArea::operator==(const NFmiLatLonArea &theArea) const
{
  try
  {
    if ((itsBottomLeftLatLon == theArea.itsBottomLeftLatLon) &&
        (itsTopRightLatLon == theArea.itsTopRightLatLon) &&
        (itsXScaleFactor == theArea.itsXScaleFactor) &&
        (itsYScaleFactor == theArea.itsYScaleFactor) && (itsWorldRect == theArea.itsWorldRect))
      return true;

    return false;
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

std::size_t NFmiLatLonArea::HashValue() const
{
  try
  {
    std::size_t hash = NFmiArea::HashValue();
    Fmi::hash_combine(hash, itsBottomLeftLatLon.HashValue());
    Fmi::hash_combine(hash, itsTopRightLatLon.HashValue());
    Fmi::hash_combine(hash, Fmi::hash_value(itsXScaleFactor));
    Fmi::hash_combine(hash, Fmi::hash_value(itsYScaleFactor));
    Fmi::hash_combine(hash, itsWorldRect.HashValue());
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ======================================================================
