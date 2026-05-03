// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiSvgTools
 */
// ======================================================================

#include "NFmiGeoTools.h"
#include "NFmiPoint.h"
#include "NFmiSvgPath.h"
#include "NFmiSvgTools.h"
#include <regression/tframe.h>
#include <cmath>

//! Protection against conflicts with global functions
namespace NFmiSvgToolsTest
{
// ----------------------------------------------------------------------
/*!
 * Tests the IsInside function
 */
// ----------------------------------------------------------------------

void isinside(void)
{
  using namespace std;
  using namespace NFmiSvgTools;

  // rectangle with corners at 0,0 and 10,10

  NFmiSvgPath path;
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementMoveto, 0, 0));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 10, 0));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 10, 10));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 0, 10));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementClosePath));

  if (!NFmiSvgTools::IsInside(path, NFmiPoint(5, 5)))
    TEST_FAILED("Point (5,5) is in the rectangle");
  if (NFmiSvgTools::IsInside(path, NFmiPoint(-1, -1)))
    TEST_FAILED("Point (-1,-1) is not in the rectangle");
  if (NFmiSvgTools::IsInside(path, NFmiPoint(11, 11)))
    TEST_FAILED("Point (11,11) is not in the rectangle");

  // add a triangle inside the rectangle

  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementMoveto, 1, 1));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 5, 9));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 9, 1));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementClosePath));

  if (NFmiSvgTools::IsInside(path, NFmiPoint(5, 5)))
    TEST_FAILED("Point (5,5) is in the triangle hole");
  if (NFmiSvgTools::IsInside(path, NFmiPoint(-1, -1)))
    TEST_FAILED("Point (-1,-1) is not in the rectangle or triangle hole");
  if (NFmiSvgTools::IsInside(path, NFmiPoint(11, 11)))
    TEST_FAILED("Point (11,11) is not in the rectangle or triangle hole");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * Tests the Distance function
 */
// ----------------------------------------------------------------------

void distance(void)
{
  using namespace std;
  using namespace NFmiSvgTools;

  // rectangle with corners at 0,0 and 10,10

  NFmiSvgPath path;
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementMoveto, 0, 0));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 10, 0));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 10, 10));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 0, 10));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementClosePath));

  if (NFmiSvgTools::Distance(path, NFmiPoint(0, 0)) != 0)
    TEST_FAILED("Distance of (0,0) is 0");
  if (NFmiSvgTools::Distance(path, NFmiPoint(0, 10)) != 0)
    TEST_FAILED("Distance of (0,10) is 0");
  if (NFmiSvgTools::Distance(path, NFmiPoint(5, 5)) != 5)
    TEST_FAILED("Distance of (5,5) is 5");
  if (std::abs(NFmiSvgTools::Distance(path, NFmiPoint(-1, -1)) - std::sqrt(2.0)) > 1e-6)
    TEST_FAILED("Distance of (-1,-1) is sqrt(2)");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * Tests the GeoDistance function
 */
// ----------------------------------------------------------------------

void geodistance(void)
{
  using namespace std;
  using namespace NFmiSvgTools;

  double result, expected;

  // rectangle with corners at 0,0 and 10,10

  NFmiSvgPath path;
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementMoveto, 0, 0));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 10, 0));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 10, 10));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 0, 10));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementClosePath));

  result = NFmiSvgTools::GeoDistance(path, NFmiPoint(0, 0));
  expected = 0;
  if (result != expected)
    TEST_FAILED("GeoDistance of (0,0) is 0");

#if 0
  // Requires fixes to NFmiAzimuthalArea
  result = NFmiSvgTools::GeoDistance(path, NFmiPoint(-10, -10));
  expected = NFmiGeoTools::GeoDistance(0, 0, -10, -10);
  if (abs(result - expected) > 1) TEST_FAILED("GeoDistance of (-10,-10) failed");
#endif

  result = NFmiSvgTools::GeoDistance(path, NFmiPoint(5, 0));
  expected = 0;
  if (abs(result - expected) > 1)
    TEST_FAILED("GeoDistance of (5,0) is 0");

  result = NFmiSvgTools::GeoDistance(path, NFmiPoint(0, 5));
  expected = 0;
  if (abs(result - expected) > 1)
  {
    cout << expected << ' ' << result << endl;
    TEST_FAILED("GeoDistance of (0,5) is 0");
  }

  result = NFmiSvgTools::GeoDistance(path, NFmiPoint(5, 5));
  expected = 552465;
  if (abs(result - expected) > 1)
    TEST_FAILED("GeoDistance of (5,5) is 552465");

  result = NFmiSvgTools::GeoDistance(path, NFmiPoint(1, 1));
  expected = 111080;
  if (abs(result - expected) > 1)
    TEST_FAILED("GeoDistance of (1,1) is 111080");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * Tests the BoundingBox function
 */
// ----------------------------------------------------------------------

void boundingbox(void)
{
  using namespace std;
  using namespace NFmiSvgTools;

  // rectangle with corners at 0,1 and 10,11

  NFmiSvgPath path;
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementMoveto, 0, 1));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 10, 1));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 10, 11));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementLineto, 0, 11));
  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementClosePath));

  double xmin, ymin, xmax, ymax;
  BoundingBox(path, xmin, ymin, xmax, ymax);
  if (xmin != 0)
    TEST_FAILED("BoundingBox xmin must be 0");
  if (xmax != 10)
    TEST_FAILED("BoundingBox xmax must be 10");
  if (ymin != 1)
    TEST_FAILED("BoundingBox ymin must be 1");
  if (ymax != 11)
    TEST_FAILED("BoundingBox ymax must be 11");

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
    TEST(isinside);
    TEST(distance);
    TEST(geodistance);
    TEST(boundingbox);
  }
};

}  // namespace NFmiSvgToolsTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiSvgTools tester" << endl << "===================" << endl;
  NFmiSvgToolsTest::tests t;
  return t.run();
}

// ======================================================================
