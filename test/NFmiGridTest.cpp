// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiHPlaceDescriptor
 */
// ======================================================================

#include "NFmiAreaFactory.h"
#include "NFmiGrid.h"
#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

//! Protection against conflicts with global functions
namespace NFmiGridTest
{
// ----------------------------------------------------------------------

void gridtolatlon()
{
  auto area = NFmiAreaFactory::Create("latlon:20,60,40,70:50x50km");
  auto width = round(area->Width());
  auto height = round(area->Height());
  NFmiGrid grid(area.get(), width, height);

  {
    auto expected = NFmiPoint(20, 60);
    auto latlon = grid.GridToLatLon(NFmiPoint(0, 0));
    if (latlon.Distance(expected) > 1000)
      TEST_FAILED("Failed to get correct bottom left corner coordinate");
  }
  {
    auto expected = NFmiPoint(40, 70);
    auto latlon = grid.GridToLatLon(NFmiPoint(width - 1, height - 1));
    if (latlon.Distance(expected) > 1000)
      TEST_FAILED("Failed to get correct top right corner coordinate");
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
  void test(void) { TEST(gridtolatlon); }
};

}  // namespace NFmiGridTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiGrid tester" << endl << "===========================" << endl;
  NFmiGridTest::tests t;
  return t.run();
}

// ======================================================================
