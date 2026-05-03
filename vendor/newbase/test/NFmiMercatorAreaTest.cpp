// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiAreaFactory
 */
// ======================================================================

#include "NFmiMercatorArea.h"

#include <regression/tframe.h>

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
  NFmiMercatorArea area(NFmiPoint(20, 60), NFmiPoint(40, 70));
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void areastr()
{
  NFmiMercatorArea area(NFmiPoint(20, 60), NFmiPoint(40, 70));

  std::string expected = "mercator:20,60,40,70";

  std::string result = area.AreaStr();
  if (result != expected)
    TEST_FAILED("Expected " + expected + ", got " + result + " instead");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void wkt()
{
  NFmiMercatorArea area(NFmiPoint(20, 60), NFmiPoint(40, 70));

  std::string expected =
      "PROJCS[\"FMI_Transverse_Mercator\",GEOGCS[\"FMI_Sphere\",DATUM[\"FMI_2007\",SPHEROID[\"FMI_"
      "Sphere\",6371220,0]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.0174532925199433]],"
      "PROJECTION[\"Transverse_Mercator\"],UNIT[\"Metre\",1.0]]";
  std::string result = area.WKT();
  if (result != expected)
    TEST_FAILED("Expected " + expected + ", got " + result + " instead");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void tolatlon()
{
  NFmiMercatorArea area(NFmiPoint(20, 60), NFmiPoint(40, 70));
  {
    NFmiPoint xy = NFmiPoint(1, 0);
    NFmiPoint expect = NFmiPoint(40, 70);
    NFmiPoint latlon = area.ToLatLon(xy);
    if (expect.Distance(latlon) > 0.001)  // degrees
      TEST_FAILED("Failed to project " + tostr(xy) + " to " + tostr(expect) + ", got " +
                  tostr(latlon) + " instead");
  }

  {
    NFmiPoint xy = NFmiPoint(0, 1);
    NFmiPoint expect = NFmiPoint(20, 60);
    NFmiPoint latlon = area.ToLatLon(xy);
    if (expect.Distance(latlon) > 0.001)  // degrees
      TEST_FAILED("Failed to project " + tostr(xy) + " to " + tostr(expect) + ", got " +
                  tostr(latlon) + " instead");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void toxy()
{
  // YKJ
  NFmiMercatorArea area(NFmiPoint(20, 60), NFmiPoint(40, 70));

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
  NFmiMercatorArea area(NFmiPoint(20, 60), NFmiPoint(40, 70));

  NFmiPoint expect = NFmiPoint(2779969, 8390628);
  NFmiPoint xy = area.LatLonToWorldXY(NFmiPoint(25, 60));
  if (expect.Distance(xy) > 1)  // meters
    TEST_FAILED("Failed to project 25,60 to " + tostr(expect) + ", got " + tostr(xy) + " instead");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void worldxytolatlon()
{
  NFmiMercatorArea area(NFmiPoint(20, 60), NFmiPoint(40, 70));

  NFmiPoint wxy = NFmiPoint(2779969, 8390628);
  NFmiPoint expect = NFmiPoint(25, 60);
  NFmiPoint xy = area.WorldXYToLatLon(wxy);
  if (expect.Distance(xy) > 0.01)  // degrees
    TEST_FAILED("Failed to project " + tostr(wxy) + " to " + tostr(expect) + ", got " + tostr(xy) +
                " instead");

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
    TEST(wkt);
    TEST(tolatlon);
    TEST(toxy);
    TEST(latlontoworldxy);
    TEST(worldxytolatlon);
  }
};

}  // namespace NFmiAreaFactoryTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiMercatorArea tester" << endl << "=======================" << endl;
  NFmiAreaFactoryTest::tests t;
  return t.run();
}

// ======================================================================
