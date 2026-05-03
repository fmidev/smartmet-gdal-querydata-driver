// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiAreaFactory
 */
// ======================================================================

#include "NFmiGdalArea.h"

#include <regression/tframe.h>
#include <macgyver/StringConversion.h>
#include <gdal_version.h>
#include <stdexcept>
#include <string>

std::string tostr(const NFmiPoint& p)
{
  std::ostringstream out;
  out << std::fixed << "(" << p.X() << "," << p.Y() << ")";
  return out.str();
}

//! Protection against conflicts with global functions
namespace NFmiAreaFactoryTest
{
// ----------------------------------------------------------------------

void create()
{
  {
    // WGS84
    NFmiGdalArea area("FMI", "EPSG:4326", NFmiPoint(20, 60), NFmiPoint(40, 70));
  }

  {
    // YKJ
    NFmiGdalArea area("FMI", "EPSG:2393", NFmiPoint(20, 60), NFmiPoint(40, 70));
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void areastr()
{
  NFmiGdalArea area("FMI", "EPSG:2393", NFmiPoint(20, 60), NFmiPoint(40, 70));

  std::string expected = "FMI:EPSG:2393|20,60,40,70";

  std::string result = area.AreaStr();
  if (result != expected)
    TEST_FAILED("Expected " + expected + ", got " + result + " instead");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void wkt()
{
  NFmiGdalArea area("FMI", "EPSG:2393", NFmiPoint(20, 60), NFmiPoint(40, 70));

  std::string expected =
      "PROJCS[\"KKJ / Finland Uniform Coordinate "
      "System\",GEOGCS[\"KKJ\",DATUM[\"Kartastokoordinaattijarjestelma_1966\",SPHEROID["
      "\"International "
      "1924\",6378388,297,AUTHORITY[\"EPSG\",\"7022\"]],TOWGS84[-96.062,-82.428,-121.753,4.801,0."
      "345,-1.376,1.496],AUTHORITY[\"EPSG\",\"6123\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\","
      "\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY["
      "\"EPSG\",\"4123\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],"
      "PARAMETER[\"central_meridian\",27],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_"
      "easting\",3500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\","
      "\"9001\"]],AUTHORITY[\"EPSG\",\"2393\"]]";

  std::string result = area.WKT();
  if (result != expected)
    TEST_FAILED("Expected " + expected + ", got " + result + " instead");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void tolatlon()
{
  // YKJ
  NFmiGdalArea area("FMI", "EPSG:2393", NFmiPoint(20, 60), NFmiPoint(40, 70));

  {
    NFmiPoint xy = NFmiPoint(1, 0);
    NFmiPoint expect = NFmiPoint(40, 70);
    NFmiPoint latlon = area.ToLatLon(xy);
    if (expect.Distance(latlon) > 1)  // degrees
      TEST_FAILED("Failed to project " + tostr(xy) + " to " + tostr(expect) + ", got " +
                  tostr(latlon) + " instead");
  }

  {
    NFmiPoint xy = NFmiPoint(0, 1);
    NFmiPoint expect = NFmiPoint(20, 60);
    NFmiPoint latlon = area.ToLatLon(xy);
    if (expect.Distance(latlon) > 1)  // degrees
      TEST_FAILED("Failed to project " + tostr(xy) + " to " + tostr(expect) + ", got " +
                  tostr(latlon) + " instead");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void toxy()
{
  // YKJ
  NFmiGdalArea area("FMI", "EPSG:2393", NFmiPoint(20, 60), NFmiPoint(40, 70));

  {
    NFmiPoint expect = NFmiPoint(1, 0);
    NFmiPoint latlon = NFmiPoint(40, 70);
    NFmiPoint xy = area.ToXY(latlon);
    if (expect.Distance(xy) > 0.01)  // xy-units
      TEST_FAILED("Failed to project " + tostr(latlon) + " to " + tostr(expect) + ", got " +
                  tostr(xy) + " instead");
  }

  {
    NFmiPoint expect = NFmiPoint(0, 1);
    NFmiPoint latlon = NFmiPoint(20, 60);
    NFmiPoint xy = area.ToXY(latlon);
    if (expect.Distance(xy) > 0.01)  // xy-units
      TEST_FAILED("Failed to project " + tostr(latlon) + " to " + tostr(expect) + ", got " +
                  tostr(xy) + " instead");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void latlontoworldxy()
{
  // YKJ
  NFmiGdalArea area("FMI", "EPSG:2393", NFmiPoint(20, 60), NFmiPoint(40, 70));

  NFmiPoint expect = NFmiPoint(3388405, 6655915);
  NFmiPoint xy = area.LatLonToWorldXY(NFmiPoint(25, 60));
  if (expect.Distance(xy) > 1)  // meters
    TEST_FAILED("Failed to project 25,60 to " + tostr(expect) + ", got " + tostr(xy) + " instead");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void worldxytolatlon()
{
  // YKJ
  NFmiGdalArea area("FMI", "EPSG:2393", NFmiPoint(20, 60), NFmiPoint(40, 70));

  NFmiPoint wxy = NFmiPoint(3388888, 6655555);
  NFmiPoint expect = NFmiPoint(25, 60);
  NFmiPoint xy = area.WorldXYToLatLon(wxy);
  if (expect.Distance(xy) > 0.01)  // degrees
    TEST_FAILED("Failed to project " + tostr(wxy) + " to " + tostr(expect) + ", got " + tostr(xy) +
                " instead");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void worldxyheight()
{
  {
    // YKJ
    NFmiGdalArea area("FMI", "EPSG:2393", NFmiPoint(20, 60), NFmiPoint(40, 70));
    double h = std::round(area.WorldXYHeight() / 1000.0);
    if (h != 1147)
      TEST_FAILED("YKJ height should be 1147 km, got " + Fmi::to_string(h));
  }
  {
    NFmiGdalArea area("FMI", "WGS84", NFmiPoint(20, 60), NFmiPoint(40, 70));
    double h = std::round(area.WorldXYHeight() / 1000.0);
    if (h != 1112)
      TEST_FAILED("WGS84 height should be 1112, got " + Fmi::to_string(h));
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void worldxywidth()
{
  {
    // YKJ
    NFmiGdalArea area("FMI", "EPSG:2393", NFmiPoint(20, 60), NFmiPoint(40, 70));
    int h = std::round(area.WorldXYWidth() / 1000.0);
    if (h != 883)
      TEST_FAILED("YKJ width should be 883, got " + Fmi::to_string(h));
  }
  {
    NFmiGdalArea area("FMI", "WGS84", NFmiPoint(20, 60), NFmiPoint(40, 70));
    int h = std::round(area.WorldXYWidth() / 1000.0);
    if (h != 940)
      TEST_FAILED("WGS84 width should be 940, got " + Fmi::to_string(h));
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * The actual test suite
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  virtual const char* error_message_prefix() const { return "\n\t"; }
  void test(void)
  {
    TEST(create);
    TEST(areastr);
    TEST(tolatlon);
    TEST(toxy);
#if GDAL_VERSION_MAJOR < 3
    TEST(wkt);
    TEST(latlontoworldxy);
    TEST(worldxytolatlon);
    TEST(worldxyheight);
    TEST(worldxywidth);
#endif
  }
};

}  // namespace NFmiAreaFactoryTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiGdalArea tester" << endl << "===================" << endl;
  NFmiAreaFactoryTest::tests t;
  return t.run();
}

// ======================================================================
