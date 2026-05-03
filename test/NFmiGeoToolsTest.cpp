// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiGeoTools
 */
// ======================================================================

#include "NFmiGeoTools.h"
#include "NFmiGlobals.h"
#include <regression/tframe.h>
#include <cmath>

//! Protection against conflicts with global functions
namespace NFmiGeoToolsTest
{
// ----------------------------------------------------------------------
/*!
 * Tests the Distance function
 */
// ----------------------------------------------------------------------

void distance(void)
{
  using namespace std;
  using namespace NFmiGeoTools;

  if (Distance(0, 0, 0, 0) != 0)
    TEST_FAILED("Distance between (0,0) and (0,0) is not 0");

  if (Distance(0, 0, 1, 0) != 1)
    TEST_FAILED("Distance between (0,0) and (1,0) is not 1");

  // pythagoras: 3*3+4*4=5*5
  if (Distance(1, 2, 4, 6) != 5)
    TEST_FAILED("Distance between (1,2) and (4,6) is not 5");

  if (Distance(0, 0, 0, 0, 0, 0) != 0)
    TEST_FAILED("Distance between (0,0,0) and (0,0,0) is not 0");

  if (Distance(0, 0, 0, 1, 0, 0) != 1)
    TEST_FAILED("Distance between (0,0,0) and (1,0,0) is not 1");

  if (std::abs(Distance(1, 2, 3, 6, 5, 4) - std::sqrt(35.0)) > 1e-6)
    TEST_FAILED("Distance between (1,2,3) and (6,5,4) is not sqrt(35)");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * Tests the DistanceFromLineSegment
 */
// ----------------------------------------------------------------------

void distancefromlinesegment(void)
{
  using namespace std;
  using namespace NFmiGeoTools;

  if (DistanceFromLineSegment(0, 0, 0, 0, 1, 0) != 0)
    TEST_FAILED("Distance of (0,0) from (0,0)-(1,0) is not 0");

  if (DistanceFromLineSegment(1, 0, 0, 0, 1, 0) != 0)
    TEST_FAILED("Distance of (1,0) from (0,0)-(1,0) is not 0");

  if (DistanceFromLineSegment(0.5, 0, 0, 0, 1, 0) != 0)
    TEST_FAILED("Distance of (0.5,0) from (0,0)-(1,0) is not 0");

  if (DistanceFromLineSegment(0, 1, 0, 0, 1, 0) != 1)
    TEST_FAILED("Distance of (0,1) from (0,0)-(1,0) is not 1");

  if (DistanceFromLineSegment(-1, 0, 0, 0, 1, 0) != 1)
    TEST_FAILED("Distance of (-1,0) from (0,0)-(1,0) is not 1");

  if (DistanceFromLineSegment(0.5, 1, 0, 0, 1, 0) != 1)
    TEST_FAILED("Distance of (0.5,1) from (0,0)-(1,0) is not 1");

  if (DistanceFromLineSegment(0.5, 0.5, 0, 0, 1, 0) != 0.5)
    TEST_FAILED("Distance of (0.5,0.5) from (0,0)-(1,0) is not 0.5");

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
  using namespace NFmiGeoTools;

  if (GeoDistance(25, 60, 25, 60) != 0)
    TEST_FAILED("GeoDistance of (25,60) from (25,60) is not ?");

  // 2*kPii*kRearth*30/360 = 3335962.99150298
  if (std::abs(GeoDistance(25, 60, 0, 90) - 3335962) > 1)
    TEST_FAILED("GeoDistance of (25,60) from (0,90) is not 3335962");

  //  2*kPii*kRearth*150/360 = 16679814.9575149
  if (std::abs(GeoDistance(25, 60, 0, -90) - 16679814) > 1)
    TEST_FAILED("GeoDistance of (25,60) from (0,-90) is not 16689814");

  // 7011503.079
  if (std::abs(GeoDistance(25, 60, 0, 0) - 7011503) > 0.1)
    TEST_FAILED("GeoDistance of (25,60) from (0,0) is not 7011503");

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
    TEST(distance);
    TEST(distancefromlinesegment);
    TEST(geodistance);
  }
};

}  // namespace NFmiGeoToolsTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiGeoTools tester" << endl << "===================" << endl;
  NFmiGeoToolsTest::tests t;
  return t.run();
}

// ======================================================================
