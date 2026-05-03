// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiAngle
 */
// ======================================================================

#include "NFmiAngle.h"
#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <list>
#include <set>
#include <string>
#include <vector>

using namespace std;

//! Protection against conflicts with global functions
namespace NFmiAngleTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test WindDirection class
 */
// ----------------------------------------------------------------------

void winddirection_structors()
{
  using namespace NFmiStringTools;

  {
    NFmiWindDirection dir(0.0, 0.0);
    if (dir.Value() != 0.0)
      TEST_FAILED("should get value 0 for (0,0)");
  }

  {
    NFmiWindDirection dir(0.0, 1.0);
    if (abs(dir.Value() - 180.0) > 1e-6 && abs(dir.Value()) > 1e-6)
      TEST_FAILED("should get value 180 for (0,1), not " + Convert(dir.Value()));
  }

  {
    NFmiWindDirection dir(0.0, -1.0);
    if (abs(dir.Value() - 360.0) > 1e-6)
      TEST_FAILED("should get value 360 for (0,-1), not " + Convert(dir.Value()));
  }

  {
    NFmiWindDirection dir(1.0, 0.0);
    if (abs(dir.Value() - 270.0) > 1e-6)
      TEST_FAILED("should get value 270 for (1,0), not " + Convert(dir.Value()));
  }

  {
    NFmiWindDirection dir(-1.0, 0.0);
    if (abs(dir.Value() - 90.0) > 1e-6)
      TEST_FAILED("should get value 90 for (-1,0), not " + Convert(dir.Value()));
  }

  {
    NFmiWindDirection dir(1.0, 1.0);
    if (abs(dir.Value() - 225.0) > 1e-6)
      TEST_FAILED("should get value 225 for (1,1), not " + Convert(dir.Value()));
  }

  {
    NFmiWindDirection dir(-1.0, -1.0);
    if (abs(dir.Value() - 45.0) > 1e-6)
      TEST_FAILED("should get value 45 for (-1,-1), not " + Convert(dir.Value()));
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
  void test(void) { TEST(winddirection_structors); }
};

}  // namespace NFmiAngleTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiAngle tester" << endl << "================" << endl;
  NFmiAngleTest::tests t;
  return t.run();
}

// ======================================================================
