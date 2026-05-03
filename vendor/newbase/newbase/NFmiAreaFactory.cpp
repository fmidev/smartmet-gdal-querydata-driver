// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiAreaFactory
 */
// ======================================================================
/*!
 * \namespace NFmiAreaFactory
 *
 * \brief Contains tools for creation NFmiArea objects
 *
 * The NFmiAreaFactory contains functions for creating NFmiArea
 * objects.
 *
 * The Create function takes as input a string describing the
 * desired projection, parses the description and returns
 * the desired area.
 *
 * The generic form for the string description is
 * \code
 * projection:area:grid
 * \endcode
 *
 * The possible values for the \c projection part are
 * \code
 * latlon
 * ykj
 * pkj
 * mercator
 * webmercator
 * rotlatlon,polelatitude=-90,polelongitude=0
 * invrotlatlon,polelatitude=-90,polelongitude=0
 * orthographic,azimuth=0
 * stereographic,centrallongitude=0,centrallatitude=90,truelatitude=60
 * lambertequal,centrallongitude=10,centrallatitude=0,truelatitude=52
 * lcc,centrallongitude,centrallatitude,truelatitude1,truelatitude2=truelatitude1,radius=6371220
 * gnomonic,centrallongitude=0,centrallatitude=90,truelatitude=60
 * equidist,centrallongitude=0,centrallatitude=90
 * \endcode
 * The default values for optional parameters are as indicated above.
 * The \c invrotlatlon projection is a convenience id for creating
 * the \c rotlatlon projection based on rotated coordinates.
 *
 * The possible values for the \c area part are
 * \code
 * x1,y1,x2,y2
 * cx,cy,scale
 * cx,cy,scale/aspect
 * \endcode
 * where the first form requires the bottom left and top right
 * longitude and latitude. The latter form requires the center
 * longitude and latitude plus a scale factor.
 *
 * An optional aspect ratio may be used to compress or elongate the
 * Y-axis, which is useful for example for Meteosat images. The aspect
 * ratio is used only when the image center coordinate is defined, otherwise
 * the ratio is meaningless.
 *
 * If the projection is \c invrotlatlon, the latitude and longitude values
 * are measured in \c rotlatlon coordinates.
 *
 * The possible values for the \c grid part are
 * \code
 * x1,y1,x2,y2
 * y1,y2
 * xsize,ysize<units>,  where units may be 'm' or 'km'
 * \endcode
 * Comma may be replaced by "x".
 *
 * Also, the grid part may be omitted entirely, which is assumed
 * to imply values 0,0,1,1. The units version may not be used
 * if the center coordinate & scale are used to define the area.
 *
 * If also the area part is omitted, it is given the default
 * value of "6,51.3,49,70.2" which covers the entire Skandinavia
 * and coincides with the current editor area. This is mainly useful
 * for programs which do not care about XY-coordinates, but only
 * of world coordinates.
 *
 * For example, the following defines the stereographic projection
 * used by the forecasters:
 * \code
 * stereographic,20,90,60:6,51.3,49,70.2
 * \endcode
 * The projection is thus polar stereographic with central longitude 20,
 * true latitude 60, with bottom left corner at (5,51.3) and top right
 * corner at 49,70.2.
 *
 *
 * The CreateProj function takes a string containing a Proj4 projection
 * definition and the corner points of the desired area, and returns
 * the corresponding NFmiArea - object. If the projection described in
 * the Proj4-string is not supported, a runtime_error is thrown.
 *
 * The CreateProj function supports the following FMI-projections:
 * \code
 * latlon
 * ykj
 * mercator
 * orthographic
 * stereographic
 * lambertequal
 * gnomonic
 * equidist
 * \endcode
 *
 */
// ======================================================================

#include "NFmiAreaFactory.h"
#include "NFmiEquidistArea.h"
#include "NFmiGdalArea.h"
#include "NFmiGnomonicArea.h"
#include "NFmiLambertConformalConicArea.h"
#include "NFmiLambertEqualArea.h"
#include "NFmiLatLonArea.h"
#include "NFmiMercatorArea.h"
#include "NFmiOrthographicArea.h"
#include "NFmiPKJArea.h"
#include "NFmiRotatedLatLonArea.h"
#include "NFmiStereographicArea.h"
#include "NFmiStringTools.h"
#include "NFmiWebMercatorArea.h"
#include "NFmiYKJArea.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <macgyver/Exception.h>
#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace
{
double check_longitude(double theLongitude, bool usePacificView)
{
  try
  {
    std::string rangeStr;
    if (usePacificView)
    {
      if (theLongitude >= 0 && theLongitude <= 360)
        return theLongitude;
      rangeStr = "[0,360]";
    }
    else
    {
      if (theLongitude >= -180 && theLongitude <= 180)
        return theLongitude;
      rangeStr = "[-180,180]";
    }
    string msg = "Longitude value";
    msg += NFmiStringTools::Convert(theLongitude);
    msg += " is not in the required range " + rangeStr;
    throw Fmi::Exception(BCP, msg);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double check_latitude(double theLatitude)
{
  try
  {
    if (theLatitude >= -90 && theLatitude <= 90)
      return theLatitude;
    string msg = "Latitude value";
    msg += NFmiStringTools::Convert(theLatitude);
    msg += " is not in the required range [-90,90]";
    throw Fmi::Exception(BCP, msg);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double degrees_from_projparam(const string &inParam)
{
  try
  {
    // Trims the parameter containing proj degrees and returns double.
    size_t length = inParam.size();
    try
    {
      // North
      if (boost::iends_with(inParam, "n"))
      {
        return boost::lexical_cast<double>(inParam.substr(0, length - 1));
      }
      // South
      else if (boost::iends_with(inParam, "s"))
      {
        return -boost::lexical_cast<double>(inParam.substr(0, length - 1));
      }
      // East
      else if (boost::iends_with(inParam, "e"))
      {
        return boost::lexical_cast<double>(inParam.substr(0, length - 1));
      }
      // West
      else if (boost::iends_with(inParam, "w"))
      {
        return -boost::lexical_cast<double>(inParam.substr(0, length - 1));
      }
      else
      {
        return boost::lexical_cast<double>(inParam);
      }
    }
    catch (boost::bad_lexical_cast &e)
    {
      string errStr;
      errStr += "Bad cast to double in parameter: ";
      errStr += e.what();
      throw Fmi::Exception(BCP, errStr);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

namespace NFmiAreaFactory
{
bool DoPossiblePacificFix(NFmiPoint &bottomLeftLatlon, NFmiPoint &topRightLatlon, bool &pacificView)
{
  try
  {
    if (pacificView)
    {
      // Fix top right longitude to Pacific view if it seems possible
      if (topRightLatlon.X() >= -180 && topRightLatlon.X() < 0)
      {
        topRightLatlon.X(topRightLatlon.X() + 360);
        return true;
      }
      else if (bottomLeftLatlon.X() < topRightLatlon.X() && bottomLeftLatlon.X() >= 180 &&
               topRightLatlon.X() > 180)
      {  // molemmat nurkkapisteet ovat pacific-alueella (180 - 360), tehdään niistä molemmista
         // atlantisia koordinaatteja
        // koska SmartMet toimii paremmin tälläisissa tilanteissa, jos kyseinen muutos tehdään. En
        // tiedä tarkemmin miksi, koska
        // debug-versio SmartMetista näytti toimivan täysin oikein joka tapauksessa oli data miten
        // hyvänsä.
        bottomLeftLatlon.X(bottomLeftLatlon.X() - 360);
        topRightLatlon.X(topRightLatlon.X() - 360);
        pacificView = false;
        return true;
      }
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Create the desired projection
 *
 * Throws if there is an error in the projection description.
 *
 * \param theProjection String description of the projection
 * \return The created projection
 */
// ----------------------------------------------------------------------

std::shared_ptr<NFmiArea> Create(const std::string &theProjection)
{
  try
  {
    std::shared_ptr<NFmiArea> area;

    string projection = theProjection;
    NFmiStringTools::TrimAll(projection);  // siivotaan mahdolliset etu ja taka white spacet pois
                                           // ettei sotke parserointia

    // NFmiGdalArea's projection string starts with FMI{:|} or WGS84{:|}, and area definition at the
    // end is delimited with pipe, thus resulting pipe to be used as the separator below. Replace
    // possible pipe delimiter at the start with colon, because parsing must see the projection part
    // upto the area as one comma separated string.
    //
    // FMI:PROJCS["unnamed",GEOGCS[...|0.237,51.849,49.662,71.161
    // WGS84:PROJCS["unnamed",GEOGCS[...|0.237,51.849,49.662,71.161

    bool gdalArea = false;

#ifndef DISABLED_GDAL
    if ((projection.substr(0, 4) == "FMI:") || (projection.substr(0, 6) == "WGS84:") ||
        (projection.substr(0, 4) == "FMI|") || (projection.substr(0, 6) == "WGS84|"))
    {
      if (projection.substr(0, 3) == "FMI")
        projection = "FMI:" + projection.substr(4);
      else
        projection = "WGS84:" + projection.substr(6);

      gdalArea = true;
    }
#endif

    const char *separator = ":";
    if (projection.find('|') != std::string::npos)
      separator = "|";

    vector<string> parts = NFmiStringTools::Split<vector<string> >(projection, separator);

    try
    {
      if (parts.size() == 1)
        parts.emplace_back("6,51.3,49,70.2");

      if (parts.size() < 1 || parts.size() > 3)
        throw Fmi::Exception(BCP, "must have 1-3 parts separated by ':' or '|'");

      // extracts the parts separated by ','
      list<string> pparts = NFmiStringTools::Split<list<string> >(parts[0]);

      // Extract possible units
      bool units = false;
      double unitfactor = 1.0;
      if (parts.size() == 3)
      {
        // Allow both NxM and N,M
        std::replace(parts[2].begin(), parts[2].end(), 'x', ',');

        if (boost::iends_with(parts[2], "km"))
        {
          units = true;
          unitfactor = 1000;
          parts[2] = parts[2].substr(0, parts[2].size() - 2);
        }
        else if (boost::iends_with(parts[2], "m"))
        {
          units = true;
          parts[2] = parts[2].substr(0, parts[2].size() - 1);
        }
      }

      const vector<string> aparts = NFmiStringTools::Split<vector<string> >(parts[1], "/");
      if (aparts.empty())
        throw Fmi::Exception(BCP, "area specification must have 3-4 numbers");
      const vector<double> avec = NFmiStringTools::Split<vector<double> >(aparts[0]);
      double aspect = (aparts.size() == 1 ? 1 : NFmiStringTools::Convert<double>(aparts[1]));

      deque<double> gvec =
          NFmiStringTools::Split<deque<double> >(parts.size() == 2 ? "0,0,1,1" : parts[2]);

      // intermediate validity checks
      if (pparts.size() < 1)
        throw Fmi::Exception(BCP, "projection part missing");
      if (avec.size() < 3 || avec.size() > 4)
        throw Fmi::Exception(BCP, "area specification must have 3-4 numbers");
      if (gvec.size() != 2 && gvec.size() != 4)
        throw Fmi::Exception(BCP, "grid specification must have 2 or 4 numbers");
      if (gvec.size() != 2 && units)
        throw Fmi::Exception(BCP,
                             "grid specification must have 2 numbers when length units are used");

      string proj = gdalArea ? parts[0] : pparts.front();
      pparts.pop_front();

      vector<double> pvec;

      if (!gdalArea)
        for (list<string>::const_iterator it = pparts.begin(); it != pparts.end(); ++it)
          pvec.push_back(NFmiStringTools::Convert<double>(*it));

      // fixate gvec to size 4

      if (gvec.size() == 2)
      {
        gvec.push_front(0);
        gvec.push_front(0);
      }

      // extract the numeric parts

      const bool centered = (avec.size() == 3);

      NFmiPoint bottomleft = NFmiPoint(avec[0], avec[1]);
      NFmiPoint topright = (centered ? bottomleft : NFmiPoint(avec[2], avec[3]));

      bool usePacificView = NFmiArea::IsPacificView(bottomleft, topright);
      DoPossiblePacificFix(bottomleft, topright, usePacificView);

      check_longitude(bottomleft.X(), usePacificView);
      check_latitude(bottomleft.Y());
      check_longitude(topright.X(), usePacificView);
      check_latitude(topright.Y());

      // More validity checks

      if (units && centered)
        throw Fmi::Exception(
            BCP, "Cannot use a centered projection speficiation with grid size of specific length");

      const NFmiPoint corner1(gvec[0], gvec[1]);
      const NFmiPoint corner2(gvec[2], gvec[3]);

      if (proj == "latlon")
      {
        if (pvec.size() != 0)
          throw Fmi::Exception(BCP, "latlon area does not require any parameters");
        area.reset(new NFmiLatLonArea(bottomleft, topright, corner1, corner2, usePacificView));
      }
      else if (proj == "ykj")
      {
        if (pvec.size() != 0)
          throw Fmi::Exception(BCP, "ykj area does not require any parameters");
        area.reset(new NFmiYKJArea(bottomleft, topright, corner1, corner2, usePacificView));
      }
      else if (proj == "pkj")
      {
        if (pvec.size() != 0)
          throw Fmi::Exception(BCP, "pkj area does not require any parameters");
        area.reset(new NFmiPKJArea(bottomleft, topright, corner1, corner2, usePacificView));
      }
      else if (proj == "mercator")
      {
        if (pvec.size() > 0)
          throw Fmi::Exception(BCP, "mercator area requires no parameters");

        area.reset(new NFmiMercatorArea(bottomleft, topright, corner1, corner2, usePacificView));
      }
      else if (proj == "webmercator")
      {
        if (pvec.size() > 0)
          throw Fmi::Exception(BCP, "webmercator area requires no parameters");

        area.reset(new NFmiWebMercatorArea(bottomleft, topright, corner1, corner2, usePacificView));
      }
      else if (proj == "rotlatlon")
      {
        if (pvec.size() > 2)
          throw Fmi::Exception(BCP, "rotlatlon area requires max 2 parameters");
        const double pole_lon = check_longitude(pvec.size() >= 2 ? pvec[1] : 0, usePacificView);
        const double pole_lat = check_latitude(pvec.size() >= 1 ? pvec[0] : -90);
        area.reset(new NFmiRotatedLatLonArea(
            bottomleft, topright, NFmiPoint(pole_lon, pole_lat), corner1, corner2, usePacificView));
      }
      else if (proj == "invrotlatlon")
      {
        if (pvec.size() > 2)
          throw Fmi::Exception(BCP, "invrotlatlon area requires max 2 parameters");
        const double pole_lon = check_longitude(pvec.size() >= 2 ? pvec[1] : 0, usePacificView);
        const double pole_lat = check_latitude(pvec.size() >= 1 ? pvec[0] : -90);

        // Invert the coordinates. Note that only the pole really matters in
        // the following constructor

        NFmiRotatedLatLonArea rot(bottomleft, topright, NFmiPoint(pole_lon, pole_lat));
        NFmiPoint bl = rot.ToRegLatLon(bottomleft);
        NFmiPoint tr = rot.ToRegLatLon(topright);

        area.reset(new NFmiRotatedLatLonArea(
            bl, tr, NFmiPoint(pole_lon, pole_lat), corner1, corner2, usePacificView));
      }
      else if (proj == "orthographic")
      {
        if (pvec.size() > 1)
          throw Fmi::Exception(BCP, "orthographi area requires max 1 parameter");
        const double azimuth = check_longitude(pvec.size() >= 1 ? pvec[0] : 0, usePacificView);
        area.reset(new NFmiOrthographicArea(
            bottomleft, topright, azimuth, corner1, corner2, usePacificView));
      }
      else if (proj == "stereographic")
      {
        if (pvec.size() > 3)
          throw Fmi::Exception(BCP, "stereographic area requires max 3 parameters");
        const double clon = check_longitude(pvec.size() >= 1 ? pvec[0] : 0, usePacificView);
        const double clat = check_latitude(pvec.size() >= 2 ? pvec[1] : 90);
        const double tlat = check_latitude(pvec.size() >= 3 ? pvec[2] : 60);
        area.reset(new NFmiStereographicArea(
            bottomleft, topright, clon, corner1, corner2, clat, tlat, usePacificView));
      }
      else if (proj == "lambertequal")
      {
        if (pvec.size() > 3)
          throw Fmi::Exception(BCP, "lambertequal area requires max 3 parameters");
        const double clon = check_longitude(pvec.size() >= 1 ? pvec[0] : 0, usePacificView);
        const double clat = check_latitude(pvec.size() >= 2 ? pvec[1] : 90);
        const double tlat = check_latitude(pvec.size() >= 3 ? pvec[2] : 60);
        area.reset(new NFmiLambertEqualArea(
            bottomleft, topright, clon, corner1, corner2, clat, tlat, usePacificView));
      }
      else if (proj == "lcc")
      {
        // *
        // lcc,centrallongitude,centrallatitude,truelatitude1,truelatitude2=truelatitude1,radius=6371220
        if (pvec.size() < 3 || pvec.size() > 5)
          throw Fmi::Exception(BCP, "lcc area requires max 3-5 parameters");
        const double clon = check_longitude(pvec[0], usePacificView);
        const double clat = check_latitude(pvec[1]);
        const double tlat1 = check_latitude(pvec[2]);
        const double tlat2 = check_latitude(pvec.size() >= 4 ? pvec[3] : tlat1);
        const double rad = (pvec.size() >= 5 ? pvec[4] : kRearth);
        area.reset(new NFmiLambertConformalConicArea(
            bottomleft, topright, clon, clat, tlat1, tlat2, rad, usePacificView));
      }
      else if (proj == "gnomonic")
      {
        if (pvec.size() > 3)
          throw Fmi::Exception(BCP, "gnomonic area requires max 3 parameters");
        const double clon = check_longitude(pvec.size() >= 1 ? pvec[0] : 0, usePacificView);
        const double clat = check_latitude(pvec.size() >= 2 ? pvec[1] : 90);
        const double tlat = check_latitude(pvec.size() >= 3 ? pvec[2] : 60);
        area.reset(new NFmiGnomonicArea(
            bottomleft, topright, clon, corner1, corner2, clat, tlat, usePacificView));
      }
      else if (proj == "equidist")
      {
        if (pvec.size() > 2)
          throw Fmi::Exception(BCP, "equidist area requires max 2 parameters");
        const double clon = check_longitude(pvec.size() >= 1 ? pvec[0] : 0, usePacificView);
        const double clat = check_latitude(pvec.size() >= 2 ? pvec[1] : 90);
        area.reset(new NFmiEquidistArea(
            bottomleft, topright, clon, corner1, corner2, clat, usePacificView));
      }
#ifdef UNIX
#ifndef DISABLED_GDAL
      else
      {
        // Allow FMI: or WGS84: prefix to identify datum, default is WGS84
        std::string datum = "WGS84";
        if (proj.substr(0, 4) == "FMI:")
        {
          datum = "FMI";
          proj = proj.substr(4, std::string::npos);
        }
        else if (proj.substr(0, 6) == "WGS84:")
        {
          proj = proj.substr(6, std::string::npos);
        }

        area.reset(
            new NFmiGdalArea(datum, proj, bottomleft, topright, corner1, corner2, usePacificView));
      }
#else
      else
      {
        throw Fmi::Exception(BCP, "Unknown projection (GDAL not linked in): " + theProjection);
      }
#endif  // DISABLED_GDAL
#else
      else
      {
        throw Fmi::Exception(BCP, "Unknown projection: " + theProjection);
      }
#endif  // UNIX

      // recalculate corners if center coordinate was given
      // instead of corners

      const double width = gvec[2] - gvec[0];
      const double height = gvec[3] - gvec[1];

      if (units && (width <= 0 || height <= 0))
        throw Fmi::Exception(BCP, "Cannot use negative lengths when specifying the grid size");

      if (centered)
      {
        if (width <= 0 || height <= 0)
          throw Fmi::Exception(BCP,
                               "Width and height must be positive when center coordinate is given");

        const double scale = avec[2];
        const NFmiPoint c = area->LatLonToWorldXY(bottomleft);  // bottomleft = center here
        const NFmiPoint bl(c.X() - scale * 1000 * width, c.Y() - scale * 1000 * aspect * height);
        const NFmiPoint tr(c.X() + scale * 1000 * width, c.Y() + scale * 1000 * aspect * height);
        NFmiPoint BL = area->WorldXYToLatLon(bl);
        NFmiPoint TR = area->WorldXYToLatLon(tr);

        if (proj == "invrotlatlon")
        {
          BL = (static_cast<NFmiRotatedLatLonArea *>(area.get()))->ToRegLatLon(BL);
          TR = (static_cast<NFmiRotatedLatLonArea *>(area.get()))->ToRegLatLon(TR);
        }

        area.reset(area->NewArea(BL, TR));
      }
      else if (width < 0 && height > 0)
      {
        // calculate width to preserve aspect ratio
        double scale = area->WorldXYAspectRatio();
        double w = round(scale * height);
        NFmiRect rect(gvec[0], gvec[1], gvec[0] + w, gvec[3]);
        area->SetXYArea(rect);
      }
      else if (height < 0 && width > 0)
      {
        // calculate height to preserve aspect ratio
        double scale = area->WorldXYAspectRatio();
        double h = round(width / scale);
        NFmiRect rect(gvec[0], gvec[1], gvec[2], gvec[1] + h);
        area->SetXYArea(rect);
      }
      else if (width < 0 && height < 0)
        throw Fmi::Exception(BCP, "Width and height cannot both be negative");
      else if (units)
      {
        double w = area->WorldXYWidth();
        double h = area->WorldXYHeight();
        auto xsize = static_cast<int>(round(w / (unitfactor * width)));
        auto ysize = static_cast<int>(round(h / (unitfactor * height)));
        NFmiRect rect(0, 0, xsize, ysize);
        area->SetXYArea(rect);
      }
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP,
                                  "Projection specification '" + theProjection + "' is invalid");
    }

    return area;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Create the desired projection from Proj4 string
 *
 * Throws if there is an error in the projection description or when the projection is not supported
 *
 * \param projString String description of the projection
 * \return The created projection
 *
 * The following projections are not supported by the Proj4 factory: NFmiPKJArea,
 * NFmiRotatedLatLonArea, NFmiKKJArea
 */
// ----------------------------------------------------------------------

return_type CreateProj(const std::string &projString,
                       const NFmiPoint &bottomLeftLatLon,
                       const NFmiPoint &topRightLatLon,
                       const NFmiPoint &topLeftXY,
                       const NFmiPoint &bottomRightXY)
{
  try
  {
    // Map to hold the parameters
    map<string, string> projParams;

    // Map to hold parameters that are actually used in parsing, this can be used to print
    // parameters that are ignored
    set<string> usedParams;

    // Tokenize the input string
    vector<string> tokens, splitToken;
    boost::split(tokens, projString, boost::is_any_of(" \t\n"), boost::token_compress_on);

    // The result
    return_type result;

    // Build token map, while validating the proj-string
    for (auto &token : tokens)
    {
      if (token.empty())
        continue;
      // Split token into key->value pairs
      boost::split(splitToken, token, boost::is_any_of("="));

      if (!boost::starts_with(splitToken[0], "+"))
      {
        // Proj parameter keys start with a "+"

        throw Fmi::Exception(BCP, "Proj4 parameters must start with a '+'");
      }

      if (splitToken.size() < 2)
      {
        // Only key but no value, use empty string as value
        projParams.insert(make_pair(splitToken[0].erase(0, 1), string("")));
      }
      else
        projParams.insert(make_pair(splitToken[0].erase(0, 1), splitToken[1]));
    }

    //	print_parameter_map(projParams);
    // Now parameter map is ready for processing
    map<string, string>::iterator map_it;
    map_it = projParams.find("proj");
    if (map_it == projParams.end())
    {
      throw Fmi::Exception(BCP, "No projection specified");
    }

    usedParams.insert(map_it->first);
    string projId = map_it->second;

    if (projId == "stere")
    {
      // Stereographic projection, find mandatory parameters

      // Central longitude
      map_it = projParams.find("lon_0");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(
            BCP, "Central meridian 'lon_0' must be specified for stereographic projection");
      }

      double centralLon = degrees_from_projparam(map_it->second);

      usedParams.insert(map_it->first);

      // True scale latitude
      map_it = projParams.find("lat_ts");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(
            BCP, "True scale latitude 'lat_ts' must be specified for stereographic projection");
      }
      double truescaleLat = degrees_from_projparam(map_it->second);

      usedParams.insert(map_it->first);
      // Center latitude
      map_it = projParams.find("lat_0");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(
            BCP, "Center latitude 'lat_0' must be specified for stereographic projection");
      }
      double centerLat = degrees_from_projparam(map_it->second);

      usedParams.insert(map_it->first);

      result = return_type(new NFmiStereographicArea(bottomLeftLatLon,
                                                     topRightLatLon,
                                                     centralLon,
                                                     topLeftXY,
                                                     bottomRightXY,
                                                     centerLat,
                                                     truescaleLat));
    }

    else if (projId == "aeqd")
    {
      // Azimuthal equidistant projection

      map_it = projParams.find("lat_0");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(
            BCP, "Center latitude 'lat_0' must be specified for azimuthal equidistant projection");
      }
      double centerLat = degrees_from_projparam(map_it->second);
      usedParams.insert(map_it->first);

      map_it = projParams.find("lon_0");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(
            BCP, "Center latitude 'lat_0' must be specified for azimuthal equidistant projection");
      }
      double centerLon = degrees_from_projparam(map_it->second);
      usedParams.insert(map_it->first);

      result = return_type(new NFmiEquidistArea(
          bottomLeftLatLon, topRightLatLon, centerLon, topLeftXY, bottomRightXY, centerLat));
    }

    else if ((projId == "latlon") || (projId == "lonlat"))
    {
      // Latlon area

      result = return_type(
          new NFmiLatLonArea(bottomLeftLatLon, topRightLatLon, topLeftXY, bottomRightXY));
    }

    else if (projId == "ortho")
    {
      // Orthographic projection
      result = return_type(
          new NFmiOrthographicArea(bottomLeftLatLon, topRightLatLon, 0, topLeftXY, bottomRightXY));
    }

    else if (projId == "gnom")
    {
      // Gnomonic projection
      map_it = projParams.find("lon_0");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(BCP,
                             "Central meridian 'lon_0' must be specified for gnomonic projection");
      }
      double centralLon = degrees_from_projparam(map_it->second);
      usedParams.insert(map_it->first);

      // True scale latitude
      map_it = projParams.find("lat_ts");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(
            BCP, "True scale latitude 'lat_ts' must be specified for gnomonic projection");
      }
      double truescaleLat = degrees_from_projparam(map_it->second);
      usedParams.insert(map_it->first);

      // Center latitude
      map_it = projParams.find("lat_0");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(BCP,
                             "Center latitude 'lat_0' must be specified for gnomonic projection");
      }
      double centerLat = degrees_from_projparam(map_it->second);
      usedParams.insert(map_it->first);

      result = return_type(new NFmiGnomonicArea(bottomLeftLatLon,
                                                topRightLatLon,
                                                centralLon,
                                                topLeftXY,
                                                bottomRightXY,
                                                centerLat,
                                                truescaleLat));
    }

    else if (projId == "merc")
    {
      // Mercator projection
      result = return_type(
          new NFmiMercatorArea(bottomLeftLatLon, topRightLatLon, topLeftXY, bottomRightXY));
    }

    else if (projId == "webmerc")
    {
      // WebMercator, newbase supports only +datum=WGS84
      map_it = projParams.find("datum");
      if (map_it == projParams.end() || map_it->second == "WGS84")
        result = return_type(
            new NFmiWebMercatorArea(bottomLeftLatLon, topRightLatLon, topLeftXY, bottomRightXY));
      else
        throw Fmi::Exception(BCP, "Datum " + map_it->second + " not supported for WebMercator");
    }

    else if (projId == "tmerc")
    {
      // The only supported Transverse Mercator projection is YKJ-projection (EPSG:2393), must check
      // that other proj-params are
      // according to http://spatialreference.org/ref/epsg/2393/proj4/. Fails otherwise.
      // Validates using string comparisons
      map_it = projParams.find("lon_0");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(BCP, "Central meridian 'lon_0' must be specified for YKJ projection");
      }
      usedParams.insert(map_it->first);

      if (map_it->second != "27")
      {
        string errStr;
        errStr += "Invalid lon_0 for YKJ projection: ";
        errStr += map_it->second;
        errStr += ". Should be 27";
        throw Fmi::Exception(BCP, errStr);
      }
      usedParams.insert(map_it->first);

      map_it = projParams.find("lat_0");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(BCP, "Center latitude 'lat_0' must be specified for YKJ projection");
      }
      usedParams.insert(map_it->first);

      if (map_it->second != "0")
      {
        string errStr;
        errStr += "Invalid lat_0 for YKJ projection: ";
        errStr += map_it->second;
        errStr += ". Should be 0";
        throw Fmi::Exception(BCP, errStr);
      }
      usedParams.insert(map_it->first);

      map_it = projParams.find("k");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(BCP, "k must be specified for YKJ projection");
      }
      usedParams.insert(map_it->first);

      if (map_it->second != "1")
      {
        string errStr;
        errStr += "Invalid k for YKJ projection: ";
        errStr += map_it->second;
        errStr += ". Should be 1";
        throw Fmi::Exception(BCP, errStr);
      }
      usedParams.insert(map_it->first);

      map_it = projParams.find("x_0");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(BCP, "x_0 must be specified for YKJ projection");
      }
      usedParams.insert(map_it->first);

      if (map_it->second != "3500000")
      {
        string errStr;
        errStr += "Invalid k for YKJ projection: ";
        errStr += map_it->second;
        errStr += ". Should be 3500000";
        throw Fmi::Exception(BCP, errStr);
      }
      usedParams.insert(map_it->first);

      // y_0 can be left out as it is 0
      map_it = projParams.find("y_0");
      if (!(map_it == projParams.end()))
      {
        if (map_it->second != "0")
        {
          string errStr;
          errStr += "Invalid y_0 for YKJ projection: ";
          errStr += map_it->second;
          errStr += ". Should be 0";
          throw Fmi::Exception(BCP, errStr);
        }
      }
      usedParams.insert(map_it->first);

      map_it = projParams.find("ellps");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(BCP, "ellps must be specified for YKJ projection");
      }
      usedParams.insert(map_it->first);

      if (map_it->second != "intl")
      {
        string errStr;
        errStr += "Invalid ellps for YKJ projection: ";
        errStr += map_it->second;
        errStr += ". Should be intl";
        throw Fmi::Exception(BCP, errStr);
      }
      usedParams.insert(map_it->first);

      map_it = projParams.find("units");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(BCP, "units must be specified for YKJ projection");
      }
      usedParams.insert(map_it->first);

      if (map_it->second != "m")
      {
        string errStr;
        errStr += "Invalid units for YKJ projection: ";
        errStr += map_it->second;
        errStr += ". Should be m";
        throw Fmi::Exception(BCP, errStr);
      }
      usedParams.insert(map_it->first);

      map_it = projParams.find("no_defs");
      if (map_it == projParams.end())
      {
        throw Fmi::Exception(BCP, "no_defs must be specified for YKJ projection");
      }
      usedParams.insert(map_it->first);

      // If we are here the projection is valid YKJ (EPSG 2393)
      result =
          return_type(new NFmiYKJArea(bottomLeftLatLon, topRightLatLon, topLeftXY, bottomRightXY));
    }

    else
    {
      string errStr = "Unsupported projection: ";
      errStr += projId;
      throw Fmi::Exception(BCP, errStr);
    }

    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace NFmiAreaFactory

// ======================================================================
